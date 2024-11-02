#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "vole.h"

using namespace vole;

#define OS_HEX1 std::hex << std::uppercase
#define OS_HEX2 std::hex << std::uppercase << std::setfill('0') << std::setw(2)

Machine::Machine(std::ostream &logStream, const std::array<ControlUnitBuilder, 16> cuFactory)
	: log(logStream), controlUnitFactory(cuFactory) {}

Machine::Machine(const std::array<ControlUnitBuilder, 16> cuFactory) : log(std::cerr), controlUnitFactory(cuFactory) {}

bool Machine::LoadProgram(const std::string &path, uint8_t addr) {
	std::ifstream ifs(path);
	if (ifs.fail()) {
		log << "Loading program failed.\n";
		return false;
	}
	return LoadProgram(ifs, addr);
}

bool Machine::LoadProgram(std::istream &stream, uint8_t addr) {
	uint16_t inst;
	for (size_t i = addr; !stream.eof(); i += 2) {
		if (i >= 2 * 128) {
			log << "Program too big (>128 instructions).\n";
			return false;
		}
		stream >> std::hex >> inst;
		if (stream.fail()) {
			log << "Loading program failed.\n";
			return false;
		}
		mem[i] = inst >> 8;
		mem[i + 1] = inst & 0x00FF;
	}
	return true;
}

void Machine::Reset() {
	reg.Reset(); // CPU
	mem.Reset(); // RAM
}

void Machine::Run() {
	do {
	} while (Step() != ShouldHalt::YES);
}

ShouldHalt Machine::Step() {
	ControlUnit *cu = ControlUnit::Decode(this);
	ShouldHalt shouldHalt = cu->Execute();
	delete cu;
	return shouldHalt;
}

Memory::Memory() : m_Array() {}

void Memory::Reset() { m_Array.fill(0); }

uint8_t &Memory::operator[](uint8_t idx) { return m_Array[idx]; }

uint8_t Memory::operator[](uint8_t idx) const { return m_Array[idx]; }

std::array<uint8_t, Memory::SIZE> *Memory::Array() { return &m_Array; }

Registers::Registers() : pc(0), m_Array() {}

void Registers::Reset() { m_Array.fill(0); }

uint8_t &Registers::operator[](uint8_t i) { return m_Array[i]; }

uint8_t Registers::operator[](uint8_t i) const { return m_Array[i]; }

ControlUnit::ControlUnit(Machine *machine) : ControlUnit(machine, machine->reg.pc) {}

ControlUnit::ControlUnit(Machine *machine, uint8_t at) : mac(machine) {
	inst = mac->mem[at];
	inst = (inst << 8) | mac->mem[at + 1];
	opcode = inst >> 12;
	operand1 = (inst >> 8) & 0x0F;
	operand2 = (inst >> 4) & 0x00F;
	operand3 = inst & 0x000F;
	operandXY = inst & 0x00FF;
}

ControlUnit *ControlUnit::Decode(Machine *mac) {
	auto cu = ControlUnit::Decode(mac, mac->reg.pc);
	mac->reg.pc += 2;
	return cu;
}

ControlUnit *ControlUnit::Decode(Machine *mac, uint8_t at) {
	uint8_t opcode = mac->mem[at] >> 4;
	auto controlUnitBuilder = mac->controlUnitFactory[opcode];
	auto cu = controlUnitBuilder(mac, at);
	return cu;
}

ShouldHalt Nothing::Execute() { return ShouldHalt::NO; }

std::string Nothing::Humanize() {
	std::ostringstream os;
	return os.str();
}

ShouldHalt Load1::Execute() {
	uint8_t r = operand1;
	uint16_t xy = operandXY;
	mac->reg[r] = mac->mem[xy];
	return ShouldHalt::NO;
}

std::string Load1::Humanize() {
	std::ostringstream os;
	os << "Copy bits at cell " << OS_HEX2 << operandXY << " to register " << OS_HEX1 << operand1;
	return os.str();
}

ShouldHalt Load2::Execute() {
	uint8_t r = operand1;
	uint16_t xy = operandXY;
	mac->reg[r] = xy;
	return ShouldHalt::NO;
}

std::string Load2::Humanize() {
	std::ostringstream os;
	os << "Copy bit-string " << OS_HEX2 << operandXY << " to register " << OS_HEX1 << operand1;
	return os.str();
}

ShouldHalt Store::Execute() {
	uint8_t r = operand1;
	uint16_t xy = operandXY;
	mac->mem[xy] = mac->reg[r];
	return ShouldHalt::NO;
}

std::string Store::Humanize() {
	std::ostringstream os;
	os << "Copy bits in register " << OS_HEX1 << operand1 << " to cell " << OS_HEX2 << operandXY;
	return os.str();
}

ShouldHalt Move::Execute() {
	uint8_t r = operand2;
	uint8_t s = operand3;
	mac->reg[s] = mac->reg[r];
	return ShouldHalt::NO;
}

std::string Move::Humanize() {
	std::ostringstream os;
	os << "Copy bits in register " << OS_HEX1 << operand2 << " to register " << OS_HEX1 << operand3;
	return os.str();
}

ShouldHalt Add1::Execute() {
	uint8_t r = operand1;
	uint8_t s = operand2;
	uint8_t t = operand3;
	mac->reg[r] = mac->reg[s] + mac->reg[t];
	return ShouldHalt::NO;
}

std::string Add1::Humanize() {
	std::ostringstream os;
	os << "Add bits in registers " << OS_HEX1 << operand2 << " and " << OS_HEX1 << operand3
	   << " (two's-complement), put in " << OS_HEX1 << operand1;
	return os.str();
}

ShouldHalt Add2::Execute() {
	uint8_t r = operand1;
	uint8_t s = operand2;
	uint8_t t = operand3;
	mac->reg[r] = Float::Encode(Float::Decode(mac->reg[s]) + Float::Decode(mac->reg[t]));
	return ShouldHalt::NO;
}

std::string Add2::Humanize() {
	std::ostringstream os;
	os << "Add bits in register " << OS_HEX1 << operand2 << " and " << OS_HEX1 << operand3 << " (float), put in "
	   << OS_HEX1 << operand1;
	return os.str();
}

ShouldHalt Or::Execute() {
	uint8_t r = operand1;
	uint8_t s = operand2;
	uint8_t t = operand3;
	mac->reg[r] = mac->reg[s] | mac->reg[t];
	return ShouldHalt::NO;
}

std::string Or::Humanize() {
	std::ostringstream os;
	os << "Bitwise OR bits in register " << OS_HEX1 << operand2 << " and " << OS_HEX1 << operand3 << ", put in "
	   << OS_HEX1 << operand1;
	return os.str();
}

ShouldHalt And::Execute() {
	uint8_t r = operand1;
	uint8_t s = operand2;
	uint8_t t = operand3;
	mac->reg[r] = mac->reg[s] & mac->reg[t];
	return ShouldHalt::NO;
}

std::string And::Humanize() {
	std::ostringstream os;
	os << "Bitwise AND bits in register " << OS_HEX1 << operand2 << " and " << OS_HEX1 << operand3 << ", put in "
	   << OS_HEX1 << operand1;
	return os.str();
}

ShouldHalt Xor::Execute() {
	uint8_t r = operand1;
	uint8_t s = operand2;
	uint8_t t = operand3;
	mac->reg[r] = mac->reg[s] ^ mac->reg[t];
	return ShouldHalt::NO;
}

std::string Xor::Humanize() {
	std::ostringstream os;
	os << "Bitwise XOR bits in register " << OS_HEX1 << operand1 << " and " << OS_HEX1 << operand2 << ", put in "
	   << OS_HEX1 << operand3;
	return os.str();
}

ShouldHalt Rotate::Execute() {
	uint8_t r = operand1;
	uint8_t t = operand3 % 8;
	mac->reg[r] = ((mac->reg[r] >> t) | (mac->reg[r] << (8 - t)));
	return ShouldHalt::NO;
}

std::string Rotate::Humanize() {
	std::ostringstream os;
	os << "Rotate bits in register " << OS_HEX1 << operand1 << " cyclically right " << OS_HEX1 << operand3 << " steps";
	return os.str();
}

ShouldHalt Jump::Execute() {

	uint8_t r = operand1;
	uint16_t xy = operandXY;
	if (xy % 2 != 0) // Not a full instruction at `xy`.
		xy--;
	if (mac->reg[r] == mac->reg[0])
		mac->reg.pc = xy;
	return ShouldHalt::NO;
}

std::string Jump::Humanize() {
	std::ostringstream os;
	os << "Jump to cell " << OS_HEX2 << operandXY << " if register " << OS_HEX1 << operand1 << " equals register 0";
	return os.str();
}

ShouldHalt Halt::Execute() { return ShouldHalt::YES; }

std::string Halt::Humanize() {
	std::ostringstream os;
	os << "Halt";
	return os.str();
}

ShouldHalt Unused::Execute() {
	mac->log << "Unspecified op-code " << opcode << " used. Halting.\n";
	return ShouldHalt::YES;
}

std::string Unused::Humanize() {
	std::ostringstream os;
	os << "Unspecified op-code " << OS_HEX1 << opcode;
	return os.str();
}

ControlUnit::~ControlUnit() = default;
Nothing::~Nothing() = default;
Load1::~Load1() = default;
Load2::~Load2() = default;
Store::~Store() = default;
Move::~Move() = default;
Add1::~Add1() = default;
Add2::~Add2() = default;
Or::~Or() = default;
And::~And() = default;
Xor::~Xor() = default;
Rotate::~Rotate() = default;
Jump::~Jump() = default;
Halt::~Halt() = default;
Unused::~Unused() = default;

float Float::Decode(uint8_t val) {
	bool sign;	  // 1 bit
	int exponent; // 3 bits
	int mantissa; // 4 bits

	sign = (val >> 7) & 0x1;
	exponent = (val >> 4) & 0x7;
	mantissa = val & 0xF;

	float mantissa_value = mantissa / 16.f; // Normalizing the mantissa
	float exponent_value = exponent - 4;	// Adjusting with bias
	float result = mantissa_value * std::pow(2, exponent_value);
	return sign ? -result : result;
}

uint8_t Float::Encode(float val) {
	bool sign;	  // 1 bit
	int exponent; // 3 bits
	int mantissa; // 4 bits

	if (val < 0) {
		sign = 1;
		val = -val;
	}

	exponent = 0;
	while (val >= 1.0f) {
		val /= 2.0f;
		exponent++;
	}
	while (val < 0.5f) {
		val *= 2.0f;
		exponent--;
	}

	exponent = exponent + 4; // Apply bias
	mantissa = static_cast<int>(val * 16) & 0xF;

	return (sign << 7) | (exponent << 4) | (mantissa);
}
