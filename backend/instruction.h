#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <memory>
#include <string>
#include <vector>

#include "asm_types.h"
#include "aliases.h"
#include "state.h"

namespace lc3
{
namespace core
{
    class IOperand
    {
    public:
        enum class Type {
              FIXED = 0
            , NUM
            , LABEL
            , REG
            , INVALID
        } type;

        IOperand(Type type, std::string const & type_str, uint32_t width);
        virtual ~IOperand(void) = default;

        // Used by assembler.
        std::string type_str;
        uint32_t width;

        virtual optional<uint32_t> encode(asmbl::Statement const & statement, asmbl::StatementPiece const & piece,
            SymbolTable const & regs, SymbolTable const & symbols, lc3::utils::AssemblerLogger & logger) = 0;
        bool isEqualType(Type other) const;

        // Used by simulator.
        uint16_t value;
    };

    using PIOperand = std::shared_ptr<IOperand>;

    class InstructionHandler
    {
    public:
        InstructionHandler(void);
        virtual ~InstructionHandler(void) = default;

        lc3::core::SymbolTable const & getRegs(void) { return regs; }
    protected:
        std::vector<PIInstruction> instructions;
        lc3::core::SymbolTable regs;
    };

    class IInstruction
    {
    public:
        std::string name;
        std::vector<PIOperand> operands;

        IInstruction(std::string const & name, std::vector<PIOperand> const & operands);
        IInstruction(IInstruction const & that);
        virtual ~IInstruction(void) = default;

        uint32_t getNumOperands(void) const;
        void assignOperands(uint32_t encoded_inst);

        virtual PIEvent execute(MachineState const & state) const = 0;
        std::string toFormatString(void) const;
        std::string toValueString(void) const;
    };
};
};

#endif
