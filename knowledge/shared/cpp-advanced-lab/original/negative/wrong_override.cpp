struct Base { virtual int Read() const = 0; };
struct Derived : Base { int Read() override { return 1; } };
