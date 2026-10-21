#include "inventory.hpp"
#include <iomanip>
#include <set>
#include <sstream>
#include <stdexcept>

namespace study {
namespace {

// 이 번역 단위에서만 쓰는 검사다. 저장 전과 완전히 읽은 뒤에 같은 계약을 적용한다.
void Validate(const Inventory& items) {
    if (items.size() > 1000) throw std::runtime_error("too many items");
    std::set<int> ids;
    for (const auto& item : items) {
        if (item.id <= 0 || !ids.insert(item.id).second) throw std::runtime_error("invalid/duplicate id");
        if (item.quantity < 0 || item.quantity > 1000000) throw std::runtime_error("quantity range");
        // name.size()는 char 원소 수다. UTF-8 표시 글자 수나 인코딩 유효성 검사가 아니다.
        if (item.name.empty() || item.name.size() > 256 || item.name.find_first_of("\r\n") != std::string::npos)
            throw std::runtime_error("invalid name");
    }
}

void RequireEnd(std::istream& line) {
    // 필드를 읽고 남은 공백은 허용하지만 다른 토큰은 거부한다.
    line >> std::ws;
    if (!line.eof()) throw std::runtime_error("trailing data");
}

}

void Save(std::ostream& output, const Inventory& items) {
    // 데이터 오류로 일부만 기록하는 일을 피하려고 먼저 전체 값을 검사한다.
    Validate(items);
    output << "INV1 " << items.size() << '\n';
    for (const auto& item : items)
        output << item.id << ' ' << std::quoted(item.name) << ' ' << item.quantity << '\n';
    // 여기까지의 스트림 상태를 확인한다. 이후 close, 원자적 교체, 내구성은 별도다.
    if (!output) throw std::runtime_error("write failed");
}

Inventory Load(std::istream& input) {
    std::string line;
    if (!std::getline(input, line)) throw std::runtime_error("missing header");
    std::istringstream header{line};
    std::string magic;
    long long count{};
    // 개수 상한을 먼저 확인하고 나서 저장 공간을 예약한다.
    if (!(header >> magic >> count) || magic != "INV1" || count < 0 || count > 1000)
        throw std::runtime_error("bad header");
    RequireEnd(header);

    Inventory items;
    items.reserve(static_cast<std::size_t>(count));
    for (long long i = 0; i < count; ++i) {
        if (!std::getline(input, line)) throw std::runtime_error("truncated file");
        std::istringstream row{line};
        Item item;
        // quoted는 따옴표 문자열 외에 공백 없는 비인용 토큰도 받아들인다.
        if (!(row >> item.id >> std::quoted(item.name) >> item.quantity)) throw std::runtime_error("bad row");
        RequireEnd(row);
        items.push_back(std::move(item));
    }

    // 명시한 개수 뒤에는 빈 줄이나 지정한 공백 문자만 허용한다.
    while (std::getline(input, line)) {
        if (line.find_first_not_of(" \t\r") != std::string::npos) throw std::runtime_error("extra row");
    }
    // 정상적인 EOF와 실제 스트림 오류를 구별한다.
    if (input.bad() || (input.fail() && !input.eof())) throw std::runtime_error("read failed");
    // 행을 전부 읽은 뒤 ID 중복·필드 범위를 검사하고 완성된 결과만 반환한다.
    Validate(items);
    return items;
}

}
