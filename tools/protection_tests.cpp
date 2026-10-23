#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "../src/protection.h"
#include "../src/native_signatures.h"
#include "../src/physics_signatures.h"
#include <cassert>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <cstdio>

namespace {
using Next=bool(*)(void*,void*,size_t);
uint8_t functions[5][4096]{};
Next hooks[10]{};
unsigned hooksAdded=0,cursor=0,kind=0;
struct Context {void* runtime;uint32_t id;};
uint8_t runtime[0xc0]{},descriptors[0x100]{},components[2][0x80]{};
int storage=0,otherStorage=0;
int actors[2]{},unrelatedActor{};
void* actorOverride{};
uint8_t movement[0x80]{},velocity[16]{};
uint8_t unrelatedDamage[0x80]{};void* damageOverride{};
template<class T>void Put(void* p,size_t n,T value){memcpy(static_cast<uint8_t*>(p)+n,&value,sizeof value);}
template<class T>T Get(void* p,size_t n){T value;memcpy(&value,static_cast<uint8_t*>(p)+n,sizeof value);return value;}
uint32_t* Entity(void* c,uint32_t* result){*result=static_cast<Context*>(c)->id;return result;}
void Dirty(void*,uint64_t,uint64_t,uint64_t,uint16_t,uint8_t){}
bool Iterate(void* c,void* d,size_t) {
    if(cursor>=2)return false;
    static_cast<Context*>(c)->id=++cursor;
    if(kind==4){Put(d,0x18,static_cast<void*>(movement));Put(d,0x28,static_cast<void*>(velocity));}
    Put(d,kind==0?0x20:8,actorOverride?actorOverride:static_cast<void*>(&actors[cursor-1]));
    Put(d,kind==0?0x50:kind==1?0x38:kind==4?0x30:0x28,damageOverride?damageOverride:static_cast<void*>(components[cursor-1]));return true;
}
uint8_t* Find(const char* pattern,void*) {
    using namespace teleport::native;
    if(!strcmp(pattern,fall_damagePattern))return functions[0];
    if(!strcmp(pattern,gliderPattern))return functions[1];
    if(!strcmp(pattern,gravityPattern))return functions[2];
    if(!strcmp(pattern,player_motionPattern))return functions[3];
    if(!strcmp(pattern,fall_detectionPattern))return functions[4];
    if(!strcmp(pattern,nextPattern))return functions[0]+1800;
    if(!strcmp(pattern,entityPattern))return reinterpret_cast<uint8_t*>(Entity);
    if(!strcmp(pattern,dirtyPattern))return reinterpret_cast<uint8_t*>(Dirty);
    return nullptr;
}
void Read(const void* from,void* to,size_t n){memcpy(to,from,n);}
void* Redirect(uint8_t*,void* detour){assert(hooksAdded<10);hooks[hooksAdded++]=reinterpret_cast<Next>(detour);return reinterpret_cast<void*>(Iterate);}
bool Enter(void*){return true;}
void Leave(){}
void Log(const char* message){assert(!strstr(message,"fault")&&!strstr(message,"unavailable"));}
void PrepareSites() {
    const size_t offsets[5][2]={{0x34,0x4d0},{0x2a,0x160},{0x31,0x25e},{0x4c,0x955},{0x34,0x50d}};
    for(unsigned i=0;i<5;++i)for(auto offset:offsets[i]) {
        auto site=functions[i]+offset;site[0]=0xe8;
        Put(site,1,static_cast<int32_t>((functions[0]+1800)-(site+5)));
    }
}
}
int main() {
    using namespace teleport::protection;
    GreeneyeHost host{};host.size=sizeof host;host.apiVersion=2;host.modDir=L"unused";
    host.enter=Enter;host.leave=Leave;host.find=Find;host.read=Read;host.redirectCall=Redirect;
    host.hook=[](uint8_t*,void*,size_t,const uint8_t*,size_t)->void*{return nullptr;};host.log=Log;
    greeneye::UseHost(&host);PrepareSites();assert(Init(&host));assert(hooksAdded==10);
    Put(runtime,0x30,static_cast<void*>(&storage));Context context{runtime,0};
    assert(!Enable(nullptr,1)&&!Enable(&storage,0)&&!Enable(&storage,17));
    assert(Enable(&storage,1)&&Busy()&&!Observed(&storage,1));
    kind=0;cursor=0;Put<void*>(runtime,0x30,nullptr);
    assert(hooks[0](&context,descriptors,sizeof descriptors)&&context.id==1);
    assert(!Observed(&storage,1)); // entity ID alone must never grant protection
    Put(runtime,0x30,static_cast<void*>(&storage));
    // Bind the actor through the registry-bearing system before worker jobs run.
    kind=1;cursor=0;assert(hooks[2](&context,descriptors,sizeof descriptors));
    assert(hooks[3](&context,descriptors,sizeof descriptors));
    assert(!hooks[3](&context,descriptors,sizeof descriptors));
    // Fall detection binds the damage component via Actor, resetting accumulated fall.
    Put<void*>(runtime,0x30,nullptr);kind=4;cursor=0;
    Put<uint8_t>(components[0],0,1);Put<float>(components[0],8,100.f);Put<float>(components[0],0xc,.9f);
    actorOverride=&unrelatedActor;
    assert(hooks[8](&context,descriptors,sizeof descriptors)&&context.id==1);
    assert(Get<float>(components[0],8)==100.f);
    actorOverride=nullptr;cursor=0;
    assert(hooks[8](&context,descriptors,sizeof descriptors)&&context.id==2);
    assert(Get<uint8_t>(components[0],0)==0&&Get<float>(components[0],8)==0&&Get<float>(components[0],0xc)==0);
    assert(!hooks[9](&context,descriptors,sizeof descriptors));
    // Both players are yielded by the native iterator; only player 1 is protected.
    kind=0;cursor=0;
    Put<void*>(runtime,0x30,nullptr); // real fall/gravity workers omit the registry
    Put<float>(components[0],0xc,0.9f);Put<float>(components[1],0xc,0.7f);
    assert(hooks[0](&context,descriptors,sizeof descriptors)&&context.id==2);
    assert(Get<float>(components[0],0xc)==0&&Get<float>(components[1],0xc)==0.7f);
    assert(!hooks[1](&context,descriptors,sizeof descriptors));
    damageOverride=unrelatedDamage;cursor=0;
    assert(hooks[0](&context,descriptors,sizeof descriptors)&&context.id==1);
    damageOverride=nullptr; // same ID in another worker world must not match
    // A different world's entity 1 must not inherit this world's protection.
    Put(runtime,0x30,static_cast<void*>(&otherStorage));cursor=0;
    assert(hooks[0](&context,descriptors,sizeof descriptors)&&context.id==1);
    Put(runtime,0x30,static_cast<void*>(&storage));
    kind=1;cursor=0;
    assert(hooks[2](&context,descriptors,sizeof descriptors));
    assert(!Disable(&storage,1)); // cannot remove protection inside native update
    Put<uint32_t>(components[0],0x34,0x140);
    assert(hooks[3](&context,descriptors,sizeof descriptors));
    assert(Get<uint32_t>(components[0],0x34)==0x100);
    Put<uint32_t>(components[1],0x34,0x140);
    assert(!hooks[3](&context,descriptors,sizeof descriptors));
    assert(Get<uint32_t>(components[1],0x34)==0x140);
    RestoreGlider(&storage,1);
    kind=1;cursor=0;assert(hooks[2](&context,descriptors,sizeof descriptors));
    Put<uint32_t>(components[0],0x34,0x140);hooks[3](&context,descriptors,sizeof descriptors);hooks[3](&context,descriptors,sizeof descriptors);
    assert(Get<uint32_t>(components[0],0x34)==0x140&&Busy()); // glider restored while fall protection remains
    float ordinaryVelocity=0;
    kind=2;
    Put<void*>(runtime,0x30,nullptr);
    for(unsigned frame=0;frame<100;++frame) {
        ordinaryVelocity=(std::max)(-50.f,ordinaryVelocity-1.f);
        cursor=0;assert(hooks[4](&context,descriptors,sizeof descriptors));
        assert(!Disable(&storage,1));
        float y=Get<float>(components[0],4);Put<float>(components[0],4,(std::max)(-50.f,y-1.f));
        assert(hooks[5](&context,descriptors,sizeof descriptors));
        assert(std::fabs(Get<float>(components[0],4)-ordinaryVelocity*.5f)<.0001f);
        y=Get<float>(components[1],4);Put<float>(components[1],4,(std::max)(-50.f,y-1.f));
        assert(!hooks[5](&context,descriptors,sizeof descriptors));
        assert(Get<float>(components[1],4)==ordinaryVelocity);
    }
    assert(Observed(&storage,1)&&!Observed(&storage,2)&&!Faulted());
    // A legitimate ECS migration changes Actor's address. A worker cannot
    // renew it, but a matching registry/entity update can; readiness resets.
    Put(runtime,0x30,static_cast<void*>(&storage));actorOverride=&unrelatedActor;
    kind=1;cursor=0;assert(hooks[2](&context,descriptors,sizeof descriptors));
    assert(hooks[3](&context,descriptors,sizeof descriptors));
    assert(!hooks[3](&context,descriptors,sizeof descriptors));
    assert(!Observed(&storage,1)&&!Faulted());
    Put<void*>(runtime,0x30,nullptr);
    kind=0;cursor=0;assert(hooks[0](&context,descriptors,sizeof descriptors)&&context.id==1);
    kind=4;cursor=0;assert(hooks[8](&context,descriptors,sizeof descriptors)&&context.id==2);
    assert(!hooks[9](&context,descriptors,sizeof descriptors));
    kind=0;cursor=0;assert(hooks[0](&context,descriptors,sizeof descriptors)&&context.id==2);
    assert(!hooks[1](&context,descriptors,sizeof descriptors));
    kind=2;cursor=0;assert(hooks[4](&context,descriptors,sizeof descriptors));
    assert(hooks[5](&context,descriptors,sizeof descriptors));
    assert(!hooks[5](&context,descriptors,sizeof descriptors));
    assert(Observed(&storage,1)&&!Faulted());
    assert(Grounded(&storage,1));
    movement[0x59]=2;kind=4;cursor=0;hooks[8](&context,descriptors,sizeof descriptors);assert(!Grounded(&storage,1));
    movement[0x59]=0;Put<float>(velocity,4,-5.f);cursor=0;hooks[8](&context,descriptors,sizeof descriptors);assert(!Grounded(&storage,1));
    Put<float>(velocity,4,0.f);cursor=0;hooks[8](&context,descriptors,sizeof descriptors);assert(Grounded(&storage,1));
    assert(Disable(&storage,1)&&!Busy());actorOverride=nullptr;
    kind=0;cursor=0;assert(hooks[0](&context,descriptors,sizeof descriptors)&&context.id==1);
    puts("PASS: protection adapter with simulated native iterators: player/world isolation, glider flags, fall-damage exclusion, half fall speed/terminal speed, restoration interlock");
}

