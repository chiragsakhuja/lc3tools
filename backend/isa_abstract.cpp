#include <sstream>

#include "isa_abstract.h"

using namespace lc3::core;

IOperand::IOperand(IOperand::Type type, std::string const & type_str, uint32_t width) :
    type(type), type_str(type_str), width(width), value(0)
{ }

bool IOperand::isEqualType(IOperand::Type other) const
{
    return type == other;
}

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
    return "";
}

std::string IInstruction::toValueString(void) const
{
    return "";
}
