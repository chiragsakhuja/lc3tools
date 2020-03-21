/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include <algorithm>
#ifdef _ENABLE_DEBUG
    #include <chrono>
#endif
#include <iostream>
#include <vector>
#include <sstream>
#include <string>

#include "common.h"
#include "console_printer.h"
#include "console_inputter.h"
#include "interface.h"

std::string previous_command = "";

struct Breakpoint
{
    uint16_t loc;
    uint64_t id;
    lc3::sim * sim_inst;

    Breakpoint(uint16_t loc, uint64_t id, lc3::sim * sim_inst) : loc(loc), id(id), sim_inst(sim_inst) { }
};

std::vector<Breakpoint> breakpoints;
uint64_t cur_breakpoint_id = 0;

void help(void);
bool prompt(lc3::sim & simulator);
bool promptMain(lc3::sim & simulator, std::stringstream & command_tokens);
void promptBreak(lc3::sim & simulator, std::stringstream & command_tokens);
void list(lc3::sim const & simulator, int32_t context);
std::string formatMem(lc3::sim const & simulator, uint32_t addr);
std::ostream & operator<<(std::ostream & out, Breakpoint const & x);
void breakpointCallback(lc3::core::CallbackType type, lc3::core::MachineState & state);

struct CLIArgs
{
    uint32_t print_level = DEFAULT_PRINT_LEVEL;
    bool ignore_privilege = false;
};

int main(int argc, char * argv[])
{
    CLIArgs args;
    std::vector<std::pair<std::string, std::string>> parsed_args = parseCLIArgs(argc, argv);
    for(auto const & arg : parsed_args) {
        if(std::get<0>(arg) == "print-level") {
            args.print_level = std::stoi(std::get<1>(arg));
        } else if(std::get<0>(arg) == "ignore-privilege") {
            args.ignore_privilege = true;
        } else if(std::get<0>(arg) == "h" || std::get<0>(arg) == "help") {
            std::cout << "usage: " << argv[0] << " [OPTIONS]\n";
            std::cout << "\n";
            std::cout << "  -h,--help              Print this message\n";
            std::cout << "  --print-level=N        Output verbosity [0-9]\n";
            std::cout << "  --ignore-privilege     Ignore access violations\n";
            return 0;
        }
    }

    help();

    lc3::ConsolePrinter printer;
    lc3::ConsoleInputter inputter;
    lc3::sim simulator(printer, inputter, args.print_level);

    simulator.registerCallback(lc3::core::CallbackType::BREAKPOINT, breakpointCallback);
    if(args.ignore_privilege) {
        simulator.setIgnorePrivilege(true);
    }

    for(int i = 1; i < argc; i += 1) {
        std::string arg(argv[i]);
        if(arg[0] != '-') {
            simulator.loadObjFile(std::string(argv[i]));
        }
    }

    while(prompt(simulator)) {}

    return 0;
}

void help(void)
{
    std::cout << "break <action> [args...] - performs action (see break help for details)\n"
              << "help                     - display this message\n"
              << "list [N]                 - display the next instruction to be executed with N rows of context\n"
              << "load <filename>          - loads an object file\n"
              << "mem <start> [<end>]      - display values in memory addresses start to end\n"
#ifdef _ENABLE_DEBUG
              << "printlevel N             - sets the print level to N\n"
#endif
              << "quit                     - exit the simulator\n"
              << "randomize                - randomize the memory and general purpose registers\n"
              << "regs                     - display register values\n"
              << "restart                  - restart program (and go to user mode)\n"
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

bool prompt(lc3::sim & simulator)
{
    std::cout << "Executed " << simulator.getInstExecCount() << " instructions\n";
    std::cout << "> ";
    std::string command_line;
    std::getline(std::cin, command_line);
    if(command_line == "") {
        command_line = previous_command;
    }

    std::stringstream command_tokens(command_line);
    bool result = promptMain(simulator, command_tokens);

    previous_command = command_line;

    return result;
}

bool promptMain(lc3::sim & simulator, std::stringstream & command_tokens)
{
    std::string command;
    command_tokens >> command;

    if(command == "break") {
        promptBreak(simulator, command_tokens);
    } else if(command == "help") {
        help();
    } else if(command == "list") {
        int32_t context;
        command_tokens >> context;
        if(command_tokens.fail()) {
            context = 2;
        }
        list(simulator, context);
    } else if(command == "load") {
        std::string filename;
        command_tokens >> filename;
        if(command_tokens.fail()) {
            std::cout << "must supply filename argument to load\n";
            return true;
        }

        simulator.loadObjFile(filename);
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
            (void) e;
            std::cout << "invalid address\n";
            return true;
        }

        for(uint32_t addr = start; addr <= end; addr += 1) {
            if(addr < 0xffff) {
                std::cout << formatMem(simulator, addr) << "\n";
            }
        }
#ifdef _ENABLE_DEBUG
    } else if(command == "printlevel") {
        uint32_t print_level;
        command_tokens >> print_level;
        if(command_tokens.fail()) {
            std::cout << "must supply print level\n";
            return true;
        }
        simulator.setPrintLevel(print_level);
#endif
    } else if(command == "quit") {
        return false;
    } else if(command == "randomize") {
        simulator.randomizeState();
    } else if(command == "restart") {
        simulator.setup();
    } else if(command == "regs") {
        for(uint32_t i = 0; i < 2; i += 1) {
            for(uint32_t j = 0; j < 4; j += 1) {
                uint32_t reg = i * 4 + j;
                uint32_t value = simulator.readReg(reg);
                std::cout << lc3::utils::ssprintf("R%u: 0x%0.4X (%5d)", reg, value, value);
                if(j != 3) {
                    std::cout << "    ";
                }
            }
            std::cout << "\n";
        }
        std::cout << lc3::utils::ssprintf("PC: 0x%0.4X\n", simulator.readPC());
        std::cout << lc3::utils::ssprintf("PSR: 0x%0.4X\n", simulator.readPSR());
        std::cout << lc3::utils::ssprintf("CC: %c\n", simulator.readCC());
        std::cout << lc3::utils::ssprintf("MCR: 0x%0.4X\n", simulator.readMCR());
    } else if(command == "run") {
        uint32_t inst_limit;
        command_tokens >> inst_limit;
        if(! command_tokens.fail()) {
            simulator.setRunInstLimit(inst_limit);
        } else {
            simulator.setRunInstLimit(0);
        }
        simulator.run();
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
            (void) e;
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
            simulator.writeReg(id, val);
        } else if(loc_s == "pc") {
            simulator.writePC(val);
        } else if(loc_s == "psr") {
            simulator.writePSR(val);
        } else if(loc_s == "mcr") {
            simulator.writeMCR(val);
        } else {
            uint32_t addr;
            try {
                addr = std::stoi(loc_s, 0, 0);
            } catch(std::exception const & e) {
                (void) e;
                std::cout << "invalid address\n";
                return true;
            }
            simulator.writeMem(addr, val);
        }
    } else if(command == "step") {
        std::string sub_command;
        command_tokens >> sub_command;
        if(command_tokens.fail()) {
            std::cout << "must specify type of step\n";
            return true;
        }

        if(sub_command == "in") {
            simulator.stepIn();
            list(simulator, 2);
        } else if(sub_command == "out") {
            simulator.stepOut();
            list(simulator, 2);
        } else if(sub_command == "over") {
            simulator.stepOver();
            list(simulator, 2);
        } else {
            std::cout << "invalid step operation\n";
        }
        return true;
    } else {
        std::cout << "unknown command\n";
    }

    return true;
}

void promptBreak(lc3::sim & simulator, std::stringstream & command_tokens)
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

        auto bp_search = std::find_if(breakpoints.begin(), breakpoints.end(), [id](Breakpoint const & x) {
            return x.id == id;
        });
        if(bp_search != breakpoints.end()) {
            simulator.removeBreakpoint(bp_search->loc);
            breakpoints.erase(bp_search);
        } else {
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
            (void) e;
            std::cout << "invalid value\n";
            return;
        }

        simulator.setBreakpoint(loc);
        Breakpoint bp{static_cast<uint16_t>(loc), cur_breakpoint_id, &simulator};
        breakpoints.push_back(bp);
        ++cur_breakpoint_id;
        std::cout << bp << "\n";

    } else  {
        std::cout << "unknown command\n";
    }
}

void list(lc3::sim const & simulator, int32_t context)
{
    uint32_t pc = simulator.readPC();
    for(int32_t pos = -context; pos <= context; pos += 1) {
        if(((int32_t) pc) + pos >= 0 && ((int32_t) pc) + pos <= 0xffff) {
            if(pos == 0) {
                std::cout << "--> ";
            } else {
                std::cout << "    ";
            }
            std::cout << formatMem(simulator, pc + pos) << "\n";
        }
    }
}

std::string formatMem(lc3::sim const & simulator, uint32_t addr)
{
    std::stringstream out;
    uint32_t value = simulator.readMem(addr);
    std::string line = simulator.getMemLine(addr);
    out << lc3::utils::ssprintf("0x%0.4X: 0x%0.4X %s", addr, value, line.c_str());
    return out.str();
}

std::ostream & operator<<(std::ostream & out, Breakpoint const & x)
{
    out << "#" << x.id << ": " << formatMem(*(x.sim_inst), x.loc);
    return out;
}

void breakpointCallback(lc3::core::CallbackType type, lc3::core::MachineState & state)
{
    (void) type;

    uint16_t pc = state.readPC();
    auto bp_search = std::find_if(breakpoints.begin(), breakpoints.end(), [pc](Breakpoint const & x) {
        return x.loc == pc;
    });
    if(bp_search != breakpoints.end()) {
        // Should always be the case.
        std::cout << "hit a breakpoint\n" << *bp_search << "\n";
    }
}
