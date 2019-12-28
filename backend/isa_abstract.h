#ifndef ISA_ABSTRACT_H
#define ISA_ABSTRACT_H

#include <string>

#include "aliases.h"
#include "asm_types.h"
#include "state.h"
#include "utils.h"

namespace lc3
{
namespace core
{
    class ISAHandler
    {
    public:
        ISAHandler(void);
        virtual ~ISAHandler(void) = default;

        SymbolTable const & getRegs(void) { return regs; }

    protected:
        std::vector<PIInstruction> instructions;
        SymbolTable regs;
    };

    class IOperand
    {
    public:
        enum class Type {
              FIXED = 0
            , NUM
            , LABEL
            , REG
            , INVALID
        };

        IOperand(Type type, std::string const & type_str, uint32_t width);
        virtual ~IOperand(void) = default;

        virtual optional<uint32_t> encode(asmbl::Statement const & statement, asmbl::StatementPiece const & piece,
            SymbolTable const & regs, SymbolTable const & symbols, lc3::utils::AssemblerLogger & logger) = 0;
        bool isEqualType(Type other) const;

        Type getType(void) const { return type; }
        uint32_t getWidth(void) const { return width; }
        uint16_t getValue(void) const { return value; }
        void setValue(uint16_t new_value) { value = new_value; }

    protected:
        Type type;

        // Used by assembler.
        std::string type_str;
        uint32_t width;

        // Used by simulator.
        uint16_t value;
    };

    class IInstruction
    {
    public:
        IInstruction(std::string const & name, std::vector<PIOperand> const & operands);
        IInstruction(IInstruction const & that);
        virtual ~IInstruction(void) = default;

        virtual PIMicroOp buildMicroOps(MachineState const & state) const = 0;
        std::string toFormatString(void) const;
        std::string toValueString(void) const;

        std::string const & getName(void) const { return name; }
        std::vector<PIOperand> const & getOperands(void) const { return operands; }
        PIOperand getOperand(uint32_t idx) const { return operands[idx]; }

    protected:
        std::string name;
        std::vector<PIOperand> operands;
    };

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
};
};

#endif
