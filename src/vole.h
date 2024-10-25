#ifndef VOLE_SIM_VOLE_H
#define VOLE_SIM_VOLE_H

#include <cstdint>
#include <functional>
#include <ostream>

namespace vole {
class Memory {
public:
	Memory();
	uint8_t &operator[](uint8_t);
	uint8_t operator[](uint8_t) const;

private:
	uint8_t m_Array[256];
};

class Registers {
public:
	uint8_t pc;

	Registers();
	uint8_t &operator[](uint8_t);
	uint8_t operator[](uint8_t) const;

private:
	uint8_t m_Array[16];
};

class Machine {
public:
	Memory mem;
	Registers reg;
	std::ostream &log;
	bool shouldHalt;

	Machine(std::ostream &logStream = std::cerr);

	/// @brief Load program from `from` and put it in memory starting at cell
	/// `at`
	/// @param fromPath The input file path to load the program from (in
	/// hexadecimal).
	/// @param at The starting memory address to put the program at.
	/// @return `true` if `from.fail()` never returned `true` after a read,
	/// otherwise `false`.
	bool loadProgram(const std::string &fromPath, uint8_t at = 0);

	/// @brief Load program from `from` and put it in memory starting at cell
	/// `at`
	/// @param from The input stream to load the program from (in hexadecimal).
	/// @param at The starting memory address to put the program at.
	/// @return `true` if `from.fail()` never returned `true` after a read,
	/// otherwise `false`.
	bool loadProgram(std::istream &from, uint8_t at = 0);

	/// @brief Run indefinitely.
	void run();

	/// @brief Only execute the next instruction.
	void step();
};

class Instruction {
public:
	explicit Instruction(Machine *);
	virtual void execute() = 0;

protected:
	Machine *m_Machine;
};

class ControlUnit {
public:
	explicit ControlUnit(Machine *);
	Instruction *decodeInstruction();

private:
	Machine *m_Machine;
};

class Load1 : public Instruction {
public:
	explicit Load1(Machine *);
	virtual void execute() override;
};

class Load2 : public Instruction {
public:
	explicit Load2(Machine *);
	virtual void execute() override;
};

class Store : public Instruction {
public:
	explicit Store(Machine *);
	virtual void execute() override;
};

class Move : public Instruction {
public:
	explicit Move(Machine *);
	virtual void execute() override;
};

class Add1 : public Instruction {
public:
	explicit Add1(Machine *);
	virtual void execute() override;
};

class Add2 : public Instruction {
public:
	explicit Add2(Machine *);
	virtual void execute() override;
};

class Or : public Instruction {
public:
	explicit Or(Machine *);
	virtual void execute() override;
};

class And : public Instruction {
public:
	explicit And(Machine *);
	virtual void execute() override;
};

class Xor : public Instruction {
public:
	explicit Xor(Machine *);
	virtual void execute() override;
};

class Rotate : public Instruction {
public:
	explicit Rotate(Machine *);
	virtual void execute() override;
};

class Jump : public Instruction {
public:
	explicit Jump(Machine *);
	virtual void execute() override;
};

class Halt : public Instruction {
public:
	explicit Halt(Machine *);
	virtual void execute() override;
};

inline extern const std::function<Instruction *(Machine *)>
	instructionFactory[]{
		[](Machine *mac) { return new Load1(mac); },
		[](Machine *mac) { return new Load2(mac); },
		[](Machine *mac) { return new Store(mac); },
		[](Machine *mac) { return new Move(mac); },
		[](Machine *mac) { return new Add1(mac); },
		[](Machine *mac) { return new Add2(mac); },
		[](Machine *mac) { return new Or(mac); },
		[](Machine *mac) { return new And(mac); },
		[](Machine *mac) { return new Xor(mac); },
		[](Machine *mac) { return new Rotate(mac); },
		[](Machine *mac) { return new Jump(mac); },
		[](Machine *mac) { return new Halt(mac); },
	};
} // namespace vole

#endif // VOLE_SIM_VOLE_H
