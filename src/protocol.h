#pragma once
#include "countdown.h"
#include <charconv>
#include <string_view>
namespace teleport {
struct Request { uint64_t nonce{}; Destination position{}; };
inline bool ParseRequest(std::string_view text, Request& out) {
    if (!text.starts_with("/mt1 ") || text.size()>96) return false;
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
    if(!number(p.nonce,false)||!number(p.position.x,false)||!number(p.position.z,true)) return false;
    constexpr int64_t bound=100000LL*4294967296LL;
    if(!p.nonce || p.position.x<0 || p.position.z<0 || p.position.x>bound || p.position.z>bound) return false;
    out=p;return true;
}
}
