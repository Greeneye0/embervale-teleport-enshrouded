#pragma once
#include "countdown.h"
#include <string_view>
namespace teleport {
enum class TravelMode { normal, deluxe, prank };
struct Settings {bool deluxe=false,prank=true;int testScenario=-1;};
constexpr TravelMode SelectMode(Settings s,bool experienced) {
    return s.prank&&!experienced?TravelMode::prank:(s.deluxe?TravelMode::deluxe:TravelMode::normal);
}
struct Scenario {std::string_view opening,problem,correction;};
inline constexpr Scenario scenarios[]={
 {"You have been upgraded to the deluxe scenic teleport experience. For your safety, we have disabled your glider. Enjoy the ride, and thank you for choosing Embervale Teleport Services.",
  "There has been a small mistake. Please hold on one second.",
  "We apologize. Flameborn appear to react poorly to gravity. This accidental deluxe upgrade will not happen again. All future trips use your selected service. Returning you safely to the ground now."},
 {"Your destination was entered in metres. Our system processed it in kilometres. Please enjoy this complimentary aerial preview of your destination.",
  "Our measurements department has requested one moment of your patience.",
  "We have located the decimal point. It was on holiday. We have revoked its leave, so this error will not happen again. Correcting your destination now. Thank you for choosing Embervale Teleport Services."},
 {"Your teleport is complete. Please remain still while we locate the ground. It was here when we booked this.",
  "We have located the ground. Unfortunately, it is approaching ahead of schedule.",
  "Please disregard the scenery moving rapidly past you. We are cancelling your appointment with the ground and arranging a gentler introduction."}
};
enum class Phase { idle, countdown, ascending, scenic, problem, correction, returning, restoring, complete };
inline constexpr uint64_t scenicMs=15000,problemMs=5000,correctionMs=12000;
// Native adapters acknowledge every move and protection change. Elapsed time alone
// never constitutes arrival or successful restoration.
struct Journey {
    TravelMode mode=TravelMode::normal;
    Phase phase=Phase::idle;
    unsigned scenario=0;
    uint64_t since=0;
    Destination ground{},high{};
    bool protectedFall=false;
    bool begin(TravelMode m,unsigned choice,uint64_t now,Destination g,Destination h,bool protectionConfirmed) {
        if(phase!=Phase::idle&&phase!=Phase::complete)return false;
        if(m==TravelMode::prank&&!protectionConfirmed)return false;
        mode=m;scenario=choice%3;ground=g;high=h;since=now;
        protectedFall=m==TravelMode::prank;
        phase=protectedFall?Phase::countdown:Phase::ascending;return true;
    }
    unsigned countdown(uint64_t now)const {
        if(phase!=Phase::countdown)return 0;
        auto elapsed=now>=since?now-since:0;return elapsed>=3000?0:3-unsigned(elapsed/1000);
    }
    void tick(uint64_t now) {
        if(now<since)return;
        auto elapsed=now-since;
        if(phase==Phase::countdown&&elapsed>=3000){phase=Phase::ascending;since=now;}
        else if(phase==Phase::scenic&&elapsed>=scenicMs){phase=Phase::problem;since=now;}
        else if(phase==Phase::problem&&elapsed>=problemMs){phase=Phase::correction;since=now;}
        else if(phase==Phase::correction&&elapsed>=correctionMs){phase=Phase::returning;since=now;}
    }
    bool needsTravel()const{return phase==Phase::ascending||phase==Phase::returning;}
    Destination destination()const{return phase==Phase::returning||mode==TravelMode::normal?ground:high;}
    void arrived(uint64_t now) {
        if(phase==Phase::ascending){phase=mode==TravelMode::prank?Phase::scenic:Phase::complete;since=now;}
        else if(phase==Phase::returning){phase=Phase::restoring;since=now;}
    }
    void restored(){if(phase==Phase::restoring){protectedFall=false;phase=Phase::complete;}}
    void recover(){if(protectedFall&&phase!=Phase::restoring)phase=Phase::returning;}
    bool rememberFirstUse()const{return mode==TravelMode::prank&&phase==Phase::complete&&!protectedFall;}
    std::string_view message()const {
        const auto& s=scenarios[scenario%3];
        if(phase==Phase::scenic)return s.opening;
        if(phase==Phase::problem)return s.problem;
        if(phase==Phase::correction||phase==Phase::returning)return s.correction;
        return {};
    }
};
}
