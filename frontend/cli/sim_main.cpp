#include <algorithm>
#ifdef _ENABLE_DEBUG
    #include <chrono>
#endif
#include <iostream>
#include <sstream>
#include <string>

#include "common.h"
#include "console_printer.h"
#include "console_inputter.h"
#include "interface.h"

std::string previous_command = "";

void help(void);
bool prompt(lc3::sim & simulator);
bool promptMain(lc3::sim & simulator, std::stringstream & command_tokens);
void promptBreak(lc3::sim & simulator, std::stringstream & command_tokens);
std::string formatMem(lc3::sim const & simulator, uint32_t addr);
std::ostream & operator<<(std::ostream & out, lc3::Breakpoint const & x);
void breakpointCallback(lc3::core::MachineState & state, lc3::Breakpoint const & bp);

struct CLIArgs
{
    uint32_t print_level = DEFAULT_PRINT_LEVEL;
};

int main(int argc, char * argv[])
{
    CLIArgs args;
    std::vector<std::pair<std::string, std::string>> parsed_args = parseCLIArgs(argc, argv);
    for(auto const & arg : parsed_args) {
        if(std::get<0>(arg) == "print-level") {
            args.print_level = std::stoi(std::get<1>(arg));
        }
    }

    help();

    lc3::ConsolePrinter printer;
    lc3::ConsoleInputter inputter;
    lc3::sim simulator(printer, inputter, args.print_level);

    simulator.registerBreakpointCallback(breakpointCallback);

    for(int i = 1; i < argc; i += 1) {
        std::string arg(argv[i]);
        if(arg[0] != '-') {
            simulator.loadObjectFile(std::string(argv[i]));
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
#ifdef _DEBUG
              << "printlevel N             - sets the print level to N\n"
#endif
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

bool prompt(lc3::sim & simulator)
{
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

        uint32_t pc = simulator.getPC();
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
    } else if(command == "load") {
        std::string filename;
        command_tokens >> filename;
        if(command_tokens.fail()) {
            std::cout << "must supply filename argument to load\n";
            return true;
        }

        simulator.loadObjectFile(filename);
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
                std::cout << formatMem(simulator, addr) << "\n";
            }
        }
#ifdef _DEBUG
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
        simulator.randomize();
    } else if(command == "regs") {
        for(uint32_t i = 0; i < 2; i += 1) {
            for(uint32_t j = 0; j < 4; j += 1) {
                uint32_t reg = i * 4 + j;
                uint32_t value = simulator.getReg(reg);
                std::cout << lc3::utils::ssprintf("R%u: 0x%0.4X (%5d)", reg, value, value);
                if(j != 3) {
                    std::cout << "    ";
                }
            }
            std::cout << "\n";
        }
        std::cout << lc3::utils::ssprintf("PC: 0x%0.4X\n", simulator.getPC());
        std::cout << lc3::utils::ssprintf("PSR: 0x%0.4X\n", simulator.getPSR());
        std::cout << lc3::utils::ssprintf("CC: %c\n", simulator.getCC());
        std::cout << lc3::utils::ssprintf("MCR: 0x%0.4X\n", simulator.getMCR());
    } else if(command == "run") {
        uint32_t inst_limit;
        command_tokens >> inst_limit;
#ifdef _ENABLE_DEBUG
        uint32_t start_count = simulator.getInstExecCount();
        auto start_time = std::chrono::steady_clock::now();
#endif
        if(! command_tokens.fail()) {
            simulator.setRunInstLimit(inst_limit);
        }
        simulator.run();
#ifdef _ENABLE_DEBUG
        auto end_time = std::chrono::steady_clock::now();
        uint32_t end_count = simulator.getInstExecCount();
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
            simulator.setReg(id, val);
        } else if(loc_s == "pc") {
            simulator.setPC(val);
        } else if(loc_s == "psr") {
            simulator.setPSR(val);
        } else if(loc_s == "mcr") {
            simulator.setMCR(val);
        } else {
            uint32_t addr;
            try {
                addr = std::stoi(loc_s, 0, 0);
            } catch(std::exception const & e) {
                std::cout << "invalid address\n";
                return true;
            }
            simulator.setMem(addr, val);
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
        } else if(sub_command == "out") {
            simulator.stepOut();
        } else if(sub_command == "over") {
            simulator.stepOver();
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

        bool removed = simulator.removeBreakpointByID(id);
        if(! removed) {
            std::cout << "invalid id\n";
            return;
        }
    } else if(command == "help") {
        breakHelp();
    } else if(command == "list") {
        for(auto x : simulator.getBreakpoints()) {
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

        lc3::Breakpoint bp = simulator.setBreakpoint(loc);
        std::cout << bp << "\n";

    } else  {
        std::cout << "unknown command\n";
    }
}

std::string formatMem(lc3::sim const & simulator, uint32_t addr)
{
    std::stringstream out;
    uint32_t value = simulator.getMem(addr);
    std::string line = simulator.getMemLine(addr);
    out << lc3::utils::ssprintf("0x%0.4X: 0x%0.4X %s", addr, value, line.c_str());
    return out.str();
}

std::ostream & operator<<(std::ostream & out, lc3::Breakpoint const & x)
{
    out << "#" << x.id << ": " << formatMem(*x.sim_int, x.loc);
    return out;
}

void breakpointCallback(lc3::core::MachineState & state, lc3::Breakpoint const & bp)
{
    (void) state;
    std::cout << "hit a breakpoint\n" << bp << "\n";
}
