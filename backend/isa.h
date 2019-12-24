#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <memory>
#include <string>
#include <vector>

#include "asm_types.h"
#include "aliases.h"
#include "isa_abstract.h"
#include "state.h"

namespace lc3
{
namespace core
{
    class FixedOperand : public IOperand
    {
    public:
        FixedOperand(uint32_t width, uint32_t value);
        virtual optional<uint32_t> encode(asmbl::Statement const & statement, asmbl::StatementPiece const & piece,
            SymbolTable const & regs, SymbolTable const & symbols, utils::AssemblerLogger & logger) override;
    };

    class RegOperand : public IOperand
    {
    public:
        RegOperand(uint32_t width);
        virtual optional<uint32_t> encode(asmbl::Statement const & statement, asmbl::StatementPiece const & piece,
            SymbolTable const & regs, SymbolTable const & symbols, utils::AssemblerLogger & logger) override;
    };

    class NumOperand : public IOperand
    {
    public:
        NumOperand(uint32_t width, bool sext);
        virtual optional<uint32_t> encode(asmbl::Statement const & statement, asmbl::StatementPiece const & piece,
            SymbolTable const & regs, SymbolTable const & symbols, utils::AssemblerLogger & logger) override;

    private:
        bool sext;
    };

    class LabelOperand : public IOperand
    {
    public:
        LabelOperand(uint32_t width);
        virtual optional<uint32_t> encode(asmbl::Statement const & statement, asmbl::StatementPiece const & piece,
            SymbolTable const & regs, SymbolTable const & symbols, utils::AssemblerLogger & logger) override;
    };

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
