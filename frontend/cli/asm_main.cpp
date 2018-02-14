#include <string>

#include "common.h"
#include "console_printer.h"
#include "interface.h"

struct CLIArgs
{
    uint32_t print_level = DEFAULT_PRINT_LEVEL;
};

int main(int argc, char *argv[])
{
    CLIArgs args;
    std::vector<std::pair<std::string, std::string>> parsed_args = parseCLIArgs(argc, argv);
    for(auto const & arg : parsed_args) {
        if(std::get<0>(arg) == "print-level") {
            args.print_level = std::stoi(std::get<1>(arg));
        }
    }

    lc3::ConsolePrinter printer;
    lc3::as assembler(printer, args.print_level);

    for(int i = 1; i < argc; i += 1) {
        std::string asm_filename(argv[i]);
        if(asm_filename[0] != '-') {
            assembler.assemble(asm_filename);
        }
    }

    return 0;
}
