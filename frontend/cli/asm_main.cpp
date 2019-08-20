#include <string>

#include "common.h"
#include "console_printer.h"
#include "interface.h"

struct CLIArgs
{
    uint32_t print_level = DEFAULT_PRINT_LEVEL;
};

bool endsWith(std::string const & search, std::string const & suffix)
{
    if(suffix.size() > search.size()) { return false; }
    return std::equal(suffix.rbegin(), suffix.rend(), search.rbegin());
}

int main(int argc, char *argv[])
{
    CLIArgs args;
    std::vector<std::pair<std::string, std::string>> parsed_args = parseCLIArgs(argc, argv);
    for(auto const & arg : parsed_args) {
        if(std::get<0>(arg) == "print-level") {
            args.print_level = std::stoi(std::get<1>(arg));
        } else if(std::get<0>(arg) == "h" || std::get<0>(arg) == "help") {
            std::cout << "usage: " << argv[0] << " [OPTIONS]\n";
            std::cout << "\n";
            std::cout << "  -h,--help              Print this message\n";
            std::cout << "  --print-level=N        Output verbosity [0-9]\n";
            return 0;
        }
    }

    lc3::ConsolePrinter printer;
    lc3::as assembler(printer, args.print_level);
    lc3::conv converter(printer, args.print_level);

    for(int i = 1; i < argc; i += 1) {
        std::string filename(argv[i]);
        if(filename[0] != '-') {
            if(endsWith(filename, ".bin")) {
                converter.convertBin(filename);
            } else {
                assembler.assemble(filename);
            }
        }
    }

    return 0;
}
