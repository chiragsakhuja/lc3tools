/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include "asm_types.h"
#include "logger.h"
#include "utils.h"

lc3::optional<uint32_t> lc3::core::asmbl::getNum(StatementNew const & statement, StatementPiece const & piece,
    uint32_t width, bool sext, lc3::utils::AssemblerLogger & logger, bool log_enable)
{
    uint32_t token_val = piece.num & ((1 << width) - 1);

    if(sext) {
        int32_t signed_value = static_cast<int32_t>(piece.num);
        if(signed_value < -(1 << (width - 1)) || signed_value > ((1 << (width - 1)) - 1)) {
            if(log_enable) {
                logger.asmPrintf(utils::PrintType::P_ERROR, statement, piece,
                    "cannot encode as %d-bit 2's complement number", width);
                logger.newline();
            }
            return {};
        }
    } else {
        if(piece.num > ((1u << width) - 1)) {
            if(log_enable) {
                logger.asmPrintf(utils::PrintType::P_ERROR, statement, piece,
                    "cannot encode as %d-bit unsigned number", width);
                logger.newline();
            }
            return {};
        }
    }

    return token_val;
}

std::ostream & operator<<(std::ostream & out, lc3::core::asmbl::Token const & token)
{
    using namespace lc3::core::asmbl;

    out << token.row << ":" << token.col << ".." << (token.col + token.len - 1) << ": ";
    if(token.type == Token::Type::STRING) {
        out << token.str << " (str)";
    } else if(token.type == Token::Type::EOL) {
        out << "(EOL)";
    } else if(token.type == Token::Type::NUM) {
        out << static_cast<uint16_t>(token.num) << " (num)";
    } else {
        out << "(invalid)";
    }

    //out << " [" << token.line << "]";
    return out;
}

std::ostream & operator<<(std::ostream & out, lc3::core::asmbl::StatementPiece const & piece)
{
    using namespace lc3::core::asmbl;

    if(piece.type == StatementPiece::Type::INST) {
        out << piece.str << " (inst)";
    } else if(piece.type == StatementPiece::Type::PSEUDO) {
        out << piece.str << " (pseudo)";
    } else if(piece.type == StatementPiece::Type::LABEL) {
        out << piece.str << " (label)";
    } else if(piece.type == StatementPiece::Type::REG) {
        out << piece.str << " (reg)";
    } else if(piece.type == StatementPiece::Type::STRING) {
        out << piece.str << " (string)";
    } else if(piece.type == StatementPiece::Type::NUM) {
        out << static_cast<uint16_t>(piece.num) << " (num)";
    } else {
        out << "(invalid)";
    }

    return out;
}

std::ostream & operator<<(std::ostream & out, lc3::core::asmbl::StatementNew const & statement)
{
    out << std::hex << "0x" << statement.pc << std::dec;
    if(statement.label) {
        out << " (" << *statement.label << ")";
    }
    out << " : ";
    if(statement.base) {
        out << *statement.base;
    }

    if(statement.operands.size() > 0) {
        out << " <= ";
        std::string prefix = "";
        for(auto const & x : statement.operands) {
            out << prefix << x;
            prefix = ", ";
        }
    }

    out << " [" << statement.line << "]";

    return out;
}
