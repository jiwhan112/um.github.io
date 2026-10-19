#include "pipeline.hpp"
#include "check.hpp"

int main() {
    return Run("pipeline_trace", [](Checks& c) {
        using namespace pipeline;
        // This triangle is entirely inside the six clip planes. Clipping that
        // creates a polygon is tested separately in transform_clip.cpp.
        const std::array<Vertex, 3> input{{
            {{-0.8, 0.8, 0.4, 1}, {0, 0}},
            {{0.8, 0.8, 0.4, 1}, {1, 0}},
            {{0, -0.8, 0.4, 1}, {0.5, 1}}
        }};
        const auto polygon = Clip(input);
        c.That(polygon.size() == 3, "triangle survives clipping");

        std::array<ScreenVertex, 3> screen{};
        for (std::size_t i = 0; i < 3; ++i) {
            screen[i] = Project(polygon[i], {0, 0, 8, 8, 0, 1});
        }
        const std::array<Vec2, 3> xy{screen[0].p, screen[1].p, screen[2].p};
        int covered = 0;
        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 8; ++x) {
                const Vec2 point{static_cast<double>(x), static_cast<double>(y)};
                if (!Covers(xy, point)) {
                    continue;
                }
                ++covered;
                const auto weights = Barycentric(xy, point);
                c.That(weights.has_value(), "sample barycentric");
                const auto uv = PerspectiveUV(screen, *weights);
                const double depth = AffineDepth(screen, *weights);

                // An independent target sample, initially black with depth 1.
                Pixel output{{0, 0, 0, 1}, 1};
                c.That(Fragment(output, {uv.x, uv.y, 0, 1}, depth, {}),
                       "fragment passes output stage");
                c.That(uv.x >= -1e-12 && uv.x <= 1 + 1e-12 &&
                       uv.y >= -1e-12 && uv.y <= 1 + 1e-12,
                       "interpolated uv in range");
                c.Near(output.depth, 0.4, "constant depth across triangle");
            }
        }
        c.That(covered == 25, "25 covered samples");
        const auto center_weights = Barycentric(xy, {4, 4});
        if (!center_weights) {
            throw std::runtime_error("center has no barycentric coordinates");
        }
        const auto center_uv = PerspectiveUV(screen, *center_weights);
        c.Near(center_uv.x, 0.5, "center u");
        c.Near(center_uv.y, 0.5, "center v");

        auto reversed = xy;
        std::swap(reversed[1], reversed[2]);
        c.That(Edge(xy[0], xy[1], xy[2]) ==
                   -Edge(reversed[0], reversed[1], reversed[2]),
               "culling diagnostic");
        std::cout << "8x8 viewport: covered samples=" << covered
                  << ", sample (4,4): uv=(0.5,0.5), z=0.4\n";
    });
}

