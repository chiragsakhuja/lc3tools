#include "core.h"
#include "console_printer.h"
#include "console_inputter.h"

int main(int argc, char * argv[])
{
    utils::ConsolePrinter printer;
    utils::ConsoleInputter inputter;
    core::lc3 interface(printer, inputter);

    interface.initializeSimulator();

    for(int i = 1; i < argc; i += 1) {
        try {
            interface.loadSimulatorWithFile(std::string(argv[i]));
        } catch (utils::exception const & e) {
            printer.print(e.what());
            printer.newline();
        }
    }

    try {
        interface.simulate();
    } catch(utils::exception const & e) {
        printer.print(e.what());
        printer.newline();
    }

    return 0;
}
