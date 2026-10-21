#include <memory>
void F() { auto a = std::make_unique<int>(1); auto b = a; }
