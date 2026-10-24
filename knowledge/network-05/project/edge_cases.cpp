// 원본 산술 함수를 다시 구현하지 않고 직접 호출하는 추가 경계 검사입니다.
#define main source_main
#include "main.cpp"
#undef main
int main() {
    using namespace net05;
    unsigned checks = 0;
    const auto require = [&checks](bool ok) {
        ++checks;
        if (!ok) std::exit(EXIT_FAILURE);
    };
    Budget b = {11, 22, 33};
    require(!compute_budget(0, IpVersion::ipv4, Transport::tcp, b));
    require(b.ip_header == 11 && b.transport_header == 22 && b.application_payload == 33);
    require(compute_budget(1280, IpVersion::ipv6, Transport::tcp, b) && b.application_payload == 1220);
    // 16바이트 게임 헤더를 UDP 예산에서 추가 차감한 뒤에 데이터 분할을 계산합니다.
    require(compute_budget(1500, IpVersion::ipv4, Transport::udp, b) && b.application_payload == 1472);
    Plan p = {0, 0, 0};
    require(plan_payload(2000, b.application_payload - 16, p) && p.units == 2 && p.last_unit_bytes == 544);
    require(plan_payload(2912, 1456, p) && p.units == 2 && p.full_units == 2 && p.last_unit_bytes == 0);
    // 아주 큰 MTU도 함수가 받아들임을 기록합니다. 실제 IPv4 패킷 허용 검사가 아닙니다.
    require(compute_budget(65536, IpVersion::ipv4, Transport::udp, b) && b.application_payload == 65508);
    const std::uint64_t m = std::numeric_limits<std::uint64_t>::max();
    require(plan_payload(m, m, p) && p.units == 1 && p.full_units == 1 && p.last_unit_bytes == 0);
    std::cout << "PASS NET05 added checks=" << checks << "\n";
    return EXIT_SUCCESS;
}
