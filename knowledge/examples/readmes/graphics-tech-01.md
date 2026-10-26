# 그래픽스 기술 01 · 색 공간·sRGB·선형 합성·HDR 경계

## 무엇을 실행하나

검은색과 흰색을 반씩 섞었을 때 저장 신호 0.5와 선형 밝기 0.5가 왜 다른지 수치로 설명한다. 투명 UI와 발광 효과를 다룰 때 RGB 변환, alpha 합성, 노출·톤 매핑이 각각 어느 단계인지 표시한다.

예제의 RGB는 sRGB 색도계의 SDR 신호 [0,1]이고 alpha는 [0,1]의 합성 가중치다. 계산은 double, 색역 변환·디스플레이 보정·HDR10 출력은 제외한다. premultiplied 색의 내부 RGB는 이미 선형이라는 계약을 지킨다. 외부 파일이나 사용자 입력은 읽지 않는다.

이 프로젝트는 C++17 CPU 수치 실습이다. 실제 DirectX 장치·GPU 셰이더·렌더링 창을 만들지 않는다. 프로그램은 표준입력, 외부 파일, 인터넷을 사용하지 않으며 실행 파일은 ZIP에 포함하지 않는다.

## 준비와 실행

C++17 컴파일러와 CMake 3.16 이상이 필요하다. ZIP을 빈 폴더에 풀고 README 위치에서 실행한다.

```sh
cmake -S project -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
./build/graphics_lab
```

Windows의 multi-config generator에서는 `cmake --build build --config Debug`와 `ctest --test-dir build -C Debug --output-on-failure`를 사용하고 `build/Debug/graphics_lab.exe`를 실행한다. 이것은 실행 안내이며 이번 Windows/MSVC 검증 결과는 아니다.

기대 출력(부동소수점 마지막 자리는 구현에 따라 조금 달라질 수 있다):

```text
linear blend encoded=0.735357; encoded-space blend=0.5
PASS: 1013 contract checks
```

검사 함수는 `assert`가 아니라 실패 시 예외를 던지므로 Release/NDEBUG에서도 실행된다. 1013회 조건 평가에는 반복 sweep이 포함되어 있고 Debug·Release·UBSan 재실행을 독립 검사의 수로 더하지 않는다.

## 코드 읽기와 복습

기준 계산: Decode(0)=0, Decode(1)=1 → 선형 평균 0.5 → Encode(0.5)=0.735357. 잘못된 경로는 먼저 (0+1)/2=0.5 신호를 만들며 디코딩하면 0.214가 되어 같은 광량을 나타내지 못한다.

픽셀당 전송·합성은 O(1) 시간과 O(1) 추가 공간이다. N개 픽셀은 O(N)이다. CPU double의 pow 비용을 GPU 하드웨어 sRGB 변환 비용과 비교한 벤치마크는 수행하지 않았다.

실습 확장: 입력 한 조건을 바꾸기 전에 기대 값을 종이에 적고, 기존 경계 테스트를 남긴 채 새 조건을 추가한다. 검사를 지우거나 허용 오차를 키워 실패를 숨기지 않는다. 본문 #review의 여섯 질문을 코드 실행 전후에 비교한다.

## 확인한 범위

2026-10-26 Linux x86_64, GCC 14.2.0, CMake 3.31.6에서 Debug/Release 각각 구성·빌드·CTest 1/1 및 실행 통과. GCC UndefinedBehaviorSanitizer 실행도 통과했다. 실제 로그와 SHA-256은 교재의 `knowledge/shared/graphics-tech-01-lab/verification.json`에 있다.

Windows/MSVC, GPU 셰이더 컴파일·실행, 실제 영상 품질, 디스플레이 색 정확성, GPU 시간은 검증하지 않았다. 그림은 자체 제작 개념도이지 GPU 캡처가 아니다.

`project/reference.hlsl`은 셰이더 통합 설명용 보조 함수이며 entry point가 없는 파일이다. 호출부·자원·상태 계약을 설명하려고 제공하며 HLSL 컴파일이나 GPU 실행을 하지 않았다. C++의 통과 기록을 이 파일의 검증으로 대신하지 않는다.
