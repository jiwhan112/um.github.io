#include "pipeline.hpp"
#include "check.hpp"

// [한국어 해설] 8×8 뷰포트의 한 삼각형을 CPU로 단계별 추적한다. 실제 D3D9 드로콜이나 GPU 픽셀 일치 검사가 아니다.
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

// [한국어 해설] 클리핑 결과가 삼각형인 이 사례만 직접 연결한다. 일반 다각형의 삼각분할은 이 예제 범위 밖이다.
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
// [한국어 해설] 덮인 샘플만 화면 가중치→원근 UV→선형 깊이→출력 단계로 보낸다.
                const auto weights = Barycentric(xy, point);
                c.That(weights.has_value(), "sample barycentric");
                const auto uv = PerspectiveUV(screen, *weights);
                const double depth = AffineDepth(screen, *weights);

                // An independent target sample, initially black with depth 1.
// [한국어 해설] 샘플마다 독립적인 초기 타깃이다. 프레임버퍼 전체를 저장하는 렌더러는 아니다.
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

// [한국어 해설] 감김을 뒤집으면 유향 면적의 부호가 바뀐다. 실제 컬링 상태 설정은 별도 작업이다.
        auto reversed = xy;
        std::swap(reversed[1], reversed[2]);
        c.That(Edge(xy[0], xy[1], xy[2]) ==
                   -Edge(reversed[0], reversed[1], reversed[2]),
               "culling diagnostic");
        std::cout << "8x8 viewport: covered samples=" << covered
                  << ", sample (4,4): uv=(0.5,0.5), z=0.4\n";
    });
}

