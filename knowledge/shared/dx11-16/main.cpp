// CPU broad-phase quadtree. Objects spanning child boundaries remain in parent.
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <set>
#include <stdexcept>
#include <vector>
struct Rect{double xmin,zmin,xmax,zmax;};
struct Item{unsigned id;Rect box;};
bool valid(Rect b){return std::isfinite(b.xmin)&&std::isfinite(b.zmin)&&std::isfinite(b.xmax)&&std::isfinite(b.zmax)&&b.xmin<=b.xmax&&b.zmin<=b.zmax;}
bool contains(Rect outer,Rect inner){return outer.xmin<=inner.xmin&&outer.zmin<=inner.zmin&&outer.xmax>=inner.xmax&&outer.zmax>=inner.zmax;}
bool overlaps(Rect a,Rect b){return a.xmin<=b.xmax&&a.xmax>=b.xmin&&a.zmin<=b.zmax&&a.zmax>=b.zmin;}
struct Node{
 Rect bounds;unsigned depth;std::vector<Item> objects;std::array<std::unique_ptr<Node>,4> child;
 Node(Rect b,unsigned d):bounds(b),depth(d){}
 bool divided()const{return child[0]!=nullptr;}
 int childIndex(Rect r)const{
  if(!divided())return -1;
  int chosen=-1;
  for(int i=0;i<4;++i)if(contains(child[static_cast<std::size_t>(i)]->bounds,r)){
   if(chosen!=-1)return -1; // boundary-sharing object: keep at parent
   chosen=i;
  }
  return chosen;
 }
void split(unsigned maxDepth){
 if(divided()||depth>=maxDepth)return;
 const double xm=(bounds.xmin+bounds.xmax)*0.5;
 const double zm=(bounds.zmin+bounds.zmax)*0.5;
 if(xm<=bounds.xmin||xm>=bounds.xmax||zm<=bounds.zmin||zm>=bounds.zmax)return;
 const std::array<Rect,4> r{{{bounds.xmin,bounds.zmin,xm,zm},{xm,bounds.zmin,bounds.xmax,zm},
                             {bounds.xmin,zm,xm,bounds.zmax},{xm,zm,bounds.xmax,bounds.zmax}}};
 for(int i=0;i<4;++i)child[static_cast<std::size_t>(i)].reset(new Node(r[static_cast<std::size_t>(i)],depth+1));
 std::vector<Item> remaining;
 for(const Item& it:objects){const int index=childIndex(it.box);
  if(index>=0)child[static_cast<std::size_t>(index)]->objects.push_back(it);
  else remaining.push_back(it);
 }
 objects.swap(remaining);
}
void insert(const Item& item,unsigned maxObjects,unsigned maxDepth){
 const int target=childIndex(item.box);
 if(target>=0){child[static_cast<std::size_t>(target)]->insert(item,maxObjects,maxDepth);return;}
 objects.push_back(item);
 if(objects.size()>maxObjects && depth<maxDepth){
  if(!divided())split(maxDepth);
  if(divided()){
   std::vector<Item> remain;
   for(const Item& it:objects){const int idx=childIndex(it.box);
    if(idx>=0)child[static_cast<std::size_t>(idx)]->insert(it,maxObjects,maxDepth);
    else remain.push_back(it);
   }
   objects.swap(remain);
  }
 }
}
 void query(Rect area,std::vector<unsigned>& ids)const{
  if(!overlaps(bounds,area))return;
  for(const Item& item:objects)if(overlaps(area,item.box))ids.push_back(item.id);
  if(divided())for(const auto& c:child)c->query(area,ids);
 }
};
struct QuadTree{
 Node root;unsigned maxObjects,maxDepth;std::set<unsigned> ids;
 QuadTree(Rect bounds,unsigned cap,unsigned maxD):root(bounds,0),maxObjects(cap),maxDepth(maxD){
  if(!valid(bounds)||bounds.xmin>=bounds.xmax||bounds.zmin>=bounds.zmax||!cap||!maxD)throw std::invalid_argument("bad root");
 }
 void insert(const Item& item){
  if(!valid(item.box)||!contains(root.bounds,item.box)||!ids.insert(item.id).second)
   throw std::invalid_argument("item out of root or duplicate");
  root.insert(item,maxObjects,maxDepth);
 }
 std::vector<unsigned> query(Rect area)const{
  if(!valid(area))throw std::invalid_argument("query area");
  std::vector<unsigned> found;root.query(area,found);
  std::sort(found.begin(),found.end());return found;
 }
};
unsigned checks=0;
void ck(bool b,const char* label){++checks;if(!b)throw std::runtime_error(label);}
template<class F>void rejects(F f){bool caught=false;try{f();}catch(const std::invalid_argument&){caught=true;}ck(caught,"expect invalid argument");}
int main(){
 const Rect extent{0,0,16,16};
 rejects([&]{QuadTree bad({0,0,0,1},2,4);});
 rejects([&]{QuadTree bad(extent,0,4);});
 rejects([&]{QuadTree bad(extent,2,0);});
 rejects([&]{QuadTree bad({0,0,1,-1},2,4);});
 QuadTree tree(extent,2,5);
 const std::vector<Item> items{{1,{1,1,2,2}},{2,{12,12,14,14}},{3,{7,7,9,9}},
  {4,{1,12,3,15}},{5,{13,1,15,3}},{6,{4,4,4,4}},
  {7,{0,0,16,16}},{8,{2,2,4,4}},{9,{8,2,10,4}},
  {10,{2,8,4,10}},{11,{8,8,10,10}},{12,{6,0,8,1}}};
 for(const Item& i:items){tree.insert(i);ck(tree.ids.count(i.id)==1,"inserted id");}
 ck(tree.root.divided(),"root divides");
 ck(tree.query(extent).size()==items.size(),"all ids found exactly once");
 auto compare=[&](Rect region){
  std::vector<unsigned> brute,actual=tree.query(region);
  for(const Item& item:items)if(overlaps(item.box,region))brute.push_back(item.id);
  std::sort(brute.begin(),brute.end());
  ck(actual==brute,"quadtree equals brute force");
  ck(std::adjacent_find(actual.begin(),actual.end())==actual.end(),"query unique");
 };
compare({1,1,2,2});compare({15,15,16,16});compare({7,7,8,8});
compare({-1,-1,1,1});compare({20,20,22,22});compare({0,0,16,16});
compare({8,0,8,16});compare({0,8,16,8});compare({-10,5,5,10});
compare({4,4,4,4});compare({5,5,6,6});compare({2,2,4,4});
for(unsigned x=0;x<16;x+=3)for(unsigned z=0;z<16;z+=3)
 compare({static_cast<double>(x),static_cast<double>(z),static_cast<double>(x+2),static_cast<double>(z+2)});
rejects([&]{tree.insert({99,{-1,0,1,1}});});
rejects([&]{tree.insert({1,{5,5,6,6}});});
rejects([&]{tree.insert({99,{5,5,4,6}});});
 rejects([&]{tree.query({1,0,0,1});});
 rejects([&]{const double nan=std::numeric_limits<double>::quiet_NaN();tree.query({0,0,nan,1});});
 ck(tree.ids.size()==items.size(),"no invalid inserts");
 ck(overlaps({0,0,1,1},{1,1,2,2}),"inclusive boundary");
 ck(!overlaps({0,0,1,1},{2,2,3,3}),"separated");
 ck(contains(extent,{0,0,16,16}),"exact containment");
 ck(!contains({0,0,1,1},{-0.1,0,0,1}),"root rejection");
 std::cout<<"PASS "<<checks<<" quadtree CPU broadphase checks\n";
}
