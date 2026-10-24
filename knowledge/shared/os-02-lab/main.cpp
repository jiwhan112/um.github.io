#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>

// CPU-only educational page-table model. NOT an OS memory manager or MMU emulator.
struct PTE { std::uint64_t frame; bool present; bool writable; }; // [한국어 해설] 프레임 번호와 존재/쓰기 권한만 담는 교육용 항목이다. 실제 MMU 비트 배치가 아니다.
class PageTable {
 public:
  enum { PAGE_BITS=12 };
  static const std::uint64_t PageSize=4096; // [한국어 해설] 이 모델의 고정 4KiB 가정이다. 모든 시스템의 페이지 크기를 뜻하지 않는다.
  void Map(std::uint64_t page, std::uint64_t frame, bool writable) { // [한국어 해설] 같은 VPN의 이전 항목을 덮어쓰며 다시 present로 만든다. 다른 VPN이 같은 PFN을 공유해도 허용한다.
    pages_[page]=PTE{frame,true,writable};
  }
  void Unmap(std::uint64_t page) {pages_.erase(page);} // [한국어 해설] 없는 VPN을 지워도 예외가 없는 멱등 연산이다.
  void MarkAbsent(std::uint64_t page) { // [한국어 해설] 항목 자체는 유지하고 present만 끈다. 없는 항목에 호출하면 오류다.
    std::map<std::uint64_t,PTE>::iterator it=pages_.find(page);
    if(it==pages_.end()) throw std::invalid_argument("unknown page");
    it->second.present=false;
  }
  std::uint64_t Translate(std::uint64_t address, bool write=false) const { // [한국어 해설] 주소를 숫자로 변환할 뿐 해당 주소를 포인터로 역참조하지 않는다.
    const std::uint64_t vpn=address/PageSize; // [한국어 해설] 4096으로 나눈 몫은 VPN이고 나머지는 페이지 내부 offset이다.
    const std::uint64_t offset=address%PageSize;
    std::map<std::uint64_t,PTE>::const_iterator it=pages_.find(vpn);
    if(it==pages_.end() || !it->second.present) throw std::out_of_range("page not present"); // [한국어 해설] 미매핑과 present=false를 같은 예외로 표현한다. 실제 OS의 복구/종료 정책은 구현하지 않는다.
    if(write && !it->second.writable) throw std::runtime_error("write protection"); // [한국어 해설] 읽기 전용 페이지에 대한 쓰기를 거부한다. COW를 수행하는 코드는 아니다.
    const std::uint64_t max=(std::numeric_limits<std::uint64_t>::max)();
    if(it->second.frame>(max-offset)/PageSize) throw std::overflow_error("physical address overflow"); // [한국어 해설] 곱셈 전에 나눗셈으로 범위를 검사하여 unsigned wraparound를 주소로 반환하지 않는다.
    return it->second.frame*PageSize+offset;
  }
 private:
  std::map<std::uint64_t,PTE> pages_;
};
struct Test {int count; Test():count(0){} void ok(bool pass){++count;if(!pass)throw std::runtime_error("failed assertion");}}; // [한국어 해설] 검사 호출 수를 세며 예외 메시지 문자열은 원본 그대로 보존한다.
template<class F> bool Throws(F fn){try{fn();return false;}catch(const std::exception&){return true;}} // [한국어 해설] 아무 std::exception이나 참으로 인정한다. 테스트가 정확한 예외 종류까지 확인하는 것은 아니다.
int main(){
  Test t; PageTable p;
  t.ok(PageTable::PageSize==4096); t.ok(PageTable::PAGE_BITS==12);
  t.ok(Throws([&]{p.Translate(0); }));
  p.Map(0,7,false);
  t.ok(p.Translate(0)==7*4096);
  t.ok(p.Translate(1)==7*4096+1);
  t.ok(p.Translate(4095)==7*4096+4095);
  t.ok(Throws([&]{p.Translate(4096); }));
  t.ok(Throws([&]{p.Translate(10,true); }));
  t.ok(p.Translate(10,false)==7*4096+10);
  p.Map(1,9,true);
  t.ok(p.Translate(4096)==9*4096);
  t.ok(p.Translate(4097,true)==9*4096+1);
  t.ok(p.Translate(8191,true)==9*4096+4095);
  t.ok(Throws([&]{p.Translate(8192); }));
  p.Map(3,7,true); // alias: two virtual pages can reference the same frame. // [한국어 해설] 별칭은 허용하되 두 매핑의 쓰기 권한은 서로 다를 수 있다.
  t.ok(p.Translate(3*4096)==p.Translate(0));
  t.ok(p.Translate(3*4096+17)==p.Translate(17));
  t.ok(p.Translate(3*4096+17,true)==p.Translate(17));
  p.MarkAbsent(1);
  t.ok(Throws([&]{p.Translate(4096); }));
  t.ok(Throws([&]{p.Translate(4096,true); }));
  p.Map(1,15,true);
  t.ok(p.Translate(4096)==15*4096);
  p.Unmap(0);t.ok(Throws([&]{p.Translate(0); }));
  t.ok(Throws([&]{p.MarkAbsent(0); }));
  p.Unmap(0);t.ok(p.Translate(3*4096)==7*4096);
  std::uint64_t hi=(std::numeric_limits<std::uint64_t>::max)()/4096; // [한국어 해설] 표현 가능한 최대 프레임에서 offset 4095까지 허용하고 그 다음 프레임을 거부한다.
  p.Map(10,hi,true);
  t.ok(p.Translate(10*4096)==hi*4096);
  t.ok(p.Translate(10*4096+4095)==(std::numeric_limits<std::uint64_t>::max)());
  p.Map(11,hi+1,true);
  t.ok(Throws([&]{p.Translate(11*4096); }));
  t.ok(Throws([&]{p.Translate(11*4096+2,true); }));
  t.ok(p.Translate(4096+200)==15*4096+200);
  t.ok(p.Translate(3*4096+4095)==7*4096+4095);
  t.ok(Throws([&]{p.Translate((std::numeric_limits<std::uint64_t>::max)()); }));
  p.Map(9,0,true); t.ok(p.Translate(9*4096,true)==0);
  std::cout<<"PASS "<<t.count<<" virtual-memory page-table CPU checks\n";
}
