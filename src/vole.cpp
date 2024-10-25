#include <fstream>
#include <iostream>

#include "vole.h"

using namespace vole;

Machine::Machine(std::ostream &logStream) : log(logStream) {}

bool Machine::loadProgram(const std::string &path, uint8_t addr) {
	std::ifstream ifs(path);
	if (ifs.fail()) {
		return false;
	}
	return loadProgram(ifs);
}

bool Machine::loadProgram(std::istream &stream, uint8_t addr) {
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

void Machine::run() {
	reg.pc = 0;
	while (!shouldHalt)
		step();
}

void Machine::step() {
	ControlUnit *inst = ControlUnit::decode(this);
	inst->execute();
}

Memory::Memory() : m_Array() {}

uint8_t &Memory::operator[](uint8_t idx) { return m_Array[idx]; }

uint8_t Memory::operator[](uint8_t idx) const { return m_Array[idx]; }

Registers::Registers() : m_Array(), pc(0) {}

uint8_t &Registers::operator[](uint8_t i) { return m_Array[i]; }

uint8_t Registers::operator[](uint8_t i) const { return m_Array[i]; }

ControlUnit::ControlUnit(Machine *machine) : mac(machine) {
	inst = mac->mem[mac->reg.pc];
	inst = (inst << 8) | mac->mem[mac->reg.pc + 1];
	opcode = inst >> 12;
	operand1 = (inst >> 8) & 0x0F;
	operand2 = (inst >> 4) & 0x00F;
	operand3 = inst & 0x000F;
	mac->reg.pc += 2;
}

ControlUnit* ControlUnit::decode(Machine *mac) {
	uint8_t opcode = mac->mem[mac->reg.pc] >> 4;
	if (opcode < 1 || opcode > 12) {
		mac->log << "Op-code not in range [1-12]: " << opcode
					   << ". Halting.\n";
		return new Halt(mac);
	}
	auto newControlUnit = controlUnitFactory[opcode - 1];
	return newControlUnit(mac);
}

void Load1::execute() {}

void Load2::execute() {}

void Store::execute() {}

void Move::execute() {
	uint8_t r = operand2;
	uint8_t s = operand3;
	mac->reg[s] = mac->reg[r];
}

void Add1::execute() {}

void Add2::execute() {}

void Or::execute() {}

void And::execute() {}

void Xor::execute() {}

void Rotate::execute() {}

void Jump::execute() {}

void Halt::execute() { mac->shouldHalt = true; }

Load1::Load1(Machine *mac) : ControlUnit(mac) {}

Load2::Load2(Machine *mac) : ControlUnit(mac) {}

Store::Store(Machine *mac) : ControlUnit(mac) {}

Move::Move(Machine *mac) : ControlUnit(mac) {}

Add1::Add1(Machine *mac) : ControlUnit(mac) {}

Add2::Add2(Machine *mac) : ControlUnit(mac) {}

Or::Or(Machine *mac) : ControlUnit(mac) {}

And::And(Machine *mac) : ControlUnit(mac) {}

Xor::Xor(Machine *mac) : ControlUnit(mac) {}

Rotate::Rotate(Machine *mac) : ControlUnit(mac) {}

Jump::Jump(Machine *mac) : ControlUnit(mac) {}

Halt::Halt(Machine *mac) : ControlUnit(mac) {}
