# 자료구조·알고리즘 02 · 동적 배열·용량·분할 상환

size와 capacity를 구분하고 배증 성장의 총 복사 비용을 합으로 설명한다.

## 준비와 실행

C++20 지원 컴파일러와 CMake 3.16 이상이 필요합니다. ZIP을 새 폴더에 풀고 README가 있는 곳에서 다음 명령을 실행합니다. 소스만 제공하며 다운로드가 실행을 시작하지 않습니다.

```sh
cmake -S project -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Linux 직접 실행: `./build/lesson`. Windows 다중 구성 생성기는 `cmake --build build --config Release`, `ctest --test-dir build -C Release --output-on-failure`를 사용합니다. Windows 명령은 안내이며 실제 검증하지 않았습니다.

## 기대 출력

```text
1024 appends; growth copies=1023; capacity=1024
PASS: 4099 contract checks
```

표준입력·인터넷·외부 데이터 파일은 사용하지 않습니다. Check 함수는 Release에서도 유지되며 실패 시 예외로 비정상 종료합니다. 호출 횟수는 반복문을 포함하므로 서로 독립적인 테스트 사례 개수가 아닙니다.

## 코드 읽기와 계약

매 프레임 생성된 엔티티 ID를 모으는 배열은 연속 메모리 순회에 유리하다. 하지만 가득 찬 순간의 재할당은 기존 원소를 옮겨야 한다. 평균적으로 싸다는 설명이 프레임 지연이 없다는 보장은 아니다.

At은 O(1). Push는 최악 O(n), 분할 상환 O(1). 현재 저장 공간 O(n), 성장 중에는 이전·새 배열이 함께 존재해 일시적으로 추가 O(n)을 쓴다.

주의: 매번 용량을 1만 증가시키면 복사 합은 1+2+…+(N−1)=O(N²)가 된다. 반대로 배증만 강조하고 최대 용량·곱셈 오버플로 검사를 놓치면 대형 입력에서 위험하다. 여기서는1024상한이 배증을 안전하게 제한한다.

## 실제 검증

2026-10-26 Linux GCC 14.2.0 C++20 Release에서 CMake 구성·빌드·CTest 1/1 성공. 같은 소스를 UBSan으로 별도 빌드해 출력 일치와 진단 없음 확인. Windows/MSVC·Clang·GPU·성능 측정·다중 스레드는 실행하지 않았습니다. SHA-256과 실행 출력은 교재의 verification.json에 있습니다.

## 출처와 저작

개념 확인 참고: https://opendatastructures.org/ods-cpp/2_Array_Based_Lists.html

게임 상황·설명·그림·코드·테스트는 ATLAS NOTE용 독립 작성입니다. 외부 예제나 교재 본문을 복제하지 않았습니다.

## 복사·이동 정책과 회귀 검사

IdBuffer는 이 교육 구현에서 복사 생성·복사 대입·이동 생성·이동 대입을 모두 명시적으로 금지한다. 저장소만 기본 이동하고 size/head/capacity가 남으면 이동 원본을 정상 객체처럼 사용하는 순간 불변식이 깨진다. 이동 후 상태를 복구하는 구현은 이번 범위에 없으므로 컴파일 단계에서 그런 사용을 거절한다. 객체 자체의 수명은 그대로 사용하고 다른 소유 객체로 옮기는 기능이 필요하면 별도 계약을 설계해야 한다.

복사 생성·복사 대입·이동 생성·이동 대입의 type_traits를 검사하는 static_assert 4개가 있습니다. 이 컴파일 시점 검사는 출력의 런타임 Check 수에 합산하지 않습니다. 수정된 동일 소스로 Release CTest 및 UBSan을 다시 실행했습니다.
