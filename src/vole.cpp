#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "error.h"
#include "vole.h"

using namespace vole;

#define OS_HEX1 std::hex << std::uppercase
#define OS_HEX2 std::hex << std::uppercase << std::setfill('0') << std::setw(2)

Machine::Machine(Screen *screen, const std::array<ControlUnitBuilder, 16> cuFactory)
	: controlUnitFactory(cuFactory), scr(screen) {}

error::LoadProgramError Machine::LoadProgram(const std::string &path, uint8_t addr) {
	std::ifstream ifs(path);
	if (!ifs.is_open()) {
		return error::LoadProgramError::FILE_OPEN_FAILED;
	}
	return LoadProgram(ifs, addr);
}

error::LoadProgramError Machine::LoadProgram(std::istream &stream, uint8_t addr) {
	uint16_t inst;
	for (size_t i = addr; !stream.eof(); i += 2) {
		if (i >= 2 * 128) {
			return error::LoadProgramError::TOO_MUCH_INSTRUCTIONS;
		}
		stream >> std::hex >> inst;
		if (stream.fail()) {
			return error::LoadProgramError::STREAM_READ_FAILED;
		}
		mem[i] = inst >> 8;
		mem[i + 1] = inst & 0x00FF;
	}
	return error::LoadProgramError::NOT_AN_ERROR;
}

void Machine::Reset() {
	reg.Reset(); // CPU
	mem.Reset(); // RAM
}

void Machine::Run() {
	do {
		// Doin' what? nothing...
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
	uint8_t val = mac->reg[r];
	mac->mem[xy] = val;
	if (xy == 0x00) {
		if (val != 0) {
			mac->scr->write(val);
		} else {
			mac->scr->clear();
		}
	}
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

ShouldHalt Unused::Execute() { return ShouldHalt::YES; }

std::string Unused::Humanize() {
	std::ostringstream os;
	os << "Unspecified op-code " << OS_HEX1 << opcode;
	return os.str();
}

Screen::~Screen() = default;
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

float Float::Decode(uint8_t byte) {
	int sig = (byte & (1 << 7)) ? -1 : 1;
	int exp = ((byte >> 4) & 0x7) - 4;
	int man = (byte & 0xF) | 0x10;
	return (std::exp2f(exp) * ((man & 16) != 0) + std::exp2f(exp - 1) * ((man & 8) != 0) +
			std::exp2f(exp - 2) * ((man & 4) != 0) + std::exp2f(exp - 3) * ((man & 2) != 0) +
			std::exp2f(exp - 4) * ((man & 1) != 0)) *
		   sig;
}

uint8_t Float::Encode(float number) {
	uint8_t sig = (number < 0) ? 1 : 0;
	number = std::fabs(number);

	int exponent = 0;
	while (number >= 2.0f) {
		number /= 2.0f;
		exponent++;
	}
	while (number < 1.0f && number > 0.0f) {
		number *= 2.0f;
		exponent--;
	}

	uint8_t exp = exponent + 4;
	uint8_t man = static_cast<uint8_t>((number * 16) - 16);

	return (sig << 7) | (exp << 4) | man;
}
