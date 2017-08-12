#include <iostream>
#include <sstream>
#include <string>

#include "core.h"
#include "console_printer.h"
#include "console_inputter.h"

void help(void);
bool prompt(void);
void preInstructionCallback(core::MachineState & state);
void postInstructionCallback(core::MachineState & state);

utils::ConsolePrinter printer;
utils::ConsoleInputter inputter;
core::lc3 interface(printer, inputter);

uint32_t total_instructions = 0;
uint32_t target_instructions = 0;
bool limited_run = false;

int main(int argc, char * argv[])
{
    help();

    interface.registerPreInstructionCallback(preInstructionCallback);
    interface.registerPostInstructionCallback(postInstructionCallback);
    interface.initializeSimulator();
    for(int i = 1; i < argc; i += 1) {
        try {
            interface.loadSimulatorWithFile(std::string(argv[i]));
        } catch (utils::exception const & e) {}
    }

    while(prompt()) {}

    return 0;
}

void help(void)
{
    std::cout << "help               - display this information\n"
              << "list               - display the next instruction to be executed\n"
              << "load filename      - loads an object file\n"
              << "mem start end      - display values in memory addresses start to end\n"
              << "quit               - exit the simulator\n"
              << "regs               - display register values\n"
              << "run [instructions] - runs to end of program or, if specified, the number of instructions\n"
              ;
}

void displayMem(core::MachineState const & state, uint32_t addr)
{
    std::cout << utils::ssprintf("0x%0.4X: 0x%0.4X %s", addr, state.mem[addr].getValue(),
        state.mem[addr].getLine().c_str());
}

bool prompt(void)
{
    std::cout << "> ";
    std::string command_line;
    std::getline(std::cin, command_line);
    std::stringstream command_tokens(command_line);
    std::string command;
    command_tokens >> command;

    core::MachineState & state = interface.getMachineState();

    if(command == "help") {
        help();
    } else if(command == "list") {
        for(int32_t pos = -2; pos <= 2; pos += 1) {
            if(state.pc + pos > 0) {
                if(pos == 0) {
                    std::cout << "--> ";
                } else {
                    std::cout << "    ";
                }
                displayMem(state, state.pc + pos);
                std::cout << "\n";
            }
        }
    } else if(command == "mem") {
        std::string start_s, end_s;
        command_tokens >> start_s >> end_s;
        if(command_tokens.fail()) {
            std::cout << "must supply start and end addresses\n";
            return true;
        }

        uint32_t start, end;
        try {
            start = std::stoi(start_s, 0, 0);
            end = std::stoi(end_s, 0, 0);
        } catch(std::exception const & e) {
            std::cout << "invalid arguments\n";
            return true;
        }

        for(uint32_t addr = start; addr <= end; addr += 1) {
            if(addr < 0xffff) {
                displayMem(state, addr);
                std::cout << "\n";
            }
        }
    } else if(command == "load") {
        std::string filename;
        command_tokens >> filename;
        if(command_tokens.fail()) {
            std::cout << "must supply filename argument to load\n";
            return true;
        }

        try {
            interface.loadSimulatorWithFile(filename);
        } catch(utils::exception const & e) {}
    } else if(command == "quit") {
        return false;
    } else if(command == "regs") {
        for(uint32_t i = 0; i < 2; i += 1) {
            for(uint32_t j = 0; j < 4; j += 1) {
                uint32_t reg = i * 4 + j;
                std::cout << utils::ssprintf("R%u: 0x%0.4X (%5d)", reg, state.regs[reg], state.regs[reg]);
                if(j != 3) {
                    std::cout << "    ";
                }
            }
            std::cout << "\n";
        }
    } else if(command == "run") {
        uint32_t num_instructions;
        command_tokens >> num_instructions;
        if(command_tokens.fail()) {
            limited_run = false;
        } else {
            target_instructions = total_instructions + num_instructions;
            limited_run = true;
        }

        try {
            interface.simulate();
        } catch(utils::exception const & e) {}
    } else {
        std::cout << "unknown command\n";
    }

    return true;
}

void preInstructionCallback(core::MachineState & state)
{
}

void postInstructionCallback(core::MachineState & state)
{
    total_instructions += 1;
    if(limited_run && total_instructions == target_instructions) {
        state.running = false;
    }
}
