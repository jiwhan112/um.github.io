#include "grid.hpp"
#include "check.hpp"
#include <future>
using namespace study;
// [한국어 해설] 그리기와 Undo/Redo, 직렬화 왕복, 실패한 로드가 기존 문서를 보존하는지 검사한다.
int main() {
    TileDocument doc(Grid(4,3));
    doc.Paint({1,1},7); Check(doc.Data().Tile({1,1})==7,"paint");
    Check(doc.Undo()&&doc.Data().Tile({1,1})==0,"undo");
    Check(doc.Redo()&&doc.Data().Tile({1,1})==7,"redo");
    Check(doc.Undo(),"second undo"); doc.Paint({0,0},5);
    Check(!doc.Redo(),"new edit discards redo");
    const std::string original=doc.Data().Serialize();
    const auto copy=Grid::Parse(original);
    Check(copy.Serialize()==original,"map text roundtrip");
    for (std::string bad:{"MAP2 1 1 0","MAP1 0 2","MAP1 257 1","MAP1 1 1 -1", "MAP1 1 1 4096","MAP1 2 1 0","MAP1 1 1 0 junk"}) {
        Throws([&]{doc.Load(bad);});
        Check(doc.Data().Serialize()==original,"failed load preserves document");
    }
    Throws([&]{doc.Paint({-1,0},2);});
    Check(doc.Data().Serialize()==original,"failed paint preserves document");
    doc.Load("MAP1 2 2\n0 1\n2 3\n"); Check(doc.Data().Tile({1,1})==3,"valid load");
    Check(!doc.Undo()&&!doc.Redo(),"load invalidates edit history");
    // Workers receive a value snapshot. Applying to UI/document is a separate action.
// [한국어 해설] 작업자는 값 복사 스냅샷만 사용한다. 리비전 비교 예시이며 실제 UI 적용/취소 큐 구현은 아니다.
    const int submittedRevision=1, currentRevision=2;
    auto job=std::async(std::launch::async,[snapshot=Grid(3,3)]{return AStar(snapshot,{0,0},{2,2});});
    const auto path=job.get();
    Check(path&&path->size()==5,"worker computes from snapshot");
    Check(submittedRevision!=currentRevision,"stale result must be rejected");
    return Done("tile_document");
}
