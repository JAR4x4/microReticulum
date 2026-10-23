#include <cstdlib>
#include <cstdio>
#include <new>
#include <cstddef>
#include <cassert>
#include <memory>
#include <microReticulum/Link.h>
#include <microReticulum/LinkData.h>
#include <microReticulum/Resource.h>
#include <microReticulum/ResourceData.h>
#include <microReticulum/Transport.h>
#include <microReticulum/Utilities/OS.h>
using namespace RNS;
static long countdown=-1;
static size_t live_bytes=0;
struct alignas(std::max_align_t) Header {size_t size;};
void* operator new(size_t n) {if(countdown==0)throw std::bad_alloc();if(countdown>0)--countdown;auto p=static_cast<Header*>(malloc(sizeof(Header)+n));if(!p)throw std::bad_alloc();p->size=n;live_bytes+=n;return p+1;}
void operator delete(void* p) noexcept {if(p){auto h=static_cast<Header*>(p)-1;live_bytes-=h->size;free(h);}}
void operator delete(void* p,size_t) noexcept {::operator delete(p);}
void* operator new[](size_t n){return ::operator new(n);}
void operator delete[](void* p) noexcept {::operator delete(p);}
void operator delete[](void* p,size_t) noexcept {::operator delete(p);}
static Bytes hex(const char* s){Bytes b;b.assignHex(s);return b;}
static Link link_fixture(){
 Link l(Type::NONE); l._object=std::make_shared<LinkData>(Destination(Type::NONE));
 auto& d=*l._object; d._link_id=hex("00112233445566778899aabbccddeeff");d._hash=d._link_id;
 d._derived_key=hex("00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff");
 d._mdu=400;d._status=Type::Link::ACTIVE;d._last_inbound=Utilities::OS::time();d._activated_at=d._last_inbound;
 return l;
}
static Resource resource_fixture(Link& l,bool sender){
 Resource r(Bytes("NomadNet test payload"),l);r._object->_hash=hex(sender?"aa":"bb");
 r._object->_initiator=sender;r._object->_status=Type::Resource::TRANSFERRING;
 if(sender) l.register_outgoing_resource(r);else l.register_incoming_resource(r);
 return r;
}
static void cancel_no_heap(bool sender){
 auto l=link_fixture();auto r=resource_fixture(l,sender);std::weak_ptr<ResourceData> weak=r._object;
 countdown=0;r.cancel();r.cancel();countdown=-1;
 assert(r.status()==Type::Resource::FAILED);assert(l._object->_outgoing_resources.empty()&&l._object->_incoming_resources.empty());
 r=Resource(Type::NONE);assert(weak.expired());l.teardown();
}
static void close_no_heap(){
 auto l=link_fixture();auto a=resource_fixture(l,true),b=resource_fixture(l,false);
 std::weak_ptr<ResourceData> wa=a._object,wb=b._object;
 a=Resource(Type::NONE);b=Resource(Type::NONE);
 countdown=0;l.teardown();countdown=-1;
 assert(l.status()==Type::Link::CLOSED);assert(wa.expired()&&wb.expired());
 assert(l._object->_incoming_resources.empty()&&l._object->_outgoing_resources.empty());
}
static void timeout_no_heap(){
 auto l=link_fixture();auto r=resource_fixture(l,true);std::weak_ptr<ResourceData>w=r._object;r=Resource(Type::NONE);
 l._object->_last_inbound=0;l._object->_last_proof=0;l._object->_activated_at=0;
 countdown=0;l.tick_resources();countdown=-1;
 assert(l.status()==Type::Link::CLOSED&&w.expired());
}
static void wire_close_and_receive_failure(){
 auto l=link_fixture();auto r=resource_fixture(l,true);std::weak_ptr<ResourceData>w=r._object;r=Resource(Type::NONE);
 Packet close(l,l._object->_link_id);close.context(Type::Packet::LINKCLOSE);close.pack();
 Packet incoming(close.raw());assert(incoming.unpack());
 countdown=0;
 try{l.receive(incoming);}catch(const std::exception&){}
 countdown=-1;
 assert(!l._object->_watchdog_lock);
 if(l.status()!=Type::Link::CLOSED)l.receive(incoming);
 assert(l.status()==Type::Link::CLOSED&&w.expired());
}
static void transfer_timeout(bool sender,Type::Resource::status state){
 auto l=link_fixture();auto r=resource_fixture(l,sender);std::weak_ptr<ResourceData>w=r._object;
 r._object->_status=state;r._object->_last_activity=0;r._object->_last_part_sent=0;r._object->_retries_left=0;
 r.__watchdog_job();assert(r.status()==Type::Resource::FAILED);
 assert(l._object->_outgoing_resources.empty()&&l._object->_incoming_resources.empty());
 r=Resource(Type::NONE);assert(w.expired());l.teardown();
}
int main(){
 cancel_no_heap(true);cancel_no_heap(false);close_no_heap();timeout_no_heap();wire_close_and_receive_failure();
 transfer_timeout(true,Type::Resource::ADVERTISED);transfer_timeout(true,Type::Resource::TRANSFERRING);transfer_timeout(true,Type::Resource::AWAITING_PROOF);transfer_timeout(false,Type::Resource::TRANSFERRING);
 auto baseline=live_bytes;
 for(int i=0;i<1000;++i){cancel_no_heap(true);cancel_no_heap(false);close_no_heap();timeout_no_heap();}
 assert(live_bytes==baseline);
 puts("PASS: real encrypted LINKCLOSE, receive-lock release after exception, advertisement/transfer/proof and incoming-transfer timeout cleanup");
 printf("PASS: 4000 lifecycle/cleanup repetitions, allocation denial during cancellation/closure/timeout, owning references released; live byte delta=%zd\n",live_bytes-baseline);
 // Sweep every early allocation boundary of actual Resource::start(), not a mocked copy.
 // No interfaces are registered: this tests local preparation and failure cleanup only.
 int exceptions=0,completed=0,failed_cleanly=0;
 for(int fail=0;fail<220;++fail){
  auto l=link_fixture();unsigned char buf[541]={};Bytes reply(buf,sizeof(buf));Resource r(reply,l);
  std::weak_ptr<ResourceData> w=r._object;
  countdown=fail;
  try{r.request_id(hex("0123456789abcdef0123456789abcdef")).is_response(true).start();++completed;if(r.status()==Type::Resource::FAILED)++failed_cleanly;}
  catch(const std::exception&){++exceptions;}
  countdown=-1;
  r.cancel();l.teardown();r=Resource(Type::NONE);
  assert(w.expired());assert(l._object->_incoming_resources.empty()&&l._object->_outgoing_resources.empty());
 }
 printf("Cleanly failed after unsent advertisement: %d\n",failed_cleanly);
 printf("PASS: 220 Resource::start allocation-failure positions; exceptions=%d completed=%d; no retained resource owner; total live-byte delta=%zd\n",exceptions,completed,live_bytes-baseline);
 assert(live_bytes==baseline);
}
