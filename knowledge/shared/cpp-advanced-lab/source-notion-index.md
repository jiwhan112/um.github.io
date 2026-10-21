<callout icon="📘" color="blue_bg">
<b>C++ 심화문법 — 객체 설계와 안전한 구현</b>

C++20 학습 기준 · 개정판 v1.0 · <date start="2026-10-13" />. 예전 강의 메모를 여섯 개의 읽기 문서와 공통 실습으로 재작성했습니다.
</callout>

<mention url="https://app.notion.com/p/4cf20d103df6423093adae6b54ed3157">Programmer Page</mention> · <mention url="https://app.notion.com/p/580e48caa0f642d9b6d077fbe7509d4f">쥬신 게임 프로그래밍 — 강의 목차</mention>

## 학습 순서

<page url="https://app.notion.com/p/3f8a99703f4081e19a90c9b9c7e38f7e">
01 · 클래스·불변식·생성·소멸
</page>

<span color="gray">객체가 지켜야 할 규칙, 초기화 순서, 생성 실패 정리, const·static·this·접근 제어.</span>

<page url="https://app.notion.com/p/3f8a99703f40817a8e60d1004a59c357">
02 · 복사·이동·소유권·예외 안전성
</page>

<span color="gray">Rule of Zero/Five, 깊은 복사와 실패 주입, 이동 후 상태, 자기 대입, noexcept·복사 생략.</span>

<page url="https://app.notion.com/p/3f8a99703f40811ca5d2d6c7112fe161">
03 · 상속·다형성·캐스팅·인터페이스
</page>

<span color="gray">정적·동적 타입, override·이름 가림, 기반 삭제, 슬라이싱·Clone, 캐스팅, NVI.</span>

<page url="https://app.notion.com/p/3f8a99703f4081468fe0f7cf9f60680a">
04 · 연산자·템플릿·람다·제약 조건
</page>

<span color="gray">타입 변환, 대칭 연산, concepts, 의존 이름, 전달 참조, 콜백 수명, constexpr.</span>

<page url="https://app.notion.com/p/3f8a99703f408180b127fe99defa9d16">
05 · STL·반복자·알고리즘·컨테이너 선택
</page>

<span color="gray">접근 패턴, size·capacity, 원소 소멸, 무효화, 키 조회·삽입, 비교자, 측정 기준.</span>

<page url="https://app.notion.com/p/3f8a99703f408186b6f6e6196b1be1a6">
06 · 빌드 경계·Pimpl·할당·검증 실습
</page>

<span color="gray">헤더 의존성, 실제 h/cpp 빌드, Pimpl, 위치 지정 생성, pmr, 전체 테스트 기록.</span>

## 이전·다음 문서

기초 개념이 불명확하면 <mention url="https://app.notion.com/p/2275f5f2c81a404e9c908dc0296ed65e" />을 먼저 봅니다. 설계 판단을 항목별로 복습하려면 <mention url="https://app.notion.com/p/8178c17f593b4f4e83a0b42544b8fe2b" />로 이어집니다. 두 문서는 같은 실습을 서로 다른 목적에서 참조하며 코드를 이중으로 관리하지 않습니다.

## 실습과 실제 검증

<file source="notion-file-block://73d8331f-f637-4b17-9d72-49af281d846a/145d187d-b1de-4dbd-9fb8-df133567229d?space_id=815d1335-3b56-43a7-903b-d7addf097c9f&name=cpp_advanced_effective_validation_v1.md">
공통 실습 검증 기록 · 9개 실행 파일과 6개 컴파일 거부 예제
</file>

전체 소스·CMake·README·실행 스크립트·원시 로그는 이 개정 작업의 대화 첨부 <code>cpp_advanced_effective_lab_v1.zip</code>에 있습니다. 파일별 텍스트 묶음 <code>cpp_advanced_effective_sources_v1.md</code>도 대화에 제공합니다. 위 Notion 첨부는 검증 기록과 부정 예제이며 전체 ZIP을 대체하지 않습니다. ZIP 직접 전송은 실행 환경의 업로드 호스트 이름 해석 실패로 완료하지 못했습니다.

<callout icon="✅" color="green_bg">
<b>실행 확인: 정상 예제 9/9, 조건 검사 89회</b>

GCC 14.2.0, Clang 17.0.0, Clang ASan·UBSan의 C++20 빌드에서 각각 통과했습니다. 부정 예제 6개는 두 컴파일러가 각각 컴파일 단계에서 거부했습니다.

Linux x86-64 결과입니다. Windows/MSVC·GPU·회사 프로젝트·경쟁 조건·성능은 실행 미검증입니다. 같은 조건의 반복 실행과 전체 프로그램의 완전한 검증을 구분합니다.
</callout>

## 이번에 바꾼 중요한 설명

<table fit-page-width="true" header-row="true">
<tr>
<td>
기존 메모
</td>
<td>
개정 방향
</td>
</tr>
<tr>
<td>
절차 지향은 함수 없이 한 파일
</td>
<td>
프로그램 조직 방식과 함수·파일 개수를 분리
</td>
</tr>
<tr>
<td>
struct와 class의 기본 멤버 접근만 비교
</td>
<td>
기본 상속 접근과 설계 관례도 구분
</td>
</tr>
<tr>
<td>
생성자와 new·초기화와 대입을 혼동
</td>
<td>
저장 공간·초기화·수명을 단계별 설명
</td>
</tr>
<tr>
<td>
복사 생성자 값 인수는 실행 중 무한 재귀
</td>
<td>
해당 선언의 컴파일 오류로 교정하고 부정 예제 추가
</td>
</tr>
<tr>
<td>
기본 복사·가상 함수·추상 타입의 원리를 단정
</td>
<td>
특수 멤버 조건, 정적·동적 타입, 언어 규칙과 ABI 구분
</td>
</tr>
<tr>
<td>
기반 삭제를 배열 delete로 해결
</td>
<td>
삭제 경로와 virtual 소멸자·소유권 계약으로 교정
</td>
</tr>
<tr>
<td>
dynamic\_cast는 항상 nullptr로 실패
</td>
<td>
포인터 실패·참조 실패·수명 전제 구분
</td>
</tr>
<tr>
<td>
템플릿은 예외 없이 헤더에만 구현
</td>
<td>
암묵적·명시적 인스턴스화와 지원 타입 범위 설명
</td>
</tr>
<tr>
<td>
vector 성장률 고정·clear는 메모리 삭제 안 함
</td>
<td>
구현 정책·원소 파괴·저장 용량·대상 소유를 분리
</td>
</tr>
<tr>
<td>
컨테이너 선택·emplace·inline의 무조건적 성능 주장
</td>
<td>
전제·비용·무효화·측정 결과 중심으로 변경
</td>
</tr>
</table>

## 보강한 범위와 표준 버전

불변식, 생성 도중 실패, Rule of Zero/Five, 명시적 이동 계약, 값 반환과 복사 생략, noexcept, 강한 예외 보장, NVI·Clone, 비소유 뷰 수명, concepts·forwarding, pmr·할당 실패 주입을 보강했습니다. 새 전문 용어는 본문에서 정의하고 실습과 연결합니다.

C++11의 unique\_ptr·이동·override, C++14의 make\_unique, C++17의 string\_view·pmr·if constexpr, C++20의 concepts·ranges·erase\_if 등을 구분합니다. 회사 프로젝트의 표준을 바꾸라는 지시가 아닙니다.

## 원본과 검토 범위

<toggle>
개정 전 강의 메모·이미지 보기

<mention url="https://app.notion.com/p/3f7a99703f4081b2b572e154c7c3e9c8" />

원본 복제본은 보관함에 있습니다. 원문 텍스트와 주제를 검토한 뒤 새 설명·실습을 작성했습니다. 기존 이미지 속 모든 코드를 개별 전사·실행한 것은 아닙니다.
</toggle>

언어 규칙은 C++20 공개 초안 N4861, 설계 지침은 C++ Core Guidelines, 진단은 Clang 공식 문서 등을 관련 절에 연결했습니다. Context7 조회 결과도 공식 원문과 대조했습니다. 설계 지침을 언어 표준과 동일하게 취급하지 않습니다.

취업·면접 자료와 게임 엔진 아키텍처 책은 이번 변경에서 제외했습니다. 아키텍처는 별도 요청 시 1장부터 진행합니다.
