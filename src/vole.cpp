#include <cmath>
#include <fstream>
#include <iostream>

#include "vole.h"

using namespace vole;

Machine::Machine(std::ostream &logStream) : log(logStream) {}

bool Machine::loadProgram(const std::string &path, uint8_t addr)
{
	std::ifstream ifs(path);
	if (ifs.fail()) {
		return false;
	}
	return loadProgram(ifs, addr);
}

bool Machine::loadProgram(std::istream &stream, uint8_t addr)
{
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

void Machine::run()
{
	shouldHalt = false;
	reg.pc = 0;
	while (!shouldHalt)
		step();
}

void Machine::step()
{
	ControlUnit *cu = ControlUnit::decode(this);
	cu->execute();
	delete cu;
}

Memory::Memory() : m_Array() {}

uint8_t &Memory::operator[](uint8_t idx)
{
	return m_Array[idx];
}

uint8_t Memory::operator[](uint8_t idx) const
{
	return m_Array[idx];
}

Registers::Registers() : pc(0), m_Array() {}

uint8_t &Registers::operator[](uint8_t i)
{
	return m_Array[i];
}

uint8_t Registers::operator[](uint8_t i) const
{
	return m_Array[i];
}

ControlUnit::ControlUnit(Machine *machine) : mac(machine)
{
	inst = mac->mem[mac->reg.pc];
	inst = (inst << 8) | mac->mem[mac->reg.pc + 1];
	opcode = inst >> 12;
	operand1 = (inst >> 8) & 0x0F;
	operand2 = (inst >> 4) & 0x00F;
	operand3 = inst & 0x000F;
	operandXY = inst & 0x00FF;
	mac->reg.pc += 2;
}

ControlUnit *ControlUnit::decode(Machine *mac)
{
	uint8_t opcode = mac->mem[mac->reg.pc] >> 4;
	if (opcode < 1 || opcode > 12) {
		mac->log << "Op-code not in range [1-12]: " << +opcode
				 << ". Halting.\n";
		return new Halt(mac);
	}
	auto newControlUnit = controlUnitFactory[opcode - 1];
	return newControlUnit(mac);
}

void Load1::execute()
{
	uint8_t r = operand1;
	uint16_t xy = operandXY;
	mac->reg[r] = mac->mem[xy];
}

void Load2::execute()
{
	uint8_t r = operand1;
	uint16_t xy = operandXY;
	mac->reg[r] = xy;
}

void Store::execute()
{
	uint8_t r = operand1;
	uint16_t xy = operandXY;
	mac->reg[xy] = mac->mem[r];
}

void Move::execute()
{
	uint8_t r = operand2;
	uint8_t s = operand3;
	mac->reg[s] = mac->reg[r];
}

void Add1::execute()
{
	uint8_t r = operand1;
	uint8_t s = operand2;
	uint8_t t = operand3;
	mac->reg[r] = mac->reg[s] + mac->reg[t];
}

float decodeFloat(uint8_t val);
uint8_t encodeFloat(float val);

void Add2::execute()
{
	uint8_t r = operand1;
	uint8_t s = operand2;
	uint8_t t = operand3;
	mac->reg[r] =
		encodeFloat(decodeFloat(mac->reg[s]) + decodeFloat(mac->reg[t]));
}

void Or::execute()
{
	uint8_t r = operand1;
	uint8_t s = operand2;
	uint8_t t = operand3;
	mac->reg[r] = mac->reg[s] | mac->reg[t];
}

void And::execute()
{
	uint8_t r = operand1;
	uint8_t s = operand2;
	uint8_t t = operand3;
	mac->reg[r] = mac->reg[s] & mac->reg[t];
}

void Xor::execute()
{
	uint8_t r = operand1;
	uint8_t s = operand2;
	uint8_t t = operand3;
	mac->reg[r] = mac->reg[s] ^ mac->reg[t];
}

void Rotate::execute()
{
	uint8_t r = operand1;
	uint8_t t = operand3 % 8;
	mac->reg[r] = ((mac->reg[r] >> t) | (mac->reg[r] << (8 - t)));
}

void Jump::execute()
{

	uint8_t r = operand1;
	uint16_t xy = operandXY;
	if (xy % 2 != 0) // Not a full instruction at `xy`.
		xy--;
	if (mac->reg[r] == mac->reg[0])
		mac->reg.pc = xy;
}

void Halt::execute()
{
	mac->shouldHalt = true;
}

ControlUnit::~ControlUnit() {}
Load1::~Load1() {}
Load2::~Load2() {}
Store::~Store() {}
Move::~Move() {}
Add1::~Add1() {}
Add2::~Add2() {}
Or::~Or() {}
And::~And() {}
Xor::~Xor() {}
Rotate::~Rotate() {}
Jump::~Jump() {}
Halt::~Halt() {}

float decodeFloat(uint8_t val)
{
	bool sign;	  // 1 bit
	int exponent; // 3 bits
	int mantissa; // 4 bits

	sign = (val >> 7) & 0x1;
	exponent = (val >> 4) & 0x7;
	mantissa = val & 0xF;

	float mantissa_value =
		1.0f + (mantissa / 16.0f);		 // Normalizing the mantissa
	float exponent_value = exponent - 4; // Adjusting with bias
	float result = mantissa_value * std::pow(2, exponent_value);
	return sign ? -result : result;
}

uint8_t encodeFloat(float val)
{
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
