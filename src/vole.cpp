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
	for (uint8_t i = addr; !stream.eof(); i += 2) {
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
	ControlUnit cu(this);
	Instruction *inst = cu.decodeInstruction();
	inst->execute();
}

Memory::Memory() : m_Array() {}

uint8_t &Memory::operator[](uint8_t idx) { return m_Array[idx]; }

uint8_t Memory::operator[](uint8_t idx) const { return m_Array[idx]; }

Registers::Registers() : m_Array(), pc(0) {}

uint8_t &Registers::operator[](uint8_t i) { return m_Array[i]; }

uint8_t Registers::operator[](uint8_t i) const { return m_Array[i]; }

ControlUnit::ControlUnit(Machine *mac) : m_Machine(mac) {}

Instruction *ControlUnit::decodeInstruction() {
	uint8_t opcode = m_Machine->mem[m_Machine->reg.pc] >> 4;
	if (opcode < 1 || opcode > 12) {
		m_Machine->log << "Op-code not in range [1-12]: " << opcode
					   << ". Halting.\n";
		return new Halt(m_Machine);
	}
	auto newInstruction = instructionFactory[opcode - 1];
	return newInstruction(m_Machine);
}

Instruction::Instruction(Machine *mac) : m_Machine(mac) {}

void Load1::execute() {}

void Load2::execute() {}

void Store::execute() {}

void Move::execute() {
	uint8_t rs = m_Machine->mem[1 + m_Machine->reg.pc];
	uint8_t r = rs >> 4;
	uint8_t s = rs & 0x0F;
	m_Machine->reg[s] = m_Machine->reg[r];
	m_Machine->reg.pc += 2;
}

void Add1::execute() {}

void Add2::execute() {}

void Or::execute() {}

void And::execute() {}

void Xor::execute() {}

void Rotate::execute() {}

void Jump::execute() {}

void Halt::execute() { m_Machine->shouldHalt = true; }

Load1::Load1(Machine *mac) : Instruction(mac) {}

Load2::Load2(Machine *mac) : Instruction(mac) {}

Store::Store(Machine *mac) : Instruction(mac) {}

Move::Move(Machine *mac) : Instruction(mac) {}

Add1::Add1(Machine *mac) : Instruction(mac) {}

Add2::Add2(Machine *mac) : Instruction(mac) {}

Or::Or(Machine *mac) : Instruction(mac) {}

And::And(Machine *mac) : Instruction(mac) {}

Xor::Xor(Machine *mac) : Instruction(mac) {}

Rotate::Rotate(Machine *mac) : Instruction(mac) {}

Jump::Jump(Machine *mac) : Instruction(mac) {}

Halt::Halt(Machine *mac) : Instruction(mac) {}
