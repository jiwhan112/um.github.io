# 그래픽스 기술 09 · Two-Bone IK·도달 영역·CCD

## 무엇을 실행하나

두 관절의 도달 영역이 원판이 아니라 길이에 따라 안쪽 구멍을 가진 고리라는 사실을 설명한다. 해석적 two-bone 해와 반복적 CCD의 결과·오차·반복 상한을 구별한다.

2D 평면, root=(0,0)이다. 두 방법의 bone 길이는 [10^-4,1000], 입력 target·CCD joint 좌표의 각 성분 절댓값은 10^6 이하다. two-bone의 target 거리는 정확히 0이거나 10^-4 이상이다. 이는 단위가 정해진 교육용 범위이며 더 작은 길이와 더 큰 좌표는 유한하더라도 거절한다. bend=±1로 팔꿈치 쪽을 선택한다. CCD는 관절 2–32개, 반복≤1000이며 각도 제한·충돌·orientation 목표가 없다. 3D 스키닝과 animation blending은 미구현이다.

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
CCD sweeps=1 error=0
PASS: 20 contract checks
```

검사 함수는 `assert`가 아니라 실패 시 예외를 던지므로 Release/NDEBUG에서도 실행된다. 20회 조건 평가에는 반복 sweep이 포함되어 있고 Debug·Release·UBSan 재실행을 독립 검사의 수로 더하지 않는다.

## 코드 읽기와 복습

a=b=1, target=(1,1)이면 거리 √2이고 두 팔꿈치 해가 존재한다. target=(10,0)은 최대 거리 2로 clamp된다. a=2,b=1,target=(0,0)은 안쪽 한계 1로 clamp되어 원점에 닿지 않는다.

two-bone 해는 O(1). J개 관절 CCD의 현재 구현은 각 관절 뒤 점을 회전하므로 sweep당 O(J²), I회면 O(IJ²), 저장 O(J). quaternion 누적 등 다른 표현의 복잡도와 구별한다.

실습 확장: 입력 한 조건을 바꾸기 전에 기대 값을 종이에 적고, 기존 경계 테스트를 남긴 채 새 조건을 추가한다. 검사를 지우거나 허용 오차를 키워 실패를 숨기지 않는다. 본문 #review의 여섯 질문을 코드 실행 전후에 비교한다.

## 확인한 범위

2026-10-26 Linux x86_64, GCC 14.2.0, CMake 3.31.6에서 Debug/Release 각각 구성·빌드·CTest 1/1 및 실행 통과. GCC UndefinedBehaviorSanitizer 실행도 통과했다. 실제 로그와 SHA-256은 교재의 `knowledge/shared/graphics-tech-09-lab/verification.json`에 있다.

Windows/MSVC, GPU 셰이더 컴파일·실행, 실제 영상 품질, 디스플레이 색 정확성, GPU 시간은 검증하지 않았다. 그림은 자체 제작 개념도이지 GPU 캡처가 아니다.
