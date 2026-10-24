// NET05: CPU-only MTU/header-budget and application-framing model.
// C++11 and C++20. Does not invoke a socket, OS PMTUD, or DirectX.
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>

namespace net05 {
// KO: 호출자는 아래에 선언한 유효한 enum 값만 전달해야 합니다. 임의 정수 cast 값은 검증하지 않습니다.
enum class IpVersion { ipv4, ipv6 };
enum class Transport { tcp, udp };

struct Budget {
    std::uint64_t ip_header;
    std::uint64_t transport_header;
    std::uint64_t application_payload;
};

// Simplified baseline headers: no IPv4/TCP options or IPv6 extensions.
// We intentionally reject a zero-payload packet. Real stacks can send one.
// KO: 실제 경로를 탐색하지 않습니다. 주어진 MTU 숫자에서 기본 IP/전송 헤더만 뺍니다.
// KO: 실제 IP 길이 필드 상한·옵션·확장헤더·게임 프레이밍·암호화 비용은 이 함수의 검사 범위 밖입니다.
// KO: 실패하면 out을 수정하지 않습니다. 0바이트 전송 예산은 모델 정책으로 거절합니다.
bool compute_budget(std::uint64_t path_mtu, IpVersion ip,
                    Transport transport, Budget& out) {
    const std::uint64_t ip_header = ip == IpVersion::ipv4 ? 20u : 40u;
    const std::uint64_t transport_header = transport == Transport::tcp ? 20u : 8u;
    if ((ip == IpVersion::ipv6 && path_mtu < 1280u) ||
        path_mtu <= ip_header + transport_header) {
        return false;
    }
    Budget value = {ip_header, transport_header,
                    path_mtu - ip_header - transport_header};
    out = value;
    return true;
}

// KO: last_unit_bytes는 마지막 추가 꼬리의 크기입니다. 정확히 나누어지면 0이지만 빈 패킷을 뜻하지 않습니다.
struct Plan {
    std::uint64_t units;
    std::uint64_t full_units;
    std::uint64_t last_unit_bytes;
};

// Pure arithmetic. For TCP these are conceptual payload budgets, NOT a
// promise that send() calls map one-to-one to on-wire TCP segments.
// For UDP, multiple units require APPLICATION-LEVEL datagram framing.
// KO: bytes + capacity - 1 대신 나눗셈과 나머지를 사용하여 올림 계산의 덧셈 오버플로를 피합니다.
// KO: bytes가 0이면 성공하며 units도 0입니다. capacity가 0이면 실패하고 out을 보존합니다.
bool plan_payload(std::uint64_t bytes, std::uint64_t capacity, Plan& out) {
    if (capacity == 0) return false;
    const std::uint64_t full = bytes / capacity;
    const std::uint64_t tail = bytes % capacity;
    if (tail != 0 && full == std::numeric_limits<std::uint64_t>::max())
        return false;
    Plan value = {full + (tail != 0 ? 1u : 0u), full, tail};
    out = value;
    return true;
}
} // namespace net05

// KO: 같은 조건을 다른 컴파일러·표준으로 반복해도 독립 조건 수가 늘지는 않습니다.
int main() {
    using namespace net05;
    unsigned conditions = 0;
    const auto check = [&conditions](bool ok, const char* name) {
        ++conditions;
        if (!ok) {
            std::cerr << "FAIL " << name << '\n';
            std::exit(EXIT_FAILURE);
        }
    };

    Budget b = {11, 22, 33};
    check(compute_budget(1500, IpVersion::ipv4, Transport::tcp, b), "v4 TCP supported");
    check(b.ip_header == 20 && b.transport_header == 20, "v4 TCP header sizes");
    check(b.application_payload == 1460, "v4 TCP baseline MSS");
    check(compute_budget(1500, IpVersion::ipv4, Transport::udp, b), "v4 UDP supported");
    check(b.application_payload == 1472, "v4 UDP budget");
    check(compute_budget(1500, IpVersion::ipv6, Transport::tcp, b), "v6 TCP supported");
    check(b.application_payload == 1440, "v6 TCP baseline payload");
    check(compute_budget(1500, IpVersion::ipv6, Transport::udp, b), "v6 UDP supported");
    check(b.application_payload == 1452, "v6 UDP budget");

    b = {11, 22, 33};
    check(!compute_budget(28, IpVersion::ipv4, Transport::udp, b), "v4 no payload rejected");
    check(b.application_payload == 33, "failure does not mutate budget");
    check(compute_budget(29, IpVersion::ipv4, Transport::udp, b), "v4 one byte fits");
    check(b.application_payload == 1, "v4 one byte budget");
    b = {11, 22, 33};
    check(!compute_budget(1279, IpVersion::ipv6, Transport::udp, b), "v6 model minimum MTU");
    check(b.application_payload == 33, "v6 failure does not mutate budget");
    check(compute_budget(1280, IpVersion::ipv6, Transport::udp, b), "v6 minimum MTU valid");
    check(b.application_payload == 1232, "v6 minimum UDP budget");

    Plan p = {91, 92, 93};
    check(plan_payload(2000, 1460, p), "2000 TCP bytes plan");
    check(p.units == 2 && p.full_units == 1 && p.last_unit_bytes == 540,
          "TCP model 1460 plus 540");
    check(plan_payload(2000, 1472, p), "2000 UDP bytes plan");
    check(p.units == 2 && p.full_units == 1 && p.last_unit_bytes == 528,
          "UDP application must frame 2 datagrams");
    check(plan_payload(0, 1460, p) && p.units == 0, "empty message zero units");
    check(plan_payload(1460, 1460, p) && p.units == 1 && p.last_unit_bytes == 0,
          "exact capacity one full unit");
    check(plan_payload(1461, 1460, p) && p.units == 2 && p.last_unit_bytes == 1,
          "boundary plus one byte");
    p = {91, 92, 93};
    check(!plan_payload(42, 0, p), "zero capacity rejected");
    check(p.units == 91 && p.full_units == 92 && p.last_unit_bytes == 93,
          "plan failure does not mutate output");
    check(plan_payload(std::numeric_limits<std::uint64_t>::max(), 1, p),
          "uint64 max bytes with one-byte units");
    check(p.units == std::numeric_limits<std::uint64_t>::max() &&
          p.full_units == p.units && p.last_unit_bytes == 0, "no arithmetic overflow");
    check(plan_payload(std::numeric_limits<std::uint64_t>::max(), 1460, p),
          "huge message bounded by division");
    check(p.units > 0 && p.last_unit_bytes < 1460, "huge message tail bounded");

    std::cout << "PASS " << conditions << " NET05 CPU conditions\n";
    return EXIT_SUCCESS;
}
