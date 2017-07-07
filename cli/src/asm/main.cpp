#include <iostream>
#include <stdexcept>
#include <string>

#include "core.h"

#include "../common/console_printer.h"

int main(int argc, char *argv[])
{
    utils::IPrinter * printer = new utils::ConsolePrinter();
    core::lc3 interface(*printer);

    if(argc < 2) {
        //printer->printf(utils::PRINT_TYPE_ERROR, "usage: %s file [file ...]", argv[0]);
    } else {
        for(int i = 1; i < argc; i += 1) {
            try {
                std::string asm_filename(argv[i]);
                std::string obj_filename(asm_filename.substr(0, asm_filename.find_last_of('.')) + ".obj");
                interface.assemble(asm_filename, obj_filename);
            } catch(utils::exception const & e) {
                std::cout << "ERROR: " << e.what() << "\n";
            }
        }
    }

    delete printer;

    return 0;
}
