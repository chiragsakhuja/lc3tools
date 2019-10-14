/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#ifndef INSTRUCTION_ENCODER_H
#define INSTRUCTION_ENCODER_H

#include <memory>

#include "instructions.h"
#include "logger.h"
#include "optional.h"

namespace lc3
{
namespace core
{
namespace asmbl
{
    class InstructionEncoder : public InstructionHandler
    {
    public:
        InstructionEncoder(lc3::utils::AssemblerLogger & logger, bool enable_liberal_assembly);

        bool isStringPseudo(std::string const & search) const;
        bool isStringValidReg(std::string const & search) const;
        bool isPseudo(StatementNew const & statement) const;
        bool isInst(StatementNew const & statement) const;
        bool isValidPseudoOrig(StatementNew const & statement, bool log_enable = false) const;
        bool isValidPseudoFill(StatementNew const & statement, bool log_enable = false) const;
        bool isValidPseudoFill(StatementNew const & statement, SymbolTable const & symbols,
            bool log_enable = false) const;
        bool isValidPseudoBlock(StatementNew const & statement, bool log_enable = false) const;
        bool isValidPseudoString(StatementNew const & statement, bool log_enable = false) const;
        bool isValidPseudoEnd(StatementNew const & statement, bool log_enable = false) const;

        uint32_t getDistanceToNearestInstructionName(std::string const & search) const;

        bool validatePseudo(StatementNew const & statement, SymbolTable const & symbols) const;
        optional<PIInstruction> validateInstruction(StatementNew const & statement) const;

        uint32_t getNum(StatementNew const & statement, StatementPiece const & piece, bool sext,
            bool log_enable = false) const;
        uint32_t getPseudoOrig(StatementNew const & statement) const;
        uint32_t getPseudoFill(StatementNew const & statement, SymbolTable const & symbols) const;
        uint32_t getPseudoBlockSize(StatementNew const & statement) const;
        uint32_t getPseudoStringSize(StatementNew const & statement) const;
        std::string getPseudoString(StatementNew const & statement) const;
        optional<uint32_t> encodeInstruction(StatementNew const & statement, SymbolTable const & symbols,
            PIInstruction pattern) const;

        void setLiberalAsm(bool enable_liberal_asm) { this->enable_liberal_asm = enable_liberal_asm; }

    private:
        lc3::utils::AssemblerLogger & logger;
        bool enable_liberal_asm;

        bool validatePseudoOperands(StatementNew const & statement, std::string const & pseudo,
            std::vector<StatementPiece::Type> const & valid_types, uint32_t operand_count, bool log_enable) const;

        std::map<std::string, std::vector<PIInstruction>> instructions_by_name;

        uint32_t levDistance(std::string const & a, std::string const & b) const;
        uint32_t levDistanceHelper(std::string const & a, uint32_t a_len, std::string const & b, uint32_t b_len) const;
    };
};
};
};

#endif
