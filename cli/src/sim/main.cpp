#include <iostream>
#include <sstream>
#include <string>

#include "core.h"
#include "console_printer.h"
#include "console_inputter.h"

std::string getMemDisplayString(uint32_t addr);
void help(void);
bool prompt(void);
bool promptMain(std::stringstream & command_tokens);
void promptBreak(std::stringstream & command_tokens);
void preInstructionCallback(core::MachineState & state);
void postInstructionCallback(core::MachineState & state);
void interruptEnterCallback(core::MachineState & state);

utils::ConsolePrinter printer;
utils::ConsoleInputter inputter;
core::lc3 interface(printer, inputter);

uint32_t total_instructions = 0;
uint32_t target_instructions = 0;
bool limited_run = false;

struct Breakpoint
{
    uint32_t id, loc;

    friend std::ostream & operator<<(std::ostream &, Breakpoint &);
};

std::ostream & operator<<(std::ostream & out, Breakpoint & x)
{
    out << "#" << x.id << ": " << getMemDisplayString(x.loc);
    return out;
}

uint32_t breakpoint_id = 0;
std::vector<Breakpoint> breakpoints;

int main(int argc, char * argv[])
{
    help();

    interface.registerPreInstructionCallback(preInstructionCallback);
    interface.registerPostInstructionCallback(postInstructionCallback);
    interface.registerInterruptEnterCallback(interruptEnterCallback);
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
    std::cout << "break <action> [args...] - performs action (set, clear, or help)\n"
              << "help                     - display this message\n"
              << "list                     - display the next instruction to be executed\n"
              << "load filename            - loads an object file\n"
              << "mem <start> <end>        - display values in memory addresses start to end\n"
              << "quit                     - exit the simulator\n"
              << "regs                     - display register values\n"
              << "run [instructions]       - runs to end of program or, if specified, the number of instructions\n"
              << "set <loc> <value>        - sets loc (either register name or memoery address) to value\n"
              ;
}

void breakHelp(void)
{
    std::cout << "break clear <id>  - clears the given breakpoint\n"
              << "break help        - display this message\n"
              << "break list        - display the active breakpoints\n"
              << "break set <loc>   - sets a breakpoint at the given location\n"
              ;
}

std::string getMemDisplayString(uint32_t addr)
{
    core::MachineState & state = interface.getMachineState();

    std::stringstream out;
    out << utils::ssprintf("0x%0.4X: 0x%0.4X %s", addr, state.mem[addr].getValue(),
        state.mem[addr].getLine().c_str());
    return out.str();
}

bool prompt(void)
{
    std::cout << "> ";
    std::string command_line;
    std::getline(std::cin, command_line);
    std::stringstream command_tokens(command_line);

    return promptMain(command_tokens);
}

bool promptMain(std::stringstream & command_tokens)
{
    std::string command;
    command_tokens >> command;

    core::MachineState & state = interface.getMachineState();

    if(command == "break") {
        promptBreak(command_tokens);
    } else if(command == "help") {
        help();
    } else if(command == "list") {
        for(int32_t pos = -2; pos <= 2; pos += 1) {
            if(state.pc + pos > 0) {
                if(pos == 0) {
                    std::cout << "--> ";
                } else {
                    std::cout << "    ";
                }
                std::cout << getMemDisplayString(state.pc + pos) << "\n";
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
                std::cout << getMemDisplayString(addr) << "\n";
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

void promptBreak(std::stringstream & command_tokens)
{
    std::string command;
    command_tokens >> command;
    if(command_tokens.fail()) {
        std::cout << "must provide action\n";
        return;
    }

    if(command == "clear") {
        uint32_t id;
        command_tokens >> id;
        if(command_tokens.fail()) {
            std::cout << "must supply\n";
            return;
        }

        if(id >= breakpoints.size()) {
            std::cout << "invalid id\n";
            return;
        }

        breakpoints.erase(breakpoints.begin() + id);
    } else if(command == "help") {
        breakHelp();
    } else if(command == "list") {
        for(auto x : breakpoints) {
            std::cout << x << "\n";
        }
    } else if(command == "set") {
        std::string loc_s;
        command_tokens >> loc_s;
        if(command_tokens.fail()) {
            std::cout << "must supply location\n";
            return;
        }

        uint32_t loc;
        try {
            loc = std::stoi(loc_s, 0, 0);
        } catch(std::exception const & e) {
            std::cout << "invalid arguments\n";
            return;
        }

        if(loc < 0xffff) {
            breakpoints.push_back(Breakpoint{breakpoint_id, loc});
            breakpoint_id += 1;
        }
    }
}

void preInstructionCallback(core::MachineState & state)
{
    for(auto x : breakpoints) {
        if(state.pc == x.loc) {
            std::cout << "hit a breakpoint\n" << x << "\n";
            state.hit_breakpoint = true;
            break;
        }
    }
}

void postInstructionCallback(core::MachineState & state)
{
    total_instructions += 1;
    if(limited_run && total_instructions == target_instructions) {
        state.running = false;
    }
}

void interruptEnterCallback(core::MachineState & state)
{
}
