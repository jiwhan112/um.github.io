# WinAPI · 게임 수학 전체 소스 — 한국어 주석본

2026-10-23 제작. `../game-math-winapi-lab/`의 보존 원본을 기반으로 한 **학습용 파생 주석본**입니다. 원본을 교체하거나 과거 검증 기록을 수정하지 않습니다. 새 프로젝트나 새 과정이 아닙니다.

## 구성과 읽는 순서

- `include/math.hpp`: 벡터, 행 벡터 행렬, 카메라, 충돌·피킹. y축·행렬 곱 규약을 먼저 읽습니다.
- `include/loop.hpp`: 입력 상태/에지, 120Hz 고정 Update, 애니메이션 시간.
- `include/grid.hpp`: 제한된 맵 포맷, Undo/Redo, 네 방향 비용 1 A*.
- `include/check.hpp`: 검사·근사 비교·예외 확인 도우미.
- `src/*.cpp`: 위 헤더를 사용하는 완전한 실행 프로그램 6개. 정상 사례뿐 아니라 경계·거부 사례를 포함합니다.
- `windows/winapi_demo.cpp`: Win32 메시지 처리, 입력, GDI 버퍼, DPI, 시간 루프의 전체 연결 예제.
- `windows/app.rc`, `windows/app.manifest`, `CMakeLists.txt`: 원본과 바이트가 같은 빌드·자원 파일입니다. 전체 폴더 구조를 함께 받아야 합니다.
- `../../winapi-04a/main.ko.cpp`: 별도 WinAPI04A 정수 시간 모델의 한국어 주석본. 이 패키지의 부동소수점 stepper와 다른 예제입니다.

원본 영어 주석·코드·문자열을 삭제하지 않고, 함수/클래스/중요 제어 흐름 앞에 `// [한국어 해설]` 독립 줄만 추가했습니다. 화면 메시지나 테스트 라벨도 의도적으로 원문 그대로입니다. 부분 예시가 아니라 전체 소스입니다.

## 동일성 확인

저장소 `knowledge` 구조 안에서:

```sh
python3 knowledge/shared/game-math-winapi-lab-ko/verify_annotations.py
```

`annotation-manifest.json`에는 원본/주석본의 SHA-256과 삽입한 줄 번호가 있습니다. 검증기는 지정된 주석 줄만 제거하여 **원본 바이트를 완전히 복원**하고, 양쪽 해시를 검사합니다. C++ 문자열을 정규식으로 지우지 않으므로 문자열 속 `//`, `/*`, raw string을 코드 주석으로 오인해 삭제하지 않습니다. 삽입 위치는 문자열·블록 주석·매크로 연속줄 바깥으로 검토했습니다. 이것은 임의 C++를 처리하는 범용 토크나이저가 아닙니다.

추가로 Linux용 7개 프로그램(공통 6개+04A)의 GCC `-E -P` 결과가 원본과 주석본에서 바이트 일치했습니다. Windows 파일은 이 전처리 검사 대상이 아닙니다.

## 빌드

CMake가 있는 환경에서 이 폴더를 기준으로:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

CMake 없는 Linux에서는 프로그램별로 빌드할 수 있습니다:

```sh
g++ -std=c++20 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror -pthread -I include src/loop_input.cpp -o /tmp/loop_input
/tmp/loop_input
# vector_motion, transforms, geometry, tile_document, pathfinding도 같은 방식
```

04A 단독 실행(저장소 루트 기준):

```sh
g++ -std=c++20 -Wall -Wextra -Wpedantic -Werror knowledge/winapi-04a/main.ko.cpp -o /tmp/winapi04a
/tmp/winapi04a
```

Windows 빌드는 MSVC C++ 도구와 Windows SDK가 필요합니다. CMake는 Windows에서 `winapi_demo`와 리소스를 추가하고 Unicode/DPI 설정을 적용합니다. 사용자 컴퓨터에 접근하거나 실행을 요청하지 않았습니다.

## 이번 실제 검증과 한계

Linux / GCC 14.2.0에서 직접 빌드·실행: vector_motion 19, transforms 29, geometry 78, loop_input 484, tile_document 26, pathfinding 93,561 검사 통과. A*는 3×3 맵/시작/목표 41,472 시나리오를 비교합니다. 04A는 23 검사 통과. 전체 입력 공간의 증명은 아닙니다.

**이 환경에는 cmake/ctest가 없어 이번 CMake 구성·CTest는 실행하지 못했습니다.** CMake 파일은 보존 원본과 같습니다. Windows/MSVC, Win32 GUI, MFC, DPI 실제 화면, GDI 자원 동작, GPU, ASan/UBSan은 이번 실행으로 검증하지 않았습니다. Linux CPU 통과를 Windows 성공으로 표시하지 않습니다. 이전 원본의 검증 기록은 `../game-math-winapi-lab/docs/validation.md`에 있으며 이번 실행과 별개입니다.

검증 수치와 정확한 파일 해시는 `validation.json`과 `annotation-manifest.json`에서 확인할 수 있습니다.
