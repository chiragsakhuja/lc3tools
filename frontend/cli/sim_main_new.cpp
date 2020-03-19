/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */

#include <iostream>
#include <string>
#include <fstream>

#include "common.h"
#include "file_printer.h"
#include "console_inputter.h"
#include "interface.h"

int main(int argc, char * argv[])
{
    if(argc < 3) {
        std::cout << "usage: " << argv[0] << " log [obj ...]\n";
        return 0;
    }

    lc3::FilePrinter printer{std::string{argv[1]}};
    lc3::ConsoleInputter inputter;
    lc3::sim simulator{printer, inputter, 4};

    for(int i = 2; i < argc; i += 1) {
        std::string arg(argv[i]);
        if(arg[0] != '-') {
            std::string filename(argv[i]);
            simulator.loadObjFile(filename);
        }
    }

    simulator.run();

    return 0;
}
