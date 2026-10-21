// Intentional undefined behavior. Compile with UBSan; never use as normal code.
#include <limits>
int main() { volatile int n = std::numeric_limits<int>::max(); return n + 1; }
