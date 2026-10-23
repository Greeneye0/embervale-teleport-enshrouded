#pragma once
#include <cmath>
#include <cstdint>
namespace teleport {
struct Destination { int64_t x, y, z; };
inline bool Convert(float sx,float sy,float ox,float oy,float zoom,float width,float height,Destination& out) {
    if (!std::isfinite(zoom)||zoom<=0 || !std::isfinite(width)||!std::isfinite(height)||width<=0||height<=0||width>100000||height>100000) return false;
    double x=(double(sx)-ox)/zoom, z=height-(double(sy)-oy)/zoom;
    if (!std::isfinite(x)||!std::isfinite(z)||x<0||z<0||x>width||z>height) return false;
    out={int64_t(x*4294967296.0),0,int64_t(z*4294967296.0)}; return true;
}
struct Countdown {
    bool active=false;
    uint64_t started=0;
    Destination target{};
    void cancel() {active=false;}
    void start(uint64_t now,Destination p) {active=true;started=now;target=p;}
    bool due(uint64_t now) const {return active&&now>=started&&now-started>=1000;}
    float progress(uint64_t now) const {return now<started?0:float(now-started>=1000?1000:now-started)/1000;}
};
}
