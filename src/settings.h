#pragma once
#include "journey.h"
#include "strings.h"
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
// -1: follow the game's language.
inline int ReadLanguage(const wchar_t* absolutePath) {
    wchar_t value[32]{};
    GetPrivateProfileStringW(L"Messages",L"Language",L"auto",value,32,absolutePath);
    std::string code;for(auto* c=value;*c;++c)code+=*c<128?char(*c):'?';
    return text::ParseCode(code);
}
// [Controller] Button: the XInput button held for one second to travel to the map's centre.
// 0 = off. Unknown names keep the default, Y.
inline unsigned ParseControllerButton(std::wstring_view value) {
    while(!value.empty()&&iswspace(value.front()))value.remove_prefix(1);
    while(!value.empty()&&iswspace(value.back()))value.remove_suffix(1);
    std::wstring text(value);for(auto& ch:text)ch=static_cast<wchar_t>(towlower(ch));
    struct Name {const wchar_t* name;unsigned mask;};
    static const Name names[]={{L"off",0},{L"none",0},{L"false",0},{L"a",0x1000},{L"b",0x2000},{L"x",0x4000},{L"y",0x8000},
        {L"lb",0x100},{L"rb",0x200},{L"ls",0x40},{L"rs",0x80},{L"back",0x20},{L"view",0x20},{L"start",0x10},{L"menu",0x10},
        {L"up",1},{L"down",2},{L"left",4},{L"right",8}};
    for(auto& n:names)if(text==n.name)return n.mask;
    return 0x8000;
}
inline unsigned ReadControllerButton(const wchar_t* absolutePath) {
    wchar_t value[32]{};
    GetPrivateProfileStringW(L"Controller",L"Button",L"Y",value,32,absolutePath);
    return ParseControllerButton(value);
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
