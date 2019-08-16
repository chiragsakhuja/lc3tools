#include "asm_types.h"
#include "utils.h"

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
