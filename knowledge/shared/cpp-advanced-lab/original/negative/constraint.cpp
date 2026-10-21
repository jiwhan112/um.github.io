#include <concepts>
template<std::integral T> T F(T value) { return value; }
int main() { return F(1.5); }
