# DX9 09C — 한국어 주석으로 읽는 전체 CPU 실습

[한국어 주석 전체 코드](main.ko.cpp)는 원래의 53개 검사 프로그램을 함수별로 펼쳐 읽기 쉽게 만든 학습판입니다. 수식·분기·검사 조건을 바꾸지 않고 한국어 설명, 들여쓰기, 줄바꿈을 추가했습니다. 브라우저에서 본 핵심 함수의 앞뒤 구조체·연산자·검사 코드를 이 파일에서 이어 읽을 수 있습니다.

## 먼저 어떤 파일을 읽을까?

| 파일 | 역할 |
|---|---|
| [main.ko.cpp](main.ko.cpp) | 구조체부터 `main()`의 53개 검사까지 포함한 한국어 주석판 |
| [original/main.cpp](original/main.cpp) | 회수한 09C 첨부의 C++ 블록을 마지막 개행까지 그대로 분리한 원본 |
| [CMakeLists.txt](CMakeLists.txt) | 원본과 주석판을 같은 표준으로 빌드하고 CTest 2개를 실행하는 학습용 빌드 |
| [original/CMakeLists.txt](original/CMakeLists.txt) | 첨부에서 그대로 분리한 원본 빌드. C++11, 실행 파일 1개, CTest 1개 |
| [source-09c.md](source-09c.md) | 회수한 전체 첨부. 원본 C++·CMake·README와 2026-10-13의 과거 실행 기록 |
| [verify_comments.py](verify_comments.py) | 첨부·원본 바이트와 주석판의 코드 토큰을 다시 확인하는 검사기 |

원문은 [DX9 09 Notion 페이지](https://app.notion.com/p/3f8a99703f4081ba8b8ae2fc6650bde2)의 `dx9_09c_integrated_cpu_full_sources.md` 첨부입니다. [기존 Notion 본문 보존본](../source-notion.md)의 상단 파일 항목과 대응합니다. 이 09C는 독립적으로 작성된 53조건 실습이며, 과거의 미완성 분할 전사나 09D의 48조건 원형과 같은 프로그램으로 취급하지 않습니다.

## 코드를 읽는 순서

1. `Color`, `Vec3`, `unit()`을 읽고 색 계산과 벡터 계산에 쓰는 자료형을 확인합니다.
2. `straight()`, `premult()`, `additive()`에서 전경·배경의 기여가 어떻게 다른지 비교합니다.
3. `draw_cutout()`과 `draw_alpha()`에서 색을 섞는 결정과 깊이를 기록하는 결정을 나누어 봅니다. `main()`의 `broken` 상태는 알파 0인 조각도 깊이를 기록할 수 있는 반례입니다.
4. `spherical()`과 `cylindrical()`에서 카메라를 보는 축을 만들고, 카메라가 바로 위에 있을 때 왜 정규화가 실패하는지 확인합니다.
5. `far_first()`의 전방 깊이 정렬과 `tick()`의 남은 수명 제한을 읽은 뒤, `main()`의 같은 이름을 쓰는 검사를 찾아봅니다.

## 빌드와 실행

저장소 루트에서 Python 3.7 이상으로 보존 검사를 실행합니다. C++ 코드를 실행하거나 컴파일러를 설치하지 않는 검사입니다.

```sh
python3 knowledge/dx9-impl-09/lab/verify_comments.py
```

CMake 3.20 이상과 C++ 컴파일러가 있으면 다음과 같이 실행합니다. CMake 표준 변수를 `LAB_CXX_STANDARD`로 지정하므로 20을 요청했는데 원본 CMake가 다시 11로 덮어쓰는 일이 없습니다. 선택한 표준은 두 실행 파일에 함께 적용합니다.

```sh
cmake -S knowledge/dx9-impl-09/lab -B /tmp/dx9-09-cxx11 -DLAB_CXX_STANDARD=11 -DCMAKE_BUILD_TYPE=Debug
cmake --build /tmp/dx9-09-cxx11 --verbose
ctest --test-dir /tmp/dx9-09-cxx11 --output-on-failure -V

cmake -S knowledge/dx9-impl-09/lab -B /tmp/dx9-09-cxx20 -DLAB_CXX_STANDARD=20 -DCMAKE_BUILD_TYPE=Debug
cmake --build /tmp/dx9-09-cxx20 --verbose
ctest --test-dir /tmp/dx9-09-cxx20 --output-on-failure -V
```

`alpha_original_cpu`와 `alpha_korean_cpu`는 각각 원본과 한국어 주석판을 실행합니다. 두 프로그램의 성공 출력은 같습니다.

```text
PASS: 53 explicit assertions (single CPU run)
```

이는 **53개의 같은 조건을 두 표현으로 재실행**한 결과입니다. CTest 2/2 또는 여러 컴파일러의 성공을 106개 이상의 독립 조건으로 합산하지 않습니다.

CMake 없이 주석판 하나만 확인하려면 다음과 같이 실행할 수 있습니다.

```sh
g++ -std=c++11 -Wall -Wextra -Wpedantic -Werror knowledge/dx9-impl-09/lab/main.ko.cpp -o /tmp/dx9-09-ko
/tmp/dx9-09-ko
```

## 주석판에서 보존한 것

`verify_comments.py`는 첨부의 SHA-256, C++·CMake 코드 블록의 바이트 동일성, 전처리 지시문, 주석판의 **전처리 전 C++ 토큰 2,563개**를 확인합니다. 주석과 공백은 비교에서 제외하지만 문자열·숫자·연산자·식별자는 보존합니다. 예외 메시지와 성공 출력의 영어 문자열도 코드의 일부이므로 번역하지 않았습니다. 검사기의 `--json` 옵션은 현재 주석판 해시를 포함한 결과를 출력합니다.

| 보존 대상 | UTF-8 바이트 | SHA-256 |
|---|---:|---|
| `source-09c.md` | 7,911 | `056fd682246d8f18a7bf599b2d8f165796959e98cbb43b4dce48aa258d42c668` |
| `original/main.cpp` | 6,361 | `65414be51c51a6b9fb22f0cf07c296c392b024913f83b241234dc266c53ea12a` |
| `original/CMakeLists.txt` | 261 | `3099695963c1324d57aeea455f884927785ab89bdfefbfa97f323307e1242c6c` |

`CHECK` 매크로의 `__LINE__`은 각 파일에서 실패한 실제 줄을 출력합니다. 주석판은 줄 수가 늘었으므로 실패 진단의 줄번호가 원본과 달라집니다. 이는 의도한 차이이며 두 파일의 바이너리나 모든 실패 진단이 바이트 단위로 같다고 주장하지 않습니다. 토큰 검사는 일반적인 C++ 프로그램의 의미 동등성 증명이 아니므로 실제 컴파일·실행 결과와 구분합니다.

## 실제 확인한 범위와 한계

2026-10-20 로컬 Linux에서 GCC 13.3.0으로 원본·주석판을 각각 C++11과 C++20으로 컴파일했습니다. 네 번 모두 `-Wall -Wextra -Wpedantic -Werror`를 통과했고 위의 53조건 성공 출력과 종료 코드 0을 확인했습니다. [로컬 실행 로그·컴파일러 정보·파일 해시](verification/local-gcc-20261020/)를 함께 보존했습니다. 이 로컬 환경에는 CMake·CTest·Clang이 없으므로 해당 실행을 로컬 검증으로 기록하지 않습니다.

GitHub CI의 Configure·Build·CTest 및 sanitizer 결과는 [DX9 09 teaching workflow 실행 목록](https://github.com/jiwhan112/um.github.io/actions/workflows/knowledge-dx9-09-teaching.yml)에서 해당 커밋의 실제 실행으로 확인합니다. 워크플로 파일의 존재만으로 검증 성공을 뜻하지 않습니다. 첨부에 적힌 2026-10-13의 GCC 14.2.0·Clang 17.0.0 기록은 당시의 기록으로 보존하며 이번 실행 결과와 합치지 않습니다.

이 프로그램은 다음 범위의 CPU 모델입니다.

- RGB는 선형 색 공간의 `double` 값입니다. sRGB 변환, 8비트 양자화, 렌더 타깃 알파, MSAA를 계산하지 않습니다.
- 좌표·깊이·RGB·입자의 기존 상태에는 적절한 유한 입력을 전달한다고 가정합니다. 알파·일부 시간·정규화 길이 검사는 있지만 모든 입력을 방어하는 엔진 API가 아닙니다.
- 깊이 비교는 `LESSEQUAL`, 위쪽은 월드 Y축입니다. `draw_alpha()`는 일반 알파 혼합 경로이며 알파 테스트를 수행하지 않습니다.
- 빌보드는 세 축만 만들며 정점·삼각형을 그리지 않습니다. 정규화할 수 없는 방향을 예외로 거부하고 이전 회전 유지 같은 대체 동작은 구현하지 않습니다.
- 정렬은 카메라 전방에 투영한 **중심 깊이**를 기준으로 합니다. 같은 깊이는 입력 순서를 유지하며, 교차하는 투명 메시의 전체 가림 관계를 해결하지 않습니다.
- Windows/MSVC·Direct3D9 장치·GPU·드라이버·HLSL 컴파일은 이 CPU 실행으로 검증하지 않았습니다.

원본이 제시한 API 설명은 [Microsoft Alpha Blending State](https://learn.microsoft.com/en-us/windows/win32/direct3d9/alpha-blending-state), [Alpha Testing State](https://learn.microsoft.com/en-us/windows/win32/direct3d9/alpha-testing-state), [D3DRENDERSTATETYPE](https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3drenderstatetype)에서 이어 볼 수 있습니다.
