#pragma once
#include "countdown.h"
#include <windows.h>
#include <map>
#include <string>
#include <charconv>
#include <sstream>
#include <limits>

namespace teleport {
// Host-owned journal. An unfinished entry is recovery work, never completed use.
// Each installation keeps its own journal, independently of character/world saves.
struct Visit {bool complete=false;Destination ground{};};
class History {
    std::wstring path;
    std::map<uint64_t,Visit> visits;
    bool available=false;
    bool save(const std::map<uint64_t,Visit>& next) const {
        std::string data="MapTeleportHistory 1\n";
        for(const auto& [account,v]:next)
            data+=std::to_string(account)+" "+(v.complete?"done":"pending")+" "+
                std::to_string(v.ground.x)+" "+std::to_string(v.ground.y)+" "+std::to_string(v.ground.z)+"\n";
        auto temporary=path+L".tmp";
        HANDLE file=CreateFileW(temporary.c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
        if(file==INVALID_HANDLE_VALUE)return false;
        DWORD written=0;
        bool ok=data.size()<=MAXDWORD&&WriteFile(file,data.data(),static_cast<DWORD>(data.size()),&written,nullptr)&&written==data.size();
        if(ok)ok=FlushFileBuffers(file)!=0;
        CloseHandle(file);
        if(ok)ok=MoveFileExW(temporary.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
        if(!ok)DeleteFileW(temporary.c_str());
        return ok;
    }
    static bool valid(const Destination& p) {
        constexpr int64_t limit=100000LL*4294967296LL;
        return p.x>=0&&p.x<=limit&&p.z>=0&&p.z<=limit&&p.y>=-limit&&p.y<=limit;
    }
public:
    bool open(std::wstring filename) {
        path=std::move(filename);available=false;visits.clear();
        HANDLE file=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
        if(file==INVALID_HANDLE_VALUE) {
            if(GetLastError()!=ERROR_FILE_NOT_FOUND)return false;
            available=save(visits);return available;
        }
        LARGE_INTEGER size{};
        if(!GetFileSizeEx(file,&size)||size.QuadPart<0||size.QuadPart>4*1024*1024){CloseHandle(file);return false;}
        std::string data(static_cast<size_t>(size.QuadPart),'\0');DWORD read=0;
        bool ok=ReadFile(file,data.data(),static_cast<DWORD>(data.size()),&read,nullptr)&&read==data.size();
        CloseHandle(file);if(!ok)return false;
        std::istringstream input(data);std::string line;
        if(!std::getline(input,line)||line!="MapTeleportHistory 1")return false;
        std::map<uint64_t,Visit> parsed;
        while(std::getline(input,line)) {
            std::istringstream row(line);std::string accountText,state,extra;Visit v;uint64_t account=0;
            if(!(row>>accountText>>state>>v.ground.x>>v.ground.y>>v.ground.z)||(row>>extra))return false;
            auto r=std::from_chars(accountText.data(),accountText.data()+accountText.size(),account);
            if(r.ec!=std::errc{}||r.ptr!=accountText.data()+accountText.size()||!account||!valid(v.ground))return false;
            if(state!="done"&&state!="pending")return false;
            v.complete=state=="done";
            if(!parsed.emplace(account,v).second)return false;
        }
        visits=std::move(parsed);available=true;return true;
    }
    bool ready()const{return available;}
    const Visit* find(uint64_t account)const {
        auto it=visits.find(account);return it==visits.end()?nullptr:&it->second;
    }
    bool begin(uint64_t account,Destination ground) {
        if(!available||!account||!valid(ground)||find(account))return false;
        auto next=visits;next.emplace(account,Visit{false,ground});
        if(!save(next))return false;
        visits=std::move(next);return true;
    }
    // Only call after arrival and protection restoration are acknowledged.
    bool finish(uint64_t account) {
        auto visit=find(account);if(!available||!visit)return false;
        if(visit->complete)return true;
        auto next=visits;next.at(account).complete=true;
        if(!save(next))return false;
        visits=std::move(next);return true;
    }
};
}
