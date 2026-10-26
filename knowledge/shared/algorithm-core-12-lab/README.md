# 자료구조·알고리즘 12 · 동적 계획법·0/1 배낭·상태 압축

상태의 의미와 전이의 참조 시점을 정의하고 역순 갱신이 필요한 이유를 설명한다.

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
0/1 loadout optimum=7; 300 item sets against exhaustive subsets
PASS: 4804 contract checks
```

표준입력·인터넷·외부 데이터 파일은 사용하지 않습니다. Check 함수는 Release에서도 유지되며 실패 시 예외로 비정상 종료합니다. 호출 횟수는 반복문을 포함하므로 서로 독립적인 테스트 사례 개수가 아닙니다.

## 코드 읽기와 계약

장비 무게 제한 안에서 가치 합을 최대화하되 같은 장비를 두 번 선택할 수 없다고 하자. 각 장비의 선택/미선택을 모두 열거하면2^n가지다. 작은 용량 정수라는 조건을 이용해 같은 하위 문제를 재사용할 수 있다.

DP 시간 O(nC), 추가 공간 O(C). 전수 열거 오라클 O(n2^n)는 테스트용이다. 최적 가치만 반환하며 선택한 장비 목록 복원은 추가 상태가 필요하다.

주의: 정방향 순회로 바꾸면 같은 장비가 여러 번 선택될 수 있다. 무게0이나 음수를 이 구현에 넣지 않는다. 무게0을 지원하는 별도 문제도 가능하지만 검증·전이 정책을 다시 정해야 한다.

## 실제 검증

2026-10-26 Linux GCC 14.2.0 C++20 Release에서 CMake 구성·빌드·CTest 1/1 성공. 같은 소스를 UBSan으로 별도 빌드해 출력 일치와 진단 없음 확인. Windows/MSVC·Clang·GPU·성능 측정·다중 스레드는 실행하지 않았습니다. SHA-256과 실행 출력은 교재의 verification.json에 있습니다.

## 출처와 저작

개념 확인 참고: https://ocw.mit.edu/courses/6-006-introduction-to-algorithms-spring-2020/7232b0ff982f501556118a8552654762_MIT6_006S20_r19.pdf

게임 상황·설명·그림·코드·테스트는 ATLAS NOTE용 독립 작성입니다. 외부 예제나 교재 본문을 복제하지 않았습니다.
