#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "../src/travel.h"
#include "../src/journey.h"
#include "../src/native_signatures.h"
#include "../src/direct_signatures.h"
#include "../src/protection.h"
#include "../src/history.h"
#include "../src/protocol.h"
#include <cassert>
#include <cstring>
#include <cstdio>
#include <vector>
#include <string>
#include <map>

uint64_t testNow=100000,testAccount=0;
namespace teleport {uint64_t TestAccount(){return testAccount;}}
namespace teleport::travel {uint64_t TestClock(){return testNow;}}
namespace teleport::protection {
std::map<std::pair<void*,uint32_t>,bool> active;
bool observed=true,canDisable=true,grounded=true;
void RestoreGlider(void*,uint32_t){}
bool Grounded(void*,uint32_t){return grounded;}
bool Enable(void* storage,uint32_t entity){active[{storage,entity}]=true;return true;}
bool Observed(void* storage,uint32_t entity){return observed&&active.contains({storage,entity});}
bool Disable(void* storage,uint32_t entity){if(!canDisable)return false;active.erase({storage,entity});return true;}
bool Busy(){return !active.empty();}
bool Faulted(){return false;}
}

namespace {
using Next=bool(*)(void*,void*,size_t);
uint8_t functions[3][10000]{};Next hooks[4]{};unsigned hookCount=0,cursor=0;bool yieldEntity=false;
// The native travel gate: refuses (1) outside area modes 0/1.
uint8_t* NativeGate(uint8_t* out,char mode,uint64_t){*out=mode!=0&&mode!=1;return out;}
using GateFn=uint8_t*(*)(uint8_t*,char,uint64_t);GateFn gateHook{};
uint8_t directQueue[0x40]{},directCommand[0xbc]{};unsigned directCount=0;bool directReady=false;
void* QueueDirect(void* queue,void* buffer,uint8_t type,uint32_t size,uint32_t id){
    assert(queue==directQueue&&buffer==directQueue+0x10&&type==3&&size==0xbc&&id==1);
    memset(directCommand,0,sizeof directCommand);++directCount;directReady=true;return directCommand;
}
bool NotifyDirect(void* queue,uint32_t id,uint32_t target){assert(queue==directQueue&&id==1&&target==1);return true;}
using BoundsFn=void(*)(void*,void*);
BoundsFn boundsHook{};
using GroundFn=void*(*)(void*,void*,void*);GroundFn groundHook{};
void* Ground(void* out,void*,void*){return out;}
using AppendFn=uint8_t*(*)(uint8_t*,void*,uint8_t,uint64_t*,uint32_t,uint32_t,uint32_t,uint64_t,uint64_t*);
AppendFn appendHook{};void(*receiveHook)(void*){};
std::string appended;unsigned appendCount=0;
void SetBounds(void*,void*){}
struct Context {void* runtime;uint32_t id;};
uint8_t runtime[0xc0]{},playerDesc[0x600]{},applyDesc[0x1b0]{},input[0x200]{},control[0x100]{},command[0x200]{},component[0x50]{};
uint8_t query[0x140]{},navigation[0x1470]{},grids[0x58]{};uint8_t storage[0x500]{};
teleport::Destination position{};
std::vector<uint8_t> terrain(0x1c08),realm(0x3870),grid(0x180);
uint8_t rayComplete=1,rayHit=1;
template<class T>void Put(void* p,size_t n,T value){memcpy(static_cast<uint8_t*>(p)+n,&value,sizeof value);}
template<class T>T Get(void* p,size_t n){T value;memcpy(&value,static_cast<uint8_t*>(p)+n,sizeof value);return value;}
uint32_t* Entity(void* c,uint32_t* out){*out=static_cast<Context*>(c)->id;return out;}
void Dirty(void*,uint64_t,uint64_t,uint64_t,uint16_t,uint8_t){}
void* Ray(void* out,void*,const float*,const float*,const uint8_t*){
    Put<uint8_t>(out,0x12,rayComplete);Put<uint8_t>(out,0x13,rayHit);Put<float>(out,4,123.25f);return out;
}
bool Iterate(void* c,void*,size_t){if(!yieldEntity||cursor++)return false;static_cast<Context*>(c)->id=1;return true;}
void Receive(void*){}
uint8_t* Append(uint8_t* result,void*,uint8_t,uint64_t* text,uint32_t,uint32_t,uint32_t,uint64_t,uint64_t*){++appendCount;appended.assign(reinterpret_cast<char*>(text[0]),size_t(text[1]));return result;}
uint8_t* Find(const char* pattern,void*){
    using namespace teleport::native;
    if(!strcmp(pattern,queueTeleportPattern))return reinterpret_cast<uint8_t*>(QueueDirect);
    if(!strcmp(pattern,notifyTeleportPattern))return reinterpret_cast<uint8_t*>(NotifyDirect);
    if(!strcmp(pattern,playerPattern))return functions[0];
    if(!strcmp(pattern,applyPattern))return functions[1];
    if(!strcmp(pattern,nextPattern))return functions[2];
    if(!strcmp(pattern,entityPattern))return reinterpret_cast<uint8_t*>(Entity);
    if(!strcmp(pattern,dirtyPattern))return reinterpret_cast<uint8_t*>(Dirty);
    if(!strcmp(pattern,rayPattern))return reinterpret_cast<uint8_t*>(Ray);
    if(strncmp(pattern,"C6 01 00",8)==0)return reinterpret_cast<uint8_t*>(NativeGate);
    if(strncmp(pattern,"40 55 53",8)==0)return functions[2]+100;
    if(strncmp(pattern,"40 55 56",8)==0)return functions[2]+200;
    if(strncmp(pattern,"48 8B C4",8)==0)return functions[2]+300;
    if(strncmp(pattern,"4C 8B DC",8)==0)return functions[2]+400;
    return nullptr;
}
void* Redirect(uint8_t* site,void* detour){
    if(site==functions[0]+0xfb2){gateHook=reinterpret_cast<GateFn>(detour);return reinterpret_cast<void*>(NativeGate);}
    assert(hookCount<4);hooks[hookCount++]=reinterpret_cast<Next>(detour);return reinterpret_cast<void*>(Iterate);}
void* Hook(uint8_t* site,void* detour,size_t length,const uint8_t*,size_t){
    if(site==functions[2]+300){assert(length==19);boundsHook=reinterpret_cast<BoundsFn>(detour);return reinterpret_cast<void*>(SetBounds);}
    if(site==functions[2]+100){receiveHook=reinterpret_cast<void(*)(void*)>(detour);return reinterpret_cast<void*>(Receive);}
    if(site==functions[2]+400){assert(length==13);groundHook=reinterpret_cast<GroundFn>(detour);return reinterpret_cast<void*>(Ground);}
    appendHook=reinterpret_cast<AppendFn>(detour);return reinterpret_cast<void*>(Append);
}
bool ObserveTest(void* ctx,void* desc,size_t n) {
    uint8_t groundDesc[0x100]{};Put(groundDesc,0,ctx);Put(groundDesc,0xf8,terrain.data());
    // Deliberately do not call groundHook: dedicated travel must not depend on material queries.
    return hooks[2](ctx,desc,n);
}
void Read(const void* p,void* out,size_t n){memcpy(out,p,n);}
void Log(const char*){}
void SetupSites(){const uint8_t gate[]={0xb2,0x0b,0x48,0x8b,0xc8,0xe8,0,0,0,0,0x84,0xc0,0x0f,0x85};memcpy(functions[0]+0xe91,gate,sizeof gate);const size_t offsets[2][2]={{0x36,0x21f6},{0x31,0x7be}};for(unsigned i=0;i<2;++i)for(auto offset:offsets[i]){auto site=functions[i]+offset;site[0]=0xe8;Put(site,1,static_cast<int32_t>(functions[2]-(site+5)));}
    auto gateSite=functions[0]+0xfb2;gateSite[0]=0xe8;Put(gateSite,1,static_cast<int32_t>(reinterpret_cast<uint8_t*>(NativeGate)-(gateSite+5)));}
}
int main(int argc,char** argv){
    using namespace teleport;using namespace teleport::travel;
    bool prankMode=argc>1&&!strcmp(argv[1],"prank");
    std::string variant=argc>2?argv[2]:"local";
    bool deluxe=argc>1,known=prankMode||argc<3;
    wchar_t temp[MAX_PATH]{};GetTempPathW(MAX_PATH,temp);
    std::wstring directory=std::wstring(temp)+L"map-teleport-travel-test-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount64());
    assert(CreateDirectoryW(directory.c_str(),nullptr));
    auto ini=directory+L"\\map-teleport.ini";
    assert(WritePrivateProfileStringW(L"Teleport",L"DeluxeTeleports",deluxe?L"true":L"false",ini.c_str()));
    assert(WritePrivateProfileStringW(L"Prank",L"NoFun",prankMode?L"false":L"true",ini.c_str()));
    if(prankMode&&variant=="restart") {
        History previous;assert(previous.open(directory+L"\\map-teleport-history.txt"));
        assert(previous.begin(123,{200LL*4294967296LL,123LL*4294967296LL,900LL*4294967296LL}));
    }
    GreeneyeHost host{};host.size=sizeof host;host.apiVersion=2;host.modDir=directory.c_str();
    host.find=Find;host.read=Read;host.redirectCall=Redirect;host.hook=Hook;host.log=Log;
    host.enter=[](void*){return true;};host.leave=[](){};greeneye::UseHost(&host);
    SetupSites();assert(Init(&host)&&hookCount==4&&gateHook);
    uint8_t manager[0xc0]{};
    Destination box[2]={{0,0,0},{10240LL*4294967296LL,2560LL*4294967296LL,10240LL*4294967296LL}};
    Put(manager,0xb0,box);Put<uint64_t>(manager,0xb8,1);
    assert(boundsHook);if(known)boundsHook(manager,nullptr);
    Put(storage,0x478,terrain.data());
    Put(runtime,8,static_cast<void*>(directQueue));
    Put(runtime,0x30,static_cast<void*>(&storage));Context context{runtime,1};
    Put(applyDesc,0x198,static_cast<void*>(query));Put(query,0x130,static_cast<void*>(navigation));
    Put(navigation,0x1458,grids);Put<uint64_t>(grids,0x10,256); // queue capacity is deliberately not a terrain pointer
    Put(terrain.data(),0x1bf8,realm.data());Put<uint64_t>(terrain.data(),0x1c00,1);
    Put(realm.data(),0,grid.data());Put<uint32_t>(grid.data(),0x10,3);
    Put<uint32_t>(grid.data(),0x14,128);Put<uint32_t>(grid.data(),0x18,32);
    Put(playerDesc,0xb8,static_cast<void*>(command));
    Put(playerDesc,0x80,static_cast<void*>(input));Put(playerDesc,0x2f8,static_cast<void*>(control));Put(playerDesc,0xe8,static_cast<void*>(command));
    Put(applyDesc,0x10,static_cast<void*>(&position));Put(applyDesc,0x100,static_cast<void*>(component));
    // World observation must work even before any Teleport component exists.
    // First-use pranks must prepare from player updates alone, without any
    // prior native Teleport component or teleport-system invocation.
    if(!prankMode)assert(!ObserveTest(&context,applyDesc,sizeof applyDesc));
    Destination target{200LL*4294967296LL,0,900LL*4294967296LL};
    if(prankMode) {
        bool remote=variant=="remote",restart=variant=="restart",disconnect=variant=="disconnect",failedMove=variant=="failed-move";
        std::vector<uint8_t> clientData(0x47d4e8),serverData(0x1e0+16*0x148);
        if(variant=="prediction") {
            testAccount=456;int clientStore=0,predictedWorldStorage=0;
            Put<uint32_t>(clientData.data(),0x466b44,0x40);
            std::vector<uint8_t> clientWorld(0x2c20);
            Put(clientData.data(),0xf8,static_cast<void*>(&clientStore));Put(clientData.data(),0x47d4e0,clientWorld.data());
            SetClient(clientData.data());TrackRemote(900,456);
            uint8_t predictedRuntime[0xc0]{};Put(predictedRuntime,0x30,static_cast<void*>(&predictedWorldStorage));Put<uint8_t>(predictedRuntime,0xa1,1);
            Context predictedContext{predictedRuntime,1};
            Destination ground{target.x,int64_t(123.25*4294967296.0),target.z};
            uint64_t currentNonce=900;
            auto feed=[&](unsigned phase,unsigned countdown=0) {
                char line[160];sprintf_s(line,"/mts1 %llu %u 2 %u 1 %lld %lld %lld",currentNonce,phase,countdown,ground.x,ground.y,ground.z);
                uint64_t text[]={reinterpret_cast<uint64_t>(line),strlen(line)},result=0,time=0;
                appendHook(reinterpret_cast<uint8_t*>(&result),&clientStore,teleport::kChatType,text,0,0,0x40,0xDEB5047AE1C664E7ULL,&time);
            };
            auto predict=[&](uint64_t elapsed=0){testNow+=elapsed;cursor=0;yieldEntity=true;assert(hooks[0](&predictedContext,playerDesc,sizeof playerDesc));assert(!hooks[1](&predictedContext,playerDesc,sizeof playerDesc));};
            feed(1,3);protection::observed=false;predict();assert(protection::Busy()&&ClientProtectionAck()==0);
            protection::observed=true;predict();assert(ClientProtectionAck()==1);
            protection::grounded=false;feed(7);predict(5000);assert(protection::Busy()&&ClientProtectionAck()==1); // still airborne
            protection::grounded=true; // Ground contact works away from the exact target too.
            predict();predict(1999);assert(protection::Busy());
            predict(1);assert(!protection::Busy()&&ClientProtectionAck()==2);
            predict();assert(!protection::Busy()&&ClientProtectionAck()==2); // must not re-enable on repeated restoring notices
            feed(8);assert(RemoteStatus().phase==NoticePhase::complete);TrackRemote(0,456);
            feed(7);predict();assert(!RemoteStatus().nonce&&!protection::Busy()); // delayed response cannot resurrect a completed prank
            currentNonce=901;TrackRemote(currentNonce,456);
            feed(1,3);predict();assert(protection::Busy());
            protection::canDisable=false;SetClient(nullptr);assert(protection::Busy());
            protection::canDisable=true;SetClient(nullptr);assert(!protection::Busy());
            DeleteFileW(ini.c_str());DeleteFileW((directory+L"\\map-teleport-history.txt").c_str());assert(RemoveDirectoryW(directory.c_str()));
            puts("PASS: client prediction protection: readiness acknowledgement, no airborne release, grounded grace, no re-enable, disconnect cleanup");return 0;
        }
        Put<uint32_t>(serverData.data(),0x1e0,1);Put<uint32_t>(serverData.data(),0x1f8,1);Put<uint64_t>(serverData.data(),0x228,123);
        receiveHook(serverData.data());
        if(!remote){testAccount=123;Put(clientData.data(),0x4671d0,static_cast<void*>(manager));SetClient(clientData.data());}
        auto chat=[&](const char* line){uint64_t text[]={reinterpret_cast<uint64_t>(line),strlen(line)},result=0,time=0;appendHook(reinterpret_cast<uint8_t*>(&result),serverData.data(),teleport::kChatType,text,0,0,0,123,&time);};
        auto tick=[&](uint64_t elapsed=0) {
            testNow+=elapsed;cursor=0;yieldEntity=true;
            auto old=Get<uint32_t>(input,0x17c);auto previousDirect=directCount;
            assert(hooks[0](&context,playerDesc,sizeof playerDesc));
            bool injected=Get<uint32_t>(input,0x17c)!=old;
            if(injected){
                Put<uint32_t>(command,8,0x1cc90f15);Put(control,0x54,Get<uint32_t>(input,0x17c));}
            assert(!hooks[1](&context,playerDesc,sizeof playerDesc));
            assert(Get<uint32_t>(input,0x17c)==old&&Get<uint32_t>(control,0x54)==old);return injected||directCount!=previousDirect;
        };
        auto move=[&](bool succeeds=true) {
            memset(component,0,sizeof component);cursor=0;
            if(directReady){
                assert(Get<uint32_t>(directCommand,0xc)==0x23384d40&&Get<uint16_t>(directCommand,0x4c)==0x50);
                assert(Get<uint32_t>(directCommand,0xa4)==0&&Get<uint8_t>(directCommand,0xb4)==0&&Get<uint8_t>(directCommand,0xb5)==0);
                memcpy(component,directCommand+0x6c,sizeof component);directReady=false;
            }
            // Terrain-top moves wait one tick for the confirming ground reading.
            if(!ObserveTest(&context,applyDesc,sizeof applyDesc)){cursor=0;assert(ObserveTest(&context,applyDesc,sizeof applyDesc));}
            if(succeeds)memcpy(&position,component,sizeof position);
            assert(!hooks[3](&context,applyDesc,sizeof applyDesc));
        };
        if(!restart){assert(!tick());if(remote)chat("/mt1 777 858993459200 3865470566400");else assert(Local(1,target));}
        protection::observed=false;
        if(!restart){assert(!tick());assert(!protection::Busy());} // first ground reading, confirmed next tick
        assert(!tick());
        assert(protection::Busy()&&LocalStatus(1).protection&&LocalStatus(1).phase==NoticePhase::waiting);
        assert(!Prepare());Resume();
        protection::observed=true;
        if(remote) {
            assert(!tick(1000)&&LocalStatus(1).phase==NoticePhase::waiting);
            chat("/mtp1 777");Status parsed;assert(ParseStatus(appended,parsed)&&parsed.protection&&parsed.ground.y==int64_t(123.25*4294967296.0));
            chat("/mta1 777 1");
        }
        if(restart)assert(tick());
        else {
            assert(!tick()&&LocalStatus(1).phase==NoticePhase::countdown);
            assert(!tick(1000)&&LocalStatus(1).countdown==2);
            assert(!tick(1000)&&LocalStatus(1).countdown==1);
            assert(tick(1000));move(!failedMove);
            if(failedMove){assert(LocalStatus(1).phase==NoticePhase::returning&&protection::Busy());assert(tick(1000));}
            else {
                assert(position.y==2556LL*4294967296LL&&LocalStatus(1).phase==NoticePhase::scenic&&protection::Busy());
                if(disconnect) {
                    // Simulate leaving the world, then a different character on
                    // the same account rejoining. No second aerial prank occurs.
                    SetClient(nullptr);Put<uint32_t>(serverData.data(),0x1e0,0);
                    cursor=0;yieldEntity=false;assert(!hooks[0](&context,playerDesc,sizeof playerDesc));
                    assert(!protection::Busy());
                    Put<uint32_t>(serverData.data(),0x1e0,1);SetClient(clientData.data());
                    assert(!tick());assert(tick()); // the recovery confirms its ground reading first
                }else {
                    assert(!tick(scenicMs)&&LocalStatus(1).phase==NoticePhase::problem);
                    assert(!tick(problemMs)&&LocalStatus(1).phase==NoticePhase::correction);
                    auto before=directCount;assert(tick(correctionMs));assert(directCount==before+1);
                }
            }
        }
        move();assert(position.y==int64_t(123.25*4294967296.0)&&LocalStatus(1).phase==NoticePhase::restoring);
        assert(!tick(1999)&&protection::Busy());
        protection::canDisable=false;assert(!tick(1)&&protection::Busy());protection::canDisable=true;
        if(remote){assert(!tick()&&protection::Busy());chat("/mta1 777 2");}
        assert(!tick()&&!protection::Busy()&&Latest(1).outcome==Outcome::arrived);
        History saved;assert(saved.open(directory+L"\\map-teleport-history.txt")&&saved.find(123)&&saved.find(123)->complete);
        // Completed first use no longer enables protections; regular deluxe is
        // independent and still uses the high native destination.
        if(remote)chat("/mt1 778 858993459200 3865470566400");else assert(Local(1,target));
        assert(tick()&&!protection::Busy());move();assert(position.y==2556LL*4294967296LL);
        assert(Prepare());SetClient(nullptr);
        DeleteFileW(ini.c_str());DeleteFileW((directory+L"\\map-teleport-history.txt").c_str());assert(RemoveDirectoryW(directory.c_str()));
        printf("PASS: full prank runtime (%s): protection gating, countdown, native hops, ground restoration, persistent first use, regular deluxe\n",variant.c_str());return 0;
    }
    assert(!Local(17,target));
    for(bool nativeMoves:{false,true}) {
        assert(Local(1,target));assert(SubmitLocal(1,target)==Submission::rejected);
        assert(Latest(1).outcome==Outcome::pending);
        Put<uint32_t>(input,0x174,77);Put<uint32_t>(input,0x17c,7);Put<uint32_t>(control,0x54,7);
        uint8_t refusal=0;assert(*gateHook(&refusal,2,0)==1); // other travel keeps the game's restrictions
        cursor=0;yieldEntity=true;assert(hooks[0](&context,playerDesc,sizeof playerDesc));
        assert(Get<uint32_t>(input,0x174)==1&&Get<uint32_t>(input,0x17c)==8);
        assert(*gateHook(&refusal,2,0)==0&&*gateHook(&refusal,0,0)==0); // the injected map teleport passes
        assert(Get<uint8_t>(input,0x178)==1); // native Normal spell-effect variant
        Put<uint32_t>(command,8,0x1cc90f15);Put<uint32_t>(control,0x54,8);
        assert(!hooks[1](&context,playerDesc,sizeof playerDesc));
        assert(Get<uint32_t>(input,0x174)==77&&Get<uint32_t>(input,0x17c)==7&&Get<uint32_t>(control,0x54)==7);
        assert(*gateHook(&refusal,2,0)==1);
        memset(component,0,sizeof component);position={};
        rayComplete=0;cursor=0;assert(!ObserveTest(&context,applyDesc,sizeof applyDesc));
        assert(Latest(1).outcome==Outcome::pending&&position.y==0);
        // The first complete reading waits one tick for its confirming second reading.
        rayComplete=1;cursor=0;assert(!ObserveTest(&context,applyDesc,sizeof applyDesc));
        assert(position.y==0&&Get<uint8_t>(component,0x48)==0);
        cursor=0;assert(ObserveTest(&context,applyDesc,sizeof applyDesc));
        assert(Latest(1).outcome==(deluxe&&!known?Outcome::failed:Outcome::pending)); // writing target is not arrival
        if(nativeMoves&&(!deluxe||known))memcpy(&position,component,sizeof position);
        if(deluxe&&!known)assert(Get<uint8_t>(component,0x48)==1&&position.y==0);
        assert(!hooks[3](&context,applyDesc,sizeof applyDesc));
        auto result=Latest(1);assert(result.outcome==(nativeMoves&&(!deluxe||known)?Outcome::arrived:Outcome::failed));
        assert(result.destination.y==(deluxe&&known?2556LL*4294967296LL:int64_t(123.25*4294967296.0)));
        assert(result.destination.x==target.x&&result.destination.z==target.z);
    }
    assert(Prepare());assert(!Local(1,target));Resume();
    // Exercise the actual chat hook, including account/nonce isolation and host
    // suppression of packets pretending to be host responses.
    std::vector<uint8_t> server(0x1e0+16*0x148),localClient(0x4671d8);
    Put<uint32_t>(server.data(),0x1e0,1);Put<uint32_t>(server.data(),0x1f8,1);Put<uint64_t>(server.data(),0x228,123);
    receiveHook(server.data());
    auto chat=[&](void* store,const char* line,uint64_t account){uint64_t text[]={reinterpret_cast<uint64_t>(line),strlen(line)},result=99,time=0;appendHook(reinterpret_cast<uint8_t*>(&result),store,teleport::kChatType,text,0,0,account==456?0x80:0x40,account,&time);};
    chat(server.data(),"/mt1 777 858993459200 3865470566400",123);
    assert(appended=="/mts1 777 0 0 0");
    chat(server.data(),"/mtp1 777",123);assert(appended=="/mts1 777 0 0 0");
    chat(server.data(),"/mtp1 777",456);assert(appended=="/mts1 777 9 0 0");
    auto before=appendCount;chat(server.data(),"/mts1 777 8 0 0",123);assert(appendCount==before);
    // Ordinary player chat (type 0) is never treated as protocol, even with protocol text.
    {uint64_t text[]={reinterpret_cast<uint64_t>("/mtp1 777"),9},result=99,time=0;appended.clear();
     appendHook(reinterpret_cast<uint8_t*>(&result),server.data(),0,text,0,0,0x40,123,&time);assert(appended=="/mtp1 777");}
    // The host answers the one-time hello in place of the hello line itself.
    chat(server.data(),"/mth1",123);assert(appended==teleport::kHelloReply);
    int clientStore=0;Put<uint32_t>(localClient.data(),0x466b44,0x40);Put(localClient.data(),0xf8,static_cast<void*>(&clientStore));SetClient(localClient.data());
    assert(!HostAnswered());
    before=appendCount;chat(&clientStore,"/mts1 0 hello",456);assert(!HostAnswered()&&appendCount==before); // another player's reply
    chat(&clientStore,"/mts1 0 hello",123);assert(HostAnswered()&&appendCount==before);                   // own reply, consumed
    TrackRemote(777,123);
    chat(&clientStore,"/mts1 777 8 0 0",456);assert(RemoteStatus().phase==NoticePhase::waiting);
    chat(&clientStore,"/mts1 778 8 0 0",123);assert(RemoteStatus().phase==NoticePhase::waiting);
    chat(&clientStore,"/mts1 777 1 2 3",123);assert(RemoteStatus().phase==NoticePhase::countdown&&RemoteStatus().countdown==3);
    // Native account identifiers differ from Steam IDs. Full player handles
    // still distinguish another player/generation even with the same slot.
    chat(&clientStore,"/mts1 777 8 0 0",0xDEB5047AE1C664E7ULL);assert(RemoteStatus().phase==NoticePhase::complete);
    chat(&clientStore,"/mts1 777 0 0 0",123);assert(RemoteStatus().phase==NoticePhase::complete);
    assert(appendCount==before);TrackRemote(0,0);SetClient(nullptr);
    assert(!Prepare()); // a host request is still queued; do not unload its hooks
    Resume();cursor=0;assert(hooks[0](&context,playerDesc,sizeof playerDesc));
    Put<uint32_t>(command,8,0x1cc90f15);assert(!hooks[1](&context,playerDesc,sizeof playerDesc));
    // Disconnect after native action acceptance. Player iteration must retain a
    // cancellation record until the already emitted Teleport is intercepted.
    Put<uint32_t>(server.data(),0x1e0,0);
    cursor=0;assert(hooks[0](&context,playerDesc,sizeof playerDesc));assert(!hooks[1](&context,playerDesc,sizeof playerDesc));
    assert(Latest(1).outcome==Outcome::failed&&!Prepare());
    memset(component,0,sizeof component);Put<uint32_t>(component,0x38,1);
    cursor=0;assert(ObserveTest(&context,applyDesc,sizeof applyDesc));
    assert(Get<uint8_t>(component,0x48)==1);assert(!hooks[3](&context,applyDesc,sizeof applyDesc));
    assert(Prepare());
    DeleteFileW(ini.c_str());DeleteFileW((directory+L"\\map-teleport-history.txt").c_str());RemoveDirectoryW(directory.c_str());
    printf("PASS: simulated native travel (%s, bounds %s): input restoration, travel gate opened only for the injected action, terrain deferral, explicit position-confirmed arrival and failure\n",deluxe?"deluxe":"normal",known?"known":"unavailable");
    puts("PASS: simulated host/client status: own-account/nonce matching, spoof suppression, phase delivery, stale terminal updates, disconnect cancellation and unload guard");
}
