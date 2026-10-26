# 그래픽스 기술 03 · BRDF·GGX·거칠기·금속도

## 무엇을 실행하나

같은 base color에서도 금속도와 거칠기를 바꾸면 어떤 항이 달라지는지 식과 수치로 추적한다. BRDF 값이 1보다 크다는 이유만으로 에너지 보존 실패라고 판단하지 않고, 방향 적분과 단일 방향 평가를 구분한다.

실습은 한 색 채널의 선형 값, 단일 직접광 radiance=1을 사용한다. nl/nv/nh/vh는 [0,1]의 내적 입력이며 본 테스트의 일부 조합은 함수 경계를 검사하는 합성 수치다. GI, IBL, 다중 산란 보정, 면적광, 투과 재질은 제외한다. perceptual roughness r를 alpha=r²로 변환한다.

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
plastic=0.203718 metal=0.63662 (BRDF response, not display signal)
PASS: 113 contract checks
```

검사 함수는 `assert`가 아니라 실패 시 예외를 던지므로 Release/NDEBUG에서도 실행된다. 113회 조건 평가에는 반복 sweep이 포함되어 있고 Debug·Release·UBSan 재실행을 독립 검사의 수로 더하지 않는다.

## 코드 읽기와 복습

base=0.5, metallic=0, r=0.5, 모든 방향 내적=1이면 alpha=0.25, F0=0.04이다. 같은 입력에서 metallic=1로 바꾸면 diffuse가 0이 되고 F0가 0.5가 된다. 출력값은 선형 BRDF 응답이며 후단 톤 매핑 전이다.

고정 방향 하나의 평가 O(1), 픽셀 P개·광원 L개를 직접 반복하면 O(P·L). 클러스터드 조명·IBL 사전 계산·실제 GPU 파이프라인 비용은 이 실습 밖이다.

실습 확장: 입력 한 조건을 바꾸기 전에 기대 값을 종이에 적고, 기존 경계 테스트를 남긴 채 새 조건을 추가한다. 검사를 지우거나 허용 오차를 키워 실패를 숨기지 않는다. 본문 #review의 여섯 질문을 코드 실행 전후에 비교한다.

## 확인한 범위

2026-10-26 Linux x86_64, GCC 14.2.0, CMake 3.31.6에서 Debug/Release 각각 구성·빌드·CTest 1/1 및 실행 통과. GCC UndefinedBehaviorSanitizer 실행도 통과했다. 실제 로그와 SHA-256은 교재의 `knowledge/shared/graphics-tech-03-lab/verification.json`에 있다.

Windows/MSVC, GPU 셰이더 컴파일·실행, 실제 영상 품질, 디스플레이 색 정확성, GPU 시간은 검증하지 않았다. 그림은 자체 제작 개념도이지 GPU 캡처가 아니다.
