#pragma once
#include "countdown.h"
#include <charconv>
#include <cstdint>
#include <string_view>
namespace teleport {
// Chat entry type that carries the protocol lines. The game uses 0-4 (0 = player chat, 1-3 = pings
// and notices, 4 = system text); every server relays other values unchanged and the chat window
// does not draw them, so players and servers without the mod never see these lines.
inline constexpr uint8_t kChatType=255;
// Sent once by a remote client after joining; a modded host answers with kHelloReply.
// kHello2 is sent beside it: only hosts that take pin targets (/mt2) answer it, with kHelloReply2.
// Older hosts relay it as hidden chat and never answer, so the client keeps sending /mt1 to them.
inline constexpr std::string_view kHello="/mth1",kHelloReply="/mts1 0 hello";
inline constexpr std::string_view kHello2="/mth2",kHelloReply2="/mts1 0 hello2";
// /mt1 nonce x z: land on the top surface at x, z.
// /mt2 nonce x y z: a map pin's position; land on the floor nearest under y (travel PinFloor).
// position.y is 0 for /mt1 and above 0 for /mt2.
struct Request { uint64_t nonce{}; Destination position{}; };
inline bool ParseRequest(std::string_view text, Request& out) {
    bool pin=text.starts_with("/mt2 ");
    if ((!pin&&!text.starts_with("/mt1 ")) || text.size()>120) return false;
    text.remove_prefix(5);
    auto number=[&](auto& value, bool last) {
        auto end=text.data()+text.size();
        auto r=std::from_chars(text.data(),end,value);
        if(r.ec!=std::errc{}) return false;
        if(last) return r.ptr==end;
        if(r.ptr==end || *r.ptr!=' ') return false;
        text.remove_prefix(size_t(r.ptr-text.data())+1); return true;
    };
    Request p;
    if(!number(p.nonce,false)||!number(p.position.x,false)) return false;
    if(pin&&!number(p.position.y,false)) return false;
    if(!number(p.position.z,true)) return false;
    constexpr int64_t bound=100000LL*4294967296LL;
    if(!p.nonce || p.position.x<0 || p.position.z<0 || p.position.x>bound || p.position.z>bound) return false;
    if(pin&&(p.position.y<=0||p.position.y>bound)) return false;
    out=p;return true;
}
}
