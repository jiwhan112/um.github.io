# 자료구조·알고리즘 04 · 스택·큐·링 버퍼

head와 size만으로 빈 큐와 가득 찬 큐를 구별하고 감긴 인덱스를 계산한다.

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
FIFO ring capacities=1,2,7; LIFO undo order=30,20,10
PASS: 9085 contract checks
```

표준입력·인터넷·외부 데이터 파일은 사용하지 않습니다. Check 함수는 Release에서도 유지되며 실패 시 예외로 비정상 종료합니다. 호출 횟수는 반복문을 포함하므로 서로 독립적인 테스트 사례 개수가 아닙니다.

## 코드 읽기와 계약

편집기의 실행 취소는 마지막 명령부터 되돌리지만 네트워크 수신 처리 대기열은 먼저 들어온 메시지부터 소비한다. 둘 다 원소를 넣고 빼더라도 순서 계약이 다르다. 수신 버퍼를 무한히 늘리는 대신 꽉 찼을 때의 정책도 정해야 한다.

고정 용량 구성 O(capacity), Push/Pop O(1), 저장 공간 O(capacity). 동적 성장·스레드 동기화·입출력은 포함하지 않는다.

주의: capacity=0에서 나머지 연산을 수행하면 잘못이다. unsigned size를 빈 상태에서 감소시키거나, 가득 찬 상태에서 size를 증가시키지 않도록 접근 전에 검사한다.

## 실제 검증

2026-10-26 Linux GCC 14.2.0 C++20 Release에서 CMake 구성·빌드·CTest 1/1 성공. 같은 소스를 UBSan으로 별도 빌드해 출력 일치와 진단 없음 확인. Windows/MSVC·Clang·GPU·성능 측정·다중 스레드는 실행하지 않았습니다. SHA-256과 실행 출력은 교재의 verification.json에 있습니다.

## 출처와 저작

개념 확인 참고: https://opendatastructures.org/ods-cpp/2_Array_Based_Lists.html

게임 상황·설명·그림·코드·테스트는 ATLAS NOTE용 독립 작성입니다. 외부 예제나 교재 본문을 복제하지 않았습니다.

## 복사·이동 정책과 회귀 검사

RingQueue는 이 교육 구현에서 복사 생성·복사 대입·이동 생성·이동 대입을 모두 명시적으로 금지한다. 저장소만 기본 이동하고 size/head/capacity가 남으면 이동 원본을 정상 객체처럼 사용하는 순간 불변식이 깨진다. 이동 후 상태를 복구하는 구현은 이번 범위에 없으므로 컴파일 단계에서 그런 사용을 거절한다. 객체 자체의 수명은 그대로 사용하고 다른 소유 객체로 옮기는 기능이 필요하면 별도 계약을 설계해야 한다.

복사 생성·복사 대입·이동 생성·이동 대입의 type_traits를 검사하는 static_assert 4개가 있습니다. 이 컴파일 시점 검사는 출력의 런타임 Check 수에 합산하지 않습니다. 수정된 동일 소스로 Release CTest 및 UBSan을 다시 실행했습니다.

용량 검사도 저장소 할당 전에 수행합니다. SIZE_MAX 거절과 허용 최대 용량1024의 전체 삽입·배출을 회귀 검사합니다.
