#include "core.h"

void core::lc3::assemble(std::string const & asm_filename, std::string const & obj_filename)
{
    as.assemble(asm_filename, obj_filename);
}

void core::lc3::initializeSimulator(void)
{
    sim.loadOS();
}

void core::lc3::loadSimulatorWithFile(std::string const & obj_filename)
{
    sim.loadObjectFile(obj_filename);
}

void core::lc3::simulate(void)
{
    sim.simulate();
}

void core::lc3::resetSimulator(void)
{
    sim.reset();
}
