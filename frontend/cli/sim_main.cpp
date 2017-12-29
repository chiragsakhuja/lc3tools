#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>
#include <string>

#include "helper.h"
#include "console_printer.h"
#include "console_inputter.h"

std::string previous_command = "";

void help(void);
bool prompt(void);
bool promptMain(std::stringstream & command_tokens);
void promptBreak(std::stringstream & command_tokens);
std::string formatMem(uint32_t addr);
std::ostream & operator<<(std::ostream & out, Breakpoint const & x);
void breakpointHitCallback(core::MachineState & state, Breakpoint const & bp);

int main(int argc, char * argv[])
{
    help();

    try {
        utils::ConsolePrinter printer;
        utils::ConsoleInputter inputter;
        simInit(printer, inputter);

        simRegisterBreakpointHitCallback(breakpointHitCallback);

        for(int i = 1; i < argc; i += 1) {
            simLoadSimulatorWithFile(std::string(argv[i]));
        }

        while(prompt()) {}

        simShutdown();
    } catch(utils::exception) { return 1; }

    return 0;
}

void help(void)
{
    std::cout << "break <action> [args...] - performs action (see break help for details)\n"
              << "help                     - display this message\n"
              << "list                     - display the next instruction to be executed\n"
              << "load <filename>          - loads an object file\n"
              << "mem <start> [<end>]      - display values in memory addresses start to end\n"
              << "quit                     - exit the simulator\n"
              << "randomize                - randomize the memory and general purpose registers\n"
              << "regs                     - display register values\n"
              << "run [<instructions>]     - runs to end of program or, if specified, the number of instructions\n"
              << "set <loc> <value>        - sets loc (either register name or memory address) to value\n"
              << "step in                  - executes a single instruction\n"
              << "step over                - executes a single instruction (treats subroutine calls as a single\n"
              << "                           instruction)\n"
              << "step out                 - steps out of a subroutine if in one\n"
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

bool prompt(void)
{
    std::cout << "> ";
    std::string command_line;
    std::getline(std::cin, command_line);
    if(command_line == "") {
        command_line = previous_command;
    }

    std::stringstream command_tokens(command_line);
    bool result = promptMain(command_tokens);

    previous_command = command_line;

    return result;
}

bool promptMain(std::stringstream & command_tokens)
{
    std::string command;
    command_tokens >> command;

    if(command == "break") {
        promptBreak(command_tokens);
    } else if(command == "help") {
        help();
    } else if(command == "list") {
        uint32_t pc = simGetPC();
        for(int32_t pos = -2; pos <= 2; pos += 1) {
            if(((int32_t) pc) + pos > 0) {
                if(pos == 0) {
                    std::cout << "--> ";
                } else {
                    std::cout << "    ";
                }
                std::cout << formatMem(pc + pos) << "\n";
            }
        }
    } else if(command == "mem") {
        std::string start_s, end_s;
        command_tokens >> start_s;
        if(command_tokens.fail()) {
            std::cout << "must supply start address\n";
            return true;
        }
        command_tokens >> end_s;
        if(command_tokens.fail()) {
            end_s = start_s;
        }

        uint32_t start, end;
        try {
            start = std::stoi(start_s, 0, 0);
            end = std::stoi(end_s, 0, 0);
        } catch(std::exception const & e) {
            std::cout << "invalid address\n";
            return true;
        }

        for(uint32_t addr = start; addr <= end; addr += 1) {
            if(addr < 0xffff) {
                std::cout << formatMem(addr) << "\n";
            }
        }
    } else if(command == "load") {
        std::string filename;
        command_tokens >> filename;
        if(command_tokens.fail()) {
            std::cout << "must supply filename argument to load\n";
            return true;
        }

        simLoadSimulatorWithFile(filename);
    } else if(command == "quit") {
        return false;
    } else if(command == "randomize") {
        simRandomizeMachine();
    } else if(command == "regs") {
        for(uint32_t i = 0; i < 2; i += 1) {
            for(uint32_t j = 0; j < 4; j += 1) {
                uint32_t reg = i * 4 + j;
                uint32_t value = simGetReg(reg);
                std::cout << utils::ssprintf("R%u: 0x%0.4X (%5d)", reg, value, value);
                if(j != 3) {
                    std::cout << "    ";
                }
            }
            std::cout << "\n";
        }
        std::cout << utils::ssprintf("PC: 0x%0.4X\n", simGetPC());
        std::cout << utils::ssprintf("PSR: 0x%0.4X\n", simGetPSR());
        std::cout << utils::ssprintf("CC: %c\n", simGetCC());
    } else if(command == "run") {
        uint32_t inst_count;
        command_tokens >> inst_count;
#ifdef _ENABLE_DEBUG
        auto start_count = inst_exec_count;
        auto start_time = std::chrono::steady_clock::now();
#endif
        if(command_tokens.fail()) {
            simRun();
        } else {
            simRunFor(inst_count);
        }
#ifdef _ENABLE_DEBUG
        auto end_time = std::chrono::steady_clock::now();
        auto end_count = inst_exec_count;
        std::cout << "executed " << (end_count - start_count) << " instructions in "
                  << std::chrono::duration<double, std::milli>(end_time - start_time).count() << " ms\n";
#endif
    } else if(command == "set") {
        std::string loc_s, val_s;
        command_tokens >> loc_s >> val_s;
        if(command_tokens.fail()) {
            std::cout << "must supply location and value\n";
            return true;
        }

        uint32_t val;
        try {
            val = std::stoi(val_s, 0, 0);
        } catch(std::exception const & e) {
            std::cout << "invalid value\n";
            return true;
        }

        std::transform(loc_s.begin(), loc_s.end(), loc_s.begin(), ::tolower);
        if(loc_s[0] == 'r') {
            if(loc_s.size() != 2 || loc_s[1] > '7') {
                std::cout << "invalid register\n";
                return true;
            }
            uint32_t id = loc_s[1] - '0';
            simSetReg(id, val);
        } else if(loc_s == "pc") {
            simSetPC(val);
        } else if(loc_s == "psr") {
            simSetPSR(val);
        } else {
            uint32_t addr;
            try {
                addr = std::stoi(loc_s, 0, 0);
            } catch(std::exception const & e) {
                std::cout << "invalid address\n";
                return true;
            }
            simSetMemVal(addr, val);
        }
    } else if(command == "step") {
        std::string sub_command;
        command_tokens >> sub_command;
        if(command_tokens.fail()) {
            std::cout << "must specify type of step\n";
            return true;
        }

        if(sub_command == "in") {
            simRunFor(1);
        } else if(sub_command == "out") {
            simStepOut();
        } else if(sub_command == "over") {
            simStepOver();
        } else {
            std::cout << "invalid step operation\n";
        }
        return true;
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
            std::cout << "must supply id\n";
            return;
        }

        bool removed = simRemoveBreakpoint(id);
        if(! removed) {
            std::cout << "invalid id\n";
            return;
        }
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
            std::cout << "invalid value\n";
            return;
        }

        Breakpoint bp = simSetBreakpoint(loc);
        std::cout << bp << "\n";

    } else  {
        std::cout << "unknown command\n";
    }
}

std::string formatMem(uint32_t addr)
{
    std::stringstream out;
    uint32_t value = simGetMemVal(addr);
    std::string line = simGetMemLine(addr);
    out << utils::ssprintf("0x%0.4X: 0x%0.4X %s", addr, value, line.c_str());
    return out.str();
}

std::ostream & operator<<(std::ostream & out, Breakpoint const & x)
{
    out << "#" << x.id << ": " << formatMem(x.loc);
    return out;
}

void breakpointHitCallback(core::MachineState & state, Breakpoint const & bp)
{
    (void) state;
    std::cout << "hit a breakpoint\n" << bp << "\n";
}
