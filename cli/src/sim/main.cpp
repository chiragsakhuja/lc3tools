#include "simulator.h"
#include "printer.h"
#include "../common/console_printer.h"

int main(int argc, char *argv[])
{
    utils::Printer * printer = new utils::ConsolePrinter();
    core::Simulator sim(true, *printer);

    if(argc < 2) {
        //printer->printf(utils::PRINT_TYPE_ERROR, "usage: %s file [file ...]", argv[0]);
    } else {
        for(int i = 1; i < argc; i += 1) {
            try {
                sim.loadObjectFile(argv[i]);
            } catch (std::runtime_error const & e) {}
        }
    }

    delete printer;

    return 0;
}
