#include "check.hpp"
#include "inventory.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <system_error>

namespace fs = std::filesystem;

// 테스트 전용 임시 디렉터리의 소유자다. 기존 사용자 파일을 대상으로 삼지 않는다.
class TempDirectory {
public:
    TempDirectory() {
        const auto token = std::chrono::steady_clock::now().time_since_epoch().count();
        for (int i = 0; i < 100; ++i) {
            auto candidate = fs::temp_directory_path() / ("cpp_basics_" + std::to_string(token) + "_" + std::to_string(i));
            if (fs::create_directory(candidate)) {
                path = std::move(candidate);
                return;
            }
        }
        throw std::runtime_error("cannot create temporary directory");
    }
    TempDirectory(const TempDirectory&) = delete;
    TempDirectory& operator=(const TempDirectory&) = delete;
    // 소멸자에서 예외를 던지지 않는 정리 방식이다. 정리 오류는 이 학습 예제에서 보고하지 않는다.
    ~TempDirectory() {
        std::error_code error;
        fs::remove_all(path, error);
    }
    fs::path path;
};

// 잘못된 입력이 Load의 runtime_error로 거부되는지 확인한다.
bool Rejects(const std::string& text) {
    try {
        std::istringstream input{text};
        (void)study::Load(input);
    }
    catch (const std::runtime_error&) { return true; }
    return false;
}

int main() {
    try {
        study::Inventory items{{1,"Health Potion",3},{2,"Mana",1}};
        std::ostringstream text;
        study::Save(text, items);
        std::istringstream input{text.str()};
        const auto loaded = study::Load(input);
        // 메모리 스트림의 저장 → 로드로 주요 필드가 돌아오는지 확인한다.
        CHECK(loaded.size() == 2);
        CHECK(loaded.at(0).name == "Health Potion");
        CHECK(loaded.at(0).quantity == 3);

        // 헤더·개수·누락 행·필드·중복·잔여 데이터가 잘못된 경우를 각각 기록한다.
        CHECK(Rejects(""));
        CHECK(Rejects("INV2 0\n"));
        CHECK(Rejects("INV1 -1\n"));
        CHECK(Rejects("INV1 1001\n"));
        CHECK(Rejects("INV1 2\n1 \"A\" 1\n"));
        CHECK(Rejects("INV1 1\n1 \"A\" -1\n"));
        CHECK(Rejects("INV1 2\n1 \"A\" 1\n1 \"B\" 2\n"));
        CHECK(Rejects("INV1 1\n1 \"A\" 1 junk\n"));
        CHECK(Rejects("INV1 0\nextra\n"));
        CHECK(Rejects("INV1 1\n1 \"\" 0\n"));

        std::istringstream empty{"INV1 0\n"};
        CHECK(study::Load(empty).empty());
        bool rejected = false;
        try {
            std::ostringstream out;
            study::Save(out, {{1,"A",-1}});
        }
        catch (const std::runtime_error&) { rejected = true; }
        CHECK(rejected);

        TempDirectory temp;
        const auto file = temp.path / "inventory.txt";
        {
            // binary는 스트림의 모드다. INV1의 필드 기반 텍스트 형식을 바꾸지 않는다.
            std::ofstream out{file, std::ios::binary};
            if(!out) throw std::runtime_error("open failed");
            study::Save(out, items);
            out.close();
            // close에서 지연된 쓰기 실패가 생길 수 있으므로 종료 상태도 확인한다.
            if(!out) throw std::runtime_error("close failed");
        }
        {
            std::ifstream in{file, std::ios::binary};
            if(!in) throw std::runtime_error("open failed");
            CHECK(study::Load(in).size() == 2);
        }
        const auto copied = temp.path / "copy.bin";
        CHECK(fs::copy_file(file, copied));
        // 이 CHECK는 파일 크기 일치만 검사한다. 바이트별 내용 비교는 아니다.
        CHECK(fs::file_size(copied) == fs::file_size(file));
        {
            std::ifstream missing{temp.path / "missing.txt"};
            CHECK(!missing);
        }
        // 상대 경로의 기준은 프로세스의 현재 작업 디렉터리다.
        std::cout << "cwd=" << fs::current_path().string() << '\n';
        Report();
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
