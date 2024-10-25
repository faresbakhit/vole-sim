#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>

#include "vole.h"

enum class base { dec, hex };

int inputInteger(base b = base::dec) {
	int i;
	if (b == base::hex) {
		std::cin >> std::hex >> i >> std::dec;
	} else {
		std::cin >> i;
	}
	if (std::cin.eof()) {
		throw std::runtime_error("End of input.");
	} else if (std::cin.fail()) {
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		std::cerr << "> error: input not an integer.\n";
		throw std::logic_error("");
	} else {
		std::cin.ignore();
		return i;
	}
}

int inputRange(int min, int max, base b = base::dec) {
	int i;
	i = inputInteger(b);
	if (i >= min && i <= max) {
		return i;
	}
	std::cerr << "> error: option " << i << " not in range [" << min << "-" << max << "].\n";
	throw std::logic_error("");
}

void inputString(std::string &str) {
	std::cin >> str;
	if (std::cin.eof())
		throw std::runtime_error("End of input.");
}

int main() try {
	std::cout << ">> Welcome to the Vole Machine Simulator & GUI\n"
			  << ">> \n"
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

	Vole::Machine mac;

	do {
		std::string s;

		std::cerr << "> ";
		inputString(s);

		try {
			if (s == "load") {
				inputString(s);
				std::ifstream fileStream(s);
				mac.loadProgram(fileStream);
			} else if (s == "run") {
				mac.run();
			} else if (s == "step") {
				mac.step();
			} else if (s == "reg") {
				inputString(s);
				if (s == "show") {
					for (int i = 0; i < 16; i += 1) {
						if (i != 0 && i % 4 == 0) {
							std::cout << "\n";
						}
						std::cout << "R" << std::dec << i << (i < 10 ? ":  " : ": ") << std::hex << std::uppercase
								  << std::setfill('0') << std::setw(2) << (int)mac.reg[i];
						if (i == 15)
							std::cout << "\n";
						else
							std::cout << ", ";
					}
				} else if (s == "get") {
					int reg = inputRange(0, 15);
					std::cout << "R" << std::dec << reg << (reg < 10 ? ":  " : ": ") << std::hex << std::uppercase
							  << std::setfill('0') << std::setw(2) << (int)mac.reg[reg] << "\n";
				} else if (s == "set") {
					int reg = inputRange(0, 15);
					int val = inputRange(0, 255, base::hex);
					mac.reg[reg] = val;
				} else {
					std::cerr << ">> Unknown command.\n";
				}
			} else if (s == "mem") {
				inputString(s);
				if (s == "show") {
					std::cerr << ">> TODO\n";
				} else if (s == "get") {
					std::cerr << ">> TODO\n";
				} else if (s == "set") {
					std::cerr << ">> TODO\n";
				} else {
					std::cerr << ">> Unknown command.\n";
				}
			} else if (s == "exit") {
				break;
			} else {
				std::cerr << ">> Unknown command.\n";
			}
		} catch (const std::logic_error &) {
			continue;
		}
	} while (true);

	std::cerr << ">> Hooray!\n";
} catch (const std::runtime_error &err) {
	// Handle EOF at input loop.
	std::cerr << err.what() << "\n";
	return 1;
}
