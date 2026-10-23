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
// [한국어 해설] 폭·높이 1..256, 타일 ID 0..4095 계약을 가진 행 우선 저장 모델이다.
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
// [한국어 해설] 범위를 검사한 뒤 y*폭+x로 선형 인덱스를 계산한다.
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
// [한국어 해설] 길찾기에서만 0을 통과 가능으로 해석한다. 모든 타일의 일반적인 의미를 정의한 것은 아니다.
    bool Walkable(Cell p) const { return Contains(p)&&Tile(p)==0; }
    std::string Serialize() const {
        std::ostringstream out; out<<"MAP1 "<<width_<<' '<<height_<<'\n';
        for (int y=0;y<height_;++y) {
            for (int x=0;x<width_;++x) out<<Tile({x,y})<<(x+1==width_?'\n':' ');
        }
        if (!out) throw std::runtime_error("serialization failed");
        return out.str();
    }
// [한국어 해설] 4MiB 제한, 헤더, 셀 수, 타일 범위, 남는 토큰까지 검사한 임시 맵만 반환한다.
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
// [한국어 해설] 모델과 Undo/Redo 이력을 소유한다. GUI 위젯이나 디스크 파일을 직접 소유하지 않는다.
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
// [한국어 해설] 이력 메모리 할당에 실패하면 데이터가 바뀌지 않도록 기록을 먼저 추가한다.
        undo_.push_back(Edit{cell,old,tile}); // allocate history before modifying data
        grid_.Set(cell,tile); redo_.clear();
    }
// [한국어 해설] 되돌리기 전에 반대 이력에 기록하여 다음 Redo에 필요한 변경 정보를 보존한다.
    bool Undo() {
        if (undo_.empty()) return false;
        const Edit edit=undo_.back(); redo_.push_back(edit);
        grid_.Set(edit.cell,edit.before); undo_.pop_back(); return true;
    }
// [한국어 해설] 다시 실행한 변경은 Undo 이력으로 돌아간다. 새 편집은 기존 Redo 이력을 지운다.
    bool Redo() {
        if (redo_.empty()) return false;
        const Edit edit=redo_.back(); undo_.push_back(edit);
        grid_.Set(edit.cell,edit.after); redo_.pop_back(); return true;
    }
// [한국어 해설] 완전히 파싱한 후보로만 교체한다. 파일 시스템 원자적 저장이나 전원 장애 복구는 별개다.
    void Load(std::string_view text) {
        Grid candidate=Grid::Parse(text); // parse fully before committing
        grid_=std::move(candidate); undo_.clear(); redo_.clear();
    }
};
// [한국어 해설] 네 방향 비용 1과 맨해튼 휴리스틱 조합이다. 대각선·가중치 이동에는 계약을 다시 정해야 한다.
inline std::optional<std::vector<Cell>> AStar(const Grid& grid,Cell start,Cell goal) {
    if (!grid.Walkable(start)||!grid.Walkable(goal)) return std::nullopt;
    const int width=grid.Width(),count=width*grid.Height(); // bounded by 256*256
    const auto id=[width](Cell p){return p.y*width+p.x;};
    const auto point=[width](int n){return Cell{n%width,n/width};};
    const auto h=[goal](Cell p){return std::abs(goal.x-p.x)+std::abs(goal.y-p.y);};
    std::vector<int> distance(static_cast<std::size_t>(count),std::numeric_limits<int>::max());
    std::vector<int> parent(static_cast<std::size_t>(count),-1);
// [한국어 해설] f=g+h 우선, 동률이면 g와 셀 ID로 비교하여 실행 순서를 일정하게 한다.
    using Entry=std::tuple<int,int,int>; // f, g, id; deterministic tiebreak
    std::priority_queue<Entry,std::vector<Entry>,std::greater<Entry>> open;
    distance[static_cast<std::size_t>(id(start))]=0;
    open.emplace(h(start),0,id(start));
    constexpr std::array<Cell,4> neighbors{{{1,0},{0,1},{-1,0},{0,-1}}};
    while (!open.empty()) {
        const auto [f,g,node]=open.top(); open.pop(); (void)f;
// [한국어 해설] 더 짧은 경로가 이미 발견된 큐 항목은 오래된 항목이므로 버린다.
        if (g!=distance[static_cast<std::size_t>(node)]) continue; // stale entry
        const Cell at=point(node);
// [한국어 해설] 부모 인덱스를 역추적한 뒤 뒤집는다. 결과에는 시작과 목표 셀이 모두 포함된다.
        if (at==goal) {
            std::vector<Cell> path;
            for (int p=node;p!=-1;p=parent[static_cast<std::size_t>(p)]) path.push_back(point(p));
            std::reverse(path.begin(),path.end()); return path;
        }
        for (Cell offset:neighbors) {
            const Cell next{at.x+offset.x,at.y+offset.y};
            if (!grid.Walkable(next)) continue;
            const int n=id(next),candidate=g+1;
// [한국어 해설] 현재보다 짧아질 때만 거리와 부모를 바꾸고 새 우선순위 항목을 넣는다.
            if (candidate>=distance[static_cast<std::size_t>(n)]) continue;
            distance[static_cast<std::size_t>(n)]=candidate; parent[static_cast<std::size_t>(n)]=node;
            open.emplace(candidate+h(next),candidate,n);
        }
    }
    return std::nullopt;
}
}
#endif
