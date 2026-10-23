# DX11 17 · 높이 경계 수정판

원본 `../dx11-17/`과 원본 동일 한국어 주석본 `../dx11-17-ko/`는 변경하지 않는다. 이 폴더는 별도 수정판이다.

- 기반: 한국어 주석본의 전체 파일. 의미 변경은 `inside`의 `r.h > 0` 검사 추가뿐이다.
- `main.cpp`: 기존 49개 검사를 그대로 유지한다.
- `height-regression.cpp`: 높이 0/−1의 inside와 Atlas.add(4개), 양수 높이 inside 및 layout(2개), 총 6개 조건.
- `EXPECT_REJECTION=0`으로 원본을 빌드해 잘못된 높이 허용을 재현한다. 수정판 기본값 1은 거절을 요구한다.
- GCC 14.2, C++11/C++20 직접 빌드에서 원본/수정판 각각 기존 49개와 기대 동작별 6개 통과. 컴파일 명령·출력·해시는 validation.json.
- CMake/CTest, Windows/MSVC, GPU, HLSL, sanitizer는 이번 수정판에서 실행하지 않았다.
- 미구현 kerning/shaping, 큰 유한 수의 연산 결과, 다중 번역 단위 헤더 설계는 이 좁은 수정의 대상이 아니다.

수정판 직접 실행:
```sh
g++ -I. -std=c++11 -Wall -Wextra -Wpedantic -Werror main.cpp -o font_cpu
./font_cpu
g++ -I. -std=c++11 -Wall -Wextra -Wpedantic -Werror height-regression.cpp -o font_height
./font_height
```
CMakeLists.txt는 두 타깃을 제공한다. CMake 실행 통과 기록으로 오인하지 않는다.
