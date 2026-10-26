# 자료구조·알고리즘 03 · 연결 리스트·링크·소유권

머리·중간·꼬리 삭제를 같은 소유 링크 갱신으로 설명하고 탐색 비용을 구분한다.

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
head/middle/tail/duplicate erase and 1000 model operations
PASS: 1523 contract checks
```

표준입력·인터넷·외부 데이터 파일은 사용하지 않습니다. Check 함수는 Release에서도 유지되며 실패 시 예외로 비정상 종료합니다. 호출 횟수는 반복문을 포함하므로 서로 독립적인 테스트 사례 개수가 아닙니다.

## 코드 읽기와 계약

이벤트 대기열에서 특정 ID의 첫 항목을 취소하는 상황을 생각하자. 연결 리스트는 노드를 이어 붙이기 쉽지만 취소 대상을 찾는 시간이 사라지는 것은 아니다. 이 단원은 raw 포인터의 주소 퍼즐보다 객체 수명과 소유권 변경을 중심으로 읽는다.

PushFront O(1). EraseFirst는 탐색 O(n), 찾은 후 링크 갱신 O(1). Values와 Clear O(n). 노드 저장 O(n); Values는 복사 결과 O(n)을 추가로 사용한다.

주의: 이전 노드의 raw 포인터만 가지고 머리 삭제를 같은 방식으로 처리하려 하면 별도 분기가 필요하다. 삭제 뒤 해제된 노드를 읽거나, next를 옮기기 전에 노드를 지우는 use-after-free를 피한다.

## 실제 검증

2026-10-26 Linux GCC 14.2.0 C++20 Release에서 CMake 구성·빌드·CTest 1/1 성공. 같은 소스를 UBSan으로 별도 빌드해 출력 일치와 진단 없음 확인. Windows/MSVC·Clang·GPU·성능 측정·다중 스레드는 실행하지 않았습니다. SHA-256과 실행 출력은 교재의 verification.json에 있습니다.

## 출처와 저작

개념 확인 참고: https://opendatastructures.org/ods-cpp/3_Linked_Lists.html

게임 상황·설명·그림·코드·테스트는 ATLAS NOTE용 독립 작성입니다. 외부 예제나 교재 본문을 복제하지 않았습니다.
