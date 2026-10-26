#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <thread>

int checks = 0;
void Check(bool ok, const char* reason) {
    ++checks;
    if (!ok) {
        throw std::runtime_error(reason);
    }
}

class Cache {
    // const 조회도 내부 캐시를 쓰므로 모든 접근을 같은 잠금으로 보호한다.
    mutable std::mutex mutex_;
    mutable bool ready_ = false;
    mutable int value_ = 0;
public:
    int Get() const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!ready_) {
            value_ = 42;
            ready_ = true;
        }
        return value_;
    }
};

int main() {
    // deferred는 아직 실행되지 않은 상태다. get이 실행을 유발한다.
    auto deferred = std::async(std::launch::deferred, [] { return 7; });
    Check(deferred.wait_for(std::chrono::seconds(0)) == std::future_status::deferred,
          "explicit deferred");
    Check(deferred.get() == 7, "get executes deferred");
    Check(!deferred.valid(), "get consumes future");

    // 작업 내부 예외는 future의 공유 상태를 통해 호출자에게 전달된다.
    auto failing = std::async(std::launch::async, []() -> int {
        throw std::runtime_error("job failed");
    });
    bool propagated = false;
    try {
        (void)failing.get();
    } catch (const std::runtime_error&) {
        propagated = true;
    }
    Check(propagated, "worker exception returned");

    // 참조 캡처 대상인 cache가 소멸하기 전에 get으로 작업을 완료한다.
    Cache cache;
    auto read = std::async(std::launch::async, [&cache] { return cache.Get(); });
    Check(cache.Get() == 42 && read.get() == 42, "const cache synchronized");

    // 값 없는 완료 통지는 한 번만 충족하는 promise/future로 표현할 수 있다.
    std::promise<void> ready;
    auto signal = ready.get_future();
    bool prepared = false;
    {
        std::jthread producer([&] {
            prepared = true;
            ready.set_value();
        });
        // set_value와 성공한 get 사이의 동기화 뒤에만 prepared를 읽는다.
        signal.get();
        Check(prepared, "one-shot notification publishes state");
        Check(!signal.valid(), "notification consumed once");
    }

    std::atomic<int> count{0};
    {
        // count 자체만 공유 결과다. 다른 payload를 게시하는 신호가 아니다.
        std::jthread a([&] {
            for (int i = 0; i < 1000; ++i) {
                count.fetch_add(1, std::memory_order_relaxed);
            }
        });
        std::jthread b([&] {
            for (int i = 0; i < 1000; ++i) {
                count.fetch_add(1, std::memory_order_relaxed);
            }
        });
        // jthread 두 개가 스코프를 나가면서 join한다.
    }
    Check(count.load() == 2000, "joined atomic count");
    std::cout << "tasks: explicit policy / exception / join / atomic\n"
              << "PASS: " << checks << " checks\n";
}
