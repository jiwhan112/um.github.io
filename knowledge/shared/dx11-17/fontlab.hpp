#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

// Portable C++11 CPU reference model, not DirectXTK or a GPU renderer.
namespace fontlab {
struct Rect { int x, y, w, h; };
struct Glyph { char32_t codepoint; Rect atlas; float bearingX, bearingY, advance; };
struct Quad { char32_t codepoint; float x0,y0,x1,y1,u0,v0,u1,v1; };
struct Layout { std::vector<Quad> quads; float maxAdvance; unsigned lines; };
bool scalar(uint32_t cp) { return cp<=0x10ffffu && !(cp>=0xd800u&&cp<=0xdfffu); }
bool good(float v) { return std::isfinite(v)!=0; }
std::vector<char32_t> decode(const std::string& s) {
  std::vector<char32_t> result;
  for(size_t i=0;i<s.size();) {
    const uint8_t b=(uint8_t)s[i];
    unsigned n=0; uint32_t cp=0; uint32_t mincp=0;
    if(b<0x80) { n=1; cp=b; mincp=0; }
    else if(b>=0xc2&&b<=0xdf) { n=2; cp=b&0x1f; mincp=0x80; }
    else if(b>=0xe0&&b<=0xef) { n=3; cp=b&0x0f; mincp=0x800; }
    else if(b>=0xf0&&b<=0xf4) { n=4; cp=b&7; mincp=0x10000; }
    else throw std::invalid_argument("illegal UTF-8 leading byte");
    if(s.size()-i<n) throw std::invalid_argument("truncated UTF-8");
    for(unsigned j=1;j<n;++j) {
      uint8_t tail=(uint8_t)s[i+j];
      if((tail&0xc0)!=0x80) throw std::invalid_argument("illegal UTF-8 continuation");
      cp=(cp<<6)|(tail&0x3f);
    }
    if(cp<mincp||!scalar(cp)) throw std::invalid_argument("invalid UTF-8 scalar");
    result.push_back(static_cast<char32_t>(cp)); i+=n;
  }
  return result;
}
bool inside(Rect r,int w,int h) {
  return w>0&&h>0&&r.x>=0&&r.y>=0&&r.w>0&&
         r.x<w&&r.y<h&&r.w<=w-r.x&&r.h<=h-r.y;
}
bool overlap(Rect a,Rect b) {
  return a.x<b.x+b.w&&b.x<a.x+a.w&&a.y<b.y+b.h&&b.y<a.y+a.h;
}
class Atlas {
  int width_,height_; float lineHeight_;
  std::map<char32_t,Glyph> glyphs_;
public:
  Atlas(int w,int h,float line):width_(w),height_(h),lineHeight_(line) {
    if(w<=0||h<=0||!good(line)||line<=0) throw std::invalid_argument("invalid atlas");
  }
  void add(const Glyph& g) {
    if(!scalar((uint32_t)g.codepoint)||!inside(g.atlas,width_,height_)||
       !good(g.advance)||g.advance<=0||!good(g.bearingX)||!good(g.bearingY)||
       glyphs_.count(g.codepoint)) throw std::invalid_argument("invalid glyph");
    for(std::map<char32_t,Glyph>::const_iterator it=glyphs_.begin();it!=glyphs_.end();++it)
      if(overlap(g.atlas,it->second.atlas)) throw std::invalid_argument("overlapping atlas rectangles");
    glyphs_.insert(std::make_pair(g.codepoint,g));
  }
  Layout draw(const std::string& utf8,float size) const {
    if(!good(size)||size<=0) throw std::invalid_argument("invalid text size");
    const std::vector<char32_t> cp=decode(utf8);
    Layout out;out.maxAdvance=0;out.lines=1;
    float penX=0,penY=0;
    for(size_t i=0;i<cp.size();++i) {
      if(cp[i]==U'\n') {out.maxAdvance=std::max(out.maxAdvance,penX);penX=0;penY+=lineHeight_*size;++out.lines;continue;}
      if(cp[i]==U'\r') throw std::invalid_argument("CR must be normalized before layout");
      std::map<char32_t,Glyph>::const_iterator it=glyphs_.find(cp[i]);
      if(it==glyphs_.end()) it=glyphs_.find(U'?');
      if(it==glyphs_.end()) throw std::invalid_argument("missing glyph and fallback");
      const Glyph& g=it->second;
      Quad q={cp[i],penX+g.bearingX*size,penY+g.bearingY*size,
              penX+(g.bearingX+g.atlas.w)*size,penY+(g.bearingY+g.atlas.h)*size,
              float(g.atlas.x)/width_,float(g.atlas.y)/height_,
              float(g.atlas.x+g.atlas.w)/width_,float(g.atlas.y+g.atlas.h)/height_};
      out.quads.push_back(q);penX+=g.advance*size;
    }
    out.maxAdvance=std::max(out.maxAdvance,penX);return out;
  }
};
} // namespace fontlab
