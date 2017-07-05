#ifndef CORE_H
#define CORE_H

#include <array>
#include <cstdint>
#include <fstream>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "utils.h"

#include "tokens.h"

#include "printer.h"
#include "logger.h"

#include "state.h"

#include "instructions.h"
#include "instruction_encoder.h"
#include "instruction_decoder.h"

#include "simulator.h"
#include "assembler.h"

namespace core {
    class lc3
    {
    public:
        lc3(utils::IPrinter & printer) : as(true, printer), sim(true, printer) {}
        ~lc3(void) = default;

        void assemble(std::string const & asm_filename, std::string const & obj_filename);
        void loadSimulator(std::string const & obj_filename);
        void simulate(void);
        void resetSimulator(void);

    private:
        Assembler as;
        Simulator sim;
    };
};

#endif
