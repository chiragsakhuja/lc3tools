#include <string>

#include "console_printer.h"
#include "interface.h"

int main(int argc, char *argv[])
{
    lc3::ConsolePrinter printer;
    lc3::as assembler(printer);

    for(int i = 1; i < argc; i += 1) {
        std::string asm_filename(argv[i]);
        assembler.assemble(asm_filename);
    }

    return 0;
}
