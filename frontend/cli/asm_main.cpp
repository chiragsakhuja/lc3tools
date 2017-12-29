#include <string>

#include "core.h"
#include "console_printer.h"
#include "console_inputter.h"

int main(int argc, char *argv[])
{
    utils::ConsolePrinter printer;
    utils::ConsoleInputter inputter;
    core::lc3 interface(printer, inputter);

    for(int i = 1; i < argc; i += 1) {
        std::string asm_filename(argv[i]);
        std::string obj_filename(asm_filename.substr(0, asm_filename.find_last_of('.')) + ".obj");
        try {
            interface.assemble(asm_filename, obj_filename);
        } catch(utils::exception const & e) { return 1; }
    }

    return 0;
}
