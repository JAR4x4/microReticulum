#include "microReticulum.h"
#include "microReticulum/Utilities/ScopedFlag.h"
#include <cassert>
#include <cstdio>
#include <new>
namespace RNS {
struct TransportTestAccess {
 static bool locked() { return Transport::_jobs_locked; }
 static void identity() { Transport::_identity = Identity(); }
 static size_t tags() { return Transport::_discovery_pr_tags.size(); }
 static void clear_tags() { Transport::_discovery_pr_tags.clear(); }
 static void add_tag(const Bytes& b) { Transport::_discovery_pr_tags.insert(b); }
};
}
static long fail_after = -1;
static bool reject_scratch = false;
static bool fail_tlsf_once = false;
extern "C" void* __real_tlsf_malloc(void*, size_t);
extern "C" void* __wrap_tlsf_malloc(void* pool, size_t n) {
 if (fail_tlsf_once) { fail_tlsf_once=false; return nullptr; }
 return __real_tlsf_malloc(pool,n);
}
extern "C" void* __real__Znwm(size_t);
extern "C" void* __wrap__Znwm(size_t n) {
 if (reject_scratch && n == 1024) throw std::bad_alloc();
 if (fail_after == 0) { fail_after = -1; throw std::bad_alloc(); }
 if (fail_after > 0) --fail_after;
 return __real__Znwm(n);
}
class DummyInterface : public RNS::InterfaceImpl {
public: DummyInterface(): InterfaceImpl("fault-injection") {}
 bool send_outgoing(const RNS::Bytes&) override { return true; }
};
int main() {
 using namespace RNS;
 using Utilities::ScopedFlag;
 bool flag=false;
 try { ScopedFlag outer(flag); { ScopedFlag inner(flag); assert(flag); } assert(flag); throw std::bad_alloc(); }
 catch(const std::bad_alloc&) {}
 assert(!flag);
 // Real inbound early return after setting the flag (malformed packet).
 TransportTestAccess::identity();
 Interface incoming(new DummyInterface());
 Bytes malformed("x");
 Transport::inbound(malformed, incoming);
 assert(!TransportTestAccess::locked());
 // Inject failures in real inbound Packet construction/unpacking allocations.
 unsigned caught=0;
 for(long i=0;i<8;++i) {
  fail_after=i;
  try { Transport::inbound(malformed, incoming); } catch(const std::bad_alloc&) { ++caught; }
  fail_after=-1;
  assert(!TransportTestAccess::locked());
 }
 assert(caught>0);
 // Unknown local-link proof must not allocate a maximum-sized path value.
 uint8_t proof[83] = {}; proof[0]=0x0f; proof[18]=0x5a;
 Bytes raw(proof,sizeof(proof));
 reject_scratch=true;
 Transport::inbound(raw, incoming);
 reject_scratch=false;
 assert(!TransportTestAccess::locked());
#if RNS_DEFAULT_ALLOCATOR == 1
 auto tag_before=Utilities::Memory::default_allocator_info;
 for (unsigned i=0;i<20;++i) { uint8_t tag[32]={}; tag[0]=i; TransportTestAccess::add_tag(Bytes(tag,32)); }
 auto tag_after=Utilities::Memory::default_allocator_info;
 std::printf("Retained tag count=%zu default allocations=%u\n", TransportTestAccess::tags(), tag_after.alloc_count-tag_after.free_count-(tag_before.alloc_count-tag_before.free_count));
 TransportTestAccess::clear_tags();
 assert(Utilities::Memory::default_allocator_info.alloc_count-Utilities::Memory::default_allocator_info.free_count == tag_before.alloc_count-tag_before.free_count);
 auto before=Utilities::Memory::default_allocator_info;
 fail_tlsf_once=true;
 try { void* p=::operator new(123); ::operator delete(p); assert(false); }
 catch(const std::bad_alloc&) {}
 auto after=Utilities::Memory::default_allocator_info;
 assert(after.alloc_count==before.alloc_count);
 assert(after.alloc_size==before.alloc_size);
 assert(after.failed_count==before.failed_count+1 && after.last_failed_size==123);
 auto cbefore=Utilities::Memory::container_allocator_info;
 fail_tlsf_once=true;
 try { Utilities::Memory::ContainerAllocator<uint8_t> a; auto p=a.allocate(123); a.deallocate(p,123); assert(false); }
 catch(const std::bad_alloc&) {}
 auto cafter=Utilities::Memory::container_allocator_info;
 assert(cafter.alloc_count==cbefore.alloc_count);
 assert(cafter.alloc_size==cbefore.alloc_size);
 assert(cafter.failed_count==cbefore.failed_count+1 && cafter.last_failed_size==123);
 std::puts("PASS: failed TLSF standard/container allocations do not inflate active counters");
#endif
 std::printf("PASS: nested flags, malformed inbound, %u injected inbound allocation failures\n",caught);
}
