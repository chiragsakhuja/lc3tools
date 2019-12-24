#include "isa.h"
#include "logger.h"

using namespace lc3::core;

AddRegInstruction::AddRegInstruction(void) : IInstruction("add", {
        std::make_shared<FixedOperand>(4, 0x1),
        std::make_shared<RegOperand>(3),
        std::make_shared<RegOperand>(3),
        std::make_shared<FixedOperand>(3, 0x0),
        std::make_shared<RegOperand>(3)
    })
{ }

AddImmInstruction::AddImmInstruction(void) : IInstruction("add", {
        std::make_shared<FixedOperand>(4, 0x1),
        std::make_shared<RegOperand>(3),
        std::make_shared<RegOperand>(3),
        std::make_shared<FixedOperand>(1, 0x1),
        std::make_shared<NumOperand>(5, true)
    })
{ }
