#pragma once
#include "greeneye.h"
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
namespace teleport::popup {
inline std::vector<std::string> Wrap(std::string_view text,size_t width=56) {
    std::vector<std::string> lines;
    if(!width)return lines;
    while(!text.empty()) {
        while(!text.empty()&&text.front()==' ')text.remove_prefix(1);
        if(text.empty())break;
        size_t take=(std::min)(text.size(),width);
        if(take<text.size()) {auto space=text.rfind(' ',take);if(space!=std::string_view::npos&&space)take=space;}
        lines.emplace_back(text.substr(0,take));text.remove_prefix(take);
    }
    return lines;
}
bool Init(const GreeneyeHost* host,void** root);
// Rendering uses the game's own widgets/labels; no input field or modal focus.
void Show(std::string title,std::string message,uint64_t durationMs,bool typewriter=true);
void Clear();
bool Visible();
void Draw(); // only inside the game's play-state UI pass
void DrawRing(float x,float y,float progress);
}
