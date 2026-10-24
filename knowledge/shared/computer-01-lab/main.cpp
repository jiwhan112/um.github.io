// 새 교육용 CPU 추적 모델: 실제 ISA/바이트 주소/파이프라인 에뮬레이터가 아니다.
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

enum class Op { Load, Add, Store, Halt };
enum class Status { Halted, BadRegister, BadAddress, NoHalt, BudgetExceeded, BadInstruction };
struct Instruction { Op op; std::size_t dst, a, b; };
struct Cpu {
    std::array<std::uint32_t, 4> registers{}; // 네 레지스터 모두 쓰기 가능하다.
    std::array<std::uint32_t, 8> memory{};    // 주소 대신 32비트 셀의 인덱스를 받는다.
    std::size_t pc = 0;                    // 바이트 주소가 아닌 명령 배열 인덱스다.
    std::size_t steps = 0;
};

Status run(Cpu& cpu, const std::vector<Instruction>& program, std::size_t budget) {
    while (cpu.pc < program.size()) {
        // 실행 상한은 명령을 실행하기 전에 검사한다. budget은 Cpu에 누적된 steps의 최대 허용값이다.
        if (cpu.steps >= budget) return Status::BudgetExceeded;
        const Instruction in = program[cpu.pc];
        switch (in.op) {
        case Op::Load:
            if (in.dst >= cpu.registers.size()) return Status::BadRegister;
            if (in.a >= cpu.memory.size()) return Status::BadAddress;
            cpu.registers[in.dst] = cpu.memory[in.a];
            break;
        case Op::Add:
            if (in.dst >= cpu.registers.size() || in.a >= cpu.registers.size()
                || in.b >= cpu.registers.size()) return Status::BadRegister;
            // uint32_t의 결과는 2^32로 나눈 나머지. signed overflow를 만들지 않는다.
            cpu.registers[in.dst] = static_cast<std::uint32_t>(
                static_cast<std::uint64_t>(cpu.registers[in.a]) + cpu.registers[in.b]);
            break;
        case Op::Store:
            if (in.a >= cpu.registers.size()) return Status::BadRegister;
            if (in.dst >= cpu.memory.size()) return Status::BadAddress;
            cpu.memory[in.dst] = cpu.registers[in.a];
            break;
        case Op::Halt:
            ++cpu.steps;
            return Status::Halted; // HALT의 PC를 남겨 종료 지점을 관찰한다.
        default:
            return Status::BadInstruction; // 정의하지 않은 명령은 조용히 넘기지 않는다.
        }
        ++cpu.pc;
        ++cpu.steps;
    }
    return Status::NoHalt; // 배열 끝 도달과 HALT 명령을 구분한다.
}

int main() {
    unsigned checks = 0;
    const auto check = [&checks](bool ok) {
        ++checks;
        if (!ok) throw std::runtime_error("contract check failed");
    };
    Cpu cpu;
    cpu.memory[2] = 7;
    const std::vector<Instruction> program = {
        {Op::Load, 0, 2, 0}, {Op::Add, 1, 0, 0},
        {Op::Store, 3, 1, 0}, {Op::Halt, 0, 0, 0}
    };
    check(run(cpu, program, 4) == Status::Halted);
    check(cpu.registers[0] == 7);
    check(cpu.registers[1] == 14);
    check(cpu.memory[3] == 14);
    check(cpu.steps == 4);
    check(cpu.pc == 3);
    // 잘못된 피연산자는 접근 전에 거절한다. 실제 page fault를 발생시키지 않는다.
    Cpu bad;
    check(run(bad, {{Op::Load, 0, 8, 0}}, 1) == Status::BadAddress);
    check(bad.registers[0] == 0 && bad.steps == 0);
    check(run(bad, {{Op::Load, 4, 0, 0}}, 1) == Status::BadRegister);
    check(run(bad, {{Op::Add, 0, 0, 4}}, 1) == Status::BadRegister);
    check(run(bad, {{Op::Store, 0, 4, 0}}, 1) == Status::BadRegister);
    check(run(bad, {{Op::Store, 8, 0, 0}}, 1) == Status::BadAddress);
    check(run(bad, {{static_cast<Op>(99), 0, 0, 0}}, 1) == Status::BadInstruction);
    Cpu empty;
    check(run(empty, {}, 1) == Status::NoHalt);
    Cpu limited;
    check(run(limited, program, 0) == Status::BudgetExceeded);
    check(limited.pc == 0 && limited.steps == 0);
    Cpu noHalt;
    check(run(noHalt, {{Op::Load, 0, 0, 0}}, 1) == Status::NoHalt);
    Cpu wrap;
    wrap.registers[0] = UINT32_MAX;
    wrap.registers[1] = 1;
    check(run(wrap, {{Op::Add, 2, 0, 1}, {Op::Halt, 0, 0, 0}}, 2) == Status::Halted);
    check(wrap.registers[2] == 0);
    std::cout << "LOAD[2]=7 ADD=14 STORE[3]=14 HALT\n";
    std::cout << "PASS: " << checks << " contract checks\n";
}
