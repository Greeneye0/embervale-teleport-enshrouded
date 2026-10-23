#pragma once
#include "countdown.h"
#include <cstring>
namespace teleport {
enum class Surface { ready, loading, invalid };
using TerrainRay=void*(*)(void*,void*,const float*,const float*,const uint8_t*);
template<class T> inline T field(const void* p,size_t offset) {
    T value;std::memcpy(&value,static_cast<const uint8_t*>(p)+offset,sizeof(T));return value;
}
// Bounds come from the realm's voxel grid, not a guessed world height. RayQuery's
// +0x12 says all traversed cells were available; +0x13 says a surface was hit.
inline Surface TopSurface(TerrainRay ray,void* terrain,uint8_t realm,Destination& p,const char** reason=nullptr) {
    if(reason)*reason="terrain resource unavailable";
    if(!terrain||!ray) return Surface::loading;
    auto count=field<uint64_t>(terrain,0x1c00);
    auto realms=field<uint8_t*>(terrain,0x1bf8);
    if(reason)*reason="invalid realm table";
    if(!realms||!count||count>256||realm>=count) return Surface::invalid;
    auto grid=field<uint8_t*>(realms+size_t(realm)*0x3870,0);
    if(reason)*reason="realm grid unavailable";
    if(!grid) return Surface::loading;
    uint32_t shift=field<uint32_t>(grid,0x10), nx=field<uint32_t>(grid,0x14), ny=field<uint32_t>(grid,0x18);
    float ox=field<float>(grid,0x70),oy=field<float>(grid,0x74),oz=field<float>(grid,0x78);
    if(reason)*reason="invalid grid bounds";
    if(shift>12||!nx||!ny||nx>65536||ny>4096||!std::isfinite(ox)||!std::isfinite(oy)||!std::isfinite(oz)) return Surface::invalid;
    double size=double(uint64_t(1)<<shift), height=ny*size;
    double x=double(p.x)/4294967296.0,z=double(p.z)/4294967296.0;
    if(reason)*reason="destination outside voxel grid";
    if(height>32768||x<ox||z<oz||x>=ox+nx*size||z>=oz+nx*size) return Surface::invalid;
    alignas(16) float from[4]={float(x),float(oy+height+0.25),float(z),0};
    alignas(16) float to[4]={float(x),oy,float(z),0};
    alignas(16) uint8_t hit[64]{};
    // Abort at an unavailable cell. The native query queues that terrain block for loading.
    const uint8_t options[4]={realm,0,1,1};
    ray(hit,terrain,from,to,options);
    if(reason)*reason="terrain cells unavailable";
    if(!hit[0x12]) return Surface::loading;
    if(reason)*reason="completed terrain ray found no surface";
    if(!hit[0x13]) return Surface::invalid;
    float y=field<float>(hit,4);
    if(reason)*reason="terrain ray returned invalid height";
    if(!std::isfinite(y)||y<oy||y>from[1]) return Surface::invalid;
    // Native voxel-ray surface includes its 1 mm contact tolerance.
    p.y=int64_t(double(y)*4294967296.0);
    if(reason)*reason="surface ready";
    return Surface::ready;
}
}
