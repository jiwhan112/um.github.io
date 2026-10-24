#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>

// CPU-only educational page-table model. NOT an OS memory manager or MMU emulator.
struct PTE { std::uint64_t frame; bool present; bool writable; };
class PageTable {
 public:
  enum { PAGE_BITS=12 };
  static const std::uint64_t PageSize=4096;
  void Map(std::uint64_t page, std::uint64_t frame, bool writable) {
    pages_[page]=PTE{frame,true,writable};
  }
  void Unmap(std::uint64_t page) {pages_.erase(page);}
  void MarkAbsent(std::uint64_t page) {
    std::map<std::uint64_t,PTE>::iterator it=pages_.find(page);
    if(it==pages_.end()) throw std::invalid_argument("unknown page");
    it->second.present=false;
  }
  std::uint64_t Translate(std::uint64_t address, bool write=false) const {
    const std::uint64_t vpn=address/PageSize;
    const std::uint64_t offset=address%PageSize;
    std::map<std::uint64_t,PTE>::const_iterator it=pages_.find(vpn);
    if(it==pages_.end() || !it->second.present) throw std::out_of_range("page not present");
    if(write && !it->second.writable) throw std::runtime_error("write protection");
    const std::uint64_t max=(std::numeric_limits<std::uint64_t>::max)();
    if(it->second.frame>(max-offset)/PageSize) throw std::overflow_error("physical address overflow");
    return it->second.frame*PageSize+offset;
  }
 private:
  std::map<std::uint64_t,PTE> pages_;
};
struct Test {int count; Test():count(0){} void ok(bool pass){++count;if(!pass)throw std::runtime_error("failed assertion");}};
template<class F> bool Throws(F fn){try{fn();return false;}catch(const std::exception&){return true;}}
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
  p.Map(3,7,true); // alias: two virtual pages can reference the same frame.
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
  std::uint64_t hi=(std::numeric_limits<std::uint64_t>::max)()/4096;
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
