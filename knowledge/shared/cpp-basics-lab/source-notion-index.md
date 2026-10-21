<callout icon="📘" color="blue_bg">
<b>C++ 기초문법 — 개정판 v1.0</b>

예전 수동 메모를 바탕으로 설명·예제·오류 교정·실습을 다시 구성했습니다. 읽기 순서는 01\~05이며 필요한 주제부터 찾아봐도 됩니다. 기준: C++20 · 개정일 <date start="2026-10-12" />.
</callout>

<mention url="https://app.notion.com/p/4cf20d103df6423093adae6b54ed3157">Programmer Page</mention> · <mention url="https://app.notion.com/p/580e48caa0f642d9b6d077fbe7509d4f">쥬신 게임 프로그래밍 — 강의 목차</mention>

## 읽는 순서

<page url="https://app.notion.com/p/3f7a99703f4081ea8b09f52fe80babc2">
01 · 빌드·타입·초기화·표현식
</page>

<span color="gray">선언·정의와 빌드, 정수·실수·진수, 초기화, const·constexpr·auto, 평가 순서와 비트 연산.</span>

<page url="https://app.notion.com/p/3f7a99703f408140a211e4afe6030e31">
02 · 제어 흐름·함수·콜백
</page>

<span color="gray">분기·반복, 입력 실패, 함수 계약·전달 방식, 오버로딩·기본 인수, 재귀·범위 검사, 함수 포인터·람다·매크로.</span>

<page url="https://app.notion.com/p/3f7a99703f40810599abdb7dfdea1598">
03 · 포인터·참조·수명·소유권
</page>

<span color="gray">주소와 대상 값, 이중 포인터, const, 저장 기간, 동적 객체·RAII, 경계·캐스팅·바이트 해석. 관계와 수명을 다이어그램으로 설명합니다.</span>

<page url="https://app.notion.com/p/3f7a99703f4081979bf0c9aa376a5774">
04 · 배열·문자열·사용자 타입·표준 도구
</page>

<span color="gray">array·vector·span, size·capacity·무효화, 다차원 배열, 동적 배열 학습 구현, 문자열·인코딩, struct·enum·union·variant, 정렬·난수.</span>

<page url="https://app.notion.com/p/3f7a99703f4081edaa44c74d0430d2cc">
05 · 파일·분할 빌드·디버깅·통합 실습
</page>

<span color="gray">현재 작업 디렉터리, 파일 오류·직렬화, 인벤토리 저장/로드, 헤더·ODR·링크, 디버깅, 빌드 방법과 실제 검증 기록.</span>

---

## 실습 코드

<file source="notion-file-block://5fb8e4e7-6ccb-4edd-889f-bba260b4392b/2d74b3e5-ff5f-4dd0-9fff-4d3559528daf?space_id=815d1335-3b56-43a7-903b-d7addf097c9f&name=cpp_basics_lab_sources_v1.md">
C++ 기초문법 실습 코드 전체 · 파일별 소스와 빌드 방법
</file>

첨부 Markdown에는 <code>CMakeLists.txt</code>, 헤더, 정상 실행 예제 6개, 컴파일 거부 예제 4개, 위험 동작 진단 예제 2개의 코드가 파일별로 들어 있습니다. 바로 빌드할 수 있게 나눈 <code>cpp_basics_lab_v1.zip</code>은 이 개정 작업의 대화에 제공했습니다. ZIP의 노션 직접 업로드는 실행 환경의 네트워크 제한으로 실패하여, 노션에는 코드 원본을 텍스트 첨부로 보존합니다. ZIP에 있는 원시 검증 로그의 요약은 05 문서에도 기록했습니다.

<callout icon="✅" color="green_bg">
<b>실제 수행한 검증</b>

GCC 14.2.0 / Clang 17.0.0의 C++20 빌드에서 정상 예제 각각 6/6 통과. Clang ASan·UBSan 정상 빌드도 6/6 통과. 컴파일 거부 예제 4개는 두 컴파일러 모두 거부, 위험 실험 2개는 예상한 진단을 확인했습니다.

환경은 Linux x86-64입니다. Windows/MSVC, DirectX·GPU, 회사 프로젝트 통합은 실행 미검증입니다. 정상 코드의 221회 조건 검사에는 루프 원소 검사도 포함되며 완전한 커버리지를 의미하지 않습니다.
</callout>

## 이번에 바로잡은 대표 내용

<table fit-page-width="true" header-row="true">
<tr>
<td>
기존 메모의 문제
</td>
<td>
개정 내용
</td>
</tr>
<tr>
<td>
이진수 00000010을 3으로 표기, XOR 결과 주석 오류
</td>
<td>
각각 2와 7로 교정하고 진수 표기·계산을 실행 검사
</td>
</tr>
<tr>
<td>
Int·True·False, 초기화와 대입 혼동
</td>
<td>
실제 C++ 키워드와 객체 생성 시 초기화·이후 대입을 구분
</td>
</tr>
<tr>
<td>
모든 타입 크기와 메모리 배치를 고정으로 설명
</td>
<td>
언어의 보장·MSVC 특성·이번 Linux 관찰값을 분리
</td>
</tr>
<tr>
<td>
모든 정수 오버플로가 같은 방식으로 돌아간다는 설명
</td>
<td>
부호 없는 연산, 부호 있는 오버플로, 정수 승격을 구분
</td>
</tr>
<tr>
<td>
후위 증가는 무조건 마지막에 수행
</td>
<td>
우선순위·값 계산·부수 효과와 순서 규칙을 분리
</td>
</tr>
<tr>
<td>
scanf의 입력 대기를 무한 루프로 설명
</td>
<td>
입력 대기·실패 상태·반환값과 CPU를 쓰는 반복을 구분
</td>
</tr>
<tr>
<td>
함수 기본 인수는 정의에 쓸 수 없음
</td>
<td>
호출자에게 보이는 선언과 중복 기본값 지정의 문제로 교정
</td>
</tr>
<tr>
<td>
포인터 값과 대상 값, 배열과 포인터 혼동
</td>
<td>
p·<i>p·&p·T</i>\*와 배열의 변환·경계를 예제로 분리
</td>
</tr>
<tr>
<td>
const 객체를 캐스팅해 수정해도 된다는 설명
</td>
<td>
원래 const인 객체의 수정은 미정의 동작임을 명시
</td>
</tr>
<tr>
<td>
참조는 무조건 메모리를 쓰지 않음
</td>
<td>
참조의 의미와 구현 저장 비용을 구분
</td>
</tr>
<tr>
<td>
해제 후 별칭 포인터·재할당의 위험 설명 부족
</td>
<td>
수명·소유권·RAII·vector 무효화와 진단 실습 추가
</td>
</tr>
<tr>
<td>
wchar\_t 크기, 문자 수, C 문자열 비교 결과 일반화
</td>
<td>
인코딩·코드 단위·플랫폼 차이·비교 결과의 부호를 구분
</td>
</tr>
<tr>
<td>
구조체 원시 저장을 일반적인 바이너리 저장으로 설명
</td>
<td>
필드 기반 포맷·버전·경계·입력 실패를 포함한 예제로 교체
</td>
</tr>
<tr>
<td>
상대 경로를 cpp 또는 exe 위치로 고정
</td>
<td>
현재 작업 디렉터리 기준으로 교정하고 실제 경로 출력 추가
</td>
</tr>
<tr>
<td>
매크로·포인터 콜백의 무조건적인 성능 장점
</td>
<td>
적용 조건·부수 효과·타입 안전성과 대안을 설명
</td>
</tr>
</table>

## 새로 보강한 학습 범위

초기화와 축소 변환, const와 constexpr의 차이, 값 범주를 변경 가능성과 혼동하지 않는 기준, 참조·포인터의 수명, std::unique\_ptr, std::array·vector·span의 소유권, 컨테이너 무효화, 문자열 뷰·내부 널·인코딩, variant, 정렬 비교자의 조건, 난수 엔진과 분포, 파일 검증·부분 로드 차단, 자동 테스트와 sanitizer 사용을 보완했습니다.

기존 노트에 있던 동적 배열·함수 포인터·파일 복사·분할 구현은 없애는 대신 새 개념과 연결했습니다. 동적 배열은 int 전용 학습 구현으로, 파일 실습은 인벤토리 저장/로드와 파일 복사로 재구성했습니다. 모든 기존 스크린샷 코드를 그대로 전사한 것은 아닙니다.

## C++11 프로젝트와 함께 보는 방법

<table fit-page-width="true" header-row="true">
<tr>
<td>
도입 기준
</td>
<td>
이 문서에서 사용하는 예
</td>
</tr>
<tr>
<td>
C++11
</td>
<td>
nullptr, enum class, using 별칭, 범위 기반 for, unique\_ptr, random 라이브러리
</td>
</tr>
<tr>
<td>
C++14
</td>
<td>
make\_unique, 0b 이진수 리터럴
</td>
</tr>
<tr>
<td>
C++17
</td>
<td>
optional, variant, string\_view, filesystem, inline 변수
</td>
</tr>
<tr>
<td>
C++20
</td>
<td>
span, bit\_cast, char8\_t 기반 UTF-8 리터럴 규칙
</td>
</tr>
</table>

실습 팩은 C++20으로 고정되어 있습니다. 기존 C++11 게임 코드에 그대로 복사하지 말고 사용 가능한 표준·라이브러리·환경을 확인하세요. 표준 도구의 사용 이유와 소유권 원칙은 유지하되 필요한 호환 구현은 별도 검증해야 합니다.

## 원본·출처·변경 범위

<toggle>
개정 전 원문과 기존 이미지 보기

<mention url="https://app.notion.com/p/3f7a99703f4081478c57ca509b8fc0d3" />

원문과 기존 코드 이미지의 복제본을 보관함으로 옮겼습니다. 이 원본에는 교정 전 설명도 있으므로 현재 학습의 기준은 위 01\~05 문서입니다. 복제본은 내용 비교와 과거 맥락 확인용입니다.
</toggle>

출처는 각 문서의 관련 절에 배치했습니다. C++20 규칙은 공개 초안 N4861의 해당 절을, MSVC·Windows 특성은 Microsoft 공식 문서를, 검사 도구는 Clang 공식 문서를 기준으로 확인했습니다. 가이드라인은 언어 표준과 구분합니다.

<b>연결 도구 상태:</b> Context7과 tldraw의 설치는 확인했지만 이번 대화의 호출 도구 목록에는 제공되지 않아 실제로 사용하지 못했습니다. 문서 검증은 공식 자료와 로컬 컴파일·실행으로 수행했고, 그림은 노션의 Mermaid 코드 블록으로 작성했습니다. tldraw 캔버스가 생성되거나 동기화된 것은 아닙니다.

<b>검토 한계:</b> 기존 페이지의 반환된 텍스트와 구조를 읽었으며, 기존 이미지 속 코드를 모두 개별 판독·전사·실행한 것은 아닙니다. 이번 새 실습 코드에 대한 실제 검증과 기존 자료 전체의 검증은 구분합니다. 다이어그램은 의미 설명용이며 실제 주소 측정이나 엔진 프로파일 결과가 아닙니다.

<b>이번 변경 대상은 이 C++ 기초문법 묶음입니다.</b> C++ 심화 등 다른 학습 본문, 취업·면접 기록, 아키텍처 책의 기존 문서는 수정하지 않았습니다. 아키텍처는 별도 요청 시 1장부터 진행합니다.
