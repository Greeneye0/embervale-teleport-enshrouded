#pragma once
#include "countdown.h"
#include <cstring>
#include <algorithm>

namespace teleport {
struct WorldBounds {Destination minimum{},maximum{};};
// Native SceneResetPlayersOutOfBounds uses fixed 32.32 metre coordinates.
// This reads the first, world-wide box copied by native e998d0; it does
// not substitute voxel-grid height or a hard-coded world ceiling.
inline bool ReadWorldBounds(const void* manager,WorldBounds& out) {
    if(!manager)return false;
    auto bytes=static_cast<const uint8_t*>(manager);
    const uint8_t* boxes=nullptr;uint64_t count=0;
    memcpy(&boxes,bytes+0xb0,sizeof boxes);memcpy(&count,bytes+0xb8,sizeof count);
    if(!boxes||!count||count>4096)return false;
    WorldBounds value;
    memcpy(&value.minimum,boxes,24);memcpy(&value.maximum,boxes+24,24);
    constexpr int64_t limit=100000LL*4294967296LL;
    const int64_t low[]={value.minimum.x,value.minimum.y,value.minimum.z};
    const int64_t high[]={value.maximum.x,value.maximum.y,value.maximum.z};
    for(unsigned i=0;i<3;++i)if(low[i]<-limit||high[i]>limit||low[i]>=high[i])return false;
    out=value;return true;
}
inline bool HighDestination(WorldBounds bounds,Destination ground,Destination& out) {
    constexpr int64_t metre=4294967296LL;
    // Four metres of headroom below the reset box; require an actual aerial trip.
    // Native enforcement and the actor's origin still require live validation.
    if(ground.x<=bounds.minimum.x||ground.x>=bounds.maximum.x||
       ground.z<=bounds.minimum.z||ground.z>=bounds.maximum.z||
       ground.y<bounds.minimum.y||ground.y>=bounds.maximum.y)return false;
    if(bounds.maximum.y-ground.y<=12*metre)return false;
    out=ground;out.y=bounds.maximum.y-4*metre;return true;
}
// Ground from the terrain ray must lie inside the out-of-bounds box, at least four metres
// under its ceiling, and clearly below where the ray started: a hit at the start means a
// freshly loaded cell answered before it held real terrain. Landing above the box sends the
// player to their altar, so such a hit is rechecked instead of used.
inline bool PlausibleGround(bool boundsKnown,const WorldBounds& bounds,Destination ground,int64_t rayTop) {
    constexpr int64_t metre=4294967296LL;
    if(rayTop&&ground.y>=rayTop-metre)return false;
    if(boundsKnown&&(ground.y<=bounds.minimum.y||ground.y>=bounds.maximum.y-4*metre))return false;
    return true;
}
// Two ground readings on consecutive ticks must agree within half a metre.
inline bool SameGround(int64_t a,int64_t b){constexpr int64_t half=4294967296LL/2;return (a>b?a-b:b-a)<=half;}
// The top surface can be the floor of a narrow pit (a shaft in a ruin, a crevice) while the
// map shows the ground around it. ring holds the surfaces on a circle around the target
// (valid=false where none was usable). When at least seven of the eight stand 20 m or more
// above the centre, the target is a pit: land on its rim, at the ring surface closest to the
// median of those, so a tower beside the pit is not chosen. Returns that index, or -1.
// Checked on the game's own voxel data: about 1 in 2000 random points is such a pit.
inline int PitRim(int64_t centre,const int64_t* ring,const bool* valid,size_t n) {
    constexpr int64_t metre=4294967296LL;
    int64_t high[16];size_t count=0;
    for(size_t i=0;i<n&&i<16;++i)if(valid[i]&&ring[i]>=centre+20*metre)high[count++]=ring[i];
    if(count<7)return -1;
    std::sort(high,high+count);auto median=high[count/2];
    int best=-1;
    for(size_t i=0;i<n&&i<16;++i) {
        if(!valid[i]||ring[i]<centre+20*metre)continue;
        auto d=ring[i]>median?ring[i]-median:median-ring[i];
        if(best<0){best=int(i);continue;}
        auto b=ring[size_t(best)]>median?ring[size_t(best)]-median:median-ring[size_t(best)];
        if(d<b)best=int(i);
    }
    return best;
}
}
