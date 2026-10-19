#ifndef STUDY_GRID_HPP
#define STUDY_GRID_HPP
#include <functional>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <optional>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>
namespace study {
struct Cell { int x{},y{}; bool operator==(const Cell&) const=default; };
class Grid {
    int width_,height_;
    std::vector<int> tiles_;
    static std::size_t Count(int w,int h) {
        if (w<=0||h<=0||w>256||h>256) throw std::invalid_argument("dimensions must be 1..256");
        return static_cast<std::size_t>(w)*static_cast<std::size_t>(h);
    }
public:
    Grid(int w,int h):width_(w),height_(h),tiles_(Count(w,h)) {}
    int Width() const noexcept { return width_; }
    int Height() const noexcept { return height_; }
    bool Contains(Cell p) const noexcept { return p.x>=0&&p.y>=0&&p.x<width_&&p.y<height_; }
    std::size_t Index(Cell p) const {
        if (!Contains(p)) throw std::out_of_range("cell");
        return static_cast<std::size_t>(p.y)*static_cast<std::size_t>(width_)+static_cast<std::size_t>(p.x);
    }
    int Tile(Cell p) const { return tiles_[Index(p)]; }
    void Set(Cell p,int tile) {
        if (tile<0||tile>4095) throw std::invalid_argument("tile id must be 0..4095");
        tiles_[Index(p)]=tile;
    }
    // For pathfinding only: 0 = walkable; all other tile IDs are blocked.
    bool Walkable(Cell p) const { return Contains(p)&&Tile(p)==0; }
    std::string Serialize() const {
        std::ostringstream out; out<<"MAP1 "<<width_<<' '<<height_<<'\n';
        for (int y=0;y<height_;++y) {
            for (int x=0;x<width_;++x) out<<Tile({x,y})<<(x+1==width_?'\n':' ');
        }
        if (!out) throw std::runtime_error("serialization failed");
        return out.str();
    }
    static Grid Parse(std::string_view text) {
        if (text.size()>4*1024*1024) throw std::invalid_argument("map exceeds 4 MiB");
        std::istringstream in{std::string(text)};
        std::string magic; int width{},height{};
        if (!(in>>magic>>width>>height)||magic!="MAP1") throw std::invalid_argument("map header");
        Grid candidate(width,height);
        for (int y=0;y<height;++y) for (int x=0;x<width;++x) {
            int tile{};
            if (!(in>>tile)) throw std::invalid_argument("missing tile");
            candidate.Set({x,y},tile);
        }
        in>>std::ws;
        if (!in.eof()) throw std::invalid_argument("trailing map data");
        return candidate;
    }
};
struct Edit { Cell cell; int before{},after{}; };
class TileDocument {
    Grid grid_;
    std::vector<Edit> undo_,redo_;
public:
    explicit TileDocument(Grid grid):grid_(std::move(grid)) {}
    const Grid& Data() const noexcept { return grid_; }
    void Paint(Cell cell,int tile) {
        if (tile<0||tile>4095) throw std::invalid_argument("tile");
        const int old=grid_.Tile(cell);
        if (old==tile) return;
        undo_.push_back(Edit{cell,old,tile}); // allocate history before modifying data
        grid_.Set(cell,tile); redo_.clear();
    }
    bool Undo() {
        if (undo_.empty()) return false;
        const Edit edit=undo_.back(); redo_.push_back(edit);
        grid_.Set(edit.cell,edit.before); undo_.pop_back(); return true;
    }
    bool Redo() {
        if (redo_.empty()) return false;
        const Edit edit=redo_.back(); undo_.push_back(edit);
        grid_.Set(edit.cell,edit.after); redo_.pop_back(); return true;
    }
    void Load(std::string_view text) {
        Grid candidate=Grid::Parse(text); // parse fully before committing
        grid_=std::move(candidate); undo_.clear(); redo_.clear();
    }
};
inline std::optional<std::vector<Cell>> AStar(const Grid& grid,Cell start,Cell goal) {
    if (!grid.Walkable(start)||!grid.Walkable(goal)) return std::nullopt;
    const int width=grid.Width(),count=width*grid.Height(); // bounded by 256*256
    const auto id=[width](Cell p){return p.y*width+p.x;};
    const auto point=[width](int n){return Cell{n%width,n/width};};
    const auto h=[goal](Cell p){return std::abs(goal.x-p.x)+std::abs(goal.y-p.y);};
    std::vector<int> distance(static_cast<std::size_t>(count),std::numeric_limits<int>::max());
    std::vector<int> parent(static_cast<std::size_t>(count),-1);
    using Entry=std::tuple<int,int,int>; // f, g, id; deterministic tiebreak
    std::priority_queue<Entry,std::vector<Entry>,std::greater<Entry>> open;
    distance[static_cast<std::size_t>(id(start))]=0;
    open.emplace(h(start),0,id(start));
    constexpr std::array<Cell,4> neighbors{{{1,0},{0,1},{-1,0},{0,-1}}};
    while (!open.empty()) {
        const auto [f,g,node]=open.top(); open.pop(); (void)f;
        if (g!=distance[static_cast<std::size_t>(node)]) continue; // stale entry
        const Cell at=point(node);
        if (at==goal) {
            std::vector<Cell> path;
            for (int p=node;p!=-1;p=parent[static_cast<std::size_t>(p)]) path.push_back(point(p));
            std::reverse(path.begin(),path.end()); return path;
        }
        for (Cell offset:neighbors) {
            const Cell next{at.x+offset.x,at.y+offset.y};
            if (!grid.Walkable(next)) continue;
            const int n=id(next),candidate=g+1;
            if (candidate>=distance[static_cast<std::size_t>(n)]) continue;
            distance[static_cast<std::size_t>(n)]=candidate; parent[static_cast<std::size_t>(n)]=node;
            open.emplace(candidate+h(next),candidate,n);
        }
    }
    return std::nullopt;
}
}
#endif
