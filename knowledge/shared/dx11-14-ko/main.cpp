// CPU reference: Geometry Shader concept only; no D3D11 runtime.
#include <array>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
struct Vec2 { float x, y; };
struct Vec4 { float x, y, z, w; };
struct Sprite { Vec4 clipCenter; float halfWidth, halfHeight; };
struct Output { Vec4 position; Vec2 uv; };
struct Stream { std::vector<Output> vertices; unsigned restarts = 0; };
bool finite(float f) { return std::isfinite(f); }
bool valid(const Sprite& s) { // [한국어 해설] 입력의 유한성·양수 크기·양수w를 검사하지만 매우 큰 유한 값의 중간곱 오버플로까지 모두 거부하지 않는다.
    return finite(s.clipCenter.x) && finite(s.clipCenter.y) &&
           finite(s.clipCenter.z) && finite(s.clipCenter.w) &&
           finite(s.halfWidth) && finite(s.halfHeight) &&
           s.clipCenter.w > 0.0f && s.halfWidth > 0.0f && s.halfHeight > 0.0f;
}
void emitPointSprite(const Sprite& sprite, Stream& output, std::size_t maxVertices) { // [한국어 해설] 점 하나를 strip 정점4개로 확장한다. NDC 반폭을 clip 이동량으로 바꾸려면 w를 곱한다.
    if (!valid(sprite)) throw std::invalid_argument("invalid sprite");
    if (maxVertices < 4 || output.vertices.size() > maxVertices - 4) // [한국어 해설] 용량 계약 위반은 쓰기 전에 거부한다. 이후 vector 할당 실패까지 강한 롤백 보장을 구현한 것은 아니다.
        throw std::length_error("GS output vertex cap exceeded");
    // Width and height are NDC half-extents: shift clip coordinates by w*NDC.
    const std::array<Vec2, 4> corners{{{-1,-1}, {-1,1}, {1,-1}, {1,1}}};
    const std::array<Vec2, 4> uv{{{0,1}, {0,0}, {1,1}, {1,0}}};
    for (std::size_t i=0; i<corners.size(); ++i) {
        const Vec4& c=sprite.clipCenter;
        const Vec2 p=corners[i];
        output.vertices.push_back({{c.x+p.x*sprite.halfWidth*c.w,
                                    c.y+p.y*sprite.halfHeight*c.w,c.z,c.w}, uv[i]});
    }
    ++output.restarts; // [한국어 해설] 다음 sprite와 strip이 이어지지 않도록 종료 이벤트를 센다. GPU GS 출력량 계측은 아니다.
}
unsigned testCount=0;
void verify(bool value, const char* message) {
    ++testCount; if (!value) throw std::runtime_error(message);
}
template<class Exception, class Callable> void expectThrow(Callable call) {
    bool caught=false; try { call(); } catch (const Exception&) { caught=true; }
    verify(caught,"expected specific exception");
}
int main() { // [한국어 해설] 정점 순서·UV·NDC크기·출력한도·거부 입력을 CPU에서 검사한다.
    const Sprite s{{0.0f,0.0f,0.5f,2.0f},0.25f,0.5f};
    Stream out;
    emitPointSprite(s,out,4);
    verify(out.vertices.size()==4,"quad has 4 vertices");
    verify(out.restarts==1,"end strip counted");
    verify(out.vertices[0].position.x==-0.5f,"left lower x");
    verify(out.vertices[1].position.x==-0.5f,"left upper x");
    verify(out.vertices[2].position.x==0.5f,"right lower x");
    verify(out.vertices[3].position.x==0.5f,"right upper x");
    verify(out.vertices[0].position.y==-1.0f,"lower y");
    verify(out.vertices[1].position.y==1.0f,"upper y");
    verify(out.vertices[2].position.y==-1.0f,"lower right y");
    verify(out.vertices[3].position.y==1.0f,"upper right y");
    verify(out.vertices[0].position.z==0.5f,"z unaffected");
    verify(out.vertices[2].position.w==2.0f,"w unaffected");
    verify(out.vertices[0].uv.x==0 && out.vertices[0].uv.y==1,"uv lower left");
    verify(out.vertices[3].uv.x==1 && out.vertices[3].uv.y==0,"uv upper right");
    verify((out.vertices[0].position.x/out.vertices[0].position.w)==-0.25f,"NDC left offset");
    verify((out.vertices[3].position.y/out.vertices[3].position.w)==0.5f,"NDC top offset");
    expectThrow<std::length_error>([&]{emitPointSprite(s,out,7);});
    verify(out.vertices.size()==4,"overflow preserves output");
    emitPointSprite(s,out,8);
    verify(out.vertices.size()==8,"second strip emits 4 vertices");
    verify(out.restarts==2,"two separate strips");
    expectThrow<std::length_error>([&]{emitPointSprite(s,out,8);});
    verify(out.restarts==2,"failed capacity preserves strips");
    expectThrow<std::length_error>([&]{Stream local;emitPointSprite(s,local,3);});
    expectThrow<std::invalid_argument>([&]{Stream local;emitPointSprite({{0,0,0,0},1,1},local,4);});
    expectThrow<std::invalid_argument>([&]{Stream local;emitPointSprite({{0,0,0,-1},1,1},local,4);});
    expectThrow<std::invalid_argument>([&]{Stream local;emitPointSprite({{0,0,0,1},0,1},local,4);});
    expectThrow<std::invalid_argument>([&]{Stream local;emitPointSprite({{0,0,0,1},1,-1},local,4);});
    const float nan=std::numeric_limits<float>::quiet_NaN();
    expectThrow<std::invalid_argument>([&]{Stream local;emitPointSprite({{nan,0,0,1},1,1},local,4);});
    expectThrow<std::invalid_argument>([&]{Stream local;emitPointSprite({{0,0,0,1},nan,1},local,4);});
    const float inf=std::numeric_limits<float>::infinity();
    expectThrow<std::invalid_argument>([&]{Stream local;emitPointSprite({{0,0,inf,1},1,1},local,4);});
    verify(out.vertices[4].uv.y==1 && out.vertices[7].uv.y==0,"repeat strip UV");
    std::cout << "PASS " << testCount << " geometry shader CPU contracts\n";
}
