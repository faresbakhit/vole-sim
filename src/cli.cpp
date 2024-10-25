#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

#include "vole.h"

enum class base
{
	dec,
	hex
};

template <typename T>
T inNumber(std::istream &in, base b = base::dec,
		   T min = std::numeric_limits<T>::min,
		   T max = std::numeric_limits<T>::max)
{
	T i;
	if (b == base::hex) {
		in >> std::hex >> i;
	} else {
		in >> std::dec >> i;
	}
	if (in.fail()) {
		in.clear();
		in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		std::cerr << "> error: option not a number.\n";
		throw std::logic_error("BAD INPUT");
	}
	in.ignore();
	if (i < min || i > max) {
		std::cerr << "> error: option " << i << " not in range [" << min << "-"
				  << max << "].\n";
		throw std::logic_error("BAD INPUT");
	}
	return i;
}

void regShow(vole::Registers &reg)
{
	for (int i = 0; i < 16; i += 1) {
		if (i != 0 && i % 4 == 0) {
			std::cout << "\n";
		}
		std::cout << "R" << std::dec << i << (i < 10 ? ":  " : ": ") << std::hex
				  << std::uppercase << std::setfill('0') << std::setw(2)
				  << (int)reg[i];
		if (i == 15)
			std::cout << "\n";
		else
			std::cout << ", ";
	}
}

void regGet(std::istream &in, const vole::Registers &reg)
{
	int i = inNumber(in, base::dec, 0, 15);
	std::cout << "R" << std::dec << i << ": " << std::hex << std::uppercase
			  << std::setfill('0') << std::setw(2) << (int)reg[i] << "\n";
}

void regSet(std::istream &in, vole::Registers &reg)
{
	int i = inNumber(in, base::dec, 0, 15);
	int val = inNumber(in, base::hex, 0, 255);
	reg[i] = val;
}

void memShow()
{
	/* TODO */
}

void memGet()
{
	/* TODO */
}

void memSet()
{
	/* TODO */
}

int main()
{
	std::cout
		<< ">> Welcome to the Vole Machine Simulator & GUI\n"
		<< ">>\n"
		<< ">> Commands\n"
		<< ">> ========\n"
		<< ">>\n"
		<< ">> - load FILE: Load program from FILE and put it in memory.\n"
		<< ">> - run: Run indefinitely.\n"
		<< ">> - step: Only execute the next instruction.\n"
		<< ">> - reg show: Show all registers and their values.\n"
		<< ">> - reg get X: Get the value stored at register X.\n"
		<< ">> - reg set X Y: Set register X to the value Y.\n"
		<< ">> - mem show: Show all memory cells and their values.\n"
		<< ">> - mem get X: Get the value stored at memory cell X.\n"
		<< ">> - mem set X Y: Set memory cell X to the value Y.\n"
		<< ">> - exit: Exit the machine simulator.\n";

	vole::Machine mac;

	do {
		std::cerr << "> ";
		std::string cmd, arg;
		std::getline(std::cin, cmd);
		if (std::cin.eof()) {
			std::cerr << "End of input.\n";
			break;
		}
		std::istringstream argstr(cmd);

		try {
			argstr >> arg;
			if (arg == "load") {
				argstr >> arg;
				mac.loadProgram(arg);
			} else if (arg == "run") {
				mac.run();
			} else if (arg == "step") {
				mac.step();
			} else if (arg == "reg") {
				argstr >> arg;
				if (arg == "show") {
					regShow(mac.reg);
				} else if (arg == "get") {
					regGet(argstr, mac.reg);
				} else if (arg == "set") {
					regSet(argstr, mac.reg);
				} else {
					std::cerr << ">> Unknown.\n";
				}
			} else if (arg == "mem") {
				argstr >> arg;
				if (arg == "show") {
					memShow();
				} else if (arg == "get") {
					memGet();
				} else if (arg == "set") {
					memSet();
				} else {
					std::cerr << ">> Unknown.\n";
				}
			} else if (arg == "exit") {
				break;
			} else {
				std::cerr << ">> Unknown.\n";
			}
		} catch (const std::logic_error &) {
			continue;
		}
	} while (1);

	std::cerr << ">> Moriturus te saluto.!\n";
}
