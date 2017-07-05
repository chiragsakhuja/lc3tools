#include "core.h"

using namespace core;

void lc3::loadSimulator(std::string const & obj_filename)
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
