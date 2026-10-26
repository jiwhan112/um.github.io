# 자료구조·알고리즘 09 · 이분 탐색·반열린 구간

lower/upper 경계를 분리해 중복 구간과 삽입 위치를 안전하게 계산한다.

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
lower([4,4,4],4)=0 upper=3; insertion boundaries include size
PASS: 6402 contract checks
```

표준입력·인터넷·외부 데이터 파일은 사용하지 않습니다. Check 함수는 Release에서도 유지되며 실패 시 예외로 비정상 종료합니다. 호출 횟수는 반복문을 포함하므로 서로 독립적인 테스트 사례 개수가 아닙니다.

## 코드 읽기와 계약

시간순으로 정렬된 리플레이 이벤트에서 특정 시각의 첫 항목을 찾고 같은 시각의 항목을 전부 묶고 싶다. 임의의 일치 원소 하나를 찾는 검색보다 경계 두 개를 찾는 방식이 요구에 잘 맞는다.

정렬된 vector에서 비교·인덱스 접근 O(log n), 추가 공간 O(1). 정렬 준비 O(n log n)나 입력 검증 O(n)은 검색 자체와 별도로 계산한다.

주의: 찾은 위치가 n일 때 역참조하면 범위 밖이다. 존재 검사에는 pos<n && a[pos]==x를 사용한다. 중복 입력에서 lower와upper를 뒤바꾸면 개수와 삽입 정책이 달라진다.

## 실제 검증

2026-10-26 Linux GCC 14.2.0 C++20 Release에서 CMake 구성·빌드·CTest 1/1 성공. 같은 소스를 UBSan으로 별도 빌드해 출력 일치와 진단 없음 확인. Windows/MSVC·Clang·GPU·성능 측정·다중 스레드는 실행하지 않았습니다. SHA-256과 실행 출력은 교재의 verification.json에 있습니다.

## 출처와 저작

개념 확인 참고: https://eel.is/c++draft/alg.binary.search

게임 상황·설명·그림·코드·테스트는 ATLAS NOTE용 독립 작성입니다. 외부 예제나 교재 본문을 복제하지 않았습니다.
