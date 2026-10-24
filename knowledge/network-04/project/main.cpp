#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// Portable IPv4 network-order integer representation, no socket API.
// KO: 문자열은 정확히 네 개의 10진 옥텟만 허용합니다. 공백, 포트, 선행 0을 자동 보정하지 않습니다.
// KO: 상위 옥텟부터 만든 정숫값이며 네트워크로 보낼 메모리 바이트열은 아닙니다.
static std::uint32_t IPv4(const std::string& s) {
    std::uint32_t ip = 0;
    std::size_t pos = 0;
    for (unsigned field = 0; field < 4; ++field) {
        if (pos >= s.size()) throw std::invalid_argument("missing octet");
        unsigned value = 0, digits = 0;
        const std::size_t begin = pos;
        while (pos < s.size() && s[pos] >= '0' && s[pos] <= '9') {
            if (++digits > 3) throw std::invalid_argument("long octet");
            value = value * 10 + static_cast<unsigned>(s[pos] - '0');
            ++pos;
        }
        if (digits == 0 || value > 255 || (digits > 1 && s[begin] == '0'))
            throw std::invalid_argument("invalid octet");
        ip = (ip << 8) | value;
        if (field < 3) {
            if (pos >= s.size() || s[pos] != '.') throw std::invalid_argument("missing dot");
            ++pos;
        }
    }
    if (pos != s.size()) throw std::invalid_argument("extra text");
    return ip;
}
// KO: /0은 별도 분기로 처리하여 32비트 폭만큼 시프트하는 경로를 피합니다.
static std::uint32_t PrefixMask(unsigned prefix) {
    if (prefix > 32) throw std::invalid_argument("invalid CIDR prefix");
    return prefix == 0 ? 0U : (0xffffffffU << (32 - prefix));
}
// KO: network는 정규화한 접두부입니다. nextHop은 이 모델에서 단순한 IPv4 값입니다.
struct Route {
    std::uint32_t network, nextHop;
    unsigned prefix;
    std::string label;
};
class RoutingTable {
public:
// KO: 먼저 파싱·중복 검사를 끝낸 뒤 vector에 넣습니다. 같은 network/prefix의 여러 경로는 지원하지 않습니다.
    void Add(const std::string& cidrIP, unsigned prefix,
             const std::string& hop, const std::string& label) {
        const std::uint32_t net = IPv4(cidrIP) & PrefixMask(prefix);
        for (std::size_t i = 0; i < routes_.size(); ++i)
            if (routes_[i].network == net && routes_[i].prefix == prefix)
                throw std::invalid_argument("duplicate prefix");
        routes_.push_back(Route{net, IPv4(hop), prefix, label});
    }
// KO: 전 경로를 검사하고 가장 긴 일치만 남깁니다. 결과 0은 경로 없음입니다.
// KO: 반환 포인터는 내부 vector를 빌립니다. 이후 Add의 재할당 또는 테이블 파괴 후에는 사용하지 마세요.
    const Route* Lookup(std::uint32_t ip) const {
        const Route* best = 0;
        for (std::size_t i = 0; i < routes_.size(); ++i) {
            const Route& r = routes_[i];
            if ((ip & PrefixMask(r.prefix)) == r.network &&
                (!best || r.prefix > best->prefix)) best = &r;
        }
        return best;
    }
private:
    std::vector<Route> routes_;
};
static unsigned checks = 0;
// KO: assert와 달리 NDEBUG 빌드에서도 실행되는 검사입니다.
static void Check(bool ok, const std::string& why) {
    ++checks; if (!ok) throw std::runtime_error(why);
}
template <typename F> static void Reject(F f) {
    bool threw = false;
    try { f(); } catch (const std::invalid_argument&) { threw = true; }
    Check(threw, "invalid address accepted");
}
// KO: 소켓·OS 라우팅 테이블을 건드리지 않는 결정적 CPU 검사입니다.
int main() {
    try {
        Check(IPv4("0.0.0.0") == 0U, "zero address");
        Check(IPv4("255.255.255.255") == 0xffffffffU, "highest address");
        Check(IPv4("192.168.1.42") == 0xc0a8012aU, "network order");
        Check(PrefixMask(0) == 0U, "default mask");
        Check(PrefixMask(24) == 0xffffff00U, "slash24 mask");
        Check(PrefixMask(32) == 0xffffffffU, "slash32 mask");
        Reject([] { IPv4("192.168.1"); });
        Reject([] { IPv4("1.2.3.256"); });
        Reject([] { IPv4("001.2.3.4"); });
        Reject([] { IPv4("1.2.3.4:80"); });
        Reject([] { IPv4("1..2.3"); });
        Reject([] { PrefixMask(33); });
        RoutingTable t;
        t.Add("0.0.0.0", 0, "1.1.1.1", "default");
        t.Add("10.2.0.0", 16, "10.2.0.1", "zone");
        t.Add("10.2.3.0", 24, "10.2.3.1", "subnet");
        t.Add("10.2.3.77", 32, "10.2.3.254", "host");
        Check(std::string(t.Lookup(IPv4("10.2.3.77"))->label) == "host", "/32 wins");
        Check(std::string(t.Lookup(IPv4("10.2.3.78"))->label) == "subnet", "/24 wins");
        Check(std::string(t.Lookup(IPv4("10.2.99.5"))->label) == "zone", "/16 wins");
        Check(std::string(t.Lookup(IPv4("203.0.113.9"))->label) == "default", "/0 wins");
        Reject([&] { t.Add("10.2.3.123", 24, "1.1.1.1", "duplicate normalized"); });
        RoutingTable empty;
        Check(empty.Lookup(IPv4("127.0.0.1")) == 0, "no route");
        std::cout << "PASS NET04 checks=" << checks << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL NET04: " << e.what() << "\n";
        return 1;
    }
}
