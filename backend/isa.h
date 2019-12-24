#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <memory>
#include <string>
#include <vector>

#include "aliases.h"
#include "isa_abstract.h"
#include "state.h"

namespace lc3
{
namespace core
{
    class AddRegInstruction : public IInstruction
    {
    public:
        AddRegInstruction(void);
        virtual PIEvent buildEvents(MachineState const & state) const override;
    };

    class AddImmInstruction : public IInstruction
    {
    public:
        AddImmInstruction(void);
        virtual PIEvent buildEvents(MachineState const & state) const override;
    };
};
};

#endif
