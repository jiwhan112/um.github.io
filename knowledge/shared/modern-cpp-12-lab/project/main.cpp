// 종료 요청과 완료 확인은 다르다. sleep 타이밍에 의존하지 않고 join으로 확인한다.
#include <atomic>
#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <stop_token>
#include <thread>
int main() {
    unsigned checks = 0;
    auto check = [&](bool ok) {
        ++checks;
        if (!ok) throw std::runtime_error("stop contract");
    };
    std::mutex mutex;
    std::condition_variable_any cv;
    bool job_ready = false;
    // mutex 아래서만 읽거나 쓴다.
    std::atomic<bool> saw_stop = false;
    std::promise<void> started;
    auto startup = started.get_future();
    // 참조 캡처 대상들은 아래 join이 끝날 때까지 모두 생존한다.
    std::jthread worker([&](std::stop_token token) {
        std::unique_lock lock(mutex);
        started.set_value();
        const bool has_job = cv.wait(lock, token, [&] {
            return job_ready;
        });
        // 작업 공급이 없으므로 종료 요청이 대기를 깨워 false를 돌려준다.
        saw_stop.store(!has_job && token.stop_requested(), std::memory_order_relaxed);
    });
    startup.get();
    // 작업 스레드가 시작했음을 확인하며 임의 sleep을 쓰지 않는다.
    check(worker.joinable());
    check(worker.request_stop());
    check(!worker.request_stop());
    // 이미 요청된 동일 stop state.
    worker.join();
    // 참조 캡처된 지역 객체들보다 먼저 스레드를 종료한다.
    check(!worker.joinable());
    check(saw_stop.load(std::memory_order_relaxed));
    std::stop_source source;
    unsigned callback_calls = 0;
    std::stop_callback callback(source.get_token(), [&] {
        ++callback_calls;
    });
    check(source.request_stop());
    // 이 예제에서는 요청한 현재 스레드가 콜백을 실행한다.
    check(callback_calls == 1 && !source.request_stop());
    std::cout << "stop-requested joined=true callback=1\nPASS: " << checks << " contract checks\n";
}
