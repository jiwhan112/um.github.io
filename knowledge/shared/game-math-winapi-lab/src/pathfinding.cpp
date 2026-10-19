#include "grid.hpp"
#include "check.hpp"
using namespace study;
int BfsDistance(const Grid& g,Cell start,Cell goal) {
    if (!g.Walkable(start)||!g.Walkable(goal)) return -1;
    const auto id=[&](Cell p){return p.y*g.Width()+p.x;};
    std::vector<int> distance(static_cast<std::size_t>(g.Width()*g.Height()),-1);
    std::queue<Cell> q; q.push(start); distance[static_cast<std::size_t>(id(start))]=0;
    const std::array<Cell,4> offsets{{{0,-1},{-1,0},{0,1},{1,0}}};
    while (!q.empty()) {
        const Cell p=q.front();q.pop(); if (p==goal) return distance[static_cast<std::size_t>(id(p))];
        for (auto d:offsets) {
            Cell n{p.x+d.x,p.y+d.y};
            if (!g.Walkable(n)||distance[static_cast<std::size_t>(id(n))]!=-1) continue;
            distance[static_cast<std::size_t>(id(n))]=distance[static_cast<std::size_t>(id(p))]+1; q.push(n);
        }
    }
    return -1;
}
int main() {
    int scenarios{};
    for (unsigned mask=0;mask<512;++mask) {
        Grid grid(3,3);
        for (unsigned bit=0;bit<9;++bit) if ((mask&(1u<<bit))!=0)
            grid.Set({static_cast<int>(bit%3),static_cast<int>(bit/3)},1);
        for (int s=0;s<9;++s) for (int t=0;t<9;++t) {
            const Cell start{s%3,s/3},goal{t%3,t/3}; ++scenarios;
            const auto path=AStar(grid,start,goal); const int distance=BfsDistance(grid,start,goal);
            Check(path.has_value()==(distance>=0),"A* reachability agrees with BFS");
            if (!path) continue;
            Check(path->front()==start&&path->back()==goal,"path endpoints");
            Check(path->size()==static_cast<std::size_t>(distance+1),"A* optimal length");
            for (std::size_t i=0;i<path->size();++i) {
                Check(grid.Walkable((*path)[i]),"path avoids obstacle");
                if (i) Check(std::abs((*path)[i].x-(*path)[i-1].x)+std::abs((*path)[i].y-(*path)[i-1].y)==1,"four-way path adjacency");
            }
        }
    }
    Check(!AStar(Grid(3,3),{-1,0},{0,0}),"outside endpoint rejected");
    std::cout<<scenarios<<" exhaustive 3x3 map/start/goal scenarios\n";
    return Done("pathfinding");
}
