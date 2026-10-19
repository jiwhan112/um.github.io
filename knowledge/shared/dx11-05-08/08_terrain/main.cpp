// C++11 terrain mesh and triangular height sampling CPU model; no Direct3D call.
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
struct V {double x,y,z;};
struct Terrain {
 std::size_t width,depth;std::vector<double> height;
 Terrain(std::size_t w,std::size_t d,std::vector<double> h):width(w),depth(d),height(std::move(h)){
  if(w<2||d<2||w>std::numeric_limits<std::size_t>::max()/d||height.size()!=w*d)
   throw std::invalid_argument("terrain shape");
  for(double x:height)if(!std::isfinite(x))throw std::invalid_argument("height finite");
 }
 std::size_t cells()const{return (width-1)*(depth-1);}
 std::size_t tris()const{return cells()*2;}
 std::vector<std::uint32_t> indices()const{
  if(width*depth>std::numeric_limits<std::uint32_t>::max()||cells()>std::numeric_limits<std::size_t>::max()/6)
   throw std::length_error("index capacity");
  std::vector<std::uint32_t> out;out.reserve(cells()*6);
  for(std::size_t z=0;z+1<depth;++z)for(std::size_t x=0;x+1<width;++x){
   const auto a=static_cast<std::uint32_t>(z*width+x);
   const auto b=a+1,c=a+static_cast<std::uint32_t>(width),d=c+1;
   out.insert(out.end(),{a,b,c,b,d,c});
  }
  return out;
 }
 double sample(double x,double z)const{
  if(!std::isfinite(x)||!std::isfinite(z)||x<0||z<0||x>width-1||z>depth-1)
   throw std::out_of_range("terrain sample");
  const std::size_t cx=x==width-1?width-2:static_cast<std::size_t>(x);
  const std::size_t cz=z==depth-1?depth-2:static_cast<std::size_t>(z);
  const double fx=x-cx,fz=z-cz;
  const double a=height[cz*width+cx],b=height[cz*width+cx+1];
  const double c=height[(cz+1)*width+cx],d=height[(cz+1)*width+cx+1];
  // Divide quad across b--c diagonal; triangles a,b,c and b,d,c.
  return fx+fz<=1? a*(1-fx-fz)+b*fx+c*fz
                  : b*(1-fz)+c*(1-fx)+d*(fx+fz-1);
 }
};
static bool validateRGBA(const std::vector<std::uint8_t>& bytes,std::size_t width,std::size_t height,std::size_t pitch){
 if(!width||!height||width>std::numeric_limits<std::size_t>::max()/4)return false;
 const auto row=width*4;
 return pitch>=row && height<=std::numeric_limits<std::size_t>::max()/pitch && bytes.size()>=pitch*height;
}
static std::size_t texOffset(std::size_t x,std::size_t y,std::size_t pitch){return y*pitch+x*4;}
static bool near(double a,double b){return std::isfinite(a)&&std::isfinite(b)&&std::fabs(a-b)<1e-9;}
int main(){unsigned n=0;auto ck=[&](bool b,const char* m){++n;if(!b)throw std::runtime_error(m);};
 Terrain t(10,10,std::vector<double>(100,0));
 ck(t.width*t.depth==100,"vertex count");ck(t.cells()==81,"cell count");ck(t.tris()==162,"triangle count");
 auto ib=t.indices();ck(ib.size()==486,"index count");ck(ib[0]==0,"first a");ck(ib[1]==1,"first b");ck(ib[2]==10,"first c");
 ck(ib[3]==1&&ib[4]==11&&ib[5]==10,"other triangle");
 ck(ib.back()==98,"last index");
 ck(near(t.sample(0,0),0),"origin");ck(near(t.sample(9,9),0),"last vertex");
 Terrain small(2,2,{0,0,0,10});ck(near(small.sample(.25,.25),0),"first triangle");
 ck(near(small.sample(.75,.75),5),"second triangle differs bilinear");
 ck(near(small.sample(1,1),10),"corner d");ck(near(small.sample(1,0),0),"corner b");
 ck(near(small.sample(0,1),0),"corner c");ck(near(small.sample(.5,.5),0),"diagonal");
 Terrain ramp(2,2,{0,10,20,30});ck(near(ramp.sample(.25,.25),7.5),"ramp 1");
 ck(near(ramp.sample(.75,.75),22.5),"ramp 2");
 bool caught=false;try{small.sample(-.01,0);}catch(const std::out_of_range&){caught=true;}ck(caught,"negative x");
 caught=false;try{small.sample(0,1.01);}catch(const std::out_of_range&){caught=true;}ck(caught,"z overflow");
 caught=false;try{small.sample(std::numeric_limits<double>::quiet_NaN(),0);}catch(const std::out_of_range&){caught=true;}ck(caught,"nan coords");
 caught=false;try{Terrain bad(1,2,{0,0});}catch(const std::invalid_argument&){caught=true;}ck(caught,"bad width");
 caught=false;try{Terrain bad(2,2,{0,1,2});}catch(const std::invalid_argument&){caught=true;}ck(caught,"missing height");
 caught=false;try{Terrain bad(2,2,{0,1,std::numeric_limits<double>::infinity(),2});}catch(const std::invalid_argument&){caught=true;}ck(caught,"infinite height");
 std::vector<std::uint8_t> rgba(32,0);
 ck(validateRGBA(rgba,2,2,16),"pitch padded");ck(texOffset(0,1,16)==16,"second row start");
 ck(texOffset(1,1,16)==20,"second pixel second row");ck(!validateRGBA(rgba,2,2,7),"short pitch");
 ck(!validateRGBA(std::vector<std::uint8_t>(20),2,2,16),"short storage");
 ck(!validateRGBA(rgba,0,2,16),"zero width");ck(!validateRGBA(rgba,2,0,16),"zero height");
 ck(!validateRGBA(rgba,std::numeric_limits<std::size_t>::max(),2,16),"overflow width");
 std::cout<<"PASS "<<n<<" terrain CPU contract checks\n";
}
