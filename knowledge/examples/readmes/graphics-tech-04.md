# 그래픽스 기술 04 · Shadow Map·깊이 비교·Bias·PCF

## 무엇을 실행하나

light-space의 receiver 깊이와 저장된 blocker 깊이의 관계로 그림자를 판단한다. PCF, bias, border의 세 정책을 나누어 각각의 반례를 만든다.

표준 Z로 가까움=0, 멂=1, 비교는 receiver-bias≤stored이면 밝음이다. 3×3 CPU 맵을 쓰며 UV는 [0,1)이다. 범위 밖 UV/탭은 밝음이라는 명시적 예제 정책을 쓴다. reverse-Z, perspective divide, 기울기 기반 raster bias, comparison sampler의 bilinear 필터는 구현하지 않는다.

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
3x3 PCF visibility=0.444444
PASS: 12 contract checks
```

검사 함수는 `assert`가 아니라 실패 시 예외를 던지므로 Release/NDEBUG에서도 실행된다. 12회 조건 평가에는 반복 sweep이 포함되어 있고 Debug·Release·UBSan 재실행을 독립 검사의 수로 더하지 않는다.

## 코드 읽기와 복습

중앙 3×3 맵: 낮은 깊이 0.2가 5개, 높은 깊이 0.8이 4개다. receiver=0.5, bias=0에서 낮은 5개는 가려짐, 높은 4개는 밝음이므로 PCF=4/9≈0.444444이다.

반경 r이면 픽셀당 (2r+1)² 비교, O(r²) 시간과 O(1) 추가 공간이다. 예제 Validate는 호출당 맵 전체 M을 검사하므로 실제 함수는 O(M+r²); 생산 코드에서는 업로드 시 한 번 검증할 부분이다.

실습 확장: 입력 한 조건을 바꾸기 전에 기대 값을 종이에 적고, 기존 경계 테스트를 남긴 채 새 조건을 추가한다. 검사를 지우거나 허용 오차를 키워 실패를 숨기지 않는다. 본문 #review의 여섯 질문을 코드 실행 전후에 비교한다.

## 확인한 범위

2026-10-26 Linux x86_64, GCC 14.2.0, CMake 3.31.6에서 Debug/Release 각각 구성·빌드·CTest 1/1 및 실행 통과. GCC UndefinedBehaviorSanitizer 실행도 통과했다. 실제 로그와 SHA-256은 교재의 `knowledge/shared/graphics-tech-04-lab/verification.json`에 있다.

Windows/MSVC, GPU 셰이더 컴파일·실행, 실제 영상 품질, 디스플레이 색 정확성, GPU 시간은 검증하지 않았다. 그림은 자체 제작 개념도이지 GPU 캡처가 아니다.

`project/reference.hlsl`은 셰이더 통합 설명용 보조 함수이며 entry point가 없는 파일이다. 호출부·자원·상태 계약을 설명하려고 제공하며 HLSL 컴파일이나 GPU 실행을 하지 않았다. C++의 통과 기록을 이 파일의 검증으로 대신하지 않는다.
