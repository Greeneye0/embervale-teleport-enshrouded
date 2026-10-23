#pragma once
#include "countdown.h"
#include <cstring>

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
}
