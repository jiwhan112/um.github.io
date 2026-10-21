#ifndef CPP_BASICS_INVENTORY_HPP
#define CPP_BASICS_INVENTORY_HPP
#include <istream>
#include <ostream>
#include <string>
#include <vector>
namespace study {
struct Item { int id{}; std::string name; int quantity{}; };
using Inventory = std::vector<Item>;
// These functions throw std::runtime_error for invalid data or stream failures.
void Save(std::ostream& output, const Inventory& items);
Inventory Load(std::istream& input);
}
#endif
