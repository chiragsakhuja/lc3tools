#include <sstream>

#include "logger.h"
#include "isa_abstract.h"

using namespace lc3::core;

IOperand::IOperand(IOperand::Type type, std::string const & type_str, uint32_t width) :
    type(type), type_str(type_str), width(width), value(0)
{ }

bool IOperand::isEqualType(IOperand::Type other) const
{
    return type == other;
}

FixedOperand::FixedOperand(uint32_t width, uint32_t value) : IOperand(IOperand::Type::FIXED, "fixed", width)
{
    this->value = value;
}

RegOperand::RegOperand(uint32_t width) : IOperand(IOperand::Type::REG, "reg", width) { }
NumOperand::NumOperand(uint32_t width, bool sext) : IOperand(IOperand::Type::NUM, "imm", width), sext(sext) { }
LabelOperand::LabelOperand(uint32_t width) : IOperand(IOperand::Type::LABEL, "label/imm", width) { }

IInstruction::IInstruction(std::string const & name, std::vector<PIOperand> const & operands)
{
    this->name = name;
    this->operands = operands;
}

IInstruction::IInstruction(IInstruction const & that)
{
    this->name = that.name;
    for(PIOperand op : that.operands) {
        this->operands.push_back(op);
    }
}

std::string IInstruction::toFormatString(void) const
{
    // TODO: Fill in code
    return "";
}

std::string IInstruction::toValueString(void) const
{
    // TODO: Fill in code
    return "";
}

lc3::optional<uint32_t> FixedOperand::encode(asmbl::Statement const & statement, asmbl::StatementPiece const & piece,
    SymbolTable const & regs, SymbolTable const & symbols, lc3::utils::AssemblerLogger & logger)
{
    (void) statement;
    (void) piece;
    (void) regs;
    (void) symbols;
    (void) logger;

    return value & ((1 << width) - 1);
}

lc3::optional<uint32_t> RegOperand::encode(asmbl::Statement const & statement, asmbl::StatementPiece const & piece,
    SymbolTable const & regs, SymbolTable const & symbols, lc3::utils::AssemblerLogger & logger)
{
    using namespace lc3::utils;

    (void) statement;
    (void) symbols;

    uint32_t token_val = regs.at(toLower(piece.str)) & ((1 << width) - 1);

    logger.printf(PrintType::P_EXTRA, true, "  reg %s := %s", piece.str.c_str(), udecToBin(token_val, width).c_str());

    return token_val;
}

lc3::optional<uint32_t> NumOperand::encode(asmbl::Statement const & statement, asmbl::StatementPiece const & piece,
    SymbolTable const & regs, SymbolTable const & symbols, lc3::utils::AssemblerLogger & logger)
{
    using namespace lc3::utils;

    (void) regs;
    (void) symbols;

    auto ret = getNum(statement, piece, this->width, this->sext, logger, true);

    if(! ret) {
        throw lc3::utils::exception("invalid immediate");
    }

    logger.printf(PrintType::P_EXTRA, true, "  imm %d := %s", piece.num, udecToBin(*ret, width).c_str());

    return *ret;
}

lc3::optional<uint32_t> LabelOperand::encode(asmbl::Statement const & statement, asmbl::StatementPiece const & piece,
    SymbolTable const & regs, SymbolTable const & symbols, lc3::utils::AssemblerLogger & logger)
{
    using namespace lc3::utils;
    using namespace asmbl;

    (void) regs;

    if(piece.type == StatementPiece::Type::NUM) {
        return NumOperand(this->width, true).encode(statement, piece, regs, symbols, logger);
    } else {
        auto search = symbols.find(toLower(piece.str));
        if(search == symbols.end()) {
            logger.asmPrintf(PrintType::P_ERROR, statement, piece, "could not find label");
            logger.newline();
            return {};
        }

        StatementPiece num_piece = piece;
        num_piece.num = static_cast<int32_t>(search->second) - (statement.pc + 1);
        auto ret = getNum(statement, num_piece, this->width, true, logger, true);

        if(! ret) {
            throw lc3::utils::exception("label too far");
        }

        logger.printf(PrintType::P_EXTRA, true, "  label %s (0x%0.4x) := %s", piece.str.c_str(), search->second,
            udecToBin(*ret, width).c_str());

        return *ret;
    }
}
