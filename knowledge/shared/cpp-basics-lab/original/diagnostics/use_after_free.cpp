// Intentional undefined behavior. Compile with ASan; never use as normal code.
int main() { int* owner = new int{7}; int* observer = owner; delete owner; return *observer; }
