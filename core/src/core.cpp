#include "core.h"

using namespace core;

void lc3::assemble(std::string const & asm_filename, std::string const & obj_filename)
{
    as.assemble(asm_filename, obj_filename);
}

void lc3::initializeSimulator(void)
{
    sim.loadOS();
}

void lc3::loadSimulatorWithFile(std::string const & obj_filename)
{
    sim.loadObjectFile(obj_filename);
}

void lc3::simulate(void)
{
    sim.simulate();
}

void lc3::resetSimulator(void)
{
    sim.reset();
}
