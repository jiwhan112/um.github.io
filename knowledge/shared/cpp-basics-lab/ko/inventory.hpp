#ifndef CPP_BASICS_INVENTORY_HPP
#define CPP_BASICS_INVENTORY_HPP
#include <istream>
#include <ostream>
#include <string>
#include <vector>

namespace study {

// 필드별 값을 저장한다. string 객체의 메모리 표현을 그대로 저장하지 않는다.
struct Item {
    int id{};
    std::string name;
    int quantity{};
};
using Inventory = std::vector<Item>;

// 유효하지 않은 데이터와 검사한 스트림 실패는 runtime_error로 알린다.
// 메모리 할당 등 표준 라이브러리 연산에서 생기는 다른 예외까지 제한하는 선언은 아니다.
void Save(std::ostream& output, const Inventory& items);
// 전부 읽고 검증한 새 결과만 반환한다. 호출자의 기존 Inventory를 변경하지 않는다.
Inventory Load(std::istream& input);

}
#endif
