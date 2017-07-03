#include <array>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "utils.h"

#include "tokens.h"

#include "printer.h"
#include "logger.h"

#include "state.h"

#include "instructions.h"

using namespace core;

Operand::Operand(OperType type, std::string const & type_str, uint32_t width)
{
    this->type = type;
    this->type_str = type_str;
    this->width = width;
    this->value = 0;
}

Instruction::Instruction(std::string const & name, std::vector<Operand *> const & operands)
{
    this->name = name;
    this->operands = operands;
}

Instruction::~Instruction(void)
{
    for(uint32_t i = 0; i < operands.size(); i += 1) {
        delete operands[i];
    }
}

uint32_t Instruction::getNumOperands() const
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

InstructionHandler::InstructionHandler(void)
{
    regs["r0"] = 0;
    regs["r1"] = 1;
    regs["r2"] = 2;
    regs["r3"] = 3;
    regs["r4"] = 4;
    regs["r5"] = 5;
    regs["r6"] = 6;
    regs["r7"] = 7;

    instructions.push_back(new ADDRInstruction());
    instructions.push_back(new ADDIInstruction());
    instructions.push_back(new ANDRInstruction());
    instructions.push_back(new ANDIInstruction());
    instructions.push_back(new JMPInstruction());
    instructions.push_back(new JSRInstruction());
    instructions.push_back(new JSRRInstruction());
    instructions.push_back(new LDInstruction());
    instructions.push_back(new LDIInstruction());
    instructions.push_back(new LDRInstruction());
    instructions.push_back(new LEAInstruction());
    instructions.push_back(new NOTInstruction());
    instructions.push_back(new RETInstruction());
    instructions.push_back(new RTIInstruction());
    instructions.push_back(new STInstruction());
    instructions.push_back(new STIInstruction());
    instructions.push_back(new STRInstruction());
    instructions.push_back(new TRAPInstruction());
}

InstructionHandler::~InstructionHandler(void)
{
    for(uint32_t i = 0; i < instructions.size(); i += 1) {
        delete instructions[i];
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
        logger.printf(PRINT_TYPE_DEBUG, true, "%d.%d: reg %s => %s", inst->row_num, oper_count,
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
            logger.printfMessage(PRINT_TYPE_WARNING, filename, operand, line, "immediate %d truncated to %d",
                operand->num, sextTo32(token_val, width));
            logger.newline();
        }
    } else {
        if(operand->num > ((1 << width) - 1)) {
            logger.printfMessage(PRINT_TYPE_WARNING, filename, operand, line, "immediate %d truncated to %u",
                operand->num, token_val);
            logger.newline();
        }
    }

    if(log_enable) {
        logger.printf(PRINT_TYPE_DEBUG, true, "%d.%d: imm %d => %s", inst->row_num, oper_count, operand->num,
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
            logger.printfMessage(PRINT_TYPE_ERROR, filename, operand, line, "unknown label \'%s\'",
                operand->str.c_str());
            logger.newline();
        }
        throw std::runtime_error("unknown label");
    }

    uint32_t token_val = (((int32_t) search->second) - (inst->pc + 1)) & ((1 << width) - 1);
    (void) registers;

    if(log_enable) {
        logger.printf(PRINT_TYPE_DEBUG, true, "%d.%d: label %s (0x%0.4x) => %s", inst->row_num, oper_count,
            operand->str.c_str(), search->second, udecToBin((uint32_t) token_val, width).c_str());
    }

    return token_val;
}

void ADDRInstruction::execute(MachineState & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr1_val = sextTo32(state.regs[operands[2]->value], 16);
    uint32_t sr2_val = sextTo32(state.regs[operands[4]->value], 16);
    uint32_t sum = (sr1_val + sr2_val) & 0xffff;
    state.psr = computePSRCC(sum, state.psr);
    state.regs[dr] = sum;
}

void ADDIInstruction::execute(MachineState & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr1_val = sextTo32(state.regs[operands[2]->value], 16);
    uint32_t imm_val = sextTo32(operands[4]->value, operands[4]->width);
    uint32_t sum = (sr1_val + imm_val) & 0xffff;
    state.psr = computePSRCC(sum, state.psr);
    state.regs[dr] = sum;
}

void ANDRInstruction::execute(MachineState & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr1_val = sextTo32(state.regs[operands[2]->value], 16);
    uint32_t sr2_val = sextTo32(state.regs[operands[4]->value], 16);
    uint32_t sum = (sr1_val & sr2_val) & 0xffff;
    state.psr = computePSRCC(sum, state.psr);
    state.regs[dr] = sum;
}

void ANDIInstruction::execute(MachineState & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr1_val = sextTo32(state.regs[operands[2]->value], 16);
    uint32_t imm_val = sextTo32(operands[4]->value, operands[4]->width);
    uint32_t sum = (sr1_val & imm_val) & 0xffff;
    state.psr = computePSRCC(sum, state.psr);
    state.regs[dr] = sum;
}

void JMPInstruction::execute(MachineState & state)
{
    state.pc = state.regs[operands[2]->value];
}

void JSRInstruction::execute(MachineState & state)
{
    state.regs[7] = state.pc;
    state.pc = computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);
}

void JSRRInstruction::execute(MachineState & state)
{
    state.regs[7] = state.pc;
    state.pc = state.regs[operands[3]->value];
}

void LDInstruction::execute(MachineState & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t addr = computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);
    uint32_t value = state.mem[addr];
    state.psr = computePSRCC(value, state.psr);
    state.regs[dr] = value;
}

void LDIInstruction::execute(MachineState & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t addr1 = computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);
    uint32_t addr2 = state.mem[addr1];
    uint32_t value = state.mem[addr2];
    state.psr = computePSRCC(value, state.psr);
    state.regs[dr] = value;
}

void LDRInstruction::execute(MachineState & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t addr = computeBasePlusSOffset(state.regs[operands[2]->value], operands[3]->value,
            operands[3]->width);
    uint32_t value = state.mem[addr];
    state.psr = computePSRCC(value, state.psr);
    state.regs[dr] = value;
}

void LEAInstruction::execute(MachineState & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t addr = computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);
    state.psr = computePSRCC(addr, state.psr);
    state.regs[dr] = addr;
}

void NOTInstruction::execute(MachineState & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr_val = sextTo32(state.regs[operands[2]->value], operands[2]->width);
    uint32_t value = (~sr_val) & 0xffff;
    state.psr = computePSRCC(value, state.psr);
    state.regs[dr] = value;
}

void RTIInstruction::execute(MachineState & state)
{

}

void STInstruction::execute(MachineState & state)
{
    uint32_t addr = computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);
    uint32_t value = state.regs[operands[1]->value] & 0xffff;
    state.mem[addr] = value;
}

void STIInstruction::execute(MachineState & state)
{
    uint32_t addr1 = computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);
    uint32_t addr2 = state.mem[addr1];
    uint32_t value = state.regs[operands[1]->value] & 0xffff;
    state.mem[addr2] = value;
}

void STRInstruction::execute(MachineState & state)
{
    uint32_t addr = computeBasePlusSOffset(state.regs[operands[2]->value], operands[3]->value,
            operands[3]->width);
    uint32_t value = state.regs[operands[1]->value] & 0xffff;
    state.mem[addr] = value;
}

void TRAPInstruction::execute(MachineState & state)
{
    state.regs[7] = state.pc;
    state.pc = operands[2]->value & 0xffff;
    state.psr &= 0x7fff;
}
