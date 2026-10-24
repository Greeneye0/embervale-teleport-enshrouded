#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_RAND_S
#include "travel.h"
#include "protocol.h"
#include "terrain.h"
#include "arrival.h"
#include "altitude.h"
#include "settings.h"
#include "history.h"
#include "protection.h"
#include "identity.h"
#include "native_signatures.h"
#include "direct_signatures.h"
#include <atomic>
#include <mutex>
#include <string_view>
#include <cstdio>
#include <cstdlib>

namespace teleport::travel {
#ifdef MAP_TELEPORT_TEST
uint64_t TestClock();
#define GetTickCount64 TestClock
#endif
namespace {
template<class T> T& at(void* p,size_t offset) {return *reinterpret_cast<T*>(static_cast<uint8_t*>(p)+offset);}
using Next=bool(*)(void*,void*,size_t);
using Entity=uint32_t*(*)(void*,uint32_t*);
using Dirty=void(*)(void*,uint64_t,uint64_t,uint64_t,uint16_t,uint8_t);
using Receive=void(*)(void*);
using GroundQuery=void*(*)(void*,void*,void*);
GroundQuery groundQuery{};
using SetBounds=void(*)(void*,void*);
using Append=uint8_t*(*)(uint8_t*,void*,uint8_t,uint64_t*,uint32_t,uint32_t,uint32_t,uint64_t,uint64_t*);
Next playerFirst{},playerMore{},applyFirst{},applyMore{};
SetBounds setBounds{};
Settings settings{};
History history;
std::atomic<uint64_t> localAccount{};
std::atomic<uint32_t> localEntity{};
WorldBounds bounds{};
bool boundsKnown=false;
Entity entityOf{}; Dirty dirty{}; TerrainRay ray{}; Receive receive{}; Append append{};
std::atomic<void*> client{};
std::atomic<void*> server{};
std::atomic<bool> hostAnswered{};
std::atomic<bool> paused{false},faulted{false};
std::mutex lock;
enum class Stage { none, resolving, injecting, armed, applying, cancelling, journey };
struct Pending {
    Stage stage{}; uint32_t entity{}; uint64_t account{},nonce{},expires{};
    void* storage{}; Destination target{};
    bool deluxe{};
    Journey trip{};bool journaled{},clientReady{},clientRestored{},recovery{};
    uint64_t retryAfter{};
    int64_t groundY{};bool groundRead{},groundLogged{}; // ground confirmation, see Ground()
    uint8_t transform[56]{};bool haveTransform=false;
};
Pending requests[16]{};
Result results[16]{};
Status remoteStatus{};
uint64_t remoteAccount{};
uint64_t completedRemote[16]{};unsigned completedCursor=0;
void* predictedStorage{};uint32_t predictedEntity{};unsigned predictionAck{};
uint64_t predictedLandingSince{};
struct World {void* storage{};void* terrain{};uint64_t seen{};uint8_t realm{};};
World hostWorld{};
World terrainSources[16]{};
struct Injection {void* input{};void* control{};void* command{};void* storage{};uint32_t entity{},consumed{};uint8_t saved[12]{};uint8_t refusal{},mode{};};
thread_local Injection injection;
using QueueCommand=void*(*)(void*,void*,uint8_t,uint32_t,uint32_t);
using NotifyTeleport=bool(*)(void*,uint32_t,uint32_t);
QueueCommand queueCommand{};NotifyTeleport notifyTeleport{};
struct Application {void* position{};void* storage{};uint32_t entity{};uint64_t nonce{};Destination destination{};};
thread_local Application application;
void Record(const Pending& p,Outcome outcome) {
    for(auto& result:results)if(result.entity==p.entity){result={p.entity,p.nonce,p.target,outcome,p.account};return;}
    for(auto& result:results)if(!result.entity){result={p.entity,p.nonce,p.target,outcome,p.account};return;}
}
void Log(const char* s) {greeneye::Host()->log(s);}
void* Storage(void* ctx) {return at<void*>(at<void*>(ctx,0),0x30);}
bool Authority(void* ctx) {return at<uint8_t>(at<void*>(ctx,0),0xa1)==0;}
uint32_t Id(void* ctx) {uint32_t result=0;return *entityOf(ctx,&result);}
void Mark(void* wrapper) {
    if(auto tracker=at<void*>(wrapper,0x28))
        dirty(tracker,at<uint64_t>(wrapper,8),at<uint64_t>(wrapper,16),at<uint64_t>(wrapper,24),at<uint16_t>(wrapper,32),at<uint8_t>(wrapper,34));
}
uint32_t SenderEntity(uint64_t account) {
    auto s=static_cast<uint8_t*>(server.load());if(!account)return 0;
    if(account==localAccount.load())return localEntity.load();
    for(int i=0;s&&i<16;++i) {
        auto slot=s+0x1e0+i*0x148;
        if(at<uint32_t>(slot,0)&&at<uint64_t>(slot,0x48)==account) return at<uint32_t>(slot,0x18);
    }
    return 0;
}
uint64_t AccountOf(uint32_t id) {
    if(id==localEntity.load()&&localAccount.load())return localAccount.load();
    auto s=static_cast<uint8_t*>(server.load());
    for(int i=0;s&&i<16;++i){auto slot=s+0x1e0+i*0x148;if(at<uint32_t>(slot,0)&&at<uint32_t>(slot,0x18)==id)return at<uint64_t>(slot,0x48);}
    return 0;
}
bool Remote(const Pending& p){return p.account&&p.account!=localAccount.load();}
Status Describe(const Pending& p) {
    Status status{p.nonce};
    if(p.journaled||p.recovery) {
        status.protection=true;status.ground=p.journaled?p.trip.ground:p.target;status.scenario=p.trip.scenario;
        status.phase=NoticePhase(unsigned(p.trip.phase));
        if(p.trip.phase==Phase::complete)status.phase=NoticePhase::complete;
        if(p.trip.phase==Phase::countdown)status.countdown=p.trip.countdown(GetTickCount64());
        if(status.phase==NoticePhase::countdown&&!status.countdown)status.countdown=1;
    }
    return status;
}
void Recover(Pending& p) {
    if(p.journaled){p.trip.recover();p.stage=Stage::journey;p.retryAfter=GetTickCount64()+1000;p.expires=GetTickCount64()+30000;}
    else{Record(p,Outcome::failed);p={};}
}
double Metres(int64_t v){return double(v)/4294967296.;}
// The ground under p.target, checked twice before use. A hit above the world limit or at
// the ray's start is cast again from four metres under the limit; landing above the limit
// would send the player to their altar. Readings must agree on two consecutive ticks, so a
// value read the moment terrain cells finish loading is never used on its own.
Surface Ground(Pending& p,Destination& target,const char** reason=nullptr) {
    int64_t top=0;auto first=target;
    auto surface=TopSurface(ray,hostWorld.terrain,hostWorld.realm,target,reason,&top);
    if(surface==Surface::ready&&!PlausibleGround(boundsKnown,bounds,target,top)) {
        int64_t from=boundsKnown?bounds.maximum.y-4*4294967296LL:top-2*4294967296LL;
        auto suspect=target;target=first;int64_t again=0;
        surface=TopSurface(ray,hostWorld.terrain,hostWorld.realm,target,reason,&again,Metres(from));
        bool fixed=surface==Surface::ready&&PlausibleGround(boundsKnown,bounds,target,again);
        if(!p.groundLogged) {
            char line[256];sprintf_s(line,"Map Teleport: ground at %.1f m is above the world limit (%s%.1f m) or at the ray start (%.1f m); recheck from %.1f m %s %.1f m",
                Metres(suspect.y),boundsKnown?"":"unknown, ",boundsKnown?Metres(bounds.maximum.y):0.,Metres(top),Metres(again),fixed?"found ground at":"found nothing usable; last",Metres(target.y));
            Log(line);p.groundLogged=true;
        }
        if(!fixed)surface=Surface::loading; // try again next tick until the request expires
    }
    if(surface!=Surface::ready){p.groundRead=false;return surface;}
    if(!p.groundRead||!SameGround(p.groundY,target.y)){p.groundRead=true;p.groundY=target.y;return Surface::checking;}
    return surface;
}
bool Retire(Pending& p) {
    if((p.journaled||p.recovery)&&!protection::Disable(p.storage,p.entity))return false;
    Record(p,Outcome::failed);p={};return true;
}
bool Queue(uint32_t entity,uint64_t account,Request request) {
    if(paused||faulted||!entity||entity>16)return false;
    std::lock_guard guard(lock);
    if(settings.prank&&(!account||!history.ready()))return false;
    uint64_t now=GetTickCount64();
    for(auto& p:requests) {
        if(p.stage!=Stage::none&&p.entity==entity)return false;
    }
    for(auto& p:requests) {
        if(p.stage==Stage::none) {
            p={Stage::resolving,entity,account,request.nonce,now+30000,nullptr,request.position,settings.deluxe};
            if(auto visit=history.find(account);visit&&!visit->complete){p.recovery=true;p.target=visit->ground;}
            Record(p,Outcome::pending);return true;
        }
    }
    return false;
}
// Called before the first iterator call, even when the world has no Teleport component.
// The iterator owns a registry, not the enclosing ECS world. Its native
// terrain getter reads +0x478; the enclosing world's +0xe16238 getter must
// never be applied to this registry pointer.
void ObserveWorld(void* ctx,void* descriptors) {
    (void)descriptors;
    if(!Authority(ctx))return;
    auto realm=at<uint8_t>(at<void*>(ctx,0),0xbb);
    std::lock_guard guard(lock);
    auto storage=Storage(ctx);
    void* terrain=storage?at<void*>(storage,0x478):nullptr;
    if(terrain&&(!at<void*>(terrain,0x1bf8)||at<uint64_t>(terrain,0x1c00)>256||realm>=at<uint64_t>(terrain,0x1c00)))terrain=nullptr;
    if(terrain&&hostWorld.terrain!=terrain)Log("Map Teleport: registry supplied terrain resource");
    for(auto& p:requests)if(p.stage!=Stage::none) {
        if(p.storage&&p.storage!=storage)Retire(p);
        else if(p.account&&SenderEntity(p.account)!=p.entity) {
            if(p.stage==Stage::armed||p.stage==Stage::cancelling){p.stage=Stage::cancelling;if(p.journaled)protection::Disable(p.storage,p.entity);}
            else Retire(p);
        }
    }
    hostWorld={storage,terrain,GetTickCount64(),realm};
}
void CaptureTerrain(void* descriptors) {
    auto ctx=at<void*>(descriptors,0);if(!Authority(ctx))return;
    World source{Storage(ctx),at<void*>(descriptors,0xf8),GetTickCount64(),at<uint8_t>(at<void*>(ctx,0),0xbb)};
    if(!source.terrain)return;
    std::lock_guard guard(lock);
    for(auto& saved:terrainSources)if(saved.storage==source.storage&&saved.realm==source.realm){saved=source;return;}
    for(size_t i=0;i<std::size(terrainSources);++i){auto& saved=terrainSources[i];if(!saved.storage||source.seen-saved.seen>=2000){bool fresh=!saved.storage&&source.storage;saved=source;if(fresh){char text[96];sprintf_s(text,"Map Teleport: terrain source %zu registered",i);Log(text);}return;}}
}
void SafeCaptureTerrain(void* descriptors) {
    __try{CaptureTerrain(descriptors);}__except(EXCEPTION_EXECUTE_HANDLER){Log("Map Teleport: terrain capture unavailable");}
}
void* GroundHook(void* result,void* material,void* descriptors) {
    greeneye::Lease lease;if(lease)SafeCaptureTerrain(descriptors);
    return groundQuery(result,material,descriptors);
}
// The native travel gate (also used by fast travel) answers 1 = travel disabled in this area
// (mode byte not 0/1), 2 = falling, 4 = under water. Map teleport goes anyway, so the game's
// own action, animation included, runs as usual. Only the action this mod injected is affected.
using Gate=uint8_t*(*)(uint8_t*,char,uint64_t);Gate gate{};
uint8_t* GateHook(uint8_t* out,char mode,uint64_t flags) {
    greeneye::Lease lease;auto result=gate(out,mode,flags);
    if(lease&&injection.input&&*result){injection.refusal=*result;injection.mode=uint8_t(mode);*result=0;}
    return result;
}
void FinishInjection() {
    auto saved=injection;injection={};if(!saved.input)return;
    bool taken=at<uint32_t>(saved.control,0x54)!=saved.consumed; // the game consumed the injected action
    // Restore the real input and its consumed version before processing the next player.
    // This prevents the temporary action from replaying an older network input next frame.
    memcpy(static_cast<uint8_t*>(saved.input)+0x174,saved.saved,12);
    at<uint32_t>(saved.control,0x54)=saved.consumed;
    bool accepted=at<uint32_t>(saved.command,8)==0x1cc90f15;
    std::lock_guard guard(lock);
    for(auto& p:requests)if(p.entity==saved.entity&&p.storage==saved.storage&&p.stage==Stage::injecting) {
        if(accepted)p.stage=Stage::armed;else Recover(p);
        p.expires=GetTickCount64()+30000;
        if(saved.refusal) {
            std::string why;
            for(auto [bit,text]:{std::pair{1,"travel disabled here"},{2,"falling"},{4,"under water"}})if(saved.refusal&bit)why+=(why.empty()?"":", ")+std::string(text);
            char line[200];sprintf_s(line,"Map Teleport: game travel gate refused (%s; area mode %u); teleporting anyway",why.c_str(),unsigned(saved.mode));
            Log(line);
        }
        Log(accepted?"Map Teleport: native travel action accepted":taken?"Map Teleport: native travel action rejected after it was taken; no movement requested":
            "Map Teleport: native travel action not taken (character busy); no movement requested");
    }
}
void Player(void* ctx,void* descriptors) {
    if(faulted)return;
    auto id=Id(ctx);auto storage=Storage(ctx);uint64_t now=GetTickCount64();
    std::lock_guard guard(lock);
    if(!Authority(ctx)) {
        if(id!=localEntity.load())return;
        Status prediction=remoteStatus;Pending* localTrip=nullptr;
        for(auto& p:requests)if(p.stage!=Stage::none&&p.entity==id&&p.account==localAccount.load()&&p.journaled){localTrip=&p;prediction=Describe(p);break;}
        if(predictedStorage&&(predictedStorage!=storage||predictedEntity!=id)) {
            if(!protection::Disable(predictedStorage,predictedEntity))return;
            predictedStorage=nullptr;predictionAck=0;
        }
        if(prediction.protection&&!Finished(prediction)) {
            if(predictionAck==2&&prediction.phase==NoticePhase::restoring)return;
            if(!protection::Enable(storage,id))return;
            predictedStorage=storage;predictedEntity=id;
            if(prediction.phase==NoticePhase::returning||prediction.phase==NoticePhase::restoring)
                protection::RestoreGlider(storage,id);
            if(protection::Observed(storage,id)){predictionAck=1;if(localTrip)localTrip->clientReady=true;}
            if(prediction.phase!=NoticePhase::restoring)predictedLandingSince=0;
            if(prediction.phase==NoticePhase::restoring) {
                bool landedNear=protection::Grounded(storage,id);
                if(!landedNear)predictedLandingSince=0;
                else if(!predictedLandingSince)predictedLandingSince=now;
                else if(now-predictedLandingSince>=2000&&protection::Disable(storage,id)){predictedStorage=nullptr;predictionAck=2;if(localTrip)localTrip->clientRestored=true;}
            }
        }
        return;
    }
    if(hostWorld.storage&&hostWorld.storage!=storage) {
        for(auto& p:requests)if(p.stage!=Stage::none)Retire(p);
        hostWorld.terrain=nullptr;
    }
    hostWorld.storage=storage;hostWorld.seen=now;
    // Pending journal records survive disconnection, character changes and restarts.
    // Re-resolve their ground height in the currently loaded world before moving.
    auto account=AccountOf(id);bool active=false;
    for(auto& p:requests)if(p.stage!=Stage::none&&p.entity==id)active=true;
    auto visit=account?history.find(account):nullptr;
    if(!active&&visit&&!visit->complete)for(auto& p:requests)if(p.stage==Stage::none) {
        p={Stage::resolving,id,account,now,now+30000,storage,visit->ground,false};p.recovery=true;Record(p,Outcome::pending);break;
    }
    for(auto& p:requests) {
        if(p.stage==Stage::none||p.entity!=id)continue;
        if(p.storage&&p.storage!=storage){Retire(p);continue;}
        if(p.account&&SenderEntity(p.account)!=id) {
            Record(p,Outcome::failed);
            if(p.stage==Stage::armed||p.stage==Stage::cancelling){p.stage=Stage::cancelling;if(p.journaled)protection::Disable(p.storage,p.entity);}
            else Retire(p);
            continue;
        }
        if((p.journaled&&p.stage==Stage::journey)||p.recovery)p.expires=now+30000;
        if(p.expires<=now) {
            Record(p,Outcome::failed);
            // Retain ownership of an emitted native action until Apply can
            // cancel it. Forgetting it here would let it execute unmodified.
            if(p.stage==Stage::resolving)Recover(p);else p.stage=Stage::cancelling;
            continue;
        }
        if(p.stage!=Stage::resolving&&p.stage!=Stage::journey)continue;
        p.storage=storage;
        if(p.recovery&&!protection::Enable(storage,id))continue;
        bool wantsPrank=p.recovery||(settings.prank&&p.account&&history.ready()&&!history.find(p.account));
        if(p.stage==Stage::resolving&&wantsPrank&&!p.journaled) {
            Destination ground=p.target,high{};
            const char* terrainReason=nullptr;
            auto surface=Ground(p,ground,&terrainReason);
            if(surface==Surface::checking)continue;
            if(surface==Surface::loading){
                if(now>=p.retryAfter){Log(hostWorld.terrain?"Map Teleport: prank preparation waiting for terrain cells":"Map Teleport: prank preparation waiting for terrain resource");p.retryAfter=now+5000;}
                continue;
            }
            if(surface!=Surface::ready){
                if(now>=p.retryAfter){char message[256];sprintf_s(message,"Map Teleport: terrain preflight blocked: %s; realm=%u destination=%.3f,%.3f,%.3f",terrainReason,unsigned(hostWorld.realm),double(ground.x)/4294967296.,double(ground.y)/4294967296.,double(ground.z)/4294967296.);Log(message);p.retryAfter=now+5000;}
                if(!p.recovery){Record(p,Outcome::failed);p={};}continue;
            }
            if(!p.recovery&&(!boundsKnown||!HighDestination(bounds,ground,high))){Log("Map Teleport: prank cancelled; scene bounds unavailable or destination outside bounds");Record(p,Outcome::failed);p={};continue;}
            if(!p.recovery&&!history.begin(p.account,ground)){Record(p,Outcome::failed);p={};continue;}
            p.journaled=true;p.trip.ground=ground;p.trip.high=high;p.trip.mode=TravelMode::prank;p.trip.protectedFall=true;
            unsigned choice=0;if(rand_s(&choice)!=0)choice=unsigned(now^(p.account>>16)^p.nonce);
            p.trip.scenario=settings.testScenario>=0?unsigned(settings.testScenario):choice%3;p.stage=Stage::journey;
        }
        if(p.journaled) {
            if(!protection::Enable(storage,id)||!protection::Observed(storage,id))continue;
            if(p.trip.phase==Phase::idle) {
                if(Remote(p)&&!p.clientReady)continue;
                p.trip.begin(TravelMode::prank,p.trip.scenario,now,p.trip.ground,p.trip.high,true);
                if(p.recovery)p.trip.recover();
            }
            if(p.recovery||p.trip.phase==Phase::returning||p.trip.phase==Phase::restoring)
                protection::RestoreGlider(storage,id);
            if(p.trip.phase==Phase::restoring) {
                if(now-p.trip.since<2000||!protection::Grounded(storage,id))continue;
                if((Remote(p)||predictedStorage)&&!p.clientRestored)continue;
                if(!protection::Disable(storage,id))continue;
                if(!history.finish(p.account))continue;
                p.trip.restored();Record(p,Outcome::arrived);p={};continue;
            }
            p.trip.tick(now);
            if(!p.trip.needsTravel()||now<p.retryAfter)continue;
            p.target=p.trip.destination();
        }
        if(p.journaled&&p.trip.phase==Phase::returning&&p.haveTransform) {
            // Native teleport events queue a Teleport component directly (270650).
            // Keep the exact native orientation/scale from the outbound hop;
            // only change its absolute position. Native Apply still performs
            // movement, replication, velocity reset and arrival confirmation.
            auto queue=at<void*>(at<void*>(ctx,0),8);
            if(!queue){p.retryAfter=now+1000;continue;}
            if(!notifyTeleport(queue,id,id)){p.retryAfter=now+1000;continue;}
            auto command=queueCommand(queue,static_cast<uint8_t*>(queue)+0x10,3,0xbc,id);
            if(!command){p.retryAfter=now+1000;continue;}
            at<uint32_t>(command,0xc)=0x23384d40;
            at<uint16_t>(command,0x4c)=0x50;
            memcpy(static_cast<uint8_t*>(command)+0x6c,p.transform,sizeof p.transform);
            memcpy(static_cast<uint8_t*>(command)+0x6c,&p.target,sizeof p.target);
            p.stage=Stage::armed;p.expires=now+30000;
            Log("Map Teleport: protected return queued directly without spell channel");
            continue;
        }
        // Read/write accesses from the verified player-system descriptor layout.
        void* input=at<void*>(descriptors,0x80);
        void* control=at<void*>(descriptors,0x2f8);
        void* command=at<void*>(descriptors,0xe8);
        if(!input||!control||!command){Recover(p);continue;}
        // Respect an already queued native travel request.
        if(at<uint32_t>(input,0x17c)!=at<uint32_t>(control,0x54)) {if(!p.journaled){Record(p,Outcome::failed);p={};}continue;}
        injection={input,control,command,storage,id,at<uint32_t>(control,0x54),{}};
        memcpy(injection.saved,static_cast<uint8_t*>(input)+0x174,12);
        at<uint32_t>(input,0x174)=id; // valid entity target; final position supplied at native apply
        // TeleporterType::Normal selects the native spell-effect asset; None
        // starts the actor animation without the regular teleport visuals.
        at<uint8_t>(input,0x178)=1;
        at<uint32_t>(input,0x17c)=injection.consumed+1;
        p.stage=Stage::injecting;
        return;
    }
}
bool Apply(void* ctx,void* descriptors) {
    if(faulted||!Authority(ctx))return true;
    auto id=Id(ctx);auto storage=Storage(ctx);
    std::lock_guard guard(lock);
    for(auto& p:requests) {
        if((p.stage!=Stage::armed&&p.stage!=Stage::cancelling)||p.entity!=id||p.storage!=storage)continue;
        void* wrapper=static_cast<uint8_t*>(descriptors)+0x100;
        void* component=at<void*>(wrapper,0);
        if(!component||at<uint8_t>(component,0x48))return true;
        uint32_t relative=at<uint32_t>(component,0x38);
        if(relative&&relative!=id){Recover(p);return true;}
        if(p.stage==Stage::cancelling||p.expires<=GetTickCount64()||(p.account&&SenderEntity(p.account)!=id)) {
            Mark(wrapper);at<uint8_t>(component,0x48)=1;Recover(p);
            Log("Map Teleport: request expired; movement cancelled");return true;
        }
        if(hostWorld.storage!=storage)return false;
        auto surface=p.journaled?Surface::ready:Ground(p,p.target);
        if(surface==Surface::checking)return false; // confirm the ground on the next tick
        if(surface==Surface::loading) {
            if(GetTickCount64()>=p.retryAfter) {
                Log(hostWorld.terrain?"Map Teleport: waiting for terrain ray cells to load":"Map Teleport: waiting for terrain resource from ECS world registry");
                p.retryAfter=GetTickCount64()+5000;
            }
            return false; // defer this entity, as native spawn queries do
        }
        if(surface==Surface::invalid) {
            Mark(wrapper);at<uint8_t>(component,0x48)=1;Record(p,Outcome::failed);p={};
            Log("Map Teleport: no terrain surface; movement cancelled");return true;
        }
        if(p.deluxe&&!p.journaled) {
            // Use the scene reset bounds, never the terrain grid's ceiling.
            // A newly loaded companion may not have observed scene setup yet.
            if(!boundsKnown||!HighDestination(bounds,p.target,p.target)) {
                Mark(wrapper);at<uint8_t>(component,0x48)=1;Record(p,Outcome::failed);p={};
                Log("Map Teleport: deluxe cancelled; verified scene bounds unavailable or destination outside bounds");return true;
            }
        }
        if(p.journaled&&!p.haveTransform){memcpy(p.transform,component,sizeof p.transform);p.haveTransform=true;}
        Mark(wrapper);
        memcpy(component,&p.target,sizeof(p.target));
        at<uint32_t>(component,0x38)=0;
        at<uint8_t>(component,0x49)=0;
        // Position is descriptor +0x10 (local_2c8 relative to local_2d8 in
        // native apply). Inspect it before the next iterator advances; never
        // treat writing the Teleport component as proof of arrival.
        application={at<void*>(descriptors,0x10),storage,id,p.nonce,p.target};
        p.stage=Stage::applying;
        char line[200];sprintf_s(line,"Map Teleport: terrain destination supplied at %.1f, %.1f, %.1f (world limit %s%.1f m); awaiting native position update",
            Metres(p.target.x),Metres(p.target.y),Metres(p.target.z),boundsKnown?"":"unknown, ",boundsKnown?Metres(bounds.maximum.y):0.);
        Log(line);return true;
    }
    return true;
}
void FinishApplication() {
    auto done=application;application={};if(!done.entity)return;
    Destination actual{};bool arrived=false;
    if(done.position){memcpy(&actual,done.position,sizeof actual);arrived=ArrivedAt(actual,done.destination);}
    std::lock_guard guard(lock);
    for(auto& p:requests)if(p.stage==Stage::applying&&p.entity==done.entity&&p.storage==done.storage&&p.nonce==done.nonce) {
        if(p.journaled) {
            if(arrived){p.trip.arrived(GetTickCount64());p.stage=Stage::journey;}else Recover(p);
        }else{Record(p,arrived?Outcome::arrived:Outcome::failed);p={};}
        Log(arrived?"Map Teleport: native position confirms arrival":"Map Teleport: native position did not reach requested destination");
    }
}
void Fault() {faulted=true;Log("Map Teleport: host travel disabled after invalid native state");}
int NativeFault(const char* stage,EXCEPTION_POINTERS* exception) {
    auto record=exception->ExceptionRecord;
    auto base=reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
    auto address=reinterpret_cast<uintptr_t>(record->ExceptionAddress);
    char message[256];
    sprintf_s(message,"Map Teleport: native fault in %s: code=%08lx address=%p exeOffset=%llx access=%llu target=%llx",
        stage,record->ExceptionCode,record->ExceptionAddress,static_cast<unsigned long long>(address-base),
        record->NumberParameters?static_cast<unsigned long long>(record->ExceptionInformation[0]):0,
        record->NumberParameters>1?static_cast<unsigned long long>(record->ExceptionInformation[1]):0);
    Log(message);return EXCEPTION_EXECUTE_HANDLER;
}
void SafeObserve(void* c,void* d){__try{ObserveWorld(c,d);}__except(NativeFault("observe world",GetExceptionInformation())){Fault();}}
void SafeFinish(){__try{FinishInjection();}__except(NativeFault("restore input",GetExceptionInformation())){Fault();}}
void SafePlayer(void* c,void* d){__try{Player(c,d);}__except(NativeFault("player",GetExceptionInformation())){Fault();}}
bool SafeApply(void* c,void* d){__try{return Apply(c,d);}__except(NativeFault("apply destination",GetExceptionInformation())){Fault();return true;}}
void SafeFinishApplication(){__try{FinishApplication();}__except(NativeFault("confirm arrival",GetExceptionInformation())){Fault();}}
bool PlayerFirst(void* c,void* d,size_t n) {greeneye::Lease lease;
    // Resolve terrain before the first teleport, and retire disconnected
    // players even when this iteration yields no remaining player entities.
    if(lease&&!faulted)SafeObserve(c,nullptr);
    bool ok=playerFirst(c,d,n);if(lease&&ok)SafePlayer(c,d);return ok;}
bool PlayerMore(void* c,void* d,size_t n) {greeneye::Lease lease;if(lease)SafeFinish();bool ok=playerMore(c,d,n);if(lease&&ok)SafePlayer(c,d);return ok;}
bool ApplyFirst(void* c,void* d,size_t n) {greeneye::Lease lease;if(lease&&!faulted)SafeObserve(c,d);bool ok=applyFirst(c,d,n);while(lease&&ok&&!SafeApply(c,d))ok=applyFirst(c,d,n);return ok;}
bool ApplyMore(void* c,void* d,size_t n) {greeneye::Lease lease;if(lease)SafeFinishApplication();bool ok=applyMore(c,d,n);while(lease&&ok&&!SafeApply(c,d))ok=applyMore(c,d,n);return ok;}
void ReceiveHook(void* s) {greeneye::Lease lease;if(lease)server=s;receive(s);}
void CaptureBounds(void* manager) {
    WorldBounds value{};bool valid=ReadWorldBounds(manager,value);
    std::lock_guard guard(lock);boundsKnown=valid;if(valid)bounds=value;
    Log(valid?"Map Teleport: scene height limits captured":"Map Teleport: scene height limits unavailable");
}
void ClearBounds(){std::lock_guard guard(lock);boundsKnown=false;}
void SafeBounds(void* manager){__try{CaptureBounds(manager);}__except(EXCEPTION_EXECUTE_HANDLER){ClearBounds();}}
void BoundsHook(void* manager,void* configuration) {
    greeneye::Lease lease;setBounds(manager,configuration);if(lease)SafeBounds(manager);
}
// 0: ordinary chat, 1: replace on host, 2: consume on client/host.
int HostChat(void* store,uint8_t type,uint64_t* text,uint32_t senderHandle,uint64_t account,char* answer,size_t capacity) {
    auto cl=client.load();
    if(type!=kChatType||!text||!text[0]||text[1]>160)return 0;
    std::string_view line(reinterpret_cast<const char*>(text[0]),size_t(text[1]));
    if(cl&&store==at<void*>(cl,0xf8)) {
        if(!line.starts_with("/mts1 "))return 0;
        // The server stamps the native player handle and account ID on replies.
        // That account ID is NOT the Steam ID returned by LocalAccount(). Use
        // the full native handle (including generation), never just its slot.
        auto ownHandle=at<uint32_t>(cl,0x466b44);
        if(!ownHandle||senderHandle!=ownHandle||!account)return 2;
        if(line==kHelloReply){if(!hostAnswered.exchange(true))Log("Map Teleport: host answered; remote teleport enabled");return 2;}
        Status status{};if(ParseStatus(line,status)) {
            std::lock_guard guard(lock);
            for(auto completed:completedRemote)if(completed&&completed==status.nonce)return 2;
            bool recover=!remoteStatus.nonce&&status.protection;
            if(recover||(status.nonce==remoteStatus.nonce&&!Finished(remoteStatus))) {
                if(status.phase!=remoteStatus.phase||status.nonce!=remoteStatus.nonce) {
                    char message[128];sprintf_s(message,"Map Teleport: host status accepted nonce=%llu phase=%u",status.nonce,unsigned(status.phase));Log(message);
                }
                remoteStatus=status;
                if(Finished(status))completedRemote[completedCursor++%16]=status.nonce;
            }
        }
        return 2;
    }
    // Never relay client-supplied responses as if they originated on the host.
    if(line.starts_with("/mts1 "))return 2;
    if(line==kHello){memcpy(answer,kHelloReply.data(),kHelloReply.size());answer[kHelloReply.size()]=0;return 1;}
    uint64_t nonce=0;NoticePhase phase=NoticePhase::failed;Status reply{};
    if(line.starts_with("/mtp1 ")||line.starts_with("/mta1 ")) {
        unsigned acknowledgement=0;
        if(line.starts_with("/mta1 ")) {
            auto last=line.rfind(' ');if(last==std::string_view::npos||last+2!=line.size()||(line.back()!='1'&&line.back()!='2'))return 2;
            acknowledgement=unsigned(line.back()-'0');
            auto value=line.substr(6,last-6);auto parsed=std::from_chars(value.data(),value.data()+value.size(),nonce);
            if(parsed.ec!=std::errc{}||parsed.ptr!=value.data()+value.size()||!nonce)return 2;
        }else if(line!="/mtp1 0"&&!ParsePoll(line,nonce))return 2;
        auto id=SenderEntity(account);std::lock_guard guard(lock);
        for(auto& p:requests)if(p.stage!=Stage::none&&p.entity==id&&p.account==account&&(!nonce||p.nonce==nonce)) {
            if(acknowledgement==1)p.clientReady=true;
            if(acknowledgement==2&&p.trip.phase==Phase::restoring)p.clientRestored=true;
            reply=Describe(p);nonce=p.nonce;break;
        }
        for(auto result:results)if(id&&result.entity==id&&result.account==account&&result.nonce==nonce) {
            phase=result.outcome==Outcome::arrived?NoticePhase::complete:
                result.outcome==Outcome::pending?NoticePhase::waiting:NoticePhase::failed;break;
        }
    } else if(line.starts_with("/mt1 ")) {
        Request r;if(!ParseRequest(line,r))return 2;
        nonce=r.nonce;if(Queue(SenderEntity(account),account,r))phase=NoticePhase::waiting;
    } else return 0;
    if(!nonce)return 2;
    if(reply.protection)sprintf_s(answer,capacity,"/mts1 %llu %u %u %u 1 %lld %lld %lld",nonce,unsigned(reply.phase),reply.scenario,reply.countdown,reply.ground.x,reply.ground.y,reply.ground.z);
    else sprintf_s(answer,capacity,"/mts1 %llu %u 0 0",nonce,unsigned(phase));return 1;
}
int SafeChat(void* s,uint8_t t,uint64_t* text,uint32_t handle,uint64_t a,char* answer,size_t capacity){__try{return HostChat(s,t,text,handle,a,answer,capacity);}__except(EXCEPTION_EXECUTE_HANDLER){Fault();return 0;}}
uint8_t* AppendHook(uint8_t* result,void* store,uint8_t type,uint64_t* text,uint32_t a,uint32_t b,uint32_t c,uint64_t account,uint64_t* time) {
    greeneye::Lease lease;char answer[160]{};
    int action=lease?SafeChat(store,type,text,c,account,answer,sizeof answer):0;
    if(action==2){memset(result,0,8);return result;}
    if(action==1) {uint64_t replacement[]={reinterpret_cast<uint64_t>(answer),strlen(answer)};return append(result,store,type,replacement,a,b,c,account,time);}
    return append(result,store,type,text,a,b,c,account,time);
}
bool CallMatches(const GreeneyeHost* host,uint8_t* site,uint8_t* expected) {
    uint8_t bytes[5];host->read(site,bytes,5);int32_t rel;memcpy(&rel,bytes+1,4);
    return bytes[0]==0xe8&&site+5+rel==expected;
}
}
bool Init(const GreeneyeHost* host) {
    if(host->modDir)settings=ReadSettings((std::wstring(host->modDir)+L"\\map-teleport.ini").c_str());
    if(host->modDir&&!history.open(std::wstring(host->modDir)+L"\\map-teleport-history.txt"))Log("Map Teleport: history unavailable; first-use travel will fail closed");
    auto p=host->find(native::playerPattern,nullptr),a=host->find(native::applyPattern,nullptr),n=host->find(native::nextPattern,nullptr);
    entityOf=reinterpret_cast<Entity>(host->find(native::entityPattern,nullptr));
    dirty=reinterpret_cast<Dirty>(host->find(native::dirtyPattern,nullptr));
    ray=reinterpret_cast<TerrainRay>(host->find(native::rayPattern,nullptr));
    auto r=host->find("40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 D8 F6 FF FF 48 81 EC 28 0A 00 00 4C 8B E9",nullptr);
    if(!r)r=host->find("40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 98 F9 FF FF 48 81 EC 68 07 00 00 4C 8B F9",nullptr);
    auto chat=host->find("40 55 56 41 54 41 55 41 57 48 83 EC 30 49 8B 41 08 33 ED",nullptr);
    auto limits=host->find("48 8B C4 48 89 58 18 48 89 68 20 57 48 81 EC A0 00 00 00 48 8D 99 B0 00 00 00 C6 40 E8 00 48 C7 43 08 00 00 00 00 48 8B FA 0F 10 4A 08 48 8B CB C7 40 EC 00 00 00 00 0F 10 42 20 F2 0F 10 52 18",nullptr);
    auto ground=host->find("4C 8B DC 49 89 5B 10 49 89 7B 20 41 56 48 81 EC B0 00 00 00 45 0F 29 43 C8 48 8B D9 49 8B 08 33 C0 F3 44 0F 10 05 ?? ?? ?? ?? 4D 8B F0 41 0F 29 73 E8 48 8B FA 41 0F 29 7B D8 66 41 89 43 19",nullptr);
    queueCommand=reinterpret_cast<QueueCommand>(host->find(native::queueTeleportPattern,nullptr));
    notifyTeleport=reinterpret_cast<NotifyTeleport>(host->find(native::notifyTeleportPattern,nullptr));
    if(!queueCommand||!notifyTeleport||!p||!a||!n||!entityOf||!dirty||!ray||!r||!chat||!limits||!ground)return false;
    if(!CallMatches(host,p+0x36,n)||!CallMatches(host,p+0x21f6,n)||!CallMatches(host,a+0x31,n)||!CallMatches(host,a+0x7be,n))return false;
    // Optional: without the gate the game's own travel restrictions still apply.
    auto g=host->find("C6 01 00 49 0F BA E0 0E 72 ?? 32 C0 84 D2 74 ?? 80 FA 01 74 ?? C6 01 01 B0 01 49 0F BA E0 2C",nullptr);
    if(g&&p&&CallMatches(host,p+0xfb2,g))gate=reinterpret_cast<Gate>(host->redirectCall(p+0xfb2,reinterpret_cast<void*>(GateHook)));
    if(!gate)Log("Map Teleport: travel gate not found; the game's travel restrictions apply to map teleport");
    playerFirst=reinterpret_cast<Next>(host->redirectCall(p+0x36,reinterpret_cast<void*>(PlayerFirst)));
    playerMore=reinterpret_cast<Next>(host->redirectCall(p+0x21f6,reinterpret_cast<void*>(PlayerMore)));
    applyFirst=reinterpret_cast<Next>(host->redirectCall(a+0x31,reinterpret_cast<void*>(ApplyFirst)));
    applyMore=reinterpret_cast<Next>(host->redirectCall(a+0x7be,reinterpret_cast<void*>(ApplyMore)));
    receive=reinterpret_cast<Receive>(host->hook(r,reinterpret_cast<void*>(ReceiveHook),13,nullptr,0));
    append=reinterpret_cast<Append>(host->hook(chat,reinterpret_cast<void*>(AppendHook),13,nullptr,0));
    setBounds=reinterpret_cast<SetBounds>(host->hook(limits,reinterpret_cast<void*>(BoundsHook),19,nullptr,0));
    groundQuery=reinterpret_cast<GroundQuery>(host->hook(ground,reinterpret_cast<void*>(GroundHook),13,nullptr,0));
    bool ok=playerFirst&&playerMore&&applyFirst&&applyMore&&receive&&append&&setBounds&&groundQuery;paused=!ok;return ok;
}
bool Local(uint32_t entity,Destination position) {
    return SubmitLocal(entity,position)==Submission::queued;
}
Submission SubmitLocal(uint32_t entity,Destination position) {
    {std::lock_guard guard(lock);if(!hostWorld.storage||GetTickCount64()-hostWorld.seen>2000)return Submission::remote;}
    return Queue(entity,localAccount.load(),{GetTickCount64(),position})?Submission::queued:Submission::rejected;
}
bool HostsWorld(){std::lock_guard guard(lock);return hostWorld.storage&&GetTickCount64()-hostWorld.seen<=2000;}
bool HostAnswered(){return hostAnswered.load();}
void SetClient(void* c){
    auto previous=client.exchange(c);
    if(previous!=c)hostAnswered=false;
    if(c){localEntity=(at<uint32_t>(c,0x466b44)&0x3f)+1;if(previous!=c){localAccount=LocalAccount();SafeBounds(at<void*>(c,0x4671d0));std::lock_guard guard(lock);remoteAccount=localAccount;}}
    else{localEntity=0;localAccount=0;std::lock_guard guard(lock);if(predictedStorage&&protection::Disable(predictedStorage,predictedEntity)){predictedStorage=nullptr;predictionAck=0;}remoteStatus={};}
}
Result Latest(uint32_t entity){std::lock_guard guard(lock);for(auto result:results)if(result.entity==entity)return result;return {};}
void TrackRemote(uint64_t nonce,uint64_t account){std::lock_guard guard(lock);remoteStatus={nonce};remoteAccount=account;if(!predictedStorage)predictionAck=0;}
Status RemoteStatus(){std::lock_guard guard(lock);return remoteStatus;}
Status LocalStatus(uint32_t entity){std::lock_guard guard(lock);for(auto& p:requests)if(p.stage!=Stage::none&&p.entity==entity)return Describe(p);for(auto r:results)if(r.entity==entity)return {r.nonce,r.outcome==Outcome::arrived?NoticePhase::complete:r.outcome==Outcome::pending?NoticePhase::waiting:NoticePhase::failed};return {};}
unsigned ClientProtectionAck(){std::lock_guard guard(lock);return predictionAck;}
bool Prepare() {
    paused=true;std::lock_guard guard(lock);
    // An expired request may still own a native Teleport component. Let the
    // native iterator cancel/finish it before allowing the hook to unload.
    for(auto& p:requests)if(p.stage!=Stage::none)return false;
    return !protection::Busy();
}
void Resume(){paused=false;}
}
