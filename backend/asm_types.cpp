#include "asm_types.h"
#include "utils.h"

lc3::core::asmbl::Token::Token(void) : type(Token::Type::INVALID) {}

lc3::core::asmbl::StatementToken::StatementToken(void) : Token(), lev_dist(0), pc(0) {}

lc3::core::asmbl::StatementToken::StatementToken(Token const & that) : Token(that) {}

/*
 *bool lc3::core::asmbl::Statement::isPseudo(void) const
 *{
 *    return (inst_or_pseudo.type == Token::Type::PSEUDO);
 *}
 *
 *bool lc3::core::asmbl::Statement::isInst(void) const
 *{
 *    return (inst_or_pseudo.type == Token::Type::INST);
 *}
 *
 *bool lc3::core::asmbl::Statement::hasLabel(void) const
 *{
 *    return (label.type == Token::Type::LABEL);
 *}
 *
 *bool lc3::core::asmbl::Statement::isLabel(void) const
 *{
 *    return hasLabel() && inst_or_pseudo.type == Token::Type::INVALID;
 *}
 */

std::ostream & operator<<(std::ostream & out, lc3::core::asmbl::StatementToken const & x)
{
    using namespace lc3::core::asmbl;

    if(x.type == Token::Type::STRING) {
        out << x.str << " (string)";
    } else if(x.type == Token::Type::NUM) {
        out << x.num << " (num)";
    /*
     *} else if(x.type == Token::Type::INST) {
     *    out << x.str << " (inst~" << x.lev_dist << ")";
     *} else if(x.type == Token::Type::PSEUDO) {
     *    out << x.str << " (pseudo)";
     *} else if(x.type == Token::Type::REG) {
     *    out << x.str << " (reg)";
     *} else if(x.type == Token::Type::LABEL) {
     *    out << x.str << " (label)";
     */
    } else if(x.type == Token::Type::EOL) {
        out << "EOS";
        return out;
    } else {
        out << "invalid token";
    }

    out << " " << (x.row + 1) << ":" << (x.col + 1) << "+" << x.len;
    return out;
}

std::ostream & operator<<(std::ostream & out, lc3::core::asmbl::Statement const & x)
{
    out << "==== (" << lc3::utils::ssprintf("0x%0.4x", x.pc) << ") " << x.line << "\n";
    std::string indent = "";
    if(x.label.type != lc3::core::asmbl::Token::Type::INVALID) {
        out << x.label << "\n";
        indent = "  ";
    }

    if(x.inst_or_pseudo.type != lc3::core::asmbl::Token::Type::INVALID) {
        out << indent << x.inst_or_pseudo << "\n";
        indent += "  ";
        for(auto const & operand : x.operands) {
            out << indent << operand << "\n";
        }
    }

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
        out << piece.num << " (num)";
    } else {
        out << "(invalid)";
    }

    return out;
}

std::ostream & operator<<(std::ostream & out, lc3::core::asmbl::StatementNew const & statement)
{
    if(statement.label) {
        out << *statement.label;
        if(statement.base) {
            out << ": ";
        }
    }
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

    return out;
}
