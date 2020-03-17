/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */

#include <string>
#include <fstream>

#include "common.h"
#include "file_printer.h"
#include "console_inputter.h"
#include "interface.h"

int main(int argc, char * argv[])
{
    lc3::FilePrinter printer("run.log");
    lc3::ConsoleInputter inputter;
    lc3::sim sim(printer, inputter, true, 9, false);

    for(int i = 2; i < argc; i += 1) {
        std::string arg(argv[i]);
        if(arg[0] != '-') {
            std::string filename(argv[i]);
            sim.loadObjFile(filename);
        }
    }

    sim.run();

    return 0;
}
