# DX9 렌더링 파이프라인 실습 검증 기록 v1.0

개정일: 2026-10-13. 대상: CPU 수치·상태 모델. 실제 Direct3D 런타임/GPU 검증이 아닙니다.

## 실행 환경
Linux x86-64; GCC 14.2.0 (Debian 14.2.0-19); Clang 17.0.0; CMake 3.31.6; C++20.

| 프로그램 | 조건 검사 | 확인 내용 |
|---|---:|---|
| vertex_input | 22 | stride, attribute offset, byte/index units, range rejection |
| transform_clip | 79 | W/V/P, homogeneous clip planes, viewport, invalid input |
| raster_uv | 71 | top-left coverage, barycentrics, perspective UV, depth, texel centers |
| depth_blend | 19 | depth test/write, alpha rejection, alpha-zero occlusion, blending order |
| pipeline_trace | 105 | 8x8 single-sample triangle trace, 25 covered samples |
| 합계 | 296 | 반복 샘플 조건 검사 포함 |

GCC, Clang, Clang+ASan/UBSan에서 각각 5/5 프로그램 및 296회 조건 검사가 통과했습니다.
동일 검사를 반복했으므로 서로 다른 조건 888개로 합산하지 않습니다.
컴파일 거부 예제는 없으며, 잘못된 입력의 함수 거부를 실행 중 검사했습니다.
최종 소스에 대해 verify.py의 구성·빌드·CTest 동작도 세 구성에서 실제 실행했습니다.

## 관찰 결과
- 로컬(1,1,0,1), 월드 이동(1,0,0), 뷰 이동(0,0,5), FOV90도/aspect4:3/near1/far11에서 clip=(1.5,1,4.4,5), NDC=(.3,.2,.88), 800x600 화면=(520,240),깊이=.88.
- near 교차 삼각형을 4정점 다각형으로 잘랐고, 새 교점의 UV도 검사했습니다.
- 세 꼭짓점이 모두 바깥에 있어도 삼각형이 클립 영역과 교차하는 사례를 검사했습니다.
- UV의 단순 보간(.25,.25)과 원근 보정(2/11,1/11)을 비교했습니다.
- 정수 샘플 중심을 이용한 두 삼각형의 공유 모서리 소속을 확인했습니다.
- 텍셀(0,1,0,1)에서 u=.25의 값=.5, u=.375의 값=1을 확인했습니다.
- 알파0도 깊이 쓰기가 켜지면 뒤 물체를 가릴 수 있음을 확인했습니다.
- 투명 파랑→빨강 RGB=(.5,0,.25), 반대 순서=(.25,0,.5).
- 통합 추적은 25개 샘플을 포함하며, 샘플(4,4)의 UV=(.5,.5),깊이=.4.

## 빌드 옵션과 로그
GCC/Clang: -Wall -Wextra -Wpedantic -Wconversion -Wshadow, C++20, 확장 비활성.
Sanitizer: -fsanitize=address,undefined -fno-omit-frame-pointer.
logs/의 gcc_*, clang_*, sanitizer_* 파일은 각 구성의 최종 configure/build/test 출력입니다.
전체 소스와 재검증 명령은 README 및 verify.py에 있습니다.

## 해석 제한
이 결과는 실제 화면 캡처나 GPU와의 비트 단위 대조가 아닙니다.
Windows/MSVC, FXC/HLSL, D3D9 자원·드로우·Present, GPU 실행과 성능은 미검증입니다.
examples/texture_sm3.hlsl은 CMake 기본 대상에서 제외한 미검증 참고 소스입니다.
스텐실·sRGB·MSAA·실제 2D 텍스처 필터·하드웨어 포맷 정밀도는 구현하지 않았습니다.
CPU 모델의 수치 범위·입력 계약과 1차원 샘플러 제한은 README에 명시했습니다.
검사기에서 진단이 없다는 사실만으로 모든 입력·환경의 안전성이 증명되지는 않습니다.

## 관련 문서
- [DX9 렌더링 파이프라인](https://app.notion.com/p/3e66577f53a549fe99a94ac26f68a685)
- [실습·진단 본문](https://app.notion.com/p/3f8a99703f408105948ddf79d5139bb1)

