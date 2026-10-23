#pragma once
#include "journey.h"
#include <windows.h>
#include <string>
#include <string_view>
#include <cwctype>

namespace teleport {
inline bool ParseSwitch(std::wstring_view value,bool fallback) {
    while(!value.empty()&&iswspace(value.front()))value.remove_prefix(1);
    while(!value.empty()&&iswspace(value.back()))value.remove_suffix(1);
    std::wstring text(value);
    for(auto& ch:text)ch=static_cast<wchar_t>(towlower(ch));
    if(text==L"true"||text==L"on"||text==L"yes"||text==L"1")return true;
    if(text==L"false"||text==L"off"||text==L"no"||text==L"0")return false;
    return fallback;
}
inline bool ReadShowBanners(const wchar_t* absolutePath) {
    wchar_t value[64]{};
    GetPrivateProfileStringW(L"Messages",L"ShowBanners",L"true",value,64,absolutePath);
    return ParseSwitch(value,true);
}
inline Settings ReadSettings(const wchar_t* absolutePath) {
    Settings result;
    wchar_t value[64]{};
    GetPrivateProfileStringW(L"Teleport",L"DeluxeTeleports",L"false",value,64,absolutePath);
    result.deluxe=ParseSwitch(value,false);
    GetPrivateProfileStringW(L"Prank",L"NoFun",L"false",value,64,absolutePath);
    result.prank=!ParseSwitch(value,false);
    auto scenario=GetPrivateProfileIntW(L"Testing",L"PrankScenario",UINT(-1),absolutePath);
    if(scenario<3)result.testScenario=int(scenario);
    return result;
}
}
