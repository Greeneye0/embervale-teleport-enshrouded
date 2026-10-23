#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <string>
#include <string_view>
#include "greeneye.h"
#include "countdown.h"
#include "travel.h"
#include "protocol.h"
#include "popup.h"
#include "identity.h"
#include "journey.h"
#include "protection.h"

namespace {
template<class T> T& at(void* p,size_t n) {return *reinterpret_cast<T*>(static_cast<uint8_t*>(p)+n);}
using MapFn=void(*)(void*,uint8_t*);
using LoopFn=void(*)(void*);
using DispatchFn=void(*)(void*,void*);
using DrawFn=void(*)(void*,const float*,const uint32_t*,const uint64_t*);
using ViewFn=void(*)(void*,void*,uint8_t);
MapFn nextMap=nullptr; LoopFn nextLoop=nullptr; DispatchFn dispatch=nullptr;
LoopFn nextPlay=nullptr;
DrawFn draw=nullptr; ViewFn updateView=nullptr;
void** root=nullptr;
void* currentApp=nullptr;
// UI snapshots and the live game client are distinct objects. Use the command
// loop's live client for world identity, player state, and command dispatch.
void* GameClient(void* app) {
    auto ui=app?at<void*>(app,0x35280):nullptr;
    return ui?at<void*>(ui,0x1b8):nullptr;
}
teleport::Countdown countdown;
uint64_t frame=0,seen=0;
void* owner=nullptr; void* world=nullptr;
bool lastDown=true, swallow=false, paused=false, broken=false;
struct Watching {
    void* client{};void* world{};uint64_t nonce{},nextPoll{},deadline{};
    uint32_t entity{};bool remote{};
    teleport::NoticePhase shown=teleport::NoticePhase::failed;unsigned countdown=0;bool prank=false;
} watching;
void Log(const char* message) {greeneye::Host()->log(message);}
bool Down(int key) {return (GetAsyncKeyState(key)&0x8000)!=0;}
bool Cursor(float& x,float& y) {
    HWND w=GetForegroundWindow(); DWORD pid=0; GetWindowThreadProcessId(w,&pid);
    POINT p{}; RECT r{};
    if(pid!=GetCurrentProcessId()||!GetCursorPos(&p)||!ScreenToClient(w,&p)||!GetClientRect(w,&r)||r.right<=0||r.bottom<=0||p.x<0||p.y<0||p.x>=r.right||p.y>=r.bottom) return false;
    x=p.x*(1080.f/r.bottom); y=p.y*(1080.f/r.bottom); return true;
}
void SendLine(void* client,std::string_view text) {
    alignas(8) uint8_t command[0x410]{};command[0]=0x74;command[1]=teleport::kChatType;
    memcpy(command+2,text.data(),text.size());at<uint16_t>(command,0x402)=uint16_t(text.size());dispatch(client,command);
}
// A remote host is asked (twice at most) per world whether it runs the mod. Until it answers, the client
// sends no protocol lines: on a host without the mod they would show up as player chat.
enum class HostMod {present,checking,absent};
struct Link {void* client{};void* world{};uint64_t helloAt{},deadline{};int asked{};bool absent{};} link;
HostMod CheckHost(void* client,void* currentWorld) {
    if(teleport::travel::HostsWorld()||teleport::travel::HostAnswered())return HostMod::present;
    auto now=GetTickCount64();
    if(link.client!=client||link.world!=currentWorld||!link.deadline)link={client,currentWorld,now+3000,now+15000};
    // Ask twice (3 s and 9 s after joining), in case the first hello is lost while the world loads.
    if(link.asked<2&&now>=link.helloAt){
        ++link.asked;link.helloAt=now+6000;SendLine(client,teleport::kHello);
        Log(link.asked==1?"Map Teleport: asking the host whether it runs Embervale Teleport":"Map Teleport: asking the host again");
    }
    if(now<link.deadline)return HostMod::checking;
    if(!link.absent){link.absent=true;Log("Map Teleport: host did not answer; remote teleport off for this world");}
    return HostMod::absent;
}
void Cancel(const char* reason="state reset") {
    if(countdown.active) {char message[160];sprintf_s(message,"Map Teleport: hold ended (%s), elapsed=%llu ms",reason,GetTickCount64()-countdown.started);Log(message);}
    countdown.cancel();owner=nullptr;world=nullptr;
}
void Ring(void*,float x,float y,float progress) {teleport::popup::DrawRing(x,y,progress);}
bool MapFrame(void* widget,uint8_t* state) {
    if(paused||broken) return false;
    bool consecutive=seen+1==frame || seen==frame;
    seen=frame;
    float x=0,y=0; bool focused=Cursor(x,y), down=Down(VK_LBUTTON);
    if(!consecutive) {Cancel("map frame gap");lastDown=down;swallow=false;}
    bool clicked=down&&!lastDown; lastDown=down;
    if(!down) Cancel("mouse released in map"); // early release cancels
    if(!focused||Down(VK_ESCAPE)||Down(VK_RBUTTON)) {Cancel("map focus or cancel key");return false;}
    auto* client=GameClient(currentApp);
    auto* currentWorld=client?at<void*>(client,0x47d4e0):nullptr;
    auto* view=state?at<uint8_t*>(state,8):nullptr;
    if(!currentWorld||!view) {Cancel("missing world or view");return false;}
    if(countdown.active&&(owner!=state||world!=currentWorld)) Cancel("map owner or world changed");
    bool chord=Down(VK_SHIFT);
    if(clicked&&chord&&!countdown.active) {
        teleport::Destination target{};
        float ox=at<float>(view,0x1e0)+at<float>(view,0x17c)-at<float>(view,0x1e8);
        float oy=at<float>(view,0x1e4)+at<float>(view,0x180)-at<float>(view,0x1ec);
        if(teleport::Convert(x,y,ox,oy,at<float>(view,0x10),at<float>(view,0x38),at<float>(view,0x3c),target)) {
            countdown.start(GetTickCount64(),target); owner=state;world=currentWorld;swallow=true;
            Log("Map Teleport: countdown started");
        }
    }
    bool block=countdown.active||swallow;
    if(block) {
        at<uint8_t>(state,0xf8)=0; at<uint64_t>(state,0x56310)=0;at<uint64_t>(state,0x28)=0;
        alignas(16) uint8_t action[64]{}; updateView(view,action,1);
    }
    if(!down) swallow=false; // swallow the release frame too
    if(countdown.active) Ring(widget,x,y,countdown.progress(GetTickCount64()));
    return block;
}
bool SafeMap(void* widget,uint8_t* state) {
    __try {return MapFrame(widget,state);} __except(EXCEPTION_EXECUTE_HANDLER) {Cancel();broken=true;Log("Map Teleport: disabled after invalid game state");return false;}
}
void MapHook(void* widget,uint8_t* state) {
    greeneye::Lease lease;
    if(!lease || !SafeMap(widget,state)) nextMap(widget,state);
}
void EndFrame(void* app) {
    if(paused||broken) {Cancel();return;}
    float x,y;
    // Recheck LMB here too: it can be released between map rendering and dispatch.
    if(seen!=frame) {Cancel("dispatch without map frame");return;}
    if(!Cursor(x,y)||!Down(VK_LBUTTON)||Down(VK_ESCAPE)||Down(VK_RBUTTON)) {Cancel("dispatch focus, mouse release or cancel key");return;}
    if(!countdown.due(GetTickCount64())) return;
    auto* client=GameClient(app);
    if(!client||at<void*>(client,0x47d4e0)!=world) {Cancel("dispatch world mismatch");return;}
    auto target=countdown.target;
    teleport::travel::SetClient(client);
    Cancel(); // one dispatch per click, including across subsequent frames
    uint32_t entity=(at<uint32_t>(client,0x466b44)&0x3f)+1;
    if(watching.nonce){teleport::popup::Show(teleport::popup::Say(teleport::text::Title),teleport::popup::Say(teleport::text::InProgress),2000);return;}
    uint64_t now=GetTickCount64();
    watching={client,at<void*>(client,0x47d4e0),0,now+500,now+35000,entity};
    auto submitted=teleport::travel::SubmitLocal(entity,target);
    if(submitted==teleport::travel::Submission::queued)watching.nonce=teleport::travel::Latest(entity).nonce;
    else if(submitted==teleport::travel::Submission::rejected) {
        watching={};teleport::popup::Show(teleport::popup::Say(teleport::text::Title),teleport::popup::Say(teleport::text::HostBusy),4000);return;
    } else {
        auto hostMod=CheckHost(client,at<void*>(client,0x47d4e0));
        if(hostMod!=HostMod::present){
            watching={};
            teleport::popup::Show(teleport::popup::Say(teleport::text::Title),teleport::popup::Say(hostMod==HostMod::checking?teleport::text::StillChecking:teleport::text::ServerMissing),6000);
            return;
        }
        auto account=teleport::LocalAccount();
        if(!account){watching={};teleport::popup::Show(teleport::popup::Say(teleport::text::Title),teleport::popup::Say(teleport::text::NoAccount),5000);return;}
        static uint64_t lastNonce=0;lastNonce=now>lastNonce?now:lastNonce+1;
        watching.nonce=lastNonce;watching.remote=true;
        teleport::travel::TrackRemote(watching.nonce,account);
        char text[96];int len=sprintf_s(text,"/mt1 %llu %lld %lld",watching.nonce,target.x,target.z);
        if(len<=0)return;
        SendLine(client,{text,size_t(len)});
    }
    // Shift-only confirmation needs no Ctrl-release delay before returning to play.
    alignas(8) uint8_t openPlay[0x410]{};openPlay[0]=0x01;openPlay[1]=1;dispatch(client,openPlay);
    Log("Map Teleport: travel request queued; waiting for host terrain resolution");
}
void StatusFrame(void* app) {
    using namespace teleport;
    auto current=GameClient(app);
    auto currentWorld=current?at<void*>(current,0x47d4e0):nullptr;
    travel::SetClient(currentWorld?current:nullptr);
    // A new session can reuse the old client and world addresses, so forget the old host here.
    if(!currentWorld){watching={};link={};popup::Clear();return;}
    uint32_t entity=(at<uint32_t>(current,0x466b44)&0x3f)+1;
    if(watching.nonce&&(watching.client!=current||watching.world!=currentWorld)){watching={};travel::TrackRemote(0,LocalAccount());popup::Clear();}
    auto hostMod=CheckHost(current,currentWorld);
    auto send=[&](const char* text){if(hostMod==HostMod::present)SendLine(current,text);};
    if(!watching.nonce) {
        auto local=travel::LocalStatus(entity),remote=travel::RemoteStatus();
        auto active=local.protection&&!Finished(local)?local:remote;
        if(active.nonce&&active.protection&&!Finished(active))watching={current,currentWorld,active.nonce,0,GetTickCount64()+35000,entity,active.nonce==remote.nonce};
        else {
            static uint64_t discovery=0;
            if(GetTickCount64()>=discovery){discovery=GetTickCount64()+2000;send("/mtp1 0");}
            return;
        }
    }
    auto now=GetTickCount64();Status status{watching.nonce};
    if(watching.remote) {
        status=travel::RemoteStatus();
        if(!Finished(status)&&now>=watching.nextPoll&&now<watching.deadline) {
            watching.nextPoll=now+500;
            char text[64];auto ack=travel::ClientProtectionAck();
            if(status.protection&&ack)sprintf_s(text,"/mta1 %llu %u",watching.nonce,ack);
            else sprintf_s(text,"/mtp1 %llu",watching.nonce);
            send(text);
        }
    } else {
        status=travel::LocalStatus(watching.entity);
        if(status.nonce!=watching.nonce)status.phase=NoticePhase::failed;
    }
    watching.prank=watching.prank||status.protection;
    if(status.phase!=watching.shown||status.countdown!=watching.countdown)watching.deadline=now+35000;
    if(!Finished(status)&&now>=watching.deadline) {
        popup::Show(popup::Say(text::Title),popup::Say(status.phase==NoticePhase::waiting?text::SlowPreparing:text::NoArrival),6000);
        if(status.protection){watching.deadline=now+35000;return;}
        watching={};travel::TrackRemote(0,LocalAccount());return;
    }
    if(status.phase!=watching.shown||status.countdown!=watching.countdown) {
        watching.shown=status.phase;watching.countdown=status.countdown;
        std::string_view message;char countdownText[96]{};
        switch(status.phase) {
            case NoticePhase::waiting:message=popup::Say(text::Preparing);break;
            case NoticePhase::countdown:sprintf_s(countdownText,popup::Say(text::Countdown),status.countdown);message=countdownText;break;
            case NoticePhase::scenic:message=text::ScenarioLine(popup::Language(),status.scenario,0);break;
            case NoticePhase::problem:message=text::ScenarioLine(popup::Language(),status.scenario,1);break;
            case NoticePhase::correction:message=text::ScenarioLine(popup::Language(),status.scenario,2);break;
            case NoticePhase::returning:break; // Keep the preceding text; do not restart its typewriter.
            case NoticePhase::complete:message=popup::Say(watching.prank?text::PrankThankYou:text::ThankYou);break;
            case NoticePhase::failed:message=popup::Say(text::Failed);break;
            case NoticePhase::ascending:popup::Clear();break;
            default:break;
        }
        uint64_t displayMs=Finished(status)?(watching.prank?7000:4000):7000;
        if(status.phase==NoticePhase::countdown)displayMs=2000;
        else if(status.phase==NoticePhase::problem)displayMs=problemMs;
        else if(status.phase==NoticePhase::scenic)displayMs=scenicMs;
        else if(status.phase==NoticePhase::correction)displayMs=correctionMs;
        if(!message.empty())popup::Show(popup::Say(text::Title),std::string(message),displayMs,status.phase!=NoticePhase::countdown);
    }
    if(Finished(status)){watching={};travel::TrackRemote(0,LocalAccount());}
}
void SafeStatus(void* app){__try{StatusFrame(app);}__except(EXCEPTION_EXECUTE_HANDLER){watching={};Log("Map Teleport: notice tracking failed; active protection retained");}}
void SafeEnd(void* app) {__try {EndFrame(app);} __except(EXCEPTION_EXECUTE_HANDLER) {Cancel();broken=true;Log("Map Teleport: dispatch disabled after invalid game state");}}
void LoopHook(void* app) {
    greeneye::Lease lease;
    if(!lease) {nextLoop(app);return;}
    // This consumes UI commands; the map is drawn BETWEEN calls, not inside this function.
    currentApp=app;
    nextLoop(app); SafeStatus(app); SafeEnd(app); ++frame;
}
void DrawPopup(){__try{teleport::popup::Draw();}__except(EXCEPTION_EXECUTE_HANDLER){teleport::popup::Clear();Log("Map Teleport: native notice rendering failed");}}
void PlayHook(void* self){greeneye::Lease lease;if(lease)DrawPopup();nextPlay(self);}
bool Prepare(){paused=true;Cancel();if(watching.nonce||!teleport::travel::Prepare())return false;teleport::popup::Clear();return true;}
bool Ready(){return true;}
void Resume(){paused=false;lastDown=true;teleport::travel::Resume();}
}
GREENEYE_MOD_START {
    if(!greeneye::HostHasHooks(host)||!host->find||!host->read) return false;
    greeneye::UseHost(host);
    auto* exe=reinterpret_cast<uint8_t*>(GetModuleHandleW(nullptr));
    auto* nt=reinterpret_cast<IMAGE_NT_HEADERS64*>(exe+reinterpret_cast<IMAGE_DOS_HEADER*>(exe)->e_lfanew);
    bool serverSide=host->side==kGreeneyeServer;
    if(nt->FileHeader.TimeDateStamp!=(serverSide?0x6ad77f14u:0x6ad77f0cu) || nt->OptionalHeader.SizeOfImage!=(serverSide?0x1dd1000u:0x2a1e000u)) {
        Log("Map Teleport: unverified game build; no hooks installed");return false;
    }
    if(!greeneye::ClaimProcess(L"MapTeleport",host)) return false;
    if(!teleport::protection::Init(host)){Log("Map Teleport: protection bindings unavailable; disabled");return false;}
    if(!teleport::travel::Init(host)) {Log("Map Teleport: host bindings unavailable; disabled");return false;}
    if(serverSide) {Log("Embervale Teleport 1.1.1: server companion loaded");return true;}
    auto mapTail=host->find("48 8D AC 24 A0 F8 FF FF 48 81 EC 60 08 00 00 48 8B 05",nullptr);
    auto loop=host->find("40 53 55 56 57 41 54 41 55 41 56 41 57 48 81 EC 98 00 00 00 48 8D 81 58 4F 03 00",nullptr);
    auto send=host->find("4C 8B DC 55 56 57 49 8D AB 18 FF FF FF",nullptr);
    auto image=host->find("4D 8B 09 45 8B 00 E9 ?? ?? ?? ?? CC",nullptr);
    auto view=host->find("48 8B C4 48 89 58 20 55 56 57 41 56 41 57 48 8D 68 98 48 81 EC 40 01 00 00",nullptr);
    auto play=host->find("4C 8B DC 55 49 8D AB E8 FD FF FF 48 81 EC 10 03 00 00",nullptr);
    if(!mapTail||!loop||!send||!image||!view||!play) {Log("Map Teleport: unsupported game signatures; no UI hooks installed");return false;}
    int32_t displacement=0;host->read(mapTail+0x12,&displacement,4);
    root=reinterpret_cast<void**>(mapTail+0x16+displacement);
    if(!teleport::popup::Init(host,root)){Log("Map Teleport: native notice bindings unavailable");return false;}
    dispatch=reinterpret_cast<DispatchFn>(send);draw=reinterpret_cast<DrawFn>(image);updateView=reinterpret_cast<ViewFn>(view);
    nextMap=reinterpret_cast<MapFn>(host->hook(mapTail-16,reinterpret_cast<void*>(MapHook),16,nullptr,0));
    nextLoop=reinterpret_cast<LoopFn>(host->hook(loop,reinterpret_cast<void*>(LoopHook),13,nullptr,0));
    nextPlay=reinterpret_cast<LoopFn>(host->hook(play,reinterpret_cast<void*>(PlayHook),18,nullptr,0));
    if(!nextMap||!nextLoop||!nextPlay) {paused=true;Log("Map Teleport: hook installation failed");return false;}
    Log("Embervale Teleport 1.1.1: Shift+hold left mouse on world map for one second; early release cancels; no release delay");return true;
}
GREENEYE_MOD_HOT {static const GreeneyeHot api{sizeof(GreeneyeHot),Prepare,Ready,Resume};return &api;}
BOOL WINAPI DllMain(HINSTANCE instance,DWORD reason,LPVOID) {if(reason==DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(instance);return TRUE;}
