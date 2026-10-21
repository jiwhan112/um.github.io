#include "check.hpp"
#include <algorithm>
#include <deque>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <ranges>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

// 원본과 동일한 실행 코드에 한국어 설명만 더했다.
// 컨테이너마다 삽입·삭제·재해시가 반복자와 참조에 미치는 영향이 다르다.
// 무효화된 반복자를 실제로 사용해 보지 않고, 유효성이 보장된 접근만 수행한다.
struct Item { int priority; int id; };

struct Less {
    bool operator()(const Item& a, const Item& b) const noexcept {
        // priority를 먼저 비교하고, 같으면 id를 비교하는 사전식 순서다.
        // tie는 멤버를 복사하지 않는 참조 튜플이다. <=를 비교자로 쓰지 않는다.
        // 아래의 세 표본 검사는 비반사성을 관찰하며, 모든 입력에 대한
        // 엄격 약순서의 추이성·동치 관계 법칙을 자동 증명하지는 않는다.
        return std::tie(a.priority, a.id) < std::tie(b.priority, b.id);
    }
};

int main() { return Run("containers", [](Checks& c) {
    // vector의 size는 현재 원소 수이고 capacity는 재할당 없이 담을 수 있는 수다.
    // 실제 초기 capacity 값이나 메모리 주소를 특정 값으로 가정하지 않는다.
    std::vector<int> v{1,2,3,4};
    const auto capacity = v.capacity();
    std::erase_if(v, [](int n) { return n % 2 == 0; });
    c.Expect(v == std::vector<int>({1,3}), "erase_if");
    c.Expect(v.capacity() == capacity, "erase retains capacity");

    // 삭제 후 새 begin()을 얻어 가운데에 삽입한다. 여기서는 기존 용량이 충분하다.
    // 재할당이 없어도 vector의 삽입 위치와 그 뒤의 반복자는 무효화되므로,
    // 삽입 전에 얻은 반복자가 있다고 해서 계속 사용할 수 있는 것은 아니다.
    v.insert(v.begin()+1, 2);
    c.Expect(v == std::vector<int>({1,2,3}), "middle insert");

    // erase는 삭제한 원소의 다음 원소를 가리키는 새 반복자를 돌려준다.
    // 삭제한 경우에는 그 반환값을 사용하고, 유지한 경우에만 ++it 한다.
    // 이전 end()를 저장하지 않고 조건 검사마다 현재 end()를 구한다.
    for (auto it = v.begin(); it != v.end();) {
        if (*it >= 2) it = v.erase(it);
        else ++it;
    }
    c.Expect(v == std::vector<int>({1}), "erase return iterator");

    // list의 keep은 처음에 값 1을 가진 노드를 가리킨다.
    // 다른 노드의 삽입과 list::sort는 이 반복자를 무효화하지 않는다.
    // list 반복자는 임의 접근 반복자가 아니므로 일반 std::sort를 쓰지 않는다.
    std::list<int> nodes{3,1,2};
    auto keep = std::next(nodes.begin());
    nodes.push_front(9);
    c.Expect(*keep == 1, "list iterator stable on insert");
    nodes.sort();
    c.Expect(*keep == 1 && nodes.front() == 1, "list member sort");

    // deque는 양쪽 끝 삽입과 인덱스 접근을 제공한다.
    // 이 검사는 값의 위치만 확인한다. 연속 메모리나 반복자 안정성의 검사가 아니다.
    std::deque<int> queue;
    queue.push_front(2);
    queue.push_back(3);
    queue.push_front(1);
    c.Expect(queue[1] == 2, "deque random access");

    // map의 find는 없는 키를 만들지 않는다. 투명 비교자 less<>는
    // 이 find 호출에서 문자열 리터럴을 조회 키로 비교할 수 있게 한다.
    std::map<std::string, int, std::less<>> counts{{"potion",3}};
    c.Expect(counts.find("absent") == counts.end() && counts.size() == 1, "find no insert");
    // 반면 operator[]는 키가 없으면 삽입하고 int를 값 초기화하여 0을 돌려준다.
    c.Expect(counts["absent"] == 0 && counts.size() == 2, "brackets insert");

    // 서로 다른 문자 배열도 std::string 키로 변환되면 같은 내용의 키다.
    // C++20의 이 at 호출은 std::string 변환을 사용하며, find처럼
    // 이종 키 조회가 제공된다는 뜻이 아니다. char* 주소를 키로 저장하지도 않는다.
    const char a[] = "potion";
    const char b[] = "potion";
    c.Expect(counts.at(a) == counts.at(b), "string content keys");

    // rehash는 반복자를 무효화하지만, 원소의 포인터·참조는 유지한다.
    // 미리 보관하는 것은 반복자가 아니라 mapped value의 주소다.
    // 100은 요청하는 최소 버킷 수이며 실제 버킷 수가 정확히 100이라는 검사는 없다.
    std::unordered_map<int, std::string> lookup{{1,"one"}};
    const std::string* value = &lookup.at(1);
    lookup.rehash(100);
    c.Expect(value == &lookup.at(1) && *value == "one", "rehash preserves reference, not iterator");

    std::vector<Item> items{{2,3},{1,8},{2,1}};
    std::ranges::sort(items, Less{});
    // 정렬 결과의 선두 두 id를 확인한다. 세 번째 id는 이 조건에서 직접 비교하지 않는다.
    c.Expect(items[0].id == 8 && items[1].id == 1, "strict comparator");
    // 호출 위치는 하나지만 원소가 세 개이므로 Expect가 세 번 실행된다.
    for (const auto& x : items) c.Expect(!Less{}(x,x), "irreflexivity sample");

    // vector가 소유하는 것은 unique_ptr 원소이고, 각 원소는 int를 소유한다.
    // clear는 원소를 소멸시켜 unique_ptr의 정리를 수행한다.
    // 아래의 관측값은 empty()이며 별도 소멸·해제 카운터를 측정하지는 않는다.
    std::vector<std::unique_ptr<int>> owners;
    owners.push_back(std::make_unique<int>(7));
    owners.clear();
    c.Expect(owners.empty(), "owning elements released");
}); }
