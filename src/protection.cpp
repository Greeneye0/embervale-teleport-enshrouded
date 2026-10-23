#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "protection.h"
#include "native_signatures.h"
#include "physics_signatures.h"
#include <mutex>
#include <atomic>
#include <cmath>
#include <cstring>

namespace teleport::protection {
namespace {
template<class T>T& at(void* p,size_t offset){return *reinterpret_cast<T*>(static_cast<uint8_t*>(p)+offset);}
using Next=bool(*)(void*,void*,size_t);
using Entity=uint32_t*(*)(void*,uint32_t*);
using Dirty=void(*)(void*,uint64_t,uint64_t,uint64_t,uint16_t,uint8_t);
Next fallFirst{},fallMore{},gliderFirst{},gliderMore{},gravityFirst{},gravityMore{};
Next motionFirst{},motionMore{};
Next detectionFirst{},detectionMore{};
Entity entityOf{};Dirty dirty{};
struct Entry {void* storage{};uint32_t entity{};unsigned observed{};bool scaled{};unsigned inFlight{};void* actor{};void* damage{};bool grounded{};uint64_t groundSeen{};bool gliderRestored{};};
Entry entries[16]{};
std::mutex mutex;
std::atomic<bool> faulted=false;
bool installed=false;
struct Edit {void* wrapper{};void* component{};void* storage{};uint32_t entity{};};
thread_local Edit gliderEdit,gravityEdit;
void Fault();
void* Storage(void* context){return at<void*>(at<void*>(context,0),0x30);}
uint32_t Id(void* context){uint32_t id=0;return *entityOf(context,&id);}
Entry* Find(void* storage,uint32_t entity) {
    for(auto& entry:entries)if(entry.storage==storage&&entry.entity==entity)return &entry;
    return nullptr;
}
Entry* FindActor(void* storage,uint32_t entity,void* actor) {
    if(!actor)return nullptr;
    if(storage){auto entry=Find(storage,entity);return entry&&entry->actor==actor?entry:nullptr;}
    // Worker contexts omit the registry. Bind through the exact Actor component
    // previously observed by the registry-bearing glider system, never ID alone.
    for(auto& entry:entries)if(entry.storage&&entry.entity==entity&&entry.actor==actor)return &entry;
    return nullptr;
}
void ObservedBit(Entry& entry,unsigned bit) {
    if(entry.observed&bit)return;
    entry.observed|=bit;
    char message[160];sprintf_s(message,"Map Teleport: protection confirmed entity=%u mask=%u registry=%p",entry.entity,entry.observed,entry.storage);
    greeneye::Host()->log(message);
}
// Record each native iterator's first call and each player once per world side.
// This distinguishes an unscheduled system from an entity/registry mismatch
// without changing physics or weakening the protection readiness gate.
void ObserveIterator(unsigned kind,void* context,void* descriptors,bool yielded) {
    static std::atomic<unsigned> calls[4]{},players[4][2]{};
    const char* names[]={"fall damage","glider","gravity","movement"};
    auto authority=unsigned(at<uint8_t>(at<void*>(context,0),0xa1)==0);
    bool first=!(calls[kind].fetch_or(1u<<authority)&(1u<<authority));
    auto id=yielded?Id(context):0;
    bool player=yielded&&id&&id<=16;
    if(player)player=!(players[kind][authority].fetch_or(1u<<(id-1))&(1u<<(id-1)));
    if(!first&&!player)return;
    auto storage=Storage(context);
    bool protectedPlayer=false;
    {std::lock_guard guard(mutex);protectedPlayer=Find(storage,id)!=nullptr;}
    constexpr size_t offsets[]={0x50,0x38,0x28,0x158};
    char message[256];
    sprintf_s(message,"Map Teleport: protection iterator=%s authority=%u yielded=%u entity=%u registry=%p protected=%u component=%u",
        names[kind],authority,unsigned(yielded),id,storage,unsigned(protectedPlayer),unsigned(yielded&&at<void*>(descriptors,offsets[kind])!=nullptr));
    greeneye::Host()->log(message);
}
void SafeObserveIterator(unsigned kind,void* c,void* d,bool ok){
    __try{ObserveIterator(kind,c,d,ok);}__except(EXCEPTION_EXECUTE_HANDLER){greeneye::Host()->log("Map Teleport: protection iterator diagnostic unavailable");}
}
void Mark(void* wrapper) {
    if(auto tracker=at<void*>(wrapper,0x28))dirty(tracker,at<uint64_t>(wrapper,8),at<uint64_t>(wrapper,16),at<uint64_t>(wrapper,24),at<uint16_t>(wrapper,32),at<uint8_t>(wrapper,34));
}
bool SkipDamage(void* context,void* descriptors) {
    std::lock_guard guard(mutex);
    // Infliction has ActorInput, not Actor. Match the exact DynamicFallDamage
    // component bound by fall detection, which has both Actor and damage data.
    auto damage=at<void*>(descriptors,0x50);auto id=Id(context);auto storage=Storage(context);
    Entry* entry=nullptr;
    for(auto& candidate:entries)if(candidate.storage&&(!storage||candidate.storage==storage)&&candidate.entity==id&&damage&&candidate.damage==damage){entry=&candidate;break;}
    if(!entry)return false;
    // Native fall_damage consumes a pending damage fraction at component +0xc.
    // Skipping without clearing it would postpone the hit until protection ends.
    auto wrapper=static_cast<uint8_t*>(descriptors)+0x50;
    if(auto component=at<void*>(wrapper,0)) {
        auto& pending=at<float>(component,0xc);
        if(pending>0){Mark(wrapper);pending=0;}
        ObservedBit(*entry,4);
    }
    return true;
}
bool SkipDetection(void* context,void* descriptors) {
    std::lock_guard guard(mutex);
    auto entry=FindActor(Storage(context),Id(context),at<void*>(descriptors,8));if(!entry)return false;
    auto wrapper=static_cast<uint8_t*>(descriptors)+0x30;
    auto component=at<void*>(wrapper,0);if(!component)return false;
    entry->damage=component;
    // Native fall detection uses Movement+0x59: 2 = airborne, 1 =
    // conditional fluid movement. Only ordinary ground mode (0) qualifies.
    auto movement=at<void*>(descriptors,0x18),velocity=at<void*>(descriptors,0x28);
    entry->grounded=movement&&velocity&&at<uint8_t>(movement,0x59)==0&&
        std::isfinite(at<float>(velocity,4))&&std::abs(at<float>(velocity,4))<0.1f;
    entry->groundSeen=GetTickCount64();
    // Reset tracked fall, jump-reset state, landed distance and pending damage.
    // Do not leave accumulated height to be charged after protection ends.
    Mark(wrapper);at<uint8_t>(component,0)=0;at<uint8_t>(component,1)=0;
    at<float>(component,8)=0;at<float>(component,0xc)=0;
    return true;
}
void BeforeGlider(void* context,void* descriptors) {
    std::lock_guard guard(mutex);
    auto storage=Storage(context);auto id=Id(context);
    if(auto entry=Find(storage,id)) {
        auto actor=at<void*>(descriptors,8);if(!actor)return;
        if(entry->actor&&entry->actor!=actor){
            // ECS migration can move a live Actor component to another address.
            // Only this registry-bearing system may renew the binding. Worker
            // jobs must confirm the new Actor/damage pair before travel resumes.
            if(entry->inFlight){Fault();return;}
            entry->damage=nullptr;entry->observed=0;entry->grounded=false;entry->groundSeen=0;
            greeneye::Host()->log("Map Teleport: player component relocated; refreshing protection binding");
        }
        entry->actor=actor;
        auto wrapper=static_cast<uint8_t*>(descriptors)+0x38;
        if(auto component=at<void*>(wrapper,0)){gliderEdit={wrapper,component,storage,id};++entry->inFlight;}
    }
}
void AfterGlider() {
    auto edit=gliderEdit;gliderEdit={};if(!edit.component)return;
    std::lock_guard guard(mutex);
    if(auto entry=Find(edit.storage,edit.entity)) {
        if(!entry->gliderRestored&&(at<uint32_t>(edit.component,0x34)&0x40)){Mark(edit.wrapper);at<uint32_t>(edit.component,0x34)&=~0x40u;}
        ObservedBit(*entry,1);--entry->inFlight;
    }
}
void BeforeGravity(void* context,void* descriptors) {
    std::lock_guard guard(mutex);
    auto storage=Storage(context);auto id=Id(context);
    if(auto entry=FindActor(storage,id,at<void*>(descriptors,8))) {
        auto wrapper=static_cast<uint8_t*>(descriptors)+0x28;
        if(auto component=at<void*>(wrapper,0)) {
            auto& y=at<float>(component,4);
            // Feed the native integrator its unscaled downward velocity. Halving
            // its output preserves acceleration and terminal speed without an
            // exponential decay from repeatedly halving the previous output.
            if(entry->scaled&&std::isfinite(y)&&y<0){Mark(wrapper);y*=2;}
            gravityEdit={wrapper,component,entry->storage,id};
            ++entry->inFlight;
        }
    }
}
void AfterGravity() {
    auto edit=gravityEdit;gravityEdit={};if(!edit.component)return;
    std::lock_guard guard(mutex);
    // Disable is only valid after confirmed grounded arrival, outside an iterator pass.
    if(auto entry=Find(edit.storage,edit.entity)) {
        auto& y=at<float>(edit.component,4);
        if(std::isfinite(y)&&y<0){Mark(edit.wrapper);y*=0.5f;}
        entry->scaled=true;ObservedBit(*entry,2);--entry->inFlight;
    }
}
void Fault(){faulted=true;greeneye::Host()->log("Map Teleport: protection adapter fault; prank must not start or release protections");}
bool SafeSkip(void* c,void* d){__try{return SkipDamage(c,d);}__except(EXCEPTION_EXECUTE_HANDLER){Fault();return true;}}
bool SafeSkipDetection(void* c,void* d){__try{return SkipDetection(c,d);}__except(EXCEPTION_EXECUTE_HANDLER){Fault();return true;}}
bool Detection(Next next,void* c,void* d,size_t n){bool ok=next(c,d,n);while(ok&&SafeSkipDetection(c,d))ok=next(c,d,n);return ok;}
bool DetectionFirst(void* c,void* d,size_t n){greeneye::Lease lease;return lease?Detection(detectionFirst,c,d,n):detectionFirst(c,d,n);}
bool DetectionMore(void* c,void* d,size_t n){greeneye::Lease lease;return lease?Detection(detectionMore,c,d,n):detectionMore(c,d,n);}
void SafeBeforeGlider(void* c,void* d){__try{BeforeGlider(c,d);}__except(EXCEPTION_EXECUTE_HANDLER){Fault();}}
void SafeAfterGlider(){__try{AfterGlider();}__except(EXCEPTION_EXECUTE_HANDLER){Fault();}}
void SafeBeforeGravity(void* c,void* d){__try{BeforeGravity(c,d);}__except(EXCEPTION_EXECUTE_HANDLER){Fault();}}
void SafeAfterGravity(){__try{AfterGravity();}__except(EXCEPTION_EXECUTE_HANDLER){Fault();}}
bool Fall(Next next,void* c,void* d,size_t n){bool ok=next(c,d,n);SafeObserveIterator(0,c,d,ok);while(ok&&SafeSkip(c,d)){ok=next(c,d,n);SafeObserveIterator(0,c,d,ok);}return ok;}
bool FallFirst(void* c,void* d,size_t n){greeneye::Lease lease;return lease?Fall(fallFirst,c,d,n):fallFirst(c,d,n);}
bool FallMore(void* c,void* d,size_t n){greeneye::Lease lease;return lease?Fall(fallMore,c,d,n):fallMore(c,d,n);}
bool GliderFirst(void* c,void* d,size_t n){greeneye::Lease lease;bool ok=gliderFirst(c,d,n);if(lease){SafeObserveIterator(1,c,d,ok);if(ok)SafeBeforeGlider(c,d);}return ok;}
bool GliderMore(void* c,void* d,size_t n){greeneye::Lease lease;if(lease)SafeAfterGlider();bool ok=gliderMore(c,d,n);if(lease){SafeObserveIterator(1,c,d,ok);if(ok)SafeBeforeGlider(c,d);}return ok;}
bool GravityFirst(void* c,void* d,size_t n){greeneye::Lease lease;bool ok=gravityFirst(c,d,n);if(lease){SafeObserveIterator(2,c,d,ok);if(ok)SafeBeforeGravity(c,d);}return ok;}
bool GravityMore(void* c,void* d,size_t n){greeneye::Lease lease;if(lease)SafeAfterGravity();bool ok=gravityMore(c,d,n);if(lease){SafeObserveIterator(2,c,d,ok);if(ok)SafeBeforeGravity(c,d);}return ok;}
void ObserveMotion(void* c,void* d){
    static std::atomic<unsigned> seen=0;
    auto id=Id(c);if(!id||id>16)return;
    unsigned bit=1u<<(id-1);if(seen.fetch_or(bit)&bit)return;
    char message[192];sprintf_s(message,"Map Teleport: candidate movement observed player=%u authority=%u movement=%u velocity=%u",id,
        unsigned(at<uint8_t>(at<void*>(c,0),0xa1)==0),unsigned(at<void*>(d,0xf8)!=nullptr),unsigned(at<void*>(d,0x158)!=nullptr));
    greeneye::Host()->log(message);
}
void SafeObserveMotion(void* c,void* d){__try{ObserveMotion(c,d);}__except(EXCEPTION_EXECUTE_HANDLER){greeneye::Host()->log("Map Teleport: candidate movement diagnostic unavailable");}}
bool MotionFirst(void* c,void* d,size_t n){greeneye::Lease lease;bool ok=motionFirst(c,d,n);if(lease){SafeObserveIterator(3,c,d,ok);if(ok)SafeObserveMotion(c,d);}return ok;}
bool MotionMore(void* c,void* d,size_t n){greeneye::Lease lease;bool ok=motionMore(c,d,n);if(lease){SafeObserveIterator(3,c,d,ok);if(ok)SafeObserveMotion(c,d);}return ok;}
}
bool Init(const GreeneyeHost* host) {
    auto next=host->find(native::nextPattern,nullptr);
    auto fall=host->find(native::fall_damagePattern,nullptr);
    auto glider=host->find(native::gliderPattern,nullptr);
    auto gravity=host->find(native::gravityPattern,nullptr);
    auto motion=host->find(native::player_motionPattern,nullptr);
    auto detection=host->find(native::fall_detectionPattern,nullptr);
    entityOf=reinterpret_cast<Entity>(host->find(native::entityPattern,nullptr));
    dirty=reinterpret_cast<Dirty>(host->find(native::dirtyPattern,nullptr));
    if(!next||!fall||!glider||!gravity||!motion||!detection||!entityOf||!dirty)return false;
    struct Hook {uint8_t* site;void* detour;Next* original;};
    Hook hooks[]={{fall+0x34,reinterpret_cast<void*>(FallFirst),&fallFirst},{fall+0x4d0,reinterpret_cast<void*>(FallMore),&fallMore},
        {glider+0x2a,reinterpret_cast<void*>(GliderFirst),&gliderFirst},{glider+0x160,reinterpret_cast<void*>(GliderMore),&gliderMore},
        {gravity+0x31,reinterpret_cast<void*>(GravityFirst),&gravityFirst},{gravity+0x25e,reinterpret_cast<void*>(GravityMore),&gravityMore},
        {motion+0x4c,reinterpret_cast<void*>(MotionFirst),&motionFirst},{motion+0x955,reinterpret_cast<void*>(MotionMore),&motionMore},
        {detection+0x34,reinterpret_cast<void*>(DetectionFirst),&detectionFirst},{detection+0x50d,reinterpret_cast<void*>(DetectionMore),&detectionMore}};
    for(auto hook:hooks){uint8_t bytes[5];host->read(hook.site,bytes,5);int32_t relative;memcpy(&relative,bytes+1,4);if(bytes[0]!=0xe8||hook.site+5+relative!=next)return false;}
    for(auto hook:hooks){*hook.original=reinterpret_cast<Next>(host->redirectCall(hook.site,hook.detour));if(!*hook.original)return false;}
    installed=true;return true;
}
bool Enable(void* storage,uint32_t entity) {
    if(!installed||faulted||!storage||!entity||entity>16)return false;
    std::lock_guard guard(mutex);if(Find(storage,entity))return true;
    for(auto& entry:entries)if(!entry.storage){entry={storage,entity,0,false};char message[160];sprintf_s(message,"Map Teleport: protection requested entity=%u registry=%p",entity,storage);greeneye::Host()->log(message);return true;}
    return false;
}
bool Observed(void* storage,uint32_t entity){std::lock_guard guard(mutex);auto entry=Find(storage,entity);return !faulted&&entry&&entry->observed==7;}
void RestoreGlider(void* storage,uint32_t entity){std::lock_guard guard(mutex);if(auto entry=Find(storage,entity))entry->gliderRestored=true;}
bool Grounded(void* storage,uint32_t entity){std::lock_guard guard(mutex);auto entry=Find(storage,entity);return !faulted&&entry&&entry->grounded&&GetTickCount64()-entry->groundSeen<500;}
bool Disable(void* storage,uint32_t entity){std::lock_guard guard(mutex);if(auto entry=Find(storage,entity)){if(entry->inFlight)return false;*entry={};}return true;}
bool Busy(){std::lock_guard guard(mutex);for(auto entry:entries)if(entry.storage)return true;return false;}
bool Faulted(){return faulted;}
}
