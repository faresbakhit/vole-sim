#pragma once

#include <cstdint>
#include <functional>
#include <iostream>

namespace vole {
class Memory
{
public:
	Memory();
	uint8_t &operator[](uint8_t);
	uint8_t operator[](uint8_t) const;

private:
	uint8_t m_Array[256];
};

class Registers
{
public:
	uint8_t pc;

	Registers();
	uint8_t &operator[](uint8_t);
	uint8_t operator[](uint8_t) const;

private:
	uint8_t m_Array[16];
};

class Machine
{
public:
	Memory mem;
	Registers reg;
	std::ostream &log;
	bool shouldHalt;

	explicit Machine(std::ostream &logStream = std::cerr);

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

class ControlUnit
{
public:
	uint16_t
		/// Full instruction (16 bits wide).
		inst,
		/// Operation code (4 bits wide). Bits [15:12]
		opcode,
		/// First instruction operand (4 bits wide). Bits [11:8].
		operand1,
		/// Second instruction operand (4 bits wide). Bits [7:4].
		operand2,
		/// Third instruction operand (4 bits wide). Bits [3:0].
		operand3;
	Machine *mac;

	explicit ControlUnit(Machine *);
	static ControlUnit *decode(Machine *);
	virtual void execute() = 0;
};

class Load1 : public ControlUnit
{
public:
	using ControlUnit::ControlUnit;
	void execute() override;
};

class Load2 : public ControlUnit
{
public:
	using ControlUnit::ControlUnit;
	void execute() override;
};

class Store : public ControlUnit
{
public:
	using ControlUnit::ControlUnit;
	void execute() override;
};

class Move : public ControlUnit
{
public:
	using ControlUnit::ControlUnit;
	void execute() override;
};

class Add1 : public ControlUnit
{
public:
	using ControlUnit::ControlUnit;
	void execute() override;
};

class Add2 : public ControlUnit
{
public:
	using ControlUnit::ControlUnit;
	void execute() override;
};

class Or : public ControlUnit
{
public:
	using ControlUnit::ControlUnit;
	void execute() override;
};

class And : public ControlUnit
{
public:
	using ControlUnit::ControlUnit;
	void execute() override;
};

class Xor : public ControlUnit
{
public:
	using ControlUnit::ControlUnit;
	void execute() override;
};

class Rotate : public ControlUnit
{
public:
	using ControlUnit::ControlUnit;
	void execute() override;
};

class Jump : public ControlUnit
{
public:
	using ControlUnit::ControlUnit;
	void execute() override;
};

class Halt : public ControlUnit
{
public:
	using ControlUnit::ControlUnit;
	void execute() override;
};

inline extern const std::function<ControlUnit *(Machine *)>
	controlUnitFactory[]{
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
