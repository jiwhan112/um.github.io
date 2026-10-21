<callout icon="🧰" color="blue_bg">
<b>데이터의 저장 방식과, 그 데이터를 바라보는 방법을 구분합니다.</b>

배열·vector·span·문자열은 비슷한 문법을 사용해도 소유권과 크기 변경 규칙이 다릅니다. 예전 직접 구현의 학습 가치는 남기고, 실제 데이터를 관리할 때 표준 도구를 선택하는 기준을 추가합니다.
</callout>

<mention url="https://app.notion.com/p/2275f5f2c81a404e9c908dc0296ed65e">C++ 기초문법 — 개정판 v1.0</mention> · 실습 파일: <code>src/containers.cpp</code>, <code>src/buffer.cpp</code> · 기준 C++20

<table-of-contents color="gray" />

## 1. 배열은 포인터가 아닙니다

<code>int values[3]{10, 20, 30};</code>는 int 객체 세 개를 가진 배열입니다. 많은 표현식에서 첫 원소를 가리키는 포인터로 변환되지만 배열의 타입 자체가 포인터가 되는 것은 아닙니다. <code>sizeof(values)</code>는 배열 전체 크기이고, 원소 수는 전체 크기를 원소 크기로 나누거나 std::size를 사용해 구할 수 있습니다.

반면 함수 매개변수에 <code>void F(int values[3])</code>라고 적으면 그 매개변수는 int\*로 조정됩니다. 3개인지 검사해 주거나 크기를 보존하는 인터페이스가 아닙니다. 그 함수 안에서 sizeof(values)로 원본 배열 길이를 얻을 수 없습니다.

배열 인덱스는 0부터 크기-1까지입니다. 범위를 벗어나도 예외가 항상 발생하지 않습니다. 잘못된 값을 읽거나 다른 메모리를 건드리는 것처럼 보여도 이미 프로그램이 보장 밖으로 나간 것입니다. <code>int values[count];</code>처럼 실행 중 정해지는 크기를 쓰는 가변 길이 배열은 표준 C++ 기능이 아니므로 vector 등의 명시적 대안을 사용합니다.

근거: [Microsoft — 배열과 다차원 배열](https://learn.microsoft.com/en-us/cpp/cpp/arrays-cpp?view=msvc-170).

## 2. 크기와 소유권으로 도구를 선택합니다

<table fit-page-width="true" header-row="true">
<tr>
<td>
도구
</td>
<td>
소유
</td>
<td>
크기
</td>
<td>
용도
</td>
</tr>
<tr>
<td>
내장 배열 T\[N\]
</td>
<td>
원소가 배열의 일부
</td>
<td>
고정
</td>
<td>
기존 API·메모리 배치 학습
</td>
</tr>
<tr>
<td>
<code>std::array<T, N></code>
</td>
<td>
원소를 직접 보유
</td>
<td>
고정
</td>
<td>
고정 개수 슬롯·좌표 집합
</td>
</tr>
<tr>
<td>
<code>std::vector<T></code>
</td>
<td>
원소 저장 공간 관리
</td>
<td>
변경 가능
</td>
<td>
런타임에 개수가 달라지는 연속 데이터
</td>
</tr>
<tr>
<td>
<code>std::span<T></code>
</td>
<td>
소유하지 않음
</td>
<td>
관찰할 범위
</td>
<td>
이미 존재하는 연속 원소를 함수에 전달
</td>
</tr>
</table>

span은 C++20의 비소유 뷰입니다. 원본이 소멸하거나 재할당되면 span도 안전하게 쓸 수 없게 될 수 있습니다. 크기를 전달한다고 자동으로 모든 인덱스가 검사되는 것은 아닙니다. 특히 이 문서의 C++20 std:\:span\::operator\[\]는 범위 검사를 보장하는 접근 함수가 아닙니다.

```c++
#include <array>
#include <span>

int Sum(std::span<const int> values) {
    int result{};
    for (int value : values) result += value;
    return result;
}
// 호출 예: std::array<int, 3> values{10, 20, 30}; Sum(values);
```

이 Sum은 합계가 int에 들어가는 작은 예제입니다. 대량 점수나 외부 입력을 누적한다면 결과 범위 정책을 추가해야 합니다. 이 경계 조건을 적는 것이 ‘실행된다’와 ‘어떤 입력에 안전한가’의 차이입니다.

근거: [N4861 — span](https://timsong-cpp.github.io/cppwp/n4861/views.span).

## 3. vector의 size와 capacity

size는 현재 원소 수이고, capacity는 재할당 없이 담을 수 있는 용량입니다. <code>reserve(n)</code>은 용량을 준비하며 원소 수를 늘리지 않습니다. <code>resize(n)</code>은 원소 수를 변경합니다. 기본 할당자를 사용하는 vector\<int>에서 resize로 추가한 정수는 0으로 초기화됩니다.

```c++
std::vector<int> scores;
scores.reserve(4);
// scores[0] = 10;  // 금지: size는 여전히 0
scores.resize(3);
scores[1] = 8;
```

push\_back은 맨 끝에 원소를 추가합니다. 재할당이 없는 경우와 있는 경우의 비용이 다르며, 연속된 추가 작업을 고려한 분할 상환 상수 시간과 특정 한 번의 선형 시간 재할당을 구분합니다. 표준이 모든 구현에 정확히 두 배의 성장률을 강제하지는 않습니다. reserve를 매번 size+1로 반복하면 오히려 적절한 성장 전략을 방해할 수 있습니다.

근거: [N4861 — vector 용량과 resize](https://timsong-cpp.github.io/cppwp/n4861/vector.capacity).

### 포인터와 반복자 무효화

재할당되면 기존 원소를 가리키던 포인터·참조·반복자는 무효화됩니다. 기존 주소가 널이 아니어도 사용할 수 없습니다. 재할당이 없더라도 삽입·삭제 위치에 따라 일부 반복자·참조와 end가 무효화될 수 있습니다.

```mermaid
flowchart LR
    A["vector가 소유한 기존 저장 공간"] -->|"새 용량 필요"| B["새 저장 공간 확보·원소 이전"]
    B --> C["기존 저장 공간 해제"]
    P["기존 원소를 가리키던 포인터"] -. "사용 중단" .-> A
    B --> D["새 위치에서 다시 접근"]
```

인벤토리 원소의 포인터를 UI나 이벤트에 오래 저장해 두고 vector를 늘리는 설계는 위험할 수 있습니다. 인덱스나 ID를 사용하더라도 삭제·정렬 후 의미가 바뀌는 문제를 따로 해결해야 합니다. 어떤 참조 방식도 아무 조건 없이 영구적으로 안정적이지 않습니다.

vector::at는 범위 밖에서 out\_of\_range를 던집니다. operator\[\]는 그 검사를 보장하지 않습니다. 외부에서 받은 인덱스는 범위를 검증하거나 요구되는 오류 정책에 맞게 at를 사용하세요.

근거: [N4861 — vector 삽입·삭제·무효화](https://timsong-cpp.github.io/cppwp/n4861/vector.modifiers).

## 4. 다차원 배열과 평탄한 데이터

```c++
int grid[2][3]{{1, 2, 3}, {4, 5, 6}};
int (*row)[3] = grid;
int value = row[1][2];  // 6
```

grid는 ‘int 세 개짜리 행 두 개’의 배열입니다. 첫 원소는 int 하나가 아니라 행 배열이므로 변환 결과는 int (<i>)\[3\]입니다. int</i>\*는 포인터를 원소로 가진 구조를 가리키는 타입이어서 같은 것이 아닙니다. 강제 캐스팅으로 메모리 구조 차이를 지울 수 없습니다.

런타임 크기의 2차원 데이터를 연속 vector에 저장하면 <code>(y * width) + x</code>로 접근할 수 있습니다. x와 y가 각각 경계 안인지, width\*height가 size\_t의 범위를 넘지 않는지, 총 원소 수가 메모리 예산 안인지 먼저 확인합니다. vector\<vector\<T>>는 행별로 별도 저장 공간을 가질 수 있으므로 하나의 평탄한 배열과 같은 연속성을 가정하지 않습니다.

공간의 의미도 중요합니다. <code>grid[y][x]</code>는 개발자가 선택한 규약입니다. 행렬 수학의 행·열, 텍스처 좌표의 방향, 화면 y축 방향을 섞지 말고 입력·저장·출력의 기준을 문서화합니다.

## 5. 직접 만드는 동적 배열: 원리를 보기 위한 제한된 실습

기존 노트의 수동 동적 배열은 크기가 부족하면 새 공간을 확보하고 기존 원소를 복사한 뒤 이전 공간을 해제하는 원리를 보여 줍니다. 이를 <code>IntBuffer</code>라는 <b>int 전용 학습 구현</b>으로 다시 작성했습니다. 실제 프로젝트에 std::vector 대신 넣으라는 뜻이 아닙니다.

상태는 <code>data_</code>, <code>size_</code>, <code>capacity_</code>입니다. PushBack에서 size가 capacity와 같으면 용량 오버플로를 검사하고 새 배열을 먼저 준비합니다. 복사가 끝난 뒤 소유권을 바꿔 이전 저장 공간을 해제합니다. At는 인덱스가 size 미만인지 검사합니다.

실습 <code>src/buffer.cpp</code>의 핵심 부분입니다.

```c++
if (size_ == capacity_) {
    constexpr auto limit = std::numeric_limits<std::size_t>::max() / sizeof(int);
    if (capacity_ > limit / 2)
        throw std::length_error("capacity overflow");
    const auto next_capacity = capacity_ == 0 ? std::size_t{2} : capacity_ * 2;
    auto next = std::make_unique<int[]>(next_capacity);
    if (size_ != 0) std::copy_n(data_.get(), size_, next.get());
    data_ = std::move(next);
    capacity_ = next_capacity;
}
data_[size_++] = value;
```

할당이 실패하기 전에 기존 저장 공간을 버리지 않도록 순서를 정한 것입니다. 이 예제는 int 복사에만 한정되어 있으며 일반 T의 생성·소멸·이동 예외, allocator, 복잡한 반복자 규칙까지 구현한 컨테이너가 아닙니다. 복사·이동을 delete하여 부분적인 소유권 설계가 안전한 범용 구현처럼 보이지 않게 했습니다. 100개 추가 시 용량 128은 <b>이 구현이 선택한 성장 정책의 결과</b>입니다.

## 6. 문자·코드 단위·문자열

char는 1 C++ 바이트를 저장하는 문자 타입입니다. 하나의 char가 모든 한국어 글자를 담는다는 뜻은 아닙니다. 문자열 인코딩은 코드 단위들의 배열을 실제 문자로 해석하는 규칙입니다.

MSVC의 wchar\_t는 2바이트이지만 다른 구현은 다를 수 있습니다. Windows의 UTF-16 문자열에서도 보조 평면 문자는 두 코드 단위를 사용할 수 있습니다. std::string에 UTF-8을 저장하더라도 size는 char 원소 수이며 사용자가 보는 글자 수나 글자소 수가 아닙니다. 화면에서 한 글자를 지우는 기능을 단순히 바이트 하나 제거로 만들면 문제가 생깁니다.

C++20의 u8 문자열 리터럴은 char8\_t 기반이므로 기존 char API에 자동으로 전달된다고 가정하지 않습니다. 인코딩 변환·검증과 바이트 전달의 의도를 구분해야 합니다.

근거: [Microsoft — 문자열·문자 리터럴](https://learn.microsoft.com/en-us/cpp/cpp/string-and-character-literals-cpp?view=msvc-170), [Microsoft — 타입 범위](https://learn.microsoft.com/en-us/cpp/cpp/data-type-ranges?view=msvc-170).

### C 문자열과 std::string

```c++
char editable[]{"abc"};       // 배열에 복사된 문자와 끝의 '\0'
const char* literal = "abc";
literal = "def";              // 포인터 재지정은 가능
std::string with_zero{"a\0b", 3};
```

문자열 리터럴의 내용을 바꾸면 안 됩니다. 그러나 const char<i>의 포인터 변수</i>\* 자체는 const가 아니므로 다른 문자열을 가리키게 할 수 있습니다. <code>char text[3] = "abc";</code>는 끝의 널 문자까지 담을 공간이 부족한 잘못된 초기화입니다.

std::string은 길이를 별도로 관리하고 내부에 널 문자를 포함할 수도 있습니다. 위 with\_zero의 size는 3이지만 <code>std::strlen(with_zero.c_str())</code>는 첫 널까지 세어서 1입니다. c\_str의 결과는 const char\*이며 별도 소유권이 없습니다. 원래 문자열의 변경·소멸로 사용 가능 조건이 깨질 수 있으므로 장기 저장하지 않습니다.

문자열 내용 비교에는 std::string의 ==나 적절한 문자열 비교 함수를 씁니다. C 문자열 포인터끼리 ==를 하면 내용이 아니라 주소 비교가 됩니다. strcmp·wcscmp의 결과는 음수·0·양수로 구분하며 정확히 -1이나 +1이라고 보장하지 않습니다.

직접 길이·복사·연결 함수를 학습할 때에는 입력이 널 종료인지, 대상 용량에 종료 문자가 들어갈 여유가 있는지, 겹치는 범위를 지원하는지 계약을 적습니다. 읽을 수 있는 범위가 불명확한 포인터에서 끝의 널을 찾겠다고 무한정 읽지 않습니다. 일반 문서·게임 텍스트 조작에는 먼저 std::string을 사용하고, C API 경계에서 필요한 변환만 합니다.

근거: [Microsoft — basic\_string](https://learn.microsoft.com/en-us/cpp/standard-library/basic-string-class?view=msvc-170), [Microsoft — strcmp·wcscmp](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/strcmp-wcscmp-mbscmp?view=msvc-170).

## 7. 사용자 정의 타입·정렬·별칭

```c++
struct Item {
    int id{};
    std::string name;
    int quantity{};
};
using ItemId = int;
enum class ItemKind { Consumable, Equipment };
```

struct는 서로 관련된 데이터를 묶어 의미 있는 타입을 만듭니다. C++의 struct는 멤버 함수도 가질 수 있습니다. class와의 기본 차이는 멤버·상속의 기본 접근 수준입니다. 이름만 다른 별칭 <code>using ItemId = int</code>는 새로운 독립 타입을 만들지 않습니다. 섞으면 안 되는 단위나 ID라면 별도 타입 설계를 검토합니다.

구조체 크기는 멤버 크기의 단순 합이나 가장 큰 멤버 크기 하나로 결정되지 않습니다. 정렬 조건에 따라 멤버 사이와 뒤에 패딩이 들어갈 수 있습니다. sizeof·alignof와, 적용 가능한 표준 레이아웃 타입에서 offsetof를 확인합니다. 포인터·std::string이 있는 구조체를 바이트 그대로 파일에 쓰면 대상 문자열까지 저장되는 것이 아닙니다. 이것은 05 문서의 직렬화 문제로 이어집니다.

자기 자신을 값 멤버로 무한히 포함할 수는 없지만, Node\* 같은 포인터 멤버는 만들 수 있습니다. 그 포인터가 다음 노드를 소유하는지 관찰만 하는지까지 정해야 자료구조 설계가 됩니다.

근거: [N4861 — 클래스](https://timsong-cpp.github.io/cppwp/n4861/class), [N4861 — 정렬](https://timsong-cpp.github.io/cppwp/n4861/basic.align).

### union은 임의 타입 재해석 도구가 아닙니다

union은 여러 멤버가 저장 공간을 공유하며 활성 멤버·수명 규칙을 따라야 합니다. 다른 멤버로 읽는 모든 형태를 일반적인 C++ 타입 변환으로 사용할 수는 없습니다. 동시에 하나의 타입 값만 담아야 하는 데이터에는 std::variant를 고려하고, 객체 표현을 관찰하려는 실험은 03 문서의 bit\_cast와 바이트 규칙을 사용합니다.

```c++
std::variant<int, std::string> data{7};
data = std::string{"text"};
bool is_text = std::holds_alternative<std::string>(data);
```

이 조각에는 <code><variant></code>와 <code><string></code>이 필요합니다. std::get으로 현재와 다른 타입을 꺼내는 경우의 실패 처리도 고려해야 합니다.

근거: [N4861 — union](https://timsong-cpp.github.io/cppwp/n4861/class.union), [Microsoft — variant](https://learn.microsoft.com/en-us/cpp/standard-library/variant?view=msvc-170).

## 8. 정렬은 비교 규칙이 핵심입니다

작은 배열을 이중 반복문으로 정렬하는 것은 상태 변화와 복잡도 학습에 유용합니다. 반복 비교가 입력 크기에 따라 어떻게 늘어나는지 설명하고, 일반적인 데이터 정렬에는 std::sort를 사용합니다. std::sort는 N log N 비교 복잡도를 요구하지만 동일 키 원소의 이전 순서를 보존하는 안정 정렬은 아닙니다.

```c++
std::sort(items.begin(), items.end(), [](const Item& a, const Item& b) {
    return a.quantity != b.quantity
        ? a.quantity > b.quantity
        : a.id < b.id;
});
```

위 예제는 수량 내림차순, 동률에서는 ID 오름차순입니다. 비교 함수는 엄격 약순서를 만족해야 하며, <code>a <= b</code>를 비교자로 넣어 자기 자신에도 true가 되게 해서는 안 됩니다. 정렬 중 데이터를 바꾸거나 매 호출마다 난수로 답하는 비교자도 일관된 순서를 만들지 못합니다. 실습은 같은 수량의 아이템까지 포함해 결과를 검사합니다.

근거: [N4861 — 정렬과 비교자 조건](https://timsong-cpp.github.io/cppwp/n4861/alg.sorting).

## 9. 난수: 엔진과 분포를 나눕니다

```c++
std::mt19937 engine{1234u};
std::uniform_int_distribution<int> die{1, 6};
int roll = die(engine);
```

<code><random></code>의 엔진은 의사난수 상태와 수열을 관리하고, 분포는 이를 원하는 범위·분포의 값으로 변환합니다. 같은 시드를 쓰면 같은 엔진 상태에서 같은 수열을 재현할 수 있다는 뜻이지 모든 호출이 같은 값이라는 뜻은 아닙니다. 실습에서는 두 mt19937 엔진의 반복 결과와 주사위 범위를 확인합니다.

표준 라이브러리의 분포 구현까지 모든 플랫폼에서 똑같은 출력 수열이라고 전제하지 않습니다. 게임의 결정적 리플레이·동기화가 목적이면 엔진뿐 아니라 분포 알고리즘·호출 횟수·시드 저장 규약을 명시해야 합니다. 보안 토큰 생성에 이 학습용 의사난수를 사용하지 않습니다.

기존 rand()%N 방식은 범위 매핑에 편향이 생길 수 있고, RAND\_MAX도 구현마다 다릅니다. 매 추첨마다 현재 시각으로 재시드하는 것도 수열의 성질을 개선하는 방법이 아닙니다. 테스트에서는 고정 시드를 사용해 재현성을 확보하고, 실제 사용 목적에 맞는 시드 정책을 별도로 정합니다.

근거: [N4861 — 난수 라이브러리](https://timsong-cpp.github.io/cppwp/n4861/rand).

## 10. 실습과 이해 확인

<code>containers</code>는 array·span·vector, 경계 거부, 재할당 후 데이터 보존, 2차원 접근, 내부 널 문자열, 정렬, variant, 난수의 반복·범위를 <b>49개 검사</b>로 확인합니다. <code>buffer</code>는 빈 상태·범위 거부·100개 추가와 데이터 보존을 <b>104개 검사</b>로 확인합니다. 반복문 각 원소 검사도 이 숫자에 포함되므로 서로 다른 알고리즘 104개를 검증했다는 뜻은 아닙니다.

<toggle>
질문: reserve로 재할당만 막으면 원소 참조는 항상 안전할까요?

아닙니다. 중간 삽입·삭제에 따른 무효화와 의미 변화, 컨테이너의 소멸을 함께 고려해야 합니다. 재할당 없는 push\_back과 erase는 조건이 다릅니다.
</toggle>

<toggle>
질문: int grid\[2\]\[3\]을 int로 받으면 안 되는 이유는 무엇일까요?

grid는 연속된 int 행 배열이고, int는 int\* 객체를 가리키는 타입입니다. 저장된 메모리 구조가 다릅니다. negative/array\_pointer.cpp에서 암시적 대입이 거부되는 것을 확인했습니다.
</toggle>

<toggle>
질문: 구조체를 통째로 저장하면 다음 실행에서도 포인터가 복원될까요?

아닙니다. 포인터의 주소값을 저장해도 대상 객체의 내용·수명이 복원되지 않습니다. 데이터 필드와 파일 형식을 명시적으로 저장해야 합니다.
</toggle>
