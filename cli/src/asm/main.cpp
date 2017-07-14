#include <string>
#include <unistd.h>
#include <termios.h>

#include "core.h"
#include "console_printer.h"
#include "console_inputter.h"

int main(int argc, char *argv[])
{
    utils::IPrinter * printer = new utils::ConsolePrinter();
    utils::IInputter * inputter = new utils::ConsoleInputter();
    core::lc3 interface(*printer, *inputter);

    for(int i = 1; i < argc; i += 1) {
        try {
            std::string asm_filename(argv[i]);
            std::string obj_filename(asm_filename.substr(0, asm_filename.find_last_of('.')) + ".obj");
            interface.assemble(asm_filename, obj_filename);
        } catch(utils::exception const & e) {
            printer->print(e.what());
            printer->newline();
        }
    }

    delete printer;
    delete inputter;

    return 0;
}
