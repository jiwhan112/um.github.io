#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>
#include <utility>

// C++11 compatible. 64 bytes is a lab assumption, NOT detected cache-line size.
struct NearCell { std::atomic<unsigned> n; NearCell() : n(0) {} }; // 한국어: 서로 독립적인 atomic 카운터의 인접 배치를 구성한다.
struct alignas(64) FarCell { std::atomic<unsigned> n; FarCell() : n(0) {} }; // 한국어: 64바이트 최소 정렬을 요청할 뿐 실제 캐시 라인 크기를 측정하지 않는다.
struct Near { NearCell a,b; };
struct Far { FarCell a,b; }; // 한국어: 두 멤버의 간격은 타입 크기와 정렬 영향을 받는다.

template<class T>
std::pair<unsigned,unsigned> count(T& v, unsigned repetitions, long long& ns) { // 한국어: 호출마다 새로 0 초기화된 객체를 쓰는 것이 이 테스트의 전제다.
    const auto start = std::chrono::steady_clock::now(); // 한국어: 시간에는 스레드 생성·스케줄링·join 비용도 포함된다.
    std::thread a([&] { // 한국어: 첫 워커는 a 카운터만 변경한다. 스레드 생성 실패 복구는 이 예제 범위 밖이다.
        for (unsigned i = 0; i < repetitions; ++i)
            v.a.n.fetch_add(1, std::memory_order_relaxed); // 한국어: relaxed는 이 카운터의 원자성만 제공하며 다른 데이터를 게시하지 않는다.
    });
    std::thread b([&] { // 한국어: 두 번째 워커는 b 카운터만 변경한다.
        for (unsigned i = 0; i < repetitions; ++i)
            v.b.n.fetch_add(1, std::memory_order_relaxed); // 한국어: 다른 atomic 객체여도 하드웨어 일관성 단위를 공유할 수 있다.
    });
    a.join(); // 한국어: 워커 종료와 성공한 join 반환의 동기화를 먼저 확보한다.
    b.join(); // 한국어: 두 워커가 모두 끝난 뒤 결과를 읽는다.
    ns = std::chrono::duration_cast<std::chrono::nanoseconds>( // 한국어: 관찰 시간은 비교 조건을 통제한 성능 증거가 아니다.
        std::chrono::steady_clock::now() - start).count();
    return std::make_pair(v.a.n.load(std::memory_order_relaxed),
                          v.b.n.load(std::memory_order_relaxed));
}

int main(int argc, char** argv) {
    if (argc != 2 || std::string(argv[1]) != "--test") return 2; // 한국어: 반복 횟수를 외부 입력으로 늘리는 벤치마크 인터페이스가 아니다.
    int checks = 0;
    int failed = 0;
    const auto verify = [&](bool ok) {
        ++checks;
        if (!ok) ++failed;
    };
    verify(alignof(FarCell) >= 64); // 한국어: 정렬 계약을 검사하며 실제 캐시의 라인 크기를 탐지하지 않는다.
    verify(sizeof(FarCell) % 64 == 0);
    verify(sizeof(Far) >= 2 * sizeof(FarCell));
    const unsigned cases[] = {0, 1, 7, 10000}; // 한국어: 최대10000회로 제한된 정확성 검사이며 비용이 큰 벤치마크가 아니다.
    for (unsigned n : cases) {
        Near nearby; // 한국어: 매 사례마다 카운터를 새로 0으로 초기화한다.
        Far separated;
        long long near_ns = 0, far_ns = 0;
        const auto x = count(nearby, n, near_ns);
        const auto y = count(separated, n, far_ns);
        verify(x.first == n && x.second == n); // 한국어: 각 워커의 최종 카운터가 반복 횟수와 같은지 확인한다.
        verify(y.first == n && y.second == n);
        verify(x == y); // 한국어: 배치가 달라도 논리 결과는 같아야 한다.
        if (n == 10000) // 한국어: 시간은 기록만 하고 빠름/느림을 PASS 조건으로 삼지 않는다.
            std::cout << "OBSERVE ONLY near_ns=" << near_ns
                      << " far_ns=" << far_ns << '\n';
    }
    std::cout << (failed ? "FAIL " : "PASS ") << checks
              << " OS06 deterministic checks\n";
    return failed ? 1 : 0;
}
