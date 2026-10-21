# C++ 심화 03–04 · GitHub Actions 원시 검증 기록

[실행 39565624710](https://github.com/jiwhan112/um.github.io/actions/runs/39565624710)은 2026-10-21에 소스 커밋 [`cfce833`](https://github.com/jiwhan112/um.github.io/commit/cfce833acbd970e7bdbd675efd8e18e3f9f5e61c)을 실제 빌드·실행한 기록이다. 이전 01–02 배치와 Notion의 2026-10-13 로그는 별도 폴더에 그대로 보존했다.

## 실제 결과

| 환경 | Configure·Build·CTest | 추가 확인 |
| --- | --- | --- |
| Ubuntu 24.04 · GCC 13.3.0 · C++20 · CMake 3.31.6 | Configure·Build 성공, **CTest 15/15** | 원본·한국어판 출력 **6쌍** 바이트 동일, unsigned 값 비트 수 32 |
| Ubuntu 24.04 · Clang 18.1.3 · C++20 · ASan/UBSan · CMake 3.31.6 | Configure·Build 성공, **CTest 15/15** | 출력 **6쌍** 바이트 동일, 실행 경로의 sanitizer 오류 없음, unsigned 값 비트 수 32 |
| GCC / Clang C++20 컴파일 거부 예제 | 각 컴파일러에서 **6/6 예상 거부** | 링크·실행하지 않음, 정상 CTest와 별도 |
| GCC 엄격 C++11 조사 | 원본 **9/9 컴파일 거부** | C++11 미지원·미실행. 검사기의 pass는 예상 미지원 확인을 뜻함 |

CTest는 원본 9개와 한국어 주석판 6개를 실행한다. 원본 전체의 검사 호출은 89회, 이미 집필한 01–04의 범위는 60회이며 이번 03–04는 **29회(12+10+7)**다. 한국어판과 컴파일러별 재실행을 독립 조건으로 더하지 않는다. 이번 `templates`의 static_assert 3개와 `interfaces`의 1개, 별도 unsigned 폭 조회는 런타임 호출·CTest 개수에 합산하지 않는다.

## 원시 출력

| 구성 | 환경·무결성 | Configure | Build | CTest | 출력 비교 |
| --- | --- | --- | --- | --- | --- |
| GCC C++20 | [환경](gcc-cxx20/environment-and-integrity.txt) | [원시 출력](gcc-cxx20/configure.txt) | [원시 출력](gcc-cxx20/build.txt) | [원시 출력](gcc-cxx20/ctest.txt) | [결과 JSON](gcc-cxx20/output-equivalence/results.json) |
| Clang ASan/UBSan | [환경](clang-cxx20-asan-ubsan/environment-and-integrity.txt) | [원시 출력](clang-cxx20-asan-ubsan/configure.txt) | [원시 출력](clang-cxx20-asan-ubsan/build.txt) | [원시 출력](clang-cxx20-asan-ubsan/ctest.txt) | [결과 JSON](clang-cxx20-asan-ubsan/output-equivalence/results.json) |

컴파일 거부 예제는 [GCC 결과](compile-rejections-gcc/negative/results.json)와 [Clang 결과](compile-rejections-clang/negative/results.json), C++11 진단은 [GCC 결과](compile-rejections-gcc/cxx11-unsupported/results.json)에 각각 연결한다. 각 결과와 같은 폴더의 이름별 `.txt`가 원시 진단이다. 거부 판정은 종료 코드와 진단 존재를 자동 확인하며, 별도로 원시 진단을 읽어 제약 불충족·값 매개변수 복사 생성자·삭제된 복사·반복자 부적합·삭제된 뷰·잘못된 override라는 의도한 이유를 확인했다.

Clang 원시 Build 출력에서 16개 번역 단위와 실행 파일 15개의 링크 모두에 ASan/UBSan이 적용됨을 확인했다. 원본 pimpl이 두 cpp 파일을 사용하므로 번역 단위 수와 프로그램 수가 다르다. `-fsanitize=address,undefined`와 컴파일 단계의 `-fno-omit-frame-pointer`가 원본과 한국어판 모두에 포함된다.

## 보존 방법과 무결성

GitHub API가 제공한 artifact ZIP **4개의 SHA-256 digest**를 다운로드 바이트와 대조한 뒤, UTF-8 원시 파일을 수정 없이 추출했다. [artifact-manifest.json](artifact-manifest.json)에 artifact ID·이름·ZIP 해시·대조 결과와 **73파일 / 296,016바이트**의 개별 SHA-256을 보존했다. 73개에는 Configure·Build·CTest 출력, 환경·무결성 출력, 실행 결과, 컴파일 진단, unsigned 폭 조회 소스와 출력이 포함된다. 이 안내와 manifest 자체는 그 73개에 포함하지 않는다.

서명된 임시 다운로드 주소는 저장하지 않았다. CI 파일에는 실행 당시 runner 경로와 시간이 남아 있다. 과거 기록을 현재 실행으로 바꾸거나 원본의 개행을 정규화하지 않았다.

Windows/MSVC·Direct3D·GPU·HLSL·경쟁 조건·ABI·성능·모든 입력의 커버리지는 미검증이다. 이 기록은 CPU 소스 검증이며 웹 UI·PR 병합·실제 Pages 배포 성공은 별도로 확인한다.
