// C01 CPU 전용 예제입니다. Windows/GPU API를 사용하지 않고 C++11과 C++20에서 빌드합니다.
#include <climits>
#include <iostream>
#include <sstream>
#include <string>

// 부호 있는 덧셈을 수행하기 전에 INT_MAX/INT_MIN을 기준으로 범위를 검사합니다.
bool checkedAdd(int a, int b, int& result) {
    if ((b > 0 && a > INT_MAX - b) || (b < 0 && a < INT_MIN - b))
        return false;
    result = a + b;
    return true;
}
// 두 정수 이외에 남는 토큰이 있으면 잘못된 입력으로 처리합니다.
bool parse(const std::string& text, int& a, int& b) {
    std::istringstream in(text);
    if (!(in >> a >> b)) return false;
    std::string extra;
    return !(in >> extra);
}
// 서로 다른 입력 계약 10개를 검사합니다. 반복 빌드 수를 독립 조건으로 합산하지 않습니다.
int selftest() {
    int v = 0, a = 0, b = 0;
    const bool t[] = {
        checkedAdd(1, 2, v) && v == 3,
        checkedAdd(-3, -4, v) && v == -7,
        !checkedAdd(INT_MAX, 1, v),
        !checkedAdd(INT_MIN, -1, v),
        checkedAdd(INT_MAX, 0, v) && v == INT_MAX,
        checkedAdd(INT_MIN, 0, v) && v == INT_MIN,
        parse("1 2", a, b) && a == 1 && b == 2,
        !parse("1 bad", a, b),
        !parse("1 2 extra", a, b),
        !parse("", a, b)
    };
    for (unsigned i = 0; i < sizeof(t) / sizeof(t[0]); ++i) {
        if (!t[i]) { std::cerr << "FAIL condition " << i << '\n'; return 1; }
    }
    std::cout << "PASS 10 independent conditions\n";
    return 0;
}
int main(int argc, char** argv) {
    if (argc == 2 && std::string(argv[1]) == "--test") return selftest();
    if (argc != 1) return 2;
    std::string line;
    if (!std::getline(std::cin, line)) return 1;
    int a = 0, b = 0, v = 0;
    if (!parse(line, a, b)) { std::cerr << "invalid input\n"; return 1; }
    // 실패 시 출력이나 상태 변경 전에 명시적 오류 코드로 종료합니다.
    if (!checkedAdd(a, b, v)) { std::cerr << "overflow\n"; return 2; }
    std::cout << v << '\n';
    return 0;
}
