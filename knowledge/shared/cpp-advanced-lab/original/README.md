# C++ 심화 · Effective C++ 학습 실습 v1.0

개정일: 2026-10-13. 독립적으로 새로 작성한 C++20 학습 예제입니다.
회사 코드나 원문 책의 예제 구현을 복제한 저장소가 아닙니다.

## 빌드

CMake 3.20 이상, C++20을 지원하는 컴파일러가 필요합니다.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

Visual Studio 생성기:
```powershell
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```
Windows/MSVC 명령은 제공했지만 이 환경에서 실행 검증하지 않았습니다.

GCC/Clang을 모두 설치한 환경에서 전체 재현:
```sh
python3 run_checks.py
```

이 스크립트는 build-gcc, build-clang, build-clang-sanitized와 logs 폴더에만
빌드 결과를 기록합니다. 사용자 프로젝트나 시스템 설정을 변경하지 않습니다.
메모리 고갈, 전역 할당 함수 교체, 실제 GPU 호출을 하지 않습니다.

## 실행 파일

| 파일 | 주제 |
|---|---|
| lifetime | 초기화·파괴 순서, 부분 생성 실패, 체력 불변식 |
| ownership | Rule of Zero/Five, 깊은 복사, 실패 주입, 이동, 자기 대입 |
| polymorphism | 가상 호출, 생성·소멸 단계, 기본 인수, cast, NVI, clone |
| templates | concept, typename, 의존 기반 클래스, forwarding, capture |
| containers | 수정·조회·반복자 무효화·정렬 비교자 |
| interfaces | explicit, const iterator, 임시 객체에서의 뷰 반환 금지 |
| transaction | 준비·검증·교체, 실패 후 상태 보존, weak_ptr |
| pimpl | 실제 h/cpp 분리, incomplete type, 깊은 복사·이동 |
| allocation | 위치 지정 생성 실패, custom deleter, 한정 pmr 자원 |

negative 폴더 6개 파일은 **컴파일에 실패해야 정상**인 학습용입니다.
기본 CMake 빌드에 포함하지 않습니다. run_checks.py는 `-c`로 거부를 확인합니다.
이 파일들은 경고를 오류로 만든 것 때문에 실패하는 것이 아니라 언어/라이브러리
요구사항 위반을 보여 줍니다. 정확한 진단 위치는 logs에서 확인하세요.

## 범위와 주의

- `Counter`, `Quantity`, `AddLater`, `PowerOfTwo` 등은 테스트가 사용하는 작은 정수와
  이 실습의 GCC/Clang/MSVC형 환경을 전제로 합니다. 범용 안전 수치 라이브러리가 아닙니다.
- IntBuffer는 int 전용 학습 타입입니다. 표준 vector 대체용이 아닙니다.
- GameState의 강한 예외 보장은 자신이 소유한 동기 상태에 관한 것입니다.
  파일/네트워크 롤백, 동시성 원자성, 저장 내구성을 제공하지 않습니다.
- Pool은 단일 Widget 교육 예입니다. 범용 정렬 지원·다중 스레드 할당자·전역 new 구현이 아닙니다.
- sanitizer 무진단은 실행하지 않은 모든 경로의 안전성을 증명하지 않습니다.
- C++11 회사 코드에 그대로 복사하지 마세요. make_unique는 C++14,
  pmr·string_view는 C++17, concepts·ranges·erase_if는 C++20 기능을 사용합니다.

## 검증

Linux x86-64에서 GCC 14.2.0, Clang 17.0.0, Clang 17 ASan/UBSan의
각 정상 빌드 9/9, 조건 검사 89회 통과. 두 컴파일러가 각각 부정 예제 6개를 거부.
세 빌드의 반복 검사는 독립된 조건 267개를 뜻하지 않습니다.
실제 실행 로그 및 시스템 정보: `logs/report.json`.

연결 문서:
- https://app.notion.com/p/aaa55bced8ff459ba67fb036fb34bb3b
- https://app.notion.com/p/8178c17f593b4f4e83a0b42544b8fe2b
