// 기존 구현은 수정하지 않고 별도 번역 단위에서 경계 조건을 더 검사합니다.
#define main source_main
#include "main.cpp"
#undef main
int main() {
    try {
        checks = 0;
        RoutingTable t;
        // 삽입 순서를 뒤집어도 최장 접두부가 우선이어야 합니다.
        t.Add("10.2.3.77", 32, "10.2.3.1", "host");
        t.Add("10.2.3.123", 24, "10.2.3.1", "subnet");
        t.Add("0.0.0.0", 0, "1.1.1.1", "default");
        Check(t.Lookup(IPv4("10.2.3.77"))->label == "host", "reverse insertion");
        Check(t.Lookup(IPv4("10.2.3.78"))->network == IPv4("10.2.3.0"), "normalize host bits");
        Reject([&] { t.Add("10.2.3.254", 24, "1.1.1.1", "duplicate"); });
        RoutingTable tiny;
        tiny.Add("192.0.2.0", 31, "192.0.2.1", "pair");
        Check(tiny.Lookup(IPv4("192.0.2.0")) != 0, "slash31 first");
        Check(tiny.Lookup(IPv4("192.0.2.1")) != 0, "slash31 second");
        Check(tiny.Lookup(IPv4("192.0.2.2")) == 0, "slash31 outside");
        // 이 모델은 접두부 일치만 다룹니다. 주소 사용 가능성·링크 동작은 별도입니다.
        Reject([] { IPv4(" 1.2.3.4"); });
        Reject([] { IPv4("1.2.3.4."); });
        std::cout << "PASS NET04 added checks=" << checks << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
}
