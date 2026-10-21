#include "inventory.hpp"
#include <iomanip>
#include <set>
#include <sstream>
#include <stdexcept>
namespace study {
namespace {
void Validate(const Inventory& items) {
    if (items.size() > 1000) throw std::runtime_error("too many items");
    std::set<int> ids;
    for (const auto& item : items) {
        if (item.id <= 0 || !ids.insert(item.id).second) throw std::runtime_error("invalid/duplicate id");
        if (item.quantity < 0 || item.quantity > 1000000) throw std::runtime_error("quantity range");
        if (item.name.empty() || item.name.size() > 256 || item.name.find_first_of("\r\n") != std::string::npos)
            throw std::runtime_error("invalid name");
    }
}
void RequireEnd(std::istream& line) {
    line >> std::ws;
    if (!line.eof()) throw std::runtime_error("trailing data");
}
}
void Save(std::ostream& output, const Inventory& items) {
    Validate(items); // Check all values before writing.
    output << "INV1 " << items.size() << '\n';
    for (const auto& item : items)
        output << item.id << ' ' << std::quoted(item.name) << ' ' << item.quantity << '\n';
    if (!output) throw std::runtime_error("write failed");
}
Inventory Load(std::istream& input) {
    std::string line;
    if (!std::getline(input, line)) throw std::runtime_error("missing header");
    std::istringstream header{line}; std::string magic; long long count{};
    if (!(header >> magic >> count) || magic != "INV1" || count < 0 || count > 1000)
        throw std::runtime_error("bad header");
    RequireEnd(header);
    Inventory items; items.reserve(static_cast<std::size_t>(count));
    for (long long i = 0; i < count; ++i) {
        if (!std::getline(input, line)) throw std::runtime_error("truncated file");
        std::istringstream row{line}; Item item;
        if (!(row >> item.id >> std::quoted(item.name) >> item.quantity)) throw std::runtime_error("bad row");
        RequireEnd(row);
        items.push_back(std::move(item));
    }
    while (std::getline(input, line)) {
        if (line.find_first_not_of(" \t\r") != std::string::npos) throw std::runtime_error("extra row");
    }
    if (input.bad() || (input.fail() && !input.eof())) throw std::runtime_error("read failed");
    Validate(items); // Return only a complete valid result.
    return items;
}
}
