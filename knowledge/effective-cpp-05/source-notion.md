<callout icon="🧬" color="blue_bg">
<b>타입을 매개변수로 받더라도 요구사항과 실패 조건은 명시해야 합니다.</b>

C++20 기준 · 개정일 <date start="2026-10-13" /> · 실습 templates·interfaces.
</callout>

<mention url="https://app.notion.com/p/8178c17f593b4f4e83a0b42544b8fe2b">Effective C++ — 55항목 개정판 v1.0</mention>

<table-of-contents color="gray" />

## 항목 41. 런타임 인터페이스와 표현식 기반 인터페이스

가상 인터페이스는 기반 타입의 함수 집합을 통해 런타임에 다른 구현을 선택할 수 있게 합니다. 템플릿은 사용하는 표현식이 타입에서 성립하는지를 바탕으로 코드를 생성하는 경우가 많습니다. 이름이 같은 함수가 있다고 의미까지 같아지는 것은 아닙니다.

C++20에서는 concept로 필요한 표현식을 코드에 드러낼 수 있습니다. 예를 들어 size()가 있고 결과가 size\_t로 변환 가능하다는 형식 조건과, 그 값이 실제 원소 수라는 의미 조건을 구분합니다. concept를 사용했다고 런타임 입력의 유효성이나 비교자의 수학적 성질까지 증명되지는 않습니다.

```c++
template<class T>
concept Sized = requires(const T& value) {
    { value.size() } -> std::convertible_to<std::size_t>;
};
```

관련 헤더와 전체 예제는 templates.cpp에 있습니다. 여러 타입을 런타임에 같은 컨테이너로 저장해야 하는지, 타입 집합이 빌드 시 고정인지에 따라 템플릿·가상 함수·타입 소거의 조합을 선택합니다.

## 항목 42. typename은 문맥에 따라 읽기

템플릿 타입 매개변수 선언의 class와 typename은 해당 문맥에서 같은 역할을 합니다. 반면 의존적인 중첩 이름이 타입이라는 사실을 알려야 하는 문맥의 typename은 다른 문제입니다. <code>typename Container::value_type</code>처럼 의존 타입 이름을 표시합니다.

모든 <code>T::...</code> 앞에 무조건 typename을 붙이는 것도 정확한 규칙이 아닙니다. 타입임이 이미 정해진 문맥, 기반 클래스 목록 등 사용 위치와 C++ 버전을 확인해야 합니다. C++20에서 타입 문맥으로 간주하는 범위가 확대된 점도 옛 예제와 다를 수 있습니다. 읽기 쉬운 using 별칭을 두면 반복되는 중첩 이름을 줄일 수 있습니다.

실습 FirstCopy는 해당 타입의 value\_type을 반환하고 빈 컨테이너는 예외로 거부합니다. 타입 문제를 해결한 뒤에도 실행 전제는 여전히 필요합니다.

## 항목 43. 의존 기반 클래스의 이름 탐색

기반 타입이 템플릿 인수에 의존하면 비의존 이름 탐색 때 그 모든 멤버를 일반 클래스처럼 확정하지 못합니다. <code>this->Read()</code>, 적절한 <code>using Base<T>::Read</code>, 명시적인 기반 한정 등이 해결 도구입니다.

```c++
template<class T> struct Base {
    int Read() const { return 42; }
};
template<class T> struct Derived : Base<T> {
    int ReadWithLog() const { return this->Read(); }
};
```

명시적인 <code>Base<T>::Read()</code>는 가상 함수에서 동적 호출을 억제할 수 있으므로 모든 해결 방법이 동작까지 같다고 가정하지 않습니다. 컴파일을 통과시키기 위해 아무 한정자를 붙이는 대신 어떤 함수가 선택되어야 하는지 정합니다. templates 실습은 위 의존 기반 호출을 실제로 검사합니다.

근거: [N4861 — 의존 이름](https://timsong-cpp.github.io/cppwp/n4861/temp.dep).

## 항목 44. 코드 중복과 일반화의 비용

템플릿 인수가 달라질 때 같은 처리 흐름이 여러 인스턴스로 생길 수 있습니다. 타입별 코드가 최적화에 유리한 경우도 있지만 실행 파일 크기·컴파일 시간·명령 캐시 비용이 늘 수 있습니다. 의미가 같은 비타입 의존 작업은 공통 함수로 분리하거나, 타입 정보가 필요 없는 부분을 좁은 비템플릿 계층으로 옮길 수 있습니다.

그렇다고 모두 void\*로 바꾸면 타입 안전성·정렬·수명·크기 검사를 호출자가 떠안게 됩니다. 타입 소거에는 함수 포인터나 가상 인터페이스 등 다양한 방식이 있으며 타입 요구사항을 잃지 않는 경계가 중요합니다. 실제 최적화가 인스턴스를 병합할 가능성도 있으므로 템플릿 수만 세어 실행 파일 크기를 확정하지 않습니다.

이 단원은 코드 크기 벤치마크를 실행하지 않았습니다. 개선을 주장할 때는 같은 기능·같은 빌드 옵션의 크기와 실행 성능, 재빌드 시간을 함께 기록해야 합니다.

## 항목 45. 멤버 템플릿과 일반 복사 연산은 다르다

스마트 포인터와 유사한 래퍼에서 Derived 포인터를 Base 포인터 래퍼로 변환하는 생성자는 두 타입을 받는 멤버 템플릿으로 표현할 수 있습니다. 하지만 모든 U<i>를 T</i>로 강제 캐스팅하도록 만들면 잘못된 계층 변환까지 허용할 수 있습니다.

```c++
template<class T> struct Borrow {
    T* pointer;
    explicit Borrow(T* p) : pointer(p) {}
    template<class U>
        requires std::convertible_to<U*, T*>
    Borrow(const Borrow<U>& other) : pointer(other.pointer) {}
};
```

Borrow는 <b>소유하지 않는 포인터 래퍼</b>를 설명하는 예입니다. 수명을 연장하거나 delete하지 않습니다. 공개·모호하지 않은 계층과 const 변환 등 허용되는 포인터 변환만 선택하도록 제약을 둡니다. 실습은 Borrow\<Cat>에서 Borrow\<const Animal>로는 가능하고 반대는 불가능함을 static\_assert로 확인합니다.

멤버 템플릿 생성자를 썼다고 그 함수가 곧 일반 복사 생성자가 되거나 암묵적 복사를 항상 대체하는 것은 아닙니다. 같은 타입 복사·대입과 서로 다른 타입 변환의 정책을 각각 정의해야 합니다.

## 항목 46. 클래스 템플릿과 숨은 friend 연산자

함수 템플릿 인수 추론 단계는 임의의 사용자 정의 변환을 적용해 모든 후보 타입을 만들어 주지 않습니다. 값 타입의 양쪽 피연산자 변환을 허용하려면 클래스 템플릿의 각 특수화에 대응하는 비템플릿 friend 연산자를 정의하는 방식이 유용할 수 있습니다.

```c++
template<class T> class Quantity {
    T value_;
public:
    Quantity(T value) : value_(value) {}
    T Value() const { return value_; }
    friend Quantity operator+(Quantity a, Quantity b) {
        return Quantity(a.value_ + b.value_);
    }
};
```

이 friend는 보통 인수 의존 탐색, ADL을 통해 발견됩니다. 실습에서는 Quantity\<int> q에 대해 q+3과 3+q가 같은 값을 만드는지 확인합니다. 작은 정수의 계산 예이며 손실·오버플로를 해결한 범용 수치 타입은 아닙니다. 단위를 강제해야 하는 타입에서는 암묵적 변환 자체를 막는 것이 더 적절할 수 있습니다.

## 항목 47. traits로 성질을 드러내되 의미를 과신하지 않기

trait는 타입과 연산의 성질을 컴파일 시점에 질의하는 인터페이스입니다. iterator\_traits, is\_integral, is\_nothrow\_move\_constructible 등으로 필요한 구현을 선택하거나 잘못된 사용을 진단할 수 있습니다. 사용자 정의 타입의 trait 특수화에는 허용된 customization 규칙을 확인해야 합니다.

C++17의 if constexpr는 템플릿에서 타입에 따라 달라지는 코드를 구성할 때 유용합니다. 선택되지 않은 가지의 모든 문법 오류가 어느 문맥에서나 무조건 사라진다는 뜻은 아닙니다. 의존성·인스턴스화와 함수 밖 일반 코드를 구분합니다.

std::move를 받을 수 있다는 trait가 참이어도 실제 전용 이동 생성자가 존재한다는 증거는 아닙니다. 이번 CopyOnly 예제처럼 const 참조 복사가 rvalue를 받을 수도 있습니다. trait 결과, 선택된 오버로드, 실제 객체 상태를 함께 봅니다.

## 항목 48. 컴파일 시점 계산은 목적이 있을 때 사용

constexpr 함수와 타입 계산, template 재귀, constexpr if 등은 컴파일 시점 검증과 상수 생성에 사용할 수 있습니다. C++20의 consteval은 즉시 함수의 호출을 상수 평가 요구와 연결하는 다른 도구입니다. constexpr라고 모든 호출이 컴파일 시점에 실행되는 것은 아닙니다.

실습 PowerOfTwo는 static\_assert에서 2의 10제곱을 확인하고, 런타임에 범위 밖 지수를 받으면 예외로 처리합니다. 재귀 템플릿으로 모든 것을 계산하기보다 일반 constexpr 함수가 읽기 쉬우면 그것을 선택합니다. 빌드 시간을 크게 늘려 런타임의 미미한 비용만 줄이는 선택인지도 살펴봅니다.

코드 생성·표현식 템플릿·정적 다형성은 별도 심화 주제입니다. 여기서는 ‘TMP는 항상 빠름’이라는 주장을 하지 않고, 결과가 언제 결정되어야 하며 어떤 오류를 더 일찍 발견하려는지 설명하는 것을 목표로 합니다.

## 실습과 질문

templates의 10조건, interfaces의 대칭 연산·뷰 계약, 제약 위반 부정 예제를 사용합니다. 환경·실행 결과: <mention url="https://app.notion.com/p/3f8a99703f408186b6f6e6196b1be1a6" />.

<toggle>
전달 참조 매개변수 value는 이름이 있으니 그냥 다음 함수로 보내도 원래 범주가 유지될까?

이름 있는 매개변수 표현식은 lvalue입니다. 추론된 범주를 보존해야 하는 문맥에서는 std::forward\<T>를 사용합니다. 즉시 소비·저장·이동 중 어떤 계약인지 먼저 결정해야 합니다.
</toggle>

<toggle>
멤버 템플릿 변환 생성자를 작성하면 같은 타입의 복사 생성자도 다 구현된 것일까?

아닙니다. 일반 복사 특수 멤버와 다른 타입에서의 변환 생성자는 별도로 보아야 합니다. 멤버의 소유권과 암묵 생성 규칙을 함께 점검합니다.
</toggle>

## 출처

[N4861 — 템플릿](https://timsong-cpp.github.io/cppwp/n4861/temp), [추론](https://timsong-cpp.github.io/cppwp/n4861/temp.deduct), [제약 조건](https://timsong-cpp.github.io/cppwp/n4861/temp.constr), [타입 변환 trait](https://timsong-cpp.github.io/cppwp/n4861/meta.trans.other), [constexpr](https://timsong-cpp.github.io/cppwp/n4861/dcl.constexpr), [Core Guidelines — 템플릿](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#S-templates).
