#include "microReticulum.h"
#include "microReticulum/Utilities/ScopedFlag.h"
#include "microReticulum/LinkData.h"
#include "microReticulum/Utilities/ScopedCounter.h"
#include <cassert>
#include <cstdio>
#include <new>
namespace RNS {
struct DestinationAdmissionTestAccess {
 static void track(Destination& d, const Link& l) { d._object->_links.insert(l); }
 static size_t& reservations(Destination& d) { return d._object->_incoming_link_reservations; }
 static void clear(Destination& d) { d._object->_links.clear(); }
};
struct TransportTestAccess {
 static bool seen(const Bytes& hash) { return Transport::_packet_hashlist.exists(hash); }
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
class TestIncomingLink : public RNS::Link {
public:
 explicit TestIncomingLink(RNS::Type::Link::status state) : Link(RNS::Type::NONE) {
  _object.reset(new RNS::LinkData(RNS::Destination(RNS::Type::NONE)));
  status(state);
 }
};
static void test_destination_admission() {
 using namespace RNS;
 Identity identity;
 Destination site(identity,Type::Destination::IN,Type::Destination::SINGLE,"admission-test","node");
 Destination management(identity,Type::Destination::IN,Type::Destination::SINGLE,"admission-test","management");
 TestIncomingLink pending(Type::Link::PENDING), active(Type::Link::ACTIVE);
 DestinationAdmissionTestAccess::track(site,pending);
 DestinationAdmissionTestAccess::track(site,active);
 assert(site.incoming_link_limit()==0 && site.can_accept_incoming_link());
 site.incoming_link_limit(2);
 assert(site.incoming_link_count()==2 && !site.can_accept_incoming_link());
 assert(management.can_accept_incoming_link());
 Packet unused(Type::NONE); Bytes empty;
 fail_after=0; // Any operator-new call in admission would fail this test.
 for(int i=0;i<1000;++i) site.incoming_link_request(empty,unused);
 assert(fail_after==0); fail_after=-1;
 assert(site.incoming_link_rejections()==1000);
 // Real Transport must refuse before recording a duplicate hash, so the
 // same LR can retry once capacity recovers.
 std::vector<uint8_t> lr(83,0); lr[0]=0x02;
 std::copy(site.hash().data(),site.hash().data()+16,lr.begin()+2);
 Bytes lr_raw(lr); Packet lr_packet(lr_raw); assert(lr_packet.unpack());
 DummyInterface* impl = new DummyInterface(); Interface radio(impl);
 Transport::inbound(lr_raw,radio); Transport::inbound(lr_raw,radio);
 assert(!TransportTestAccess::seen(lr_packet.packet_hash()));
 assert(site.incoming_link_rejections()==1002);
 assert(site.incoming_link_count()==2);
 pending.status(Type::Link::STALE);
 assert(!site.can_accept_incoming_link());
 pending.status(Type::Link::CLOSED);
 assert(site.incoming_link_count()==1 && site.can_accept_incoming_link());
 site.incoming_link_limit(1); assert(!site.can_accept_incoming_link());
 site.remove_link(active); assert(site.can_accept_incoming_link());
 try {
  Utilities::ScopedCounter reservation(DestinationAdmissionTestAccess::reservations(site));
  assert(!site.can_accept_incoming_link());
  fail_after=0;
  site.incoming_link_request(empty,unused); // Nested attempt cannot pass the reserved slot.
  assert(fail_after==0); fail_after=-1;
  throw std::bad_alloc();
 } catch(const std::bad_alloc&) {}
 assert(site.incoming_link_count()==0 && site.can_accept_incoming_link());
 assert(site.incoming_link_rejections()==1003);
 site.accepts_links(false); assert(!site.can_accept_incoming_link());
 site.accepts_links(true); site.incoming_link_limit(0); assert(site.can_accept_incoming_link());
 DestinationAdmissionTestAccess::clear(site);
 std::puts("PASS: bounded destination links, pending/stale counting, slot recovery, management isolation,1001 allocation-free rejections, retry-safe Transport admission and exception-safe reservations");
}
int main() {
 using namespace RNS;
 using Utilities::ScopedFlag;
 bool flag=false;
 try { ScopedFlag outer(flag); { ScopedFlag inner(flag); assert(flag); } assert(flag); throw std::bad_alloc(); }
 catch(const std::bad_alloc&) {}
 assert(!flag);
 // Real inbound early return after setting the flag (malformed packet).
 TransportTestAccess::identity();
 test_destination_admission();
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
