# DX9 파이프라인 전체 소스 — 한국어 주석본

2026-10-23 제작. `../dx9-pipeline-lab/` 보존 원본의 학습용 파생 주석본입니다. 원본·과거 검증 기록을 변경하지 않습니다. 실제 D3D9 드라이버나 완전한 소프트웨어 렌더러가 아니라, 제한된 파이프라인 계약을 CPU에서 설명하는 예제입니다.

## 파일과 연결

- `include/pipeline.hpp`: 행 벡터 변환, 동차 클리핑, 화면 변환, 커버리지, 보간, 주소 계산, 깊이/블렌딩 모델.
- `include/check.hpp`: 조건·근사값·예외 검사 도우미.
- `src/vertex_input.cpp`: 정점 배치와 바이트 주소, 경계 거부.
- `src/transform_clip.cpp`: 좌표 변환, 여섯 평면 절단, 부분 교차.
- `src/raster_uv.cpp`: 공유 변 소유권, 원근 UV, 화면 깊이, 텍셀 중심.
- `src/depth_blend.cpp`: 깊이 검사/기록과 알파 테스트/블렌딩 차이.
- `src/pipeline_trace.cpp`: 한 삼각형을 8×8 화면 샘플까지 연결하는 CPU 추적.
- `examples/texture_sm3.hlsl`: VS/PS 3.0 참조 예제. 호스트 정점 선언·float 행렬 상수·텍스처/샘플러 준비가 필요합니다.
- `CMakeLists.txt`: 보존 원본과 바이트가 같은 전체 CPU 빌드 구성. 이 파일은 HLSL을 컴파일하지 않습니다.

각 파일은 생략 없는 전체 소스입니다. C++ 프로그램을 실행하려면 해당 `src` 파일만이 아니라 두 `include` 헤더도 받아 폴더 구조를 유지해야 합니다. GPU에 CPU `double` 배열을 그대로 올리지 마세요. HLSL은 `float4` 행 네 개와 맞는 호스트 데이터를 요구합니다.

## 원본 동일성과 검증

문자열·코드·기존 주석·공백을 바꾸지 않고 독립 `// [한국어 해설]` 56줄을 추가했습니다. `annotation-manifest.json`의 9개 파일 해시와 삽입 줄 번호를 사용합니다.

```sh
# 저장소 루트, 원본과 주석본이 함께 있는 상태
python3 knowledge/shared/dx9-pipeline-lab-ko/verify_annotations.py
```

검증기는 지정된 주석 줄만 제거해 **원본 바이트를 완전히 복원**합니다. 문자열 속 `//`, `/*`, raw string을 정규식으로 삭제하지 않습니다. 삽입 위치는 문자열·블록 주석·매크로 연속줄 바깥으로 검토했습니다. 범용 C++/HLSL 토크나이저라는 주장은 하지 않습니다.

추가로 CPU 프로그램 5개는 GCC `-E -P` 결과가 원본과 주석본에서 바이트 일치했습니다. HLSL은 이 컴파일러 검사 대상이 아니며 원본 복원·해시만 확인했습니다.

## 빌드

CMake가 있는 환경에서 이 폴더 기준:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

CMake 없는 Linux에서 이번 실제 사용한 방식:

```sh
for name in vertex_input transform_clip raster_uv depth_blend pipeline_trace; do
  g++ -std=c++20 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror -I include src/$name.cpp -o /tmp/$name
  /tmp/$name
done
```

## 이번 실제 결과와 한계

Cloud Linux / GCC 14.2.0 직접 빌드·실행: vertex_input **22**, transform_clip **79**, raster_uv **71**, depth_blend **19**, pipeline_trace **105** 검사 통과. 8×8 예제는 **25개 샘플**, 중심 UV=(0.5,0.5), 깊이=0.4를 확인했습니다.

이 환경에 cmake/ctest가 없어 **이번 CMake 구성·CTest는 실행하지 않았습니다.** Windows/MSVC·FXC·HLSL 컴파일·GPU 출력·D3D9 드라이버 적합성·ASan/UBSan은 이번에 실행하지 않았습니다. CPU 통과가 GPU 픽셀 일치나 실제 셰이더 실행을 증명하지 않습니다. 사용자 컴퓨터에 접근하거나 실행을 요청하지 않았습니다.

원본의 과거 검증 기록은 `../dx9-pipeline-lab/docs/validation.md`에서 별도로 보존합니다. 이번 주석본 결과는 `validation.json`, 정확한 소스 해시는 `annotation-manifest.json`을 확인하세요.
