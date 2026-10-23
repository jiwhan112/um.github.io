# DX11 Frustum — 한국어 전체 주석본

2026-10-23. `../dx11-frustum/main.cpp`와 `CMakeLists.txt` 보존 원본의 파생 주석본입니다. 새 강의나 새 과정이 아닙니다. 원본은 변경하지 않았습니다.

## 규약과 범위

전체 main.cpp에 함수 계약·실패 경계를 설명하는 한국어 줄끝 주석 6개를 추가했습니다. 이 단원은 **열 벡터 clip=M*world**이고 다른 교재의 행 벡터 규약과 다릅니다. 평면은 행을 조합하며 near 평면은 z행입니다. classify는 extract에서 만든 유효 평면을 전제로 하고 임의 외부 평면을 다시 완전히 검증하지 않습니다. 정밀 범용 수치 라이브러리가 아닙니다.

`annotation-manifest.json`의 정확한 suffix만 제거하면 원본 바이트가 복원됩니다. 문자열·raw string을 정규식으로 지우지 않습니다. 물리행 수와 마지막 개행도 보존했고 SHA-256 검사를 통과했습니다. 삽입 위치는 문자열·블록 주석·연속 매크로 바깥으로 검토했습니다.

```sh
# 저장소 루트
python3 knowledge/dx11-frustum-ko/verify_annotations.py
g++ -std=c++20 -Wall -Wextra -Wpedantic -Werror knowledge/dx11-frustum-ko/main.cpp -o /tmp/frustum-ko
/tmp/frustum-ko
```

Cloud Linux / GCC 14.2.0 직접 빌드·실행: **PASS 33 frustum plane and AABB CPU checks**. `-E -P` 원본/주석본 결과도 바이트 동일합니다. 다른 폴더의 __FILE__만 각 루트에서 SOURCE로 `-fmacro-prefix-map` 매핑했고 진단 행번호는 변경하지 않았습니다.

CMake가 있는 환경에서는 이 폴더에서 `cmake -S . -B build`, `cmake --build build`, `ctest --test-dir build --output-on-failure`를 사용할 수 있습니다. **이번에는 cmake/ctest 미설치로 실행하지 않았습니다.** Windows/MSVC/GPU/FXC/DXC/sanitizer 미실행이며 CPU 통과를 실제 화면 컬링 성공으로 표시하지 않습니다. 사용자 컴퓨터에 접근하거나 실행을 요청하지 않았습니다.

파일 해시는 manifest, 이번 환경/결과는 `validation.json`을 확인하세요. 원본의 과거 검증 기록은 이번 결과와 별개로 보존합니다.
