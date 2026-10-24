#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <iostream>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

class BoundedQueue {
 public:
  explicit BoundedQueue(std::size_t limit) : limit_(limit), closed_(false) {
    if (limit == 0) throw std::invalid_argument("queue capacity");
  }
  bool Push(int x) {
    std::unique_lock<std::mutex> lock(mutex_);
    notFull_.wait(lock, [this]{ return closed_ || items_.size() < limit_; });
    if (closed_) return false;
    items_.push(x);
    lock.unlock();
    notEmpty_.notify_one();
    return true;
  }
  bool Pop(int& out) {
    std::unique_lock<std::mutex> lock(mutex_);
    notEmpty_.wait(lock, [this]{ return closed_ || !items_.empty(); });
    if (items_.empty()) return false;
    out = items_.front(); items_.pop();
    lock.unlock();
    notFull_.notify_one();
    return true;
  }
  void Close() {
    { std::lock_guard<std::mutex> lock(mutex_); closed_ = true; }
    notEmpty_.notify_all(); notFull_.notify_all();
  }
  std::size_t Size() const {
    std::lock_guard<std::mutex> lock(mutex_); return items_.size();
  }
  bool Closed() const {
    std::lock_guard<std::mutex> lock(mutex_); return closed_;
  }
 private:
  const std::size_t limit_;
  bool closed_;
  std::queue<int> items_;
  mutable std::mutex mutex_;
  std::condition_variable notEmpty_, notFull_;
};
static void Check(bool value, const char* name, int& count) {
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
  BoundedQueue blocked(1);
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
  BoundedQueue multi(5);
  std::atomic<int> consumedCount(0); std::atomic<long long> sum(0);
  std::vector<std::thread> readers;
  for (int i=0;i<3;++i) readers.emplace_back([&]{int value=0; while(multi.Pop(value)){sum.fetch_add(value); consumedCount.fetch_add(1);}});
  std::vector<std::thread> writers;
  for (int p=0;p<2;++p) writers.emplace_back([&,p]{for(int i=0;i<100;++i) if(!multi.Push(p*100+i+1)) throw std::runtime_error("premature close");});
  for(auto& th:writers) th.join(); multi.Close();
  for(auto& th:readers) th.join();
  Check(consumedCount.load()==200, "no lost messages", n);
  Check(sum.load()==20100, "sum exact", n);
  Check(multi.Size()==0, "all drained", n);
  Check(multi.Closed(), "multithread close persisted", n);
  std::cout<<"PASS "<<n<<" OS queue CPU concurrency contract checks\n";
}
