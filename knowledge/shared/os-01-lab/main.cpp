#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <iostream>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

class BoundedQueue { // [한국어 해설] 정수 작업을 담는 유한 큐다. 상태와 두 대기 조건은 같은 mutex로 보호한다.
 public:
  explicit BoundedQueue(std::size_t limit) : limit_(limit), closed_(false) { // [한국어 해설] 용량 0은 영원히 넣을 수 없는 큐이므로 생성 시 거부한다.
    if (limit == 0) throw std::invalid_argument("queue capacity");
  }
  bool Push(int x) { // [한국어 해설] 가득 차면 대기한다. 깨어나도 종료 또는 빈자리 조건을 락 아래 다시 검사한다.
    std::unique_lock<std::mutex> lock(mutex_);
    notFull_.wait(lock, [this]{ return closed_ || items_.size() < limit_; });
    if (closed_) return false; // [한국어 해설] Close 이후 새 작업을 받지 않는다. 이 분기는 기존 대기자에도 적용된다.
    items_.push(x); // [한국어 해설] 공유 상태 변경이 먼저다. 삽입 실패 예외는 호출자에게 전달되며 unique_lock이 잠금을 해제한다.
    lock.unlock();
    notEmpty_.notify_one();
    return true;
  }
  bool Pop(int& out) { // [한국어 해설] 닫혔어도 남은 작업은 배출한다. 닫힘과 비어 있음이 함께 성립해야 false다.
    std::unique_lock<std::mutex> lock(mutex_);
    notEmpty_.wait(lock, [this]{ return closed_ || !items_.empty(); });
    if (items_.empty()) return false; // [한국어 해설] wait의 조건과 합치면 닫힌 빈 큐임을 알 수 있다.
    out = items_.front(); items_.pop();
    lock.unlock();
    notFull_.notify_one();
    return true;
  }
  void Close() { // [한국어 해설] 상태 변경은 잠금 안에서, 두 종류 대기자의 깨움은 잠금 밖에서 수행한다.
    { std::lock_guard<std::mutex> lock(mutex_); closed_ = true; }
    notEmpty_.notify_all(); notFull_.notify_all();
  }
  std::size_t Size() const { // [한국어 해설] 이 순간의 크기일 뿐 다음 Push/Pop 성공을 예약하지 않는다.
    std::lock_guard<std::mutex> lock(mutex_); return items_.size();
  }
  bool Closed() const { // [한국어 해설] 읽기도 같은 mutex로 보호한다. 호출 결과와 이후 연산은 별도 시점이다.
    std::lock_guard<std::mutex> lock(mutex_); return closed_;
  }
 private:
  const std::size_t limit_;
  bool closed_;
  std::queue<int> items_;
  mutable std::mutex mutex_;
  std::condition_variable notEmpty_, notFull_;
};
static void Check(bool value, const char* name, int& count) { // [한국어 해설] 통과한 명시 조건을 센다. 컴파일러별 반복 실행을 독립 조건으로 더하지 않는다.
  if (!value) throw std::runtime_error(name); ++count;
}
int main() {
  int n = 0;
  bool threw = false;
  try { BoundedQueue invalid(0); } catch (const std::invalid_argument&) { threw = true; }
  Check(threw, "zero capacity rejected", n);
  BoundedQueue q(2);
  Check(q.Size()==0, "initial empty", n);
  Check(!q.Closed(), "initial open", n);
  Check(q.Push(7), "push first", n);
  Check(q.Push(-9), "push second", n);
  Check(q.Size()==2, "at capacity", n);
  int a=0;
  Check(q.Pop(a), "pop first", n);
  Check(a==7, "FIFO", n);
  Check(q.Size()==1, "size decremented", n);
  Check(q.Pop(a), "pop second", n);
  Check(a==-9, "negative preserved", n);
  Check(q.Size()==0, "empty after pop", n);
  Check(q.Push(44), "push before close", n);
  q.Close();
  Check(q.Closed(), "closed state", n);
  Check(!q.Push(3), "push rejected after close", n);
  Check(q.Pop(a), "drain after close", n);
  Check(a==44, "drain value", n);
  Check(!q.Pop(a), "drained queue returns false", n);
  q.Close();
  Check(q.Closed(), "close idempotent", n);
  BoundedQueue blocked(1); // [한국어 해설] 아래 테스트는 Close와 작업 스레드의 실행 순서를 고정하지 않는다. 실제 wait 진입 여부까지 증명하지 않는다.
  blocked.Push(55);
  std::atomic<bool> producerReturned(false), producerResult(true);
  std::thread producer([&]{ producerResult.store(blocked.Push(66)); producerReturned.store(true); });
  blocked.Close(); producer.join();
  Check(producerReturned.load(), "blocked producer woke", n);
  Check(!producerResult.load(), "blocked producer declined closed queue", n);
  Check(blocked.Pop(a) && a==55, "preclose element drain", n);
  Check(!blocked.Pop(a), "closed empty no wait", n);
  BoundedQueue empty(1);
  std::atomic<bool> popResult(true);
  std::thread consumer([&]{int x=0; popResult.store(empty.Pop(x));});
  empty.Close(); consumer.join();
  Check(!popResult.load(), "waiting consumer wakes on close", n);
  BoundedQueue multi(5); // [한국어 해설] 두 생산자가 1~200을 넣고 세 소비자가 배출한다. 개별 처리 순서는 비결정적이다.
  std::atomic<int> consumedCount(0); std::atomic<long long> sum(0);
  std::vector<std::thread> readers;
  for (int i=0;i<3;++i) readers.emplace_back([&]{int value=0; while(multi.Pop(value)){sum.fetch_add(value); consumedCount.fetch_add(1);}});
  std::vector<std::thread> writers;
  for (int p=0;p<2;++p) writers.emplace_back([&,p]{for(int i=0;i<100;++i) if(!multi.Push(p*100+i+1)) throw std::runtime_error("premature close");});
  for(auto& th:writers) th.join(); multi.Close(); // [한국어 해설] 생산자 완료 후 닫고 소비자를 join한다. 큐가 스레드보다 먼저 파괴되면 안 된다.
  for(auto& th:readers) th.join();
  Check(consumedCount.load()==200, "no lost messages", n);
  Check(sum.load()==20100, "sum exact", n); // [한국어 해설] 개수와 합은 이 실행의 관찰치다. 모든 가능한 스케줄이나 모든 메시지 고유성을 증명하지 않는다.
  Check(multi.Size()==0, "all drained", n);
  Check(multi.Closed(), "multithread close persisted", n);
  std::cout<<"PASS "<<n<<" OS queue CPU concurrency contract checks\n";
}
