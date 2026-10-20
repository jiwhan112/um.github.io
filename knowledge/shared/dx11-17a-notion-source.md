<callout icon="📦" color="gray_bg">
	**17A 전체 소스 영구 보관** — <mention-page url="https://app.notion.com/p/3fba99703f40811493aae2a5a2532bea">17 · Font Rendering — UTF-8·Glyph Atlas·SpriteFont·SpriteBatch</mention-page>
	GCC 14.2 C++20/C++11, Clang17 C++20 ASan/UBSan에서 Font 38 CPU 조건 통과. Windows/GPU 미검증. 소스는 연속된 코드 블록을 파일 이름별로 이어 붙여 빌드합니다.
</callout>
## CPU 모델 구조
main.cpp는 UTF-8→Unicode scalar→Glyph Map→Quad/UV·Advance·Kerning 흐름을 구현합니다. 전체 소스를 아래 native C++ 블록에 연속 보관합니다.
## main.cpp · 블록 1/4
```c++
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <functional>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace fontlab {
struct Rect { int x, y, w, h; };
struct Glyph { char32_t cp; Rect atlas; float bearingX, bearingY, advance; };
struct Quad { char32_t cp; float left, top, right, bottom; float u0,v0,u1,v1; };
struct Layout { std::vector<Quad> quads; float maxAdvance, lineHeight; unsigned lines; };
```
이 블록은 main.cpp 시작 부분이며 뒤의 연속 블록을 그대로 이어 붙입니다.
### main.cpp 다음 부분 — 수치·Atlas 판정
```c++
static bool finite(float v) { return std::isfinite(v); }
static bool overlap(Rect a, Rect b) {
    return a.w>0 && a.h>0 && b.w>0 && b.h>0 &&
           a.x<b.x+b.w && b.x<a.x+a.w && a.y<b.y+b.h && b.y<a.y+a.h;
}
```
### 17A 확정 main.cpp 분할 1/29
```c++
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <functional>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

// Pure CPU contract model. This is NOT DirectXTK SpriteFont or D3D11.
namespace fontlab {
struct Rect { int x, y, w, h; };
```
### 17A 확정 main.cpp 분할 2/29
```c++
struct Glyph { char32_t cp; Rect atlas; float bearingX, bearingY, advance; };
struct Quad { char32_t cp; float left, top, right, bottom; float u0,v0,u1,v1; };
struct Layout { std::vector<Quad> quads; float maxAdvance, lineHeight; unsigned lines; };
static bool finite(float v) { return std::isfinite(v); }
```
### 17A 확정 main.cpp 분할 3/29
```c++
static bool overlap(Rect a, Rect b) {
    return a.w>0 && a.h>0 && b.w>0 && b.h>0 &&
           a.x<b.x+b.w && b.x<a.x+a.w && a.y<b.y+b.h && b.y<a.y+a.h;
}
static bool scalar(uint32_t cp) {
    return cp<=0x10ffffu && !(cp>=0xd800u && cp<=0xdfffu);
}
std::vector<char32_t> decodeUtf8(const std::string& utf8) {
```
## 최신 전체 소스
<mention-page url="https://app.notion.com/p/3fba99703f408105a5c8c1d371e2c8cb"/> · <mention-page url="https://app.notion.com/p/3fba99703f4081aeb641e5b540bf2848"/>
