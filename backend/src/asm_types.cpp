#include "asm_types.h"
#include "utils.h"

lc3::core::asmbl::Token::Token(void) : type(TokenType::INVALID) {}

lc3::core::asmbl::StatementToken::StatementToken(void) : Token(), lev_dist(0), pc(0) {}

lc3::core::asmbl::StatementToken::StatementToken(Token const & that) : Token(that) {}

bool lc3::core::asmbl::Statement::isPseudo(void) const
{
    return (inst_or_pseudo.type == TokenType::PSEUDO);
}

bool lc3::core::asmbl::Statement::isInst(void) const
{
    return (inst_or_pseudo.type == TokenType::INST);
}

bool lc3::core::asmbl::Statement::hasLabel(void) const
{
    return (label.type == TokenType::LABEL);
}

bool lc3::core::asmbl::Statement::isLabel(void) const
{
    return hasLabel() && inst_or_pseudo.type == TokenType::INVALID;
}

std::ostream & operator<<(std::ostream & out, lc3::core::asmbl::StatementToken const & x)
{
    using namespace lc3::core::asmbl;

    if(x.type == TokenType::STRING) {
        out << x.str << " (string)";
    } else if(x.type == TokenType::NUM) {
        out << x.num << " (num)";
    } else if(x.type == TokenType::INST) {
        out << x.str << " (inst~" << x.lev_dist << ")";
    } else if(x.type == TokenType::PSEUDO) {
        out << x.str << " (pseudo)";
    } else if(x.type == TokenType::REG) {
        out << x.str << " (reg)";
    } else if(x.type == TokenType::LABEL) {
        out << x.str << " (label)";
    } else if(x.type == TokenType::EOS) {
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
    if(x.label.type != lc3::core::asmbl::TokenType::INVALID) {
        out << x.label << "\n";
        indent = "  ";
    }

    if(x.inst_or_pseudo.type != lc3::core::asmbl::TokenType::INVALID) {
        out << indent << x.inst_or_pseudo << "\n";
        indent += "  ";
        for(auto const & operand : x.operands) {
            out << indent << operand << "\n";
        }
    }

    return out;
}
