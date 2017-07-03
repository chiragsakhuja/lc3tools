#include <array>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include <iostream>

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

Instruction::Instruction(Instruction const & that)
{
    this->name = that.name;
    for(Operand * op : that.operands) {
        this->operands.push_back(op->clone());
    }
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
    instructions.push_back(new BRInstruction());
    instructions.push_back(new BRnInstruction());
    instructions.push_back(new BRzInstruction());
    instructions.push_back(new BRpInstruction());
    instructions.push_back(new BRnzInstruction());
    instructions.push_back(new BRzpInstruction());
    instructions.push_back(new BRnpInstruction());
    instructions.push_back(new BRnzpInstruction());
    instructions.push_back(new NOP0Instruction());
    instructions.push_back(new NOP1Instruction());
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
    instructions.push_back(new HALTInstruction());
}

InstructionHandler::~InstructionHandler(void)
{
    for(uint32_t i = 0; i < instructions.size(); i += 1) {
        delete instructions[i];
    }
}

// TODO: remove all these nasty parameters (maybe by returning an error message chain?)
uint32_t FixedOperand::encode(bool log_enable, AssemblerLogger const & logger,
    std::string const & filename, std::string const & line, Token const * inst,
    Token const * operand, uint32_t oper_count, 
    std::map<std::string, uint32_t> const & registers,
    std::map<std::string, uint32_t> const & labels)
{
    (void) log_enable;
    (void) logger;
    (void) filename;
    (void) line;
    (void) inst;
    (void) operand;
    (void) oper_count;
    (void) registers;
    (void) labels;

    return value & ((1 << width) - 1);
}

uint32_t RegOperand::encode(bool log_enable, AssemblerLogger const & logger,
    std::string const & filename, std::string const & line, Token const * inst,
    Token const * operand, uint32_t oper_count, 
    std::map<std::string, uint32_t> const & registers,
    std::map<std::string, uint32_t> const & labels)
{
    (void) filename;
    (void) line;
    (void) labels;

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
    (void) registers;
    (void) labels;

    uint32_t token_val = operand->num & ((1 << width) - 1);

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
    (void) registers;

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

    if(log_enable) {
        logger.printf(PRINT_TYPE_DEBUG, true, "%d.%d: label %s (0x%0.4x) => %s", inst->row_num, oper_count,
            operand->str.c_str(), search->second, udecToBin((uint32_t) token_val, width).c_str());
    }

    return token_val;
}

std::vector<IStateChange const *> ADDRInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr1_val = sextTo32(state.regs[operands[2]->value], 16);
    uint32_t sr2_val = sextTo32(state.regs[operands[4]->value], 16);
    uint32_t result = (sr1_val + sr2_val) & 0xffff;

    return std::vector<IStateChange const *> {
        new PSRStateChange(computePSRCC(result, state.psr)),
        new RegStateChange(dr, result)
    };
}

std::vector<IStateChange const *> ADDIInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr1_val = sextTo32(state.regs[operands[2]->value], 16);
    uint32_t imm_val = sextTo32(operands[4]->value, operands[4]->width);
    uint32_t result = (sr1_val + imm_val) & 0xffff;

    return std::vector<IStateChange const *> {
        new PSRStateChange(computePSRCC(result, state.psr)),
        new RegStateChange(dr, result)
    };
}

std::vector<IStateChange const *> ANDRInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr1_val = sextTo32(state.regs[operands[2]->value], 16);
    uint32_t sr2_val = sextTo32(state.regs[operands[4]->value], 16);
    uint32_t result = (sr1_val & sr2_val) & 0xffff;

    return std::vector<IStateChange const *> {
        new PSRStateChange(computePSRCC(result, state.psr)),
        new RegStateChange(dr, result)
    };
}

std::vector<IStateChange const *> ANDIInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr1_val = sextTo32(state.regs[operands[2]->value], 16);
    uint32_t imm_val = sextTo32(operands[4]->value, operands[4]->width);
    uint32_t result = (sr1_val & imm_val) & 0xffff;

    return std::vector<IStateChange const *> {
        new PSRStateChange(computePSRCC(result, state.psr)),
        new RegStateChange(dr, result)
    };
}

std::vector<IStateChange const *> BRInstruction::execute(MachineState const & state)
{
    std::vector<IStateChange const *> ret;
    uint32_t addr = computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);

    if((operands[1]->value & (state.psr & 0x0007)) != 0) {
        ret.push_back(new PCStateChange(addr));
    }

    return ret;
}

std::vector<IStateChange const *> JMPInstruction::execute(MachineState const & state)
{
    return std::vector<IStateChange const *> {
        new PCStateChange(state.regs[operands[2]->value] & 0xffff)
    };
}

std::vector<IStateChange const *> JSRInstruction::execute(MachineState const & state)
{
    return std::vector<IStateChange const *> {
        new RegStateChange(7, state.pc & 0xffff),
        new PCStateChange(computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width))
    };
}

std::vector<IStateChange const *> JSRRInstruction::execute(MachineState const & state)
{
    return std::vector<IStateChange const *> {
        new RegStateChange(7, state.pc & 0xffff),
        new PCStateChange(state.regs[operands[3]->value])
    };
}

std::vector<IStateChange const *> LDInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t addr = computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);
    uint32_t value = state.mem[addr];

    return std::vector<IStateChange const *> {
        new PSRStateChange(computePSRCC(value, state.psr)),
        new RegStateChange(dr, value)
    };
}

std::vector<IStateChange const *> LDIInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t addr1 = computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);
    uint32_t addr2 = state.mem[addr1];
    uint32_t value = state.mem[addr2];

    return std::vector<IStateChange const *> {
        new PSRStateChange(computePSRCC(value, state.psr)),
        new RegStateChange(dr, value)
    };
}

std::vector<IStateChange const *> LDRInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t addr = computeBasePlusSOffset(state.regs[operands[2]->value], operands[3]->value,
            operands[3]->width);
    uint32_t value = state.mem[addr];

    return std::vector<IStateChange const *> {
        new PSRStateChange(computePSRCC(value, state.psr)),
        new RegStateChange(dr, value)
    };
}

std::vector<IStateChange const *> LEAInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t addr = computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);

    return std::vector<IStateChange const *> {
        new PSRStateChange(computePSRCC(addr, state.psr)),
        new RegStateChange(dr, addr)
    };
}

std::vector<IStateChange const *> NOTInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr_val = sextTo32(state.regs[operands[2]->value], operands[2]->width);
    uint32_t result = (~sr_val) & 0xffff;

    return std::vector<IStateChange const *> {
        new PSRStateChange(computePSRCC(result, state.psr)),
        new RegStateChange(dr, result)
    };
}

std::vector<IStateChange const *> RTIInstruction::execute(MachineState const & state)
{
    // TODO: update state to have system stack pointer (see how existing simulator does it)
    (void) state;
    return std::vector<IStateChange const *> {};
}

std::vector<IStateChange const *> STInstruction::execute(MachineState const & state)
{
    uint32_t addr = computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);
    uint32_t value = state.regs[operands[1]->value] & 0xffff;

    return std::vector<IStateChange const *> {
        new MemStateChange(addr, value)
    };
}

std::vector<IStateChange const *> STIInstruction::execute(MachineState const & state)
{
    uint32_t addr1 = computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);
    uint32_t addr2 = state.mem[addr1];
    uint32_t value = state.regs[operands[1]->value] & 0xffff;

    return std::vector<IStateChange const *> {
        new MemStateChange(addr2, value)
    };
}

std::vector<IStateChange const *> STRInstruction::execute(MachineState const & state)
{
    uint32_t addr = computeBasePlusSOffset(state.regs[operands[2]->value], operands[3]->value,
            operands[3]->width);
    uint32_t value = state.regs[operands[1]->value] & 0xffff;

    return std::vector<IStateChange const *> {
        new MemStateChange(addr, value)
    };
}

std::vector<IStateChange const *> TRAPInstruction::execute(MachineState const & state)
{
    return std::vector<IStateChange const *> {
        new PSRStateChange(state.psr & 0x7fff),
        new RegStateChange(7, state.pc & 0xffff),
        new PCStateChange(state.mem[operands[2]->value] & 0xffff)
    };
}
