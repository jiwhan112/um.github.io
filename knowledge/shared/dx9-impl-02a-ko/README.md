# DX9 구현 02a — 한국어 전체 주석본

불투명 핸들과 공유 라이브러리 ABI. 2026-10-23 작성한 기존 교재의 학습용 파생본입니다.

보존 원본은 `../dx9-impl-02a/`에 그대로 있습니다. 새 주석본이 원본이나 과거 검증 기록을 교체하지 않습니다.

## 전체 파일

- `CMakeLists.txt` — 보존 원본과 바이트 동일
- `include/engine_api.h` — 완전한 한국어 주석본
- `src/client.cpp` — 완전한 한국어 주석본
- `src/engine_api.cpp` — 완전한 한국어 주석본
- `tests/engine_contract_tests.cpp` — 완전한 한국어 주석본

소스와 CMake, 헤더/하위 디렉터리를 함께 보존해야 합니다. 코드 일부를 생략한 예제가 아닙니다.

## 주석과 동일성

12개의 의미 설명을 기존 줄 끝에만 `// [한국어 해설]`로 덧붙였습니다. 기존 코드·문자열·주석을 지우지 않았으며 **물리행 수와 진단 행번호도 유지**합니다. 문자열·raw string·블록 주석을 정규식으로 제거하지 않습니다. 기록한 정확한 suffix만 제거하면 원본 바이트가 복원됩니다. 삽입 위치는 문자열/블록주석/매크로 연속줄 바깥으로 검토했습니다. 임의 C++를 처리하는 범용 토크나이저가 아닙니다.

```sh
# 저장소 루트에서 원본과 주석본이 함께 있는 상태
python3 knowledge/shared/dx9-impl-02a-ko/verify_annotations.py
```

`annotation-manifest.json`의 원본/주석본 SHA-256, 줄 번호와 suffix를 확인합니다. CPU 번역 단위 3개의 `g++ -std=c++20 -E -P` 결과도 일치했습니다. 서로 다른 폴더의 `__FILE__` 경로만 `-fmacro-prefix-map=<각 원본 또는 주석본 루트>=SOURCE`로 맞췄으며 코드·진단 행번호를 임의로 지우지 않았습니다.

## 빌드

CMake가 설치된 환경에서는 이 폴더 기준:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

이번 클라우드에서 실제 사용한 직접 빌드(C++20, NDEBUG 미설정):

```sh
g++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -fPIC -shared -DENGINE_BUILD -I include src/engine_api.cpp -o /tmp/libengine.so
g++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -I include tests/engine_contract_tests.cpp -L/tmp -lengine -Wl,-rpath,/tmp -o /tmp/engine_contract_tests
/tmp/engine_contract_tests
# src/client.cpp도 같은 -I/-L/-lengine/-rpath 설정으로 별도 빌드
```

## 이번 실제 결과

Cloud Linux / GCC 14.2.0에서:

- `src/client.cpp`: `Elapsed: 0.05 seconds`
- `tests/engine_contract_tests.cpp`: `checks=21 failures=0`

Linux에서 실제 libengine.so를 만들고 클라이언트와 계약 검사기를 동적 링크해 실행했습니다. 이것이 Windows DLL 내보내기·호출 규약 검증을 뜻하지는 않습니다.

**cmake/ctest가 없어 이번 CMake 구성·CTest는 실행하지 않았습니다.** Windows/MSVC/D3D9/GPU/FXC 및 sanitizer는 이번 검증으로 실행하지 않았습니다. CPU 성공을 화면/GPU 성공으로 대신 표시하지 않습니다. 사용자 컴퓨터에 접근하거나 실행을 요청하지 않았습니다. C++11 지원에 관한 과거 기록은 과거 증거이며 이번 실행은 C++20입니다.

정확한 이번 결과는 `validation.json`, 원본 복원 해시는 `annotation-manifest.json`에서 확인할 수 있습니다.
