#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <iostream>

namespace vole {
class Memory {
public:
	const static size_t SIZE = 256;
	Memory();
	void Reset();
	uint8_t &operator[](uint8_t);
	uint8_t operator[](uint8_t) const;
	std::array<uint8_t, Memory::SIZE> *Array();

private:
	std::array<uint8_t, SIZE> m_Array;
};

class Registers {
public:
	uint8_t pc;
	Registers();
	void Reset();
	uint8_t &operator[](uint8_t);
	uint8_t operator[](uint8_t) const;

private:
	std::array<uint8_t, 16> m_Array;
};

class Machine;

enum class ShouldHalt : bool { NO, YES };

class ControlUnit {
public:
	ControlUnit(Machine *);
	ControlUnit(Machine *, uint8_t at);
	static ControlUnit *Decode(Machine *);
	static ControlUnit *Decode(Machine *, uint8_t at);
	virtual ShouldHalt Execute() = 0;
	virtual std::string Humanize() = 0;
	virtual ~ControlUnit();

protected:
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
		operand3,
		/// Half instruction (8 bits wide). Bits [7:0].
		operandXY;

	Machine *mac;
};

class Nothing : public ControlUnit {
public:
	using ControlUnit::ControlUnit;
	ShouldHalt Execute() override;
	std::string Humanize() override;
	~Nothing();
};

class Load1 : public ControlUnit {
public:
	using ControlUnit::ControlUnit;
	ShouldHalt Execute() override;
	std::string Humanize() override;
	~Load1();
};

class Load2 : public ControlUnit {
public:
	using ControlUnit::ControlUnit;
	ShouldHalt Execute() override;
	std::string Humanize() override;
	~Load2();
};

class Store : public ControlUnit {
public:
	using ControlUnit::ControlUnit;
	ShouldHalt Execute() override;
	std::string Humanize() override;
	~Store();
};

class Move : public ControlUnit {
public:
	using ControlUnit::ControlUnit;
	ShouldHalt Execute() override;
	std::string Humanize() override;
	~Move();
};

class Add1 : public ControlUnit {
public:
	using ControlUnit::ControlUnit;
	ShouldHalt Execute() override;
	std::string Humanize() override;
	~Add1();
};

class Add2 : public ControlUnit {
public:
	using ControlUnit::ControlUnit;
	ShouldHalt Execute() override;
	std::string Humanize() override;
	~Add2();
};

class Or : public ControlUnit {
public:
	using ControlUnit::ControlUnit;
	ShouldHalt Execute() override;
	std::string Humanize() override;
	~Or();
};

class And : public ControlUnit {
public:
	using ControlUnit::ControlUnit;
	ShouldHalt Execute() override;
	std::string Humanize() override;
	~And();
};

class Xor : public ControlUnit {
public:
	using ControlUnit::ControlUnit;
	ShouldHalt Execute() override;
	std::string Humanize() override;
	~Xor();
};

class Rotate : public ControlUnit {
public:
	using ControlUnit::ControlUnit;
	ShouldHalt Execute() override;
	std::string Humanize() override;
	~Rotate();
};

class Jump : public ControlUnit {
public:
	using ControlUnit::ControlUnit;
	ShouldHalt Execute() override;
	std::string Humanize() override;
	~Jump();
};

class Halt : public ControlUnit {
public:
	using ControlUnit::ControlUnit;
	ShouldHalt Execute() override;
	std::string Humanize() override;
	~Halt();
};

class Unused : public ControlUnit {
public:
	using ControlUnit::ControlUnit;
	ShouldHalt Execute() override;
	std::string Humanize() override;
	~Unused();
};

typedef std::function<ControlUnit *(Machine *, uint8_t)> ControlUnitBuilder;
inline const ControlUnitBuilder NothingBuilder = [](Machine *mac, uint8_t at) { return new Nothing(mac, at); };
inline const ControlUnitBuilder UnusedBuilder = [](Machine *mac, uint8_t at) { return new Unused(mac, at); };

inline const std::array<ControlUnitBuilder, 16> DefaultControlUnitFactory = {
	NothingBuilder,
	[](Machine *mac, uint8_t at) { return new Load1(mac, at); },
	[](Machine *mac, uint8_t at) { return new Load2(mac, at); },
	[](Machine *mac, uint8_t at) { return new Store(mac, at); },
	[](Machine *mac, uint8_t at) { return new Move(mac, at); },
	[](Machine *mac, uint8_t at) { return new Add1(mac, at); },
	[](Machine *mac, uint8_t at) { return new Add2(mac, at); },
	[](Machine *mac, uint8_t at) { return new Or(mac, at); },
	[](Machine *mac, uint8_t at) { return new And(mac, at); },
	[](Machine *mac, uint8_t at) { return new Xor(mac, at); },
	[](Machine *mac, uint8_t at) { return new Rotate(mac, at); },
	[](Machine *mac, uint8_t at) { return new Jump(mac, at); },
	[](Machine *mac, uint8_t at) { return new Halt(mac, at); },
	UnusedBuilder,
	UnusedBuilder,
	UnusedBuilder,
};

class Machine {
public:
	Memory mem;
	Registers reg;
	std::ostream &log;
	const std::array<ControlUnitBuilder, 16> controlUnitFactory;

	Machine(std::ostream &logStream = std::cerr,
			std::array<ControlUnitBuilder, 16> controlUnitFactory = DefaultControlUnitFactory);
	Machine(std::array<ControlUnitBuilder, 16> controlUnitFactory);

	/// @brief Reset all registers and memory cells.
	void Reset();

	/// @brief Load program from `from` and put it in memory starting at cell
	/// `at`
	/// @param fromPath The input file path to load the program from (in
	/// hexadecimal).
	/// @param at The starting memory address to put the program at.
	/// @return `true` if `from.fail()` never returned `true` after a read,
	/// otherwise `false`.
	bool LoadProgram(const std::string &fromPath, uint8_t at = 0);

	/// @brief Load program from `from` and put it in memory starting at cell
	/// `at`
	/// @param from The input stream to load the program from (in hexadecimal).
	/// @param at The starting memory address to put the program at.
	/// @return `true` if `from.fail()` never returned `true` after a read,
	/// otherwise `false`.
	bool LoadProgram(std::istream &from, uint8_t at = 0);

	/// @brief Run instructions indefinitely. Only returns on Step() ==
	/// ShouldHalt::YES.
	void Run();

	/// @brief Only execute the next instruction.
	ShouldHalt Step();
};

struct Float {
	static float Decode(uint8_t);
	static uint8_t Encode(float);
};
} // namespace vole
