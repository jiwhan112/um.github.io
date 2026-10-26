# 자료구조·알고리즘 07 · 힙·우선순위 큐

배열 인덱스의 부모·자식 관계와 sift-up/down의 국소 복구를 설명한다.

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
min-heap tested with duplicates and mixed push/pop
PASS: 4002 contract checks
```

표준입력·인터넷·외부 데이터 파일은 사용하지 않습니다. Check 함수는 Release에서도 유지되며 실패 시 예외로 비정상 종료합니다. 호출 횟수는 반복문을 포함하므로 서로 독립적인 테스트 사례 개수가 아닙니다.

## 코드 읽기와 계약

가장 이른 예약 이벤트를 반복적으로 꺼내려면 전체 큐를 매번 정렬할 필요가 없다. 최소 힙은 루트가 최소라는 사실을 유지하면서 나머지는 부분 순서로 남긴다. 같은 우선순위의 안정적인 처리 순서는 별도 키로 정해야 한다.

Push/Pop O(log n), 최소값 읽기 O(1), 저장 O(n). 예제의 Valid 검사는 O(n)이며 테스트용이므로 제품 힙 호출 비용과 합쳐 설명하지 않는다.

주의: 두 자식이 있을 때 무조건 왼쪽을 택하면 최소 힙 불변식이 깨진다. 빈 배열에서 front/back을 읽거나 i=0에서 (i−1)/2를 계산하지 않는다.

## 실제 검증

2026-10-26 Linux GCC 14.2.0 C++20 Release에서 CMake 구성·빌드·CTest 1/1 성공. 같은 소스를 UBSan으로 별도 빌드해 출력 일치와 진단 없음 확인. Windows/MSVC·Clang·GPU·성능 측정·다중 스레드는 실행하지 않았습니다. SHA-256과 실행 출력은 교재의 verification.json에 있습니다.

## 출처와 저작

개념 확인 참고: https://opendatastructures.org/ods-cpp/10_Heaps.html

게임 상황·설명·그림·코드·테스트는 ATLAS NOTE용 독립 작성입니다. 외부 예제나 교재 본문을 복제하지 않았습니다.
