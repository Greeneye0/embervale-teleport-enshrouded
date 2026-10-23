#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "popup.h"
#include "settings.h"
#include <mutex>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cmath>

namespace teleport::popup {
namespace {
struct Text {const char* bytes;size_t length;uint64_t hash;};
Text MakeText(const char* s){Text t{s,strlen(s),0xcbf29ce484222325ull};for(size_t i=0;i<t.length;++i)t.hash=(t.hash^uint8_t(s[i]))*0x100000001b3ull;return t;}
template<class T>T& at(void* p,size_t n){return *reinterpret_cast<T*>(static_cast<uint8_t*>(p)+n);}
using Begin=void(*)(void*,uint64_t);using End=void(*)(void*);using Size=void(*)(void*,uint64_t);
using Layer=void*(*)(void*,const Text*,uint8_t,uint32_t,int32_t,uint32_t);
using Named=void*(*)(void*,const Text*);using Opaque=void*(*)(void*);
using Label=void(*)(uint8_t*,const Text*,const uint8_t*,uint8_t);
using DrawImage=void(*)(void*,const float*,const uint32_t*,const uint64_t*);
using LocaText=Text*(*)(Text*,uint32_t);LocaText loca{};
// FbUiLocaLinks (assets + 0x10) -> rpgTerms (+0x40) -> rarityLegendary (+0x1dd0): a 16-byte key.
constexpr size_t kLocaLegendary=0x10+0x40+0x1dd0;
std::atomic<int> forced{-1},detected{-1}; // -1: not known yet (English)
uint64_t nextDetect=0;
Begin begin{};End end{},endLayer{},endLabel{};Size size{};Layer layer{};Named named{};Opaque opaque{};Label label{};DrawImage draw{};
void** root=nullptr;
std::mutex mutex;std::string heading,body;uint64_t until=0,started=0,revealMs=0;bool ready=false,showBanners=true;
uint64_t Pack(float x,float y){float xy[]={x,y};uint64_t value;memcpy(&value,xy,8);return value;}
struct Widget {alignas(16) uint8_t raw[0x80]{};void* handle(){return at<void*>(raw,8);}};
struct Box {
    Widget widget;
    Box(float x,float y,float w,float h){begin(widget.raw,0);size(widget.handle(),Pack(w,h));at<uint64_t>(widget.handle(),0x70)=Pack(x,y);}
    ~Box(){end(widget.raw);}
};
void Fill(Box& box,uint32_t color) {
    uint32_t image=0;struct Color {uint32_t rgba;float brightness;} c{color,1};uint64_t packed;memcpy(&packed,&c,8);
    draw(box.widget.raw,static_cast<float*>(static_cast<void*>(static_cast<uint8_t*>(box.widget.handle())+0x114)),&image,&packed);
}
void Line(const std::string& value,void* assets,uint32_t color,uint64_t visible=~uint64_t(0)) {
    auto text=MakeText(value.c_str());alignas(16) uint8_t buffer[0x400]{};
    label(buffer,&text,static_cast<uint8_t*>(assets)+0x10a0c,0);
    buffer[0x3d]=1;at<uint32_t>(buffer,0xb0)=color;at<float>(buffer,0xb4)=1;
    // Native EndLabel uses +0xa0 as its visible-glyph limit (normally -1).
    // Shape and measure the complete line once, then reveal glyphs without
    // changing its geometry, kerning, baseline or widget identity each frame.
    at<uint64_t>(buffer,0xa0)=visible;
    endLabel(buffer);
}
void Log(const char* s){if(auto* host=greeneye::Host())host->log(s);}
uint32_t Fnv1a32(const uint8_t* p,size_t n){uint32_t h=0x811c9dc5u;for(size_t i=0;i<n;++i)h=(h^p[i])*0x1000193u;return h;}
// The game's language can change while it runs; look at its word for "Legendary" once a second.
void DetectLanguage(void* assets) {
    auto now=GetTickCount64();
    if(!loca||!assets||now<nextDetect)return;
    nextDetect=now+1000;
    Text result=MakeText("");loca(&result,Fnv1a32(static_cast<uint8_t*>(assets)+kLocaLegendary,16));
    if(!result.bytes||!result.length)return;
    std::string word(result.bytes,(std::min)(result.length,size_t(64)));text::Lang lang;
    if(!text::Detect(word,lang)||int(lang)==detected.load())return;
    detected=int(lang);
    char line[160];sprintf_s(line,"Map Teleport: notices in %s (the game's \"Legendary\": %s)",text::kCodes[size_t(lang)],word.c_str());Log(line);
}
template<class T>bool Bind(const GreeneyeHost* host,T& out,const char* pattern){out=reinterpret_cast<T>(host->find(pattern,nullptr));return out!=nullptr;}
}
bool Init(const GreeneyeHost* host,void** uiRoot) {
    if(host->modDir){
        auto ini=std::wstring(host->modDir)+L"\\map-teleport.ini";
        showBanners=ReadShowBanners(ini.c_str());forced=ReadLanguage(ini.c_str());
    }
    root=uiRoot;bool ok=true;
    ok&=Bind(host,begin,"40 53 48 83 EC 20 48 8B 05 ?? ?? ?? ?? 48 8B D9 45 33 C0");
    ok&=Bind(host,end,"48 89 5C 24 18 48 89 74 24 20 57 48 83 EC 60 80 79 20 00");
    ok&=Bind(host,size,"48 89 54 24 08 0F 57 C0 F3 0F 10 54 24 08");
    ok&=Bind(host,layer,"48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 50 48 8B 05 ?? ?? ?? ?? 45 8B D9 44 8B 8C 24 80 00 00 00 41 0F B6 D8");
    ok&=Bind(host,endLayer,"40 53 48 83 EC 20 48 89 74 24 38 48 8B D9 48 8B 35 ?? ?? ?? ?? 48 89 7C 24 40 48 8B 79 10 48 8B 09 E8");
    ok&=Bind(host,named,"48 89 5C 24 08 57 48 83 EC 20 48 8B 05 ?? ?? ?? ?? 48 8B F9 48 8B DA 45 33 C0 48 8B 52 10 48 8B 48 18 48 89 0F 48 C7 47");
    ok&=Bind(host,opaque,"40 53 48 83 EC 20 F3 0F 10 0D ?? ?? ?? ?? 48 8B D9 48 8B 49 08 0F 28 C1 0F 14 C1 66 48 0F 7E C2 E8");
    ok&=Bind(host,label,"48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 41 56 48 83 EC 50 48 8B DA 41 0F B6 F9");
    ok&=Bind(host,endLabel,"40 55 56 57 48 8D 6C 24 B9 48 81 EC F0 00 00 00 0F 29 BC 24 B0 00 00 00");
    ok&=Bind(host,draw,"4D 8B 09 45 8B 00 E9 ?? ?? ?? ?? CC");
    // Optional: without it the notices stay in English (or the ini's language).
    if(!Bind(host,loca,"48 89 5C 24 10 48 89 74 24 18 55 57 41 55 41 56 41 57 48 8D AC 24 F0 F8 FF FF"))
        Log("Map Teleport: game text lookup not found; notices use English or [Messages] Language");
    ready=ok;return ok;
}
text::Lang Language(){int f=forced.load();if(f<0)f=detected.load();return text::Lang(f>=0?f:0);}
void Show(std::string title,std::string message,uint64_t durationMs,bool typewriter){
    if(!showBanners)return;
    std::lock_guard guard(mutex);heading=std::move(title);body=std::move(message);
    started=GetTickCount64();until=started+(std::min)(durationMs,60000ULL);
    revealMs=typewriter?(std::min)(uint64_t(Codepoints(body))*16,durationMs/2):0;
}
void Clear(){std::lock_guard guard(mutex);until=0;heading.clear();body.clear();}
bool Visible(){std::lock_guard guard(mutex);return ready&&GetTickCount64()<until;}
void DrawRing(float x,float y,float progress) {
    if(!ready)return;
    if(root&&*root)DetectLanguage(at<void*>(*root,200));
    auto name=MakeText("embervaleTeleportCursor");alignas(16) uint8_t frame[0x400]{};
    layer(frame,&name,1,0x16,-1,0);
    Widget parent;named(parent.raw,&name);opaque(parent.raw);
    for(int i=0;i<96;++i) {
        float angle=float(i)*6.28318530718f/96-1.57079632679f;
        Box dot(x+24*std::cos(angle)-2,y+24*std::sin(angle)-2,4,4);
        Fill(dot,float(i)/96<progress?0xff40dfff:0xd0303030);
    }
    end(parent.raw);endLayer(frame);
}
void Draw() {
    if(!ready||!root||!*root)return;
    DetectLanguage(at<void*>(*root,200));
    if(!showBanners)return;
    std::string title,message;size_t revealed=0; // in code points
    {std::lock_guard guard(mutex);auto now=GetTickCount64();if(now>=until)return;title=heading;message=body;
        auto total=Codepoints(message);revealed=revealMs?(std::min)(total,size_t((now-started)*total/revealMs)):total;}
    auto assets=at<void*>(*root,200);if(!assets)return;
    auto name=MakeText("embervaleTeleportNotice");alignas(16) uint8_t frame[0x400]{};
    layer(frame,&name,1,0x16,-1,0);
    Widget parent;named(parent.raw,&name);opaque(parent.raw);
    auto rect=static_cast<float*>(static_cast<void*>(static_cast<uint8_t*>(parent.handle())+0x114));
    float width=(std::min)(840.f,rect[2]-40.f),left=(rect[2]-width)*.5f;
    if(width>200) {
        auto lines=Wrap(message,(std::max)(size_t(10),size_t((width-48.f)/14.f)));
        {Box background(left,90,width,90+float(lines.size())*30);Fill(background,0xf0201711);}
        {Box accent(left,90,width,3);Fill(accent,0xff68cce8);}
        {Box row(left+24,110,width-48,36);Line(title,assets,0xff68cce8);}
        // Wrap the complete message first, keeping the window and words in
        // place while the body prints one character at a time.
        size_t offset=0,before=0,counted=0; // bytes; code points before offset; bytes counted
        for(size_t i=0;i<lines.size();++i){
            offset=message.find(lines[i],offset);if(offset==std::string::npos)break;
            before+=Codepoints(std::string_view(message).substr(counted,offset-counted));counted=offset;
            auto length=Codepoints(lines[i]);
            auto count=revealed>before?(std::min)(length,revealed-before):0;
            {Box row(left+24,156+float(i)*30,width-48,30);Line(lines[i],assets,0xffffffff,count);}
            offset+=lines[i].size();
        }
    }
    end(parent.raw);endLayer(frame);
}
}
