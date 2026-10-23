<callout icon="📚" color="blue_bg">
<b>Effective C++ — 55개 항목을 현재 코드의 판단 기준으로 다시 읽기</b>

개정판 v1.0 · C++20 학습 기준 · <date start="2026-10-13" />. 기존 개인 독서 메모를 바탕으로 오류를 교정하고 표준 버전 차이·수명·실행 예제를 보완한 독립 해설입니다. 책 원문의 전재나 번역본은 아닙니다.
</callout>

<mention url="https://app.notion.com/p/4cf20d103df6423093adae6b54ed3157">Programmer Page</mention> · 선수 문서 <mention url="https://app.notion.com/p/aaa55bced8ff459ba67fb036fb34bb3b" />

## 항목별로 읽기

<page url="https://app.notion.com/p/3f8a99703f40816abc03e0fe7daf6deb">
01 · 항목 1–12 — 언어·초기화·특수 멤버
</page>

<span color="gray">패러다임, 상수·const, 초기화, 암묵 함수, 복사 금지, 소멸자, 자기 대입.</span>

<page url="https://app.notion.com/p/3f8a99703f408149b3edc2e5729e4106">
02 · 항목 13–25 — 자원·API·복사·swap
</page>

<span color="gray">RAII, 소유 정책, 핸들 접근, 인수 평가, 오용 방지 API, 값·참조 반환, ADL swap.</span>

<page url="https://app.notion.com/p/3f8a99703f4081978c8ac4ae481f0b31">
03 · 항목 26–31 — 구현·예외·컴파일 의존성
</page>

<span color="gray">정의 위치, 캐스팅, 비소유 뷰, 강한 예외 보장, inline, Pimpl.</span>

<page url="https://app.notion.com/p/3f8a99703f408165ab36f79f8376fba3">
04 · 항목 32–40 — 상속과 설계 대안
</page>

<span color="gray">대체 가능성, 이름 가림, 인터페이스·구현, NVI·Strategy, 기본 인수, 합성·다중 상속.</span>

<page url="https://app.notion.com/p/3f8a99703f4081a3993ae737187e5928">
05 · 항목 41–48 — 제네릭 프로그래밍과 메타프로그래밍
</page>

<span color="gray">concepts, typename, 의존 이름, 코드 크기, 멤버 템플릿, 숨은 friend, traits, constexpr.</span>

<page url="https://app.notion.com/p/3f8a99703f4081798361fdbf4881b8c6">
06 · 항목 49–55 — 할당·진단·표준 라이브러리
</page>

<span color="gray">new\_handler, 사용자 할당, 위치 지정 delete, 테스트, TR1 시대 기능의 현재 위치, Boost 도입 판단.</span>

## 이번 판을 읽는 기준

규칙을 암기하는 대신 <b>어떤 문제를 막는지 → 어떤 조건에서 맞는지 → 현재 코드의 대안은 무엇인지</b>를 연결합니다. 55개 항목 번호를 빠짐없이 유지했으며, 각 본문에 개념 설명·적용 조건·관련 실습과 출처를 넣었습니다. ‘항상’, ‘절대’, ‘무조건 빠르다’처럼 원문 메모에서 과하게 일반화한 부분은 조건을 분리했습니다.

책의 조언, C++ 언어 규칙, 특정 컴파일러의 구현 관찰은 다른 종류의 근거입니다. 새로운 표준 문법이 있다고 예전 프로젝트를 자동으로 바꾸지 않습니다. 이 묶음의 C++20 예제를 C++11 회사 코드에 그대로 적용하지 말고 필요한 기능의 도입 버전과 환경을 확인합니다.

## 대표적인 교정·현대화

<table fit-page-width="true" header-row="true">
<tr>
<td>
항목
</td>
<td>
이번 개정의 핵심
</td>
</tr>
<tr>
<td>
2–4
</td>
<td>
매크로·const의 사본 수를 단정하지 않음. const iterator의 역참조, 초기화 순서·정적 수명 분리
</td>
</tr>
<tr>
<td>
5–12
</td>
<td>
암묵 특수 멤버 조건, =delete, Rule of Zero/Five, 가상 소멸자, 생성 중 가상 호출, 실패 안전 복사
</td>
</tr>
<tr>
<td>
13–17
</td>
<td>
auto\_ptr 대신 현대 소유 타입. get·release 구분. C++17 이후 인수 평가 변화 명시
</td>
</tr>
<tr>
<td>
18–25
</td>
<td>
항상 const 참조 대신 의미·복사 비용·수명 비교. 값 반환, 뷰 수명, ADL swap과 제약
</td>
</tr>
<tr>
<td>
27–31
</td>
<td>
캐스트의 전제, 예외 안전성과 동시성 분리, Pimpl과 copy-and-swap 구분, inline과 최적화 구분
</td>
</tr>
<tr>
<td>
32–40
</td>
<td>
상속 계약·합성, 이름 가림, 기본 인수의 정적 선택, 다중 상속 초기화 책임
</td>
</tr>
<tr>
<td>
41–48
</td>
<td>
concepts·if constexpr·forwarding 보완. 멤버 템플릿은 일반 복사 연산과 구분
</td>
</tr>
<tr>
<td>
49–52
</td>
<td>
할당 실패와 생성 실패, 정렬·짝 함수·정상 해제, 실제 메모리 고갈 없는 실험
</td>
</tr>
<tr>
<td>
53–55
</td>
<td>
실행 검증 범위 표시. TR1·제거 기능과 현재 표준 도구 구분. 필요한 라이브러리만 도입 검토
</td>
</tr>
</table>

## 실습은 C++ 심화와 공유합니다

<mention url="https://app.notion.com/p/3f8a99703f408186b6f6e6196b1be1a6" />

기초 의미는 C++ 심화에서 자세히 배우고 이 문서에서는 설계 판단과 과거 조언의 적용 조건을 복습합니다. 전체 코드를 다른 버전으로 복제하지 않습니다. 공통 ZIP <code>cpp_advanced_effective_lab_v1.zip</code>과 파일별 전체 소스 텍스트는 이 작업의 대화 첨부로 제공합니다. 검증 기록은 C++ 심화 상위 페이지에 Notion 첨부로 보존했습니다. 실행 환경의 업로드 제한 때문에 ZIP 자체의 Notion 전송은 완료하지 못했습니다.

<callout icon="✅" color="green_bg">
<b>정상 실행 9개 · 조건 검사 89회 · 컴파일 거부 예제 6개</b>

GCC 14.2, Clang 17, Clang ASan·UBSan의 정상 구성에서 각각 통과했습니다. 부정 예제는 GCC·Clang 모두 -c 단계에서 거부되었습니다. 55개 항목 각각에 독립 실행 프로그램 55개를 만들었다는 뜻은 아닙니다.

Windows/MSVC·DirectX·회사 프로젝트·멀티스레드 경쟁·성능은 실행 미검증입니다.
</callout>

## 원본과 개정 범위

<toggle>
기존 독서 메모와 이미지 보관본

<mention url="https://app.notion.com/p/3f7a99703f4081e3beeaf3f8ab03090b" />

기존 페이지의 복제본은 보관함에 남겼습니다. 과거의 문맥과 이미지 확인용이며 교정 전 내용도 포함합니다. 이미지 속 모든 예제를 그대로 전사해 실행한 것은 아닙니다.
</toggle>

각 절의 언어 기준은 공개 C++20 초안 N4861과 관련 공식 자료입니다. Context7의 C++ Core Guidelines 조회는 보조로 사용했고, 반환된 오래된 예시도 공식 원문과 대조했습니다. 무조건 최신이거나 정확하다고 간주하지 않았습니다.

이번 작업은 취업·면접 기록과 게임 엔진 아키텍처 책을 수정하지 않았습니다. 실행 계획의 다음 분야는 게임 수학·Windows·DirectX이며 아키텍처 책은 별도 지시 전까지 보류합니다.
