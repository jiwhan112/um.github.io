# DX11 13 — 한국어 전체 주석본

2026-10-23. `../dx11-13/`의 보존 원본에 계약·실패 경계·사용법 설명을 추가한 학습용 파생본입니다. 원본 캡처와 과거 검증 로그는 원래 위치에 보존합니다.

## 전체 파일

- `CMakeLists.txt` — 원본 바이트 동일
- `instancing_example.hlsl` — 전체 한국어 주석본
- `main.cpp` — 전체 한국어 주석본

부분 코드가 아니며 원본의 전체 코드·CMake 구성을 유지합니다. 헤더 및 하위 디렉터리, HLSL 등 의존 파일을 함께 받아야 합니다. Windows 전용 파일이 포함되어 있어도 실제 Windows 실행을 확인한 것은 아닙니다.

## 원본 보존 확인

한국어 해설 8개를 기존 줄 끝에만 추가했습니다. 코드·문자열·기존 주석·물리행 수·마지막 개행 유무를 유지합니다. 특히 내장 raw string HLSL에는 주석을 끼워 넣지 않았습니다.

```sh
# 저장소 루트에서 원본/주석본이 함께 존재할 때
python3 knowledge/shared/dx11-13-ko/verify_annotations.py
```

`annotation-manifest.json`에는 양쪽 SHA-256과 정확한 추가 suffix/행 번호가 있습니다. 기록한 suffix만 제거하면 원본 바이트가 복원됩니다. 문자열을 정규식으로 지우는 범용 토크나이저를 사용하지 않습니다. 삽입 위치는 문자열·블록주석·매크로 연속줄 바깥으로 검토했습니다.

CPU 번역 단위 1개는 GCC `-E -P` 결과도 동일했습니다. 다른 폴더의 `__FILE__` 경로만 `-fmacro-prefix-map=<각 원본 또는 주석본 루트>=SOURCE`로 맞췄고 진단 행번호는 지우거나 바꾸지 않았습니다. Windows/HLSL 미실행 파일은 원본 복원·해시 검사만 확인했습니다.

## 빌드와 실제 검증

CMake가 있는 환경에서는 이 폴더 루트에서:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

이번 Cloud Linux / GCC 14.2.0 직접 C++20 빌드·실행(NDEBUG 미설정):

```sh
g++ -std=c++20 -Wall -Wextra -Wpedantic -Werror main.cpp -o /tmp/dx11-13--main
/tmp/dx11-13--main
```

- `main.cpp`: `PASS 37 DX11 instancing CPU contract checks`

이번 환경에 cmake/ctest가 없어 **CMake 구성·CTest는 실행하지 않았습니다.** Windows/MSVC, D3D11 장치, GPU 출력, FXC/DXC/HLSL 컴파일 및 sanitizer는 이번에 실행하지 않았습니다. CPU 계약 검사가 실제 셰이더·드라이버·화면의 성공을 증명하지 않습니다. 사용자 컴퓨터에 접근하거나 실행을 요청하지 않았습니다. 원문 C++11/과거 sanitizer 기록과 이번 C++20 실행을 구별합니다.

정확한 결과는 `validation.json`, 파일 해시는 `annotation-manifest.json`에서 확인할 수 있습니다.
