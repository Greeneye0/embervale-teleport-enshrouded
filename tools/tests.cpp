#include "../src/countdown.h"
#include "../src/protocol.h"
#include "../src/terrain.h"
#include "../src/settings.h"
#include "../src/history.h"
#include "../src/altitude.h"
#include "../src/arrival.h"
#include "../src/popup.h"
#include "../src/status.h"
#include <cassert>
#include <limits>
#include <cstdio>
#include <vector>
#include <initializer_list>
#include <utility>
namespace {
int rayCalls=0;uint8_t complete=1,hitFlag=1;float ground=123.25f,rayStart=256.25f;
void* FakeRay(void* output,void*,const float* from,const float* to,const uint8_t* options) {
    ++rayCalls;assert(from[1]==rayStart&&to[1]==0&&from[0]==200&&from[2]==900);
    assert(options[2]==1&&options[3]==1);
    auto p=static_cast<uint8_t*>(output);p[0x12]=complete;p[0x13]=hitFlag;
    memcpy(p+4,&ground,4);return p;
}
template<class T> void Put(void* p,size_t at,T value){memcpy(static_cast<uint8_t*>(p)+at,&value,sizeof value);}
}
int main() {
 using namespace teleport;
 uint64_t poll=0;assert(ParsePoll("/mtp1 123",poll)&&poll==123);
 for(auto invalid:{"/mtp1 0","/mtp1 -1","/mtp1 123 extra","/mtp1  123","/mtp1 18446744073709551616"})assert(!ParsePoll(invalid,poll));
 Status status{};assert(ParseStatus("/mts1 123 1 2 3",status));
 assert(status.phase==NoticePhase::countdown&&status.scenario==2&&status.countdown==3);
 assert(MatchesStatus(status,123,456,456)&&!MatchesStatus(status,124,456,456)&&!MatchesStatus(status,123,789,456)&&!MatchesStatus(status,123,0,0));
 for(auto invalid:{"/mts1 0 0 0 0","/mts1 123 10 0 0","/mts1 123 3 3 0","/mts1 123 1 0 0","/mts1 123 0 0 1","/mts1 123 1 0 4","/mts1 123 8 0 0 extra"})assert(!ParseStatus(invalid,status));
 assert(ParseStatus("/mts1 123 8 0 0",status)&&Finished(status));
 Destination p{};
 assert(Convert(500,300,100,100,2,1000,1000,p));
 assert(p.x==int64_t(200)*4294967296LL&&p.z==int64_t(900)*4294967296LL&&p.y==0);
 Destination q{};
 assert(Convert(140,70,-60,-30,1,1000,1000,q));
 assert(q.x==p.x&&q.z==p.z); // same point after zoom/pan
 assert(!Convert(-1,0,0,0,1,1000,1000,q));
 assert(!Convert(0,0,0,0,0,1000,1000,q));
 assert(!Convert(0,0,0,0,std::numeric_limits<float>::quiet_NaN(),1000,1000,q));
 assert(!Convert(0,0,0,0,1,INFINITY,1000,q));
 Countdown c;assert(!c.due(10000));c.start(500,p);
 assert(!c.due(1499)&&c.due(1500)&&c.due(1600));
 assert(c.progress(1000)==0.5f&&c.progress(1800)==1);
 assert(c.target.x==p.x);c.cancel();assert(!c.due(2000));
 c.start(3000,q);assert(!c.due(3999)&&c.due(4000));
 Request request;
 assert(ParseRequest("/mt1 123 858993459200 3865470566400",request));
 assert(request.nonce==123&&request.position.x==p.x&&request.position.z==p.z);
 for(auto bad:{"/mt1 0 1 2","/mt1 1 -1 2","/mt1 1 1 -2","/mt1 1 1 2 garbage","/mt1 1 1 2\n","/mt1 1  1 2","/mt1 1 999999999999999999999999 2","/mt1 1 429496729600001 2"})
     assert(!ParseRequest(bad,request));
 std::vector<uint8_t> terrain(0x1c08),realm(0x3870),grid(0x180);
 Put(terrain.data(),0x1bf8,realm.data());Put<uint64_t>(terrain.data(),0x1c00,1);
 Put(realm.data(),0,grid.data());Put<uint32_t>(grid.data(),0x10,3);
 Put<uint32_t>(grid.data(),0x14,128);Put<uint32_t>(grid.data(),0x18,32);
 auto dest=p;
 assert(TopSurface(FakeRay,terrain.data(),0,dest)==Surface::ready);
 assert(dest.y==int64_t(123.25*4294967296.0));
 complete=0;assert(TopSurface(FakeRay,terrain.data(),0,dest)==Surface::loading);
 complete=1;hitFlag=0;assert(TopSurface(FakeRay,terrain.data(),0,dest)==Surface::invalid);
 hitFlag=1;ground=INFINITY;assert(TopSurface(FakeRay,terrain.data(),0,dest)==Surface::invalid);
 int before=rayCalls;
 assert(TopSurface(FakeRay,terrain.data(),1,dest)==Surface::invalid);
 dest.x=1024LL*4294967296LL;assert(TopSurface(FakeRay,terrain.data(),0,dest)==Surface::invalid);
 assert(rayCalls==before);
 // A recheck starts lower; a start outside the grid falls back to the grid top.
 ground=123.25f;dest=p;int64_t top=0;assert(TopSurface(FakeRay,terrain.data(),0,dest,nullptr,&top)==Surface::ready&&top==int64_t(256.25*4294967296.0));
 rayStart=200.f;assert(TopSurface(FakeRay,terrain.data(),0,dest,nullptr,&top,200.)==Surface::ready&&top==200LL*4294967296LL);
 rayStart=256.25f;assert(TopSurface(FakeRay,terrain.data(),0,dest,nullptr,&top,9000.)==Surface::ready);
 assert(TopSurface(FakeRay,terrain.data(),0,dest,nullptr,&top,-5.)==Surface::ready);
 assert(ArrivedAt(p,p));auto moved=p;moved.y+=4294967296LL;
 assert(!ArrivedAt(moved,p));moved=p;moved.x+=4294967;assert(ArrivedAt(moved,p));
 assert(!ArrivedAt({INT64_MIN,0,0},{INT64_MAX,0,0}));
 for(const auto& scenario:scenarios)for(auto text:{scenario.opening,scenario.problem,scenario.correction}) {
     auto lines=popup::Wrap(text);assert(!lines.empty()&&lines.size()<=8);
     std::string joined;
     for(const auto& line:lines){assert(line.size()<=72);if(!joined.empty())joined+=' ';joined+=line;}
     assert(joined==text);
 }
 assert(popup::Wrap("",0).empty());assert(popup::Wrap("abcdefgh",3).size()==3);
 // Every language: all texts present, one %u in the countdown, and lines that fit and keep
 // every letter (CJK and Thai lines break without spaces).
 for(size_t lang=0;lang<size_t(text::Lang::count);++lang) {
     for(int id=0;id<text::IdCount;++id) {
         std::string_view value=text::Get(text::Lang(lang),text::Id(id));assert(!value.empty());
         auto percent=value.find('%');
         if(id==text::Countdown)assert(percent!=value.npos&&value.substr(percent,2)=="%u"&&value.find('%',percent+1)==value.npos);
         else assert(percent==value.npos);
         std::string flat,joined;for(char ch:value)if(ch!=' ')flat+=ch;
         for(const auto& line:popup::Wrap(value)) {
             size_t columns=0,n=0;
             for(size_t i=0;i<line.size();i+=n)columns+=popup::Columns(popup::Decode(line,i,n));
             assert(columns<=56&&!line.empty()&&line.front()!=' ');
             auto first=popup::Decode(line,0,n);assert(!popup::Closing(first)&&popup::Columns(first)!=0);
             for(char ch:line)if(ch!=' ')joined+=ch;
         }
         assert(joined==flat);
     }
 }
 for(unsigned s=0;s<3;++s) {
     assert(scenarios[s].opening==text::ScenarioLine(text::Lang::en,s,0));
     assert(scenarios[s].problem==text::ScenarioLine(text::Lang::en,s,1));
     assert(scenarios[s].correction==text::ScenarioLine(text::Lang::en,s,2));
 }
 assert(popup::Codepoints("Ä傳ต")==3);
 // The game's word for "Legendary" tells the language.
 for(auto [word,lang]:std::initializer_list<std::pair<const char*,text::Lang>>{
         {"Legendary",text::Lang::en},{"Légendaire",text::Lang::fr},{"Leggendario",text::Lang::it},
         {"Legendär",text::Lang::de},{"Legendario",text::Lang::es},{"レジェンダリー",text::Lang::ja},
         {"전설",text::Lang::ko},{"Legendarny",text::Lang::pl},{"Lendário",text::Lang::pt},
         {"Легендарный",text::Lang::ru},{"传说",text::Lang::zhHans},{"傳說",text::Lang::zhHant},
         {"Efsanevi",text::Lang::tr},{"Легендарний",text::Lang::uk},{"ตำนาน",text::Lang::th}}) {
     text::Lang found=text::Lang::count;assert(text::Detect(word,found)&&found==lang);
 }
 {text::Lang kept=text::Lang::de;assert(!text::Detect("",kept)&&!text::Detect("???",kept)&&kept==text::Lang::de);}
 assert(text::ParseCode("auto")==-1&&text::ParseCode("")==-1&&text::ParseCode("xx")==-1);
 assert(text::ParseCode(" DE ")==int(text::Lang::de)&&text::ParseCode("zh-TW")==int(text::Lang::zhHant));
 assert(text::ParseCode("zh-hans")==int(text::Lang::zhHans)&&text::ParseCode("pt-BR")==int(text::Lang::pt));
 assert(text::ParseCode("th")==int(text::Lang::th));
 // First-use policy is independent of the optional regular deluxe mode.
 assert(SelectMode({false,true},false)==TravelMode::prank);
 assert(SelectMode({true,true},false)==TravelMode::prank);
 assert(SelectMode({true,true},true)==TravelMode::deluxe);
 assert(SelectMode({false,true},true)==TravelMode::normal);
 assert(SelectMode({true,false},false)==TravelMode::deluxe);
 assert(SelectMode({false,false},false)==TravelMode::normal);
 Destination high=p;high.y=2000LL*4294967296LL;
 for(unsigned scenario=0;scenario<3;++scenario) {
     Journey journey;
     assert(!journey.begin(TravelMode::prank,scenario,100,p,high,false));
     assert(journey.begin(TravelMode::prank,scenario,100,p,high,true));
     assert(!journey.begin(TravelMode::normal,0,100,p,high,false));
     assert(journey.countdown(99)==3&&journey.countdown(1099)==3);
     assert(journey.countdown(1100)==2&&journey.countdown(2100)==1);
     journey.tick(3099);assert(!journey.needsTravel());
     journey.tick(3100);assert(journey.needsTravel()&&journey.destination().y==high.y);
     journey.tick(100000);assert(journey.phase==Phase::ascending); // time cannot fake arrival
     journey.arrived(100000);assert(journey.message()==scenarios[scenario].opening);
     journey.tick(114999);assert(journey.phase==Phase::scenic);
     journey.tick(115000);assert(journey.message()==scenarios[scenario].problem);
     journey.tick(120000);assert(journey.message()==scenarios[scenario].correction);
     journey.tick(132000);assert(journey.needsTravel()&&journey.destination().y==p.y);
     assert(!journey.rememberFirstUse()&&journey.protectedFall);
     journey.restored();assert(journey.protectedFall); // premature acknowledgment ignored
     journey.arrived(133000);assert(journey.phase==Phase::restoring);
     assert(!journey.rememberFirstUse());journey.restored();
     assert(journey.rememberFirstUse()&&!journey.protectedFall);
 }
 for(auto phase:{Phase::countdown,Phase::ascending,Phase::scenic,Phase::problem,Phase::correction,Phase::returning,Phase::restoring}) {
     Journey journey;assert(journey.begin(TravelMode::prank,0,0,p,high,true));
     journey.phase=phase;journey.recover();
     assert(journey.protectedFall&&!journey.rememberFirstUse());
     if(phase!=Phase::restoring) {
         assert(journey.phase==Phase::returning&&journey.destination().y==p.y);
         journey.arrived(1);
     }
     journey.restored();assert(journey.rememberFirstUse());
 }
 for(auto mode:{TravelMode::normal,TravelMode::deluxe}) {
     Journey journey;assert(journey.begin(mode,0,0,p,high,false));
     assert(!journey.protectedFall&&journey.message().empty());
     assert(journey.destination().y==(mode==TravelMode::normal?p.y:high.y));
     journey.recover();assert(journey.phase==Phase::ascending);
     journey.arrived(100);journey.tick(100000);
     assert(journey.phase==Phase::complete&&!journey.rememberFirstUse());
 }
 assert(ParseSwitch(L" TRUE ",false)&&!ParseSwitch(L"Off",true));
 assert(!ParseSwitch(L"garbage",false)&&ParseSwitch(L"garbage",true));
 wchar_t temp[MAX_PATH],filename[MAX_PATH];assert(GetTempPathW(MAX_PATH,temp));
 assert(GetTempFileNameW(temp,L"mtp",0,filename));
 assert(DeleteFileW(filename));
 std::vector<uint8_t> boundsManager(0xc0),boxes(0x38);
 WorldBounds worldBounds{{0,0,0},{10240LL*4294967296LL,2560LL*4294967296LL,10240LL*4294967296LL}};
 memcpy(boxes.data(),&worldBounds.minimum,24);memcpy(boxes.data()+24,&worldBounds.maximum,24);
 Put(boundsManager.data(),0xb0,boxes.data());Put<uint64_t>(boundsManager.data(),0xb8,1);
 WorldBounds readBounds;assert(ReadWorldBounds(boundsManager.data(),readBounds));
 Destination aerial;assert(HighDestination(readBounds,p,aerial));
 assert(aerial.x==p.x&&aerial.z==p.z&&aerial.y==2556LL*4294967296LL);
 auto outside=p;outside.x=readBounds.maximum.x;assert(!HighDestination(readBounds,outside,aerial));
 outside=p;outside.y=2550LL*4294967296LL;assert(!HighDestination(readBounds,outside,aerial));
 {   // Ground above the reset box, or at the ray start, is never used.
     constexpr int64_t m=4294967296LL;Destination g=p;g.y=100*m;
     assert(PlausibleGround(true,readBounds,g,256*m)&&PlausibleGround(false,readBounds,g,0));
     g.y=2556*m;assert(!PlausibleGround(true,readBounds,g,0));
     g.y=2555*m;assert(PlausibleGround(true,readBounds,g,0));
     g.y=3000*m;assert(!PlausibleGround(true,readBounds,g,0)&&PlausibleGround(false,readBounds,g,0));
     g.y=256*m-m/2;assert(!PlausibleGround(false,readBounds,g,256*m));
     g.y=0;assert(!PlausibleGround(true,readBounds,g,0));
     assert(SameGround(100*m,100*m+m/2)&&SameGround(100*m+m/2,100*m)&&!SameGround(100*m,101*m));
     // The pit found in game: 56 m deep, ground about 768 m, a 810 m tower beside it.
     int64_t ring[8]={768*m,769*m,810*m,769*m,768*m,768*m+m/2,810*m,769*m};bool ok[8];for(auto& v:ok)v=true;
     auto rim=PitRim(712*m+m/2,ring,ok,8);assert(rim>=0&&ring[rim]>=768*m&&ring[rim]<=769*m);
     ring[0]=716*m;assert(PitRim(712*m,ring,ok,8)>=0);                 // seven higher: a pit
     ring[1]=716*m;assert(PitRim(712*m,ring,ok,8)==-1);                // six higher: open, keep
     for(auto& v:ring)v=730*m;assert(PitRim(712*m,ring,ok,8)==-1);    // 18 m: a valley, keep
     for(auto& v:ring)v=800*m;ok[0]=ok[1]=false;assert(PitRim(712*m,ring,ok,8)==-1); // unknown
 }
 Put<uint64_t>(boundsManager.data(),0xb8,0);assert(!ReadWorldBounds(boundsManager.data(),readBounds));
 std::wstring ini=std::wstring(filename)+L".ini";
 assert(WritePrivateProfileStringW(L"Teleport",L"DeluxeTeleports",L"true",ini.c_str()));
 assert(WritePrivateProfileStringW(L"Prank",L"NoFun",L"true",ini.c_str()));
 auto settings=ReadSettings(ini.c_str());assert(settings.deluxe&&!settings.prank);
 assert(WritePrivateProfileStringW(L"Prank",L"NoFun",L"false",ini.c_str()));assert(ReadSettings(ini.c_str()).prank);
 assert(WritePrivateProfileStringW(L"Prank",L"NoFun",nullptr,ini.c_str()));assert(ReadSettings(ini.c_str()).prank);
 assert(ReadShowBanners(ini.c_str()));
 assert(WritePrivateProfileStringW(L"Messages",L"ShowBanners",L"false",ini.c_str()));assert(!ReadShowBanners(ini.c_str()));
 assert(WritePrivateProfileStringW(L"Messages",L"ShowBanners",L"true",ini.c_str()));assert(ReadShowBanners(ini.c_str()));
 assert(DeleteFileW(ini.c_str()));
 History history;assert(history.open(filename));
 assert(!history.begin(0,p));assert(history.begin(123,p));assert(!history.begin(123,high));
 History restarted;assert(restarted.open(filename));
 assert(restarted.find(123)&&!restarted.find(123)->complete);
 assert(restarted.find(123)->ground.x==p.x&&restarted.find(123)->ground.z==p.z);
 assert(!restarted.find(456));assert(!restarted.finish(456));
 assert(restarted.finish(123));
 History finished;assert(finished.open(filename)&&finished.find(123)->complete);
 assert(!finished.begin(123,p));assert(finished.begin(456,p));
 // Corrupt history must not silently become an empty first-use ledger.
 HANDLE corrupt=CreateFileW(filename,GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
 assert(corrupt!=INVALID_HANDLE_VALUE);DWORD written;
 const char invalid[]="MapTeleportHistory 1\n123 pending 0 0 0\n123 done 0 0 0\n";
 assert(WriteFile(corrupt,invalid,sizeof(invalid)-1,&written,nullptr));CloseHandle(corrupt);
 History damaged;assert(!damaged.open(filename)&&!damaged.ready()&&!damaged.begin(789,p));
 assert(DeleteFileW(filename));
 puts("PASS: map zoom/pan conversion, bounds, nonfinite data, one-second timing and cancellation");
 puts("PASS: request bounds/malformed input, native terrain completeness, surface height, missing/outside terrain");
 puts("PASS: all three prank scripts, protection/arrival acknowledgments, recovery at every phase, independent deluxe policy");
 puts("PASS: INI switches, persistent per-account history, interrupted/completed visits, corrupt history rejection");
 puts("PASS: scene-bound altitude selection, boundary rejection and insufficient headroom (native enforcement unverified)");
 puts("PASS: ground above the world limit or at the ray start is rechecked; lower ray start; two-reading agreement; pits land on the rim");
}
