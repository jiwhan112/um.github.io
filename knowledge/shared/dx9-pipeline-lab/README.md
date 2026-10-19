# DX9 렌더링 파이프라인 — CPU 학습 실습 v1.0

개정일: 2026-10-13. C++20. 외부 라이브러리 없이 CMake로 빌드하는 **수치·상태 학습 모델**입니다.
D3D9 런타임이나 GPU를 호출하는 렌더러가 아닙니다. 창이나 이미지를 출력하지 않고 결과를 콘솔과 조건 검사로 확인합니다.

## 읽는 순서

| 실행 파일 | 소스 | 관찰하는 것 | 검사 횟수 |
|---|---|---|---:|
| vertex_input | src/vertex_input.cpp | 구조체 크기, stride, 속성 offset, 인덱스·버퍼 경계 | 22 |
| transform_clip | src/transform_clip.cpp | 로컬→화면, near/far, 동차 6평면 클리핑 | 79 |
| raster_uv | src/raster_uv.cpp | Top-left 소속, 무게중심, UV 원근 보정, 깊이·텍셀 중심 | 71 |
| depth_blend | src/depth_blend.cpp | 깊이 비교/쓰기, 알파 거부, 혼합 순서, premultiplied 표현 | 19 |
| pipeline_trace | src/pipeline_trace.cpp | 8×8 타깃의 단일 삼각형 처리 흐름 | 105 |
| **합계** | | 동일 조건을 각 빌드 구성에서 반복 | **296** |

`include/pipeline.hpp`는 공통 계산, `include/check.hpp`는 테스트 보조입니다.
`examples/texture_sm3.hlsl`은 D3D9 셰이더 대응을 설명하는 **컴파일·GPU 실행 미검증 참고 소스**입니다.
`docs/validation.md`에 실제 검증과 제한을, `logs/`에 최초 제공 시 검증 원시 로그를 담았습니다.

## 일반 빌드

C++20을 지원하는 컴파일러와 CMake 3.20 이상이 필요합니다.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

상세 출력을 보려면 `ctest --test-dir build -C Debug -V`를 실행합니다.
Windows/MSVC에서도 사용할 수 있는 설정을 넣었지만 **이번 제공본에서 실제 Windows 빌드를 수행한 것은 아닙니다**.
회사의 C++11 프로젝트에 그대로 복사하지 마세요. std::span, std::lerp 등 C++20 기능을 사용합니다.

## 재검증 스크립트

Python 3.9 이상과 CMake가 있는 환경에서 다음과 같이 실행합니다.

```sh
python3 verify.py --compiler g++ --build-dir ../pipeline-check-gcc
python3 verify.py --compiler clang++ --build-dir ../pipeline-check-clang
python3 verify.py --compiler clang++ --sanitizers --build-dir ../pipeline-check-san
```

Windows Python 명령은 `python`일 수 있습니다. MSVC는 Developer Command Prompt에서 컴파일러 인자를 생략해 CMake 기본 생성기를 쓰세요.
Sanitizer 옵션은 이 프로젝트에서 GCC/Clang 계열용이며 MSVC용으로 구현하지 않았습니다.
스크립트는 한 번 실행하고 종료합니다. 기존 디렉터리를 삭제하지 않습니다. 다른 컴파일러로 바꾸려면 새로운 build-dir을 지정합니다.

## 계산 계약

행 벡터 `p' = pM`, 행별 4×4 double 배열, 뷰 공간 +Z 전방, 일반 깊이 near→0/far→1을 사용합니다.
원근 행렬은 `0<fov<pi`, `aspect>0`, `0<near<far`만 지원합니다. Reverse-Z·무한 far·역투영은 범위 밖입니다.
클리핑은 볼록 다각형에 대한 6개 동차 평면과 원래 UV의 선형 교점 생성입니다. 입력 정점 수는 최대64이며 경계 중복 정점 제거는 보장하지 않습니다.
Project는 **클리핑을 대신하지 않습니다**. 호출자는 클리핑 후 w>1e-12인 정점을 전달해야 합니다.

Covers는 방향을 맞춘 뒤 Top-left 규칙으로 정수 중심 단일 샘플을 판정합니다. 그 자체로 앞/뒷면을 제거하지 않습니다.
Barycentric/PerspectiveUV는 유효한 화면 삼각형과 그 샘플을 대상으로 합니다. 원래 UV 및 입력 데이터가 유한하고 해당 계약을 만족해야 합니다.
깊이는 이미 나눈 z/w의 화면 선형 보간입니다. UV용 원근 보정 식을 다시 적용하지 않습니다.
텍스처 실습은 1차원 level0 선형 필터와 clamp만 지원합니다. 모든 입력 텍셀은 유한한 값이어야 합니다.

출력 모델은 LESS_EQUAL, 선택적인 연속 double 알파 cutoff, ADD/source-over RGB, 별도 알파 ONE/INVSRCALPHA, RGBA 쓰기만 지원합니다.
목적지 색·깊이는 유효하게 초기화되어야 합니다. 스텐실, 다른 비교/블렌딩 연산, 쓰기 마스크, 포맷 양자화, sRGB, MSAA는 없습니다.
`depth_test=false`면 깊이 쓰기도 하지 않는 D3DRS_ZENABLE=FALSE 의미를 사용합니다.

수치 헬퍼 일부는 절댓값1e9 범위와 절대 임계값1e-12를 둡니다. 모든 크기의 입력에 대한 안정성·조건수 평가·생산용 수학 라이브러리를 대체하지 않습니다.
FetchOffset은 안전한 단일 스트림 바이트 산술 예제입니다. 드라이버의 전체 유효성 검사·선언 정합성을 구현하지 않으며 stride0의 API 특수 사용도 제외했습니다.

## 핵심 관찰

- `(1,1,0,1)` → W(1,0,0) → V(0,0,5) → P(90°,4/3,1,11) → clip(1.5,1,4.4,5) → NDC(0.3,0.2,0.88) → 화면(520,240),깊이0.88.
- 꼭짓점이 모두 화면 밖이어도 삼각형 내부가 클립 영역과 교차할 수 있습니다.
- lambda=(.5,.25,.25), w=(1,2,4), UV=((0,0),(1,0),(0,1))에서 단순 UV는(.25,.25), 원근 보정은(2/11,1/11)입니다.
- 알파0인 면도 깊이 쓰기가 켜져 있으면 뒤 물체를 가릴 수 있습니다.
- 삼각형 추적의 커버된 샘플은25개이며, 샘플(4,4)의 UV=(.5,.5),깊이=.4입니다.

## 이번에 수행하지 않은 것

실제 D3D9/Windows/MSVC/FXC/HLSL/GPU 실행, 성능 측정, 드라이버 적합성, 모든 GPU 정밀도·서브픽셀 규칙의 모사는 수행하지 않았습니다.
Sanitizer 통과가 전체 입력과 모든 환경에서 안전하다는 증명은 아닙니다.
수식 테스트는 의도한 계산을 확인합니다. 실제 GPU 캡처와 비교한 결과로 제시하지 않습니다.

## 학습 문서

[Notion 학습 목차](https://app.notion.com/p/3e66577f53a549fe99a94ac26f68a685)

