#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <memory>
#include <string>
#include <vector>

#include "aliases.h"
#include "isa_abstract.h"
#include "state.h"
#include "uop.h"

namespace lc3
{
namespace core
{
    class AddRegInstruction : public IInstruction
    {
    public:
        AddRegInstruction(void);
        virtual PIMicroOp buildMicroOps(MachineState const & state) const override;
    };

    class AddImmInstruction : public IInstruction
    {
    public:
        AddImmInstruction(void);
        virtual PIMicroOp buildMicroOps(MachineState const & state) const override;
    };
};
};

#endif
