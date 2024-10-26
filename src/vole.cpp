#include <algorithm>
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
	return loadProgram(ifs);
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

Registers::Registers() : m_Array(), pc(0) {}

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
		mac->log << "Op-code not in range [1-12]: " << opcode << ". Halting.\n";
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

void Add2::execute() {}

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

void Rotate::execute() {}

void Jump::execute()
{
	/*CHECK FOR ERRORS*/
	uint8_t r = operand1;
	uint16_t xy = operandXY;
	if (mac->reg[r] == mac->reg[0])
		mac->reg.pc = xy;
	// maybe check xy if it is even or odd, to prevent pc from using half an
	// instruction?
}

void Halt::execute()
{
	mac->shouldHalt = true;
}
