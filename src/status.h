#pragma once
#include <charconv>
#include <cstdint>
#include <string_view>
#include "countdown.h"
namespace teleport {
enum class NoticePhase : unsigned { waiting,countdown,ascending,scenic,problem,correction,returning,restoring,complete,failed,cancelled };
struct Status {uint64_t nonce{};NoticePhase phase{};unsigned scenario{},countdown{};bool protection{};Destination ground{};};
inline bool ParsePoll(std::string_view line,uint64_t& nonce) {
    if(!line.starts_with("/mtp1 ")||line.size()>32)return false;
    line.remove_prefix(6);uint64_t value{};
    auto r=std::from_chars(line.data(),line.data()+line.size(),value);
    if(r.ec!=std::errc{}||r.ptr!=line.data()+line.size()||!value)return false;
    nonce=value;return true;
}
inline bool ParseStatus(std::string_view line,Status& out) {
    if(!line.starts_with("/mts1 ")||line.size()>160)return false;
    line.remove_prefix(6);uint64_t fields[4]{};
    for(unsigned i=0;i<4;++i) {
        auto end=line.data()+line.size();auto r=std::from_chars(line.data(),end,fields[i]);
        if(r.ec!=std::errc{})return false;
        if(i==3){line.remove_prefix(size_t(r.ptr-line.data()));}
        else {if(r.ptr==end||*r.ptr!=' ')return false;line.remove_prefix(size_t(r.ptr-line.data())+1);}
    }
    if(!fields[0]||fields[1]>unsigned(NoticePhase::cancelled)||fields[2]>2||fields[3]>3)return false;
    if((fields[1]==unsigned(NoticePhase::countdown))!=(fields[3]>0))return false;
    Status value{fields[0],NoticePhase(fields[1]),unsigned(fields[2]),unsigned(fields[3])};
    if(!line.empty()) {
        if(!line.starts_with(" 1 "))return false;line.remove_prefix(3);
        int64_t* coordinates[]={&value.ground.x,&value.ground.y,&value.ground.z};
        constexpr int64_t limit=100000LL*4294967296LL;
        for(unsigned i=0;i<3;++i) {
            auto end=line.data()+line.size();auto r=std::from_chars(line.data(),end,*coordinates[i]);
            if(r.ec!=std::errc{}||*coordinates[i]<-limit||*coordinates[i]>limit)return false;
            if(i==2){if(r.ptr!=end)return false;}
            else{if(r.ptr==end||*r.ptr!=' ')return false;line.remove_prefix(size_t(r.ptr-line.data())+1);}
        }
        if(value.ground.x<0||value.ground.z<0)return false;
        value.protection=true;
    }
    out=value;return true;
}
inline bool Finished(Status s){return s.phase==NoticePhase::complete||s.phase==NoticePhase::failed||s.phase==NoticePhase::cancelled;}
// A public chat packet is not a status merely because it starts with /mts1.
// Only the local client's chat store and its own account/active nonce qualify.
inline bool MatchesStatus(Status s,uint64_t active,uint64_t sender,uint64_t localAccount) {
    return active&&s.nonce==active&&localAccount&&sender==localAccount;
}
}
