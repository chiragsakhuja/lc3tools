#include "core/src/asm/assembler.h"
#include "core/src/common/printer.h"
#include "../common/console_printer.h"

int main(int argc, char *argv[])
{
    utils::Printer * printer = new utils::ConsolePrinter();
    Assembler as(true, *printer);

    std::map<std::string, int> symbol_table;

    if(argc < 2) {
        printer->printf(utils::PrintType::ERROR, "usage: %s file [file ...]", argv[0]);
    } else {
        for(int i = 1; i < argc; i += 1) {
            as.genObjectFile(argv[i]);
        }
    }

    delete printer;

    return 0;
}
