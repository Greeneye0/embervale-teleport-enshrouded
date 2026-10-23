#pragma once
#include "greeneye.h"
#include "strings.h"
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
namespace teleport::popup {
// UTF-8: the code point at s[i], and its length in bytes.
inline uint32_t Decode(std::string_view s,size_t i,size_t& n) {
    auto b=uint8_t(s[i]);
    n=b<0x80?1:b>=0xF0?4:b>=0xE0?3:b>=0xC0?2:1;
    if(i+n>s.size()){n=1;return b;}
    uint32_t c=n==1?b:n==2?b&0x1f:n==3?b&0x0f:b&0x07;
    for(size_t k=1;k<n;++k)c=(c<<6)|(uint8_t(s[i+k])&0x3f);
    return c;
}
inline size_t Codepoints(std::string_view s) {size_t count=0;for(size_t i=0,n=0;i<s.size();i+=n){Decode(s,i,n);++count;}return count;}
// Width in Latin letters: CJK and Hangul are about two, Thai marks above or below a letter none.
inline unsigned Columns(uint32_t c) {
    if(c==0x0E31||(c>=0x0E34&&c<=0x0E3A)||(c>=0x0E47&&c<=0x0E4E))return 0;
    if((c>=0x1100&&c<=0x115F)||(c>=0x2E80&&c<=0xA4CF)||(c>=0xAC00&&c<=0xD7A3)||(c>=0xF900&&c<=0xFAFF)||
       (c>=0xFE30&&c<=0xFE4F)||(c>=0xFF00&&c<=0xFF60)||(c>=0xFFE0&&c<=0xFFE6))return 2;
    return 1;
}
// Punctuation that must not start a line in Chinese and Japanese.
inline bool Closing(uint32_t c) {
    for(uint32_t p:{0x3001u,0x3002u,0xFF0Cu,0xFF0Eu,0xFF01u,0xFF1Fu,0xFF1Au,0xFF1Bu,0xFF09u,0x300Du,0x300Fu,0x3011u,0x30FCu,0x2026u})
        if(c==p)return true;
    return c=='.'||c==','||c=='!'||c=='?'||c==':'||c==';'||c==')';
}
// Lines of at most width columns. Breaks at spaces, between CJK characters, and, with no
// other choice (Thai writes words without spaces), between letters.
inline std::vector<std::string> Wrap(std::string_view text,size_t width=56) {
    std::vector<std::string> lines;
    if(!width)return lines;
    while(!text.empty()) {
        while(!text.empty()&&text.front()==' ')text.remove_prefix(1);
        if(text.empty())break;
        size_t take=text.size(),candidate=0,columns=0,n=0;bool wideBefore=false;
        for(size_t i=0;i<text.size();i+=n) {
            auto c=Decode(text,i,n);auto w=Columns(c);
            if(i&&c==' ')candidate=i;
            else if(i&&w&&(w==2||wideBefore)&&!Closing(c))candidate=i;
            if(i&&columns+w>width){take=candidate?candidate:i;break;}
            columns+=w;if(w)wideBefore=w==2;
        }
        lines.emplace_back(text.substr(0,take));text.remove_prefix(take);
    }
    return lines;
}
bool Init(const GreeneyeHost* host,void** root);
// The language of the notices: [Messages] Language in the ini, else the game's own language.
text::Lang Language();
inline const char* Say(text::Id id){return text::Get(Language(),id);}
// Rendering uses the game's own widgets/labels; no input field or modal focus.
void Show(std::string title,std::string message,uint64_t durationMs,bool typewriter=true);
void Clear();
bool Visible();
void Draw(); // only inside the game's play-state UI pass
void DrawRing(float x,float y,float progress);
}
