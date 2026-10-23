#pragma once
#include <windows.h>
#include <cstdint>
namespace teleport {
#ifdef MAP_TELEPORT_TEST
uint64_t TestAccount();
#endif
inline uint64_t LocalAccount() {
#ifdef MAP_TELEPORT_TEST
    return TestAccount();
#else
    auto steam=GetModuleHandleW(L"steam_api64.dll");if(!steam)return 0;
    using User=void*(*)();using Id=uint64_t(*)(void*);
    auto user=reinterpret_cast<User>(GetProcAddress(steam,"SteamAPI_SteamUser_v023"));
    auto id=reinterpret_cast<Id>(GetProcAddress(steam,"SteamAPI_ISteamUser_GetSteamID"));
    if(!user||!id)return 0;
    auto instance=user();return instance?id(instance):0;
#endif
}
}
