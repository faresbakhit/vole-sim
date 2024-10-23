
#ifndef VOLE_SIM_VOLE_H
#define VOLE_SIM_VOLE_H

#include <functional>
#include <cstdint>

enum Reg {
    R0 = 0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
};

class VoleMachine {
private:
    uint8_t pc;
    uint8_t mem[256];
    uint8_t reg[16];
};

class Instruction {
public:
    explicit Instruction(VoleMachine& mac): machine(mac) {};
    virtual void run() {};
private:
    VoleMachine& machine;
};

class Load1 : public Instruction {
public:
    explicit Load1(VoleMachine& mac) : Instruction(mac) {}
};

class Load2 : public Instruction {
public:
    explicit Load2(VoleMachine& mac) : Instruction(mac) {}
};
class Store : public Instruction {
public:
    explicit Store(VoleMachine& mac) : Instruction(mac) {}
};
class Move : public Instruction {
public:
    explicit Move(VoleMachine& mac) : Instruction(mac) {}
};

class Add1 : public Instruction {
public:
    explicit Add1(VoleMachine& mac) : Instruction(mac) {}
};

class Add2 : public Instruction {
public:
    explicit Add2(VoleMachine& mac) : Instruction(mac) {}
};

class Or : public Instruction {
public:
    explicit Or(VoleMachine& mac) : Instruction(mac) {}
};

class And : public Instruction {
public:
    explicit And(VoleMachine& mac) : Instruction(mac) {}
};

class Xor : public Instruction {
public:
    explicit Xor(VoleMachine& mac) : Instruction(mac) {}
};

class Rotate : public Instruction {
public:
    explicit Rotate(VoleMachine& mac) : Instruction(mac) {}
};

class Jump : public Instruction {
public:
    explicit Jump(VoleMachine& mac) : Instruction(mac) {}
};

class Halt : public Instruction {
public:
    explicit Halt(VoleMachine& mac) : Instruction(mac) {}
};

inline extern const std::function<Instruction *(VoleMachine&)> instructionFactory[]{
        [](VoleMachine& mac) { return new Load1(mac); },
        [](VoleMachine& mac) { return new Load2(mac); },
        [](VoleMachine& mac) { return new Move(mac); },
        [](VoleMachine& mac) { return new Store(mac); },
        [](VoleMachine& mac) { return new Add1(mac); },
        [](VoleMachine& mac) { return new Add2(mac); },
        [](VoleMachine& mac) { return new Or(mac); },
        [](VoleMachine& mac) { return new And(mac); },
        [](VoleMachine& mac) { return new Xor(mac); },
        [](VoleMachine& mac) { return new Rotate(mac); },
        [](VoleMachine& mac) { return new Jump(mac); },
        [](VoleMachine& mac) { return new Halt(mac); },
};

#endif //VOLE_SIM_VOLE_H
