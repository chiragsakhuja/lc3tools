#include <stdexcept>
#include "instructions.h"

using namespace core;

Operand::Operand(OperType type, uint32_t width)
{
    this->type = type;
    this->width = width;
}

Instruction::Instruction(bool setcc, std::string const & name, std::vector<Operand *> const & operands)
{
    this->setcc = setcc;
    this->name = name;
    this->operands = operands;
}

Instruction::~Instruction(void)
{
    for(uint32_t i = 0; i < operands.size(); i += 1) {
        delete operands[i];
    }
}

uint32_t Instruction::getNumOperands()
{
    uint32_t ret = 0;
    for(Operand * operand : operands) {
        if(operand->type != OPER_TYPE_FIXED) {
            ret += 1;
        }
    }
    return ret;
}

bool Operand::isEqualType(OperType other) const
{
    return type == other;
}

InstructionGenerator::InstructionGenerator(void)
{
    regs["r0"] = 0;
    regs["r1"] = 1;
    regs["r2"] = 2;
    regs["r3"] = 3;
    regs["r4"] = 4;
    regs["r5"] = 5;
    regs["r6"] = 6;
    regs["r7"] = 7;

    instructions["add"].push_back(new ADDInstruction({
        new FixedOperand(4, 0x1),
        new RegOperand(3),
        new RegOperand(3),
        new FixedOperand(3, 0x0),
        new RegOperand(3)
    }));

    instructions["add"].push_back(new ADDInstruction({
        new FixedOperand(4, 0x1),
        new RegOperand(3),
        new RegOperand(3),
        new FixedOperand(1, 0x1),
        new NumOperand(5, true)
    }));

    instructions["and"].push_back(new ANDInstruction({
        new FixedOperand(4, 0x5),
        new RegOperand(3),
        new RegOperand(3),
        new FixedOperand(3, 0x0),
        new RegOperand(3)
    }));

    instructions["and"].push_back(new ANDInstruction({
        new FixedOperand(4, 0x5),
        new RegOperand(3),
        new RegOperand(3),
        new FixedOperand(1, 0x1),
        new NumOperand(5, true)
    }));

    instructions["jmp"].push_back(new JMPInstruction({
        new FixedOperand(4, 0xc),
        new FixedOperand(3, 0x0),
        new RegOperand(3),
        new FixedOperand(6, 0x0)
    }));

    instructions["jsr"].push_back(new JSRInstruction({
        new FixedOperand(4, 0x4),
        new FixedOperand(1, 0x1),
        new LabelOperand(11)
    }));

    instructions["jsrr"].push_back(new JSRInstruction({
        new FixedOperand(4, 0x4),
        new FixedOperand(1, 0x0),
        new FixedOperand(2, 0x0),
        new RegOperand(3),
        new FixedOperand(6, 0x0)
    }));

    instructions["ld"].push_back(new LDInstruction({
        new FixedOperand(4, 0x2),
        new RegOperand(3),
        new LabelOperand(9)
    }));

    instructions["ldi"].push_back(new LDIInstruction({
        new FixedOperand(4, 0xa),
        new RegOperand(3),
        new LabelOperand(9)
    }));

    instructions["ldr"].push_back(new LDRInstruction({
        new FixedOperand(4, 0x6),
        new RegOperand(3),
        new RegOperand(3),
        new LabelOperand(6)
    }));

    instructions["lea"].push_back(new LEAInstruction({
        new FixedOperand(4, 0xe),
        new RegOperand(3),
        new LabelOperand(9)
    }));

    instructions["not"].push_back(new NOTInstruction({
        new FixedOperand(4, 0x0),
        new RegOperand(3),
        new RegOperand(3),
        new FixedOperand(6, 0x3f)
    }));

    instructions["ret"].push_back(new JMPInstruction({
        new FixedOperand(4, 0xc),
        new FixedOperand(3, 0x0),
        new FixedOperand(3, 0x7),
        new FixedOperand(6, 0x0)
    }));

    instructions["rti"].push_back(new RTIInstruction({
        new FixedOperand(4, 0x0),
        new RegOperand(3),
        new RegOperand(3),
        new FixedOperand(6, 0x3f)
    }));

    instructions["st"].push_back(new STInstruction({
        new FixedOperand(4, 0x3),
        new RegOperand(3),
        new LabelOperand(9)
    }));

    instructions["sti"].push_back(new STIInstruction({
        new FixedOperand(4, 0xb),
        new RegOperand(3),
        new LabelOperand(9)
    }));

    instructions["str"].push_back(new STRInstruction({
        new FixedOperand(4, 0x7),
        new RegOperand(3),
        new RegOperand(3),
        new LabelOperand(6)
    }));

    instructions["trap"].push_back(new TRAPInstruction({
        new FixedOperand(4, 0xf),
        new FixedOperand(4, 0x0),
        new NumOperand(8, false)
    }));
}

InstructionGenerator::~InstructionGenerator(void)
{
    // TODO: Go through map and delete all instructions
}

std::string core::udecToBin(uint32_t value, uint32_t num_bits)
{
    char * bits = new char[num_bits + 1];
    for(uint32_t i = 0; i < num_bits; i += 1) {
        bits[num_bits - i - 1] = (value & 1) + '0';
        value >>= 1;
    }
    bits[num_bits] = 0;

    return std::string(bits);
}

uint32_t core::sextTo32(uint32_t value, uint32_t num_bits)
{
    uint32_t extension = ~((1 << num_bits) - 1);
    if((value >> (num_bits - 1)) & 1) {
        return extension | value;
    } else {
        return value;
    }
}

uint32_t FixedOperand::encode(bool log_enable, AssemblerLogger const & logger,
    std::string const & filename, std::string const & line, Token const * inst,
    Token const * operand, uint32_t oper_count, 
    std::map<std::string, uint32_t> const & registers,
    std::map<std::string, uint32_t> const & labels)
{
    (void) operand;
    (void) registers;
    return value & ((1 << width) - 1);
}

uint32_t RegOperand::encode(bool log_enable, AssemblerLogger const & logger,
    std::string const & filename, std::string const & line, Token const * inst,
    Token const * operand, uint32_t oper_count, 
    std::map<std::string, uint32_t> const & registers,
    std::map<std::string, uint32_t> const & labels)
{
    uint32_t token_val = registers.at(std::string(operand->str)) & ((1 << width) - 1);

    if(log_enable) {
        logger.printf(utils::PrintType::DEBUG, "%d.%d: reg %s => %s", inst->row_num, oper_count,
            operand->str.c_str(), udecToBin(token_val, width).c_str());
    }

    return token_val;
}

uint32_t NumOperand::encode(bool log_enable, AssemblerLogger const & logger,
    std::string const & filename, std::string const & line, Token const * inst,
    Token const * operand, uint32_t oper_count, 
    std::map<std::string, uint32_t> const & registers,
    std::map<std::string, uint32_t> const & labels)
{
    uint32_t token_val = operand->num & ((1 << width) - 1);
    (void) registers;

    if(sext) {
        if((int32_t) operand->num < -(1 << (width - 1)) || (int32_t) operand->num > ((1 << (width - 1)) - 1)) {
            logger.printfMessage(utils::PrintType::WARNING, filename, operand, line, "immediate %d truncated to %d",
                operand->num, sextTo32(token_val, width));
        }
    } else {
        if(operand->num > ((1 << width) - 1)) {
            logger.printfMessage(utils::PrintType::WARNING, filename, operand, line, "immediate %d truncated to %u",
                operand->num, token_val);
        }
    }

    if(log_enable) {
        logger.printf(utils::PrintType::DEBUG, "%d.%d: imm %d => %s", inst->row_num, oper_count, operand->num,
            udecToBin(token_val, width).c_str());
    }

    return token_val;
}

uint32_t LabelOperand::encode(bool log_enable, AssemblerLogger const & logger,
    std::string const & filename, std::string const & line, Token const * inst,
    Token const * operand, uint32_t oper_count, 
    std::map<std::string, uint32_t> const & registers,
    std::map<std::string, uint32_t> const & labels)
{
    auto search = labels.find(operand->str);
    if(search == labels.end()) {
        if(log_enable) {
            logger.printfMessage(utils::PrintType::ERROR, filename, operand, line, "unknown label \'%s\'",
                operand->str.c_str());
        }
        throw std::runtime_error("unknown label");
    }

    int32_t token_val = ((int32_t) search->second) - (inst->pc + 1);
    (void) registers;

    if(log_enable) {
        logger.printf(utils::PrintType::DEBUG, "%d.%d: label %s (0x%0.4x) => %s", inst->row_num, oper_count,
            operand->str.c_str(), search->second, udecToBin((uint32_t) token_val, width).c_str());
    }

    return token_val;
}
