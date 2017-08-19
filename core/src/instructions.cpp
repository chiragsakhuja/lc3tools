#include <array>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <iostream>

#include "utils.h"

#include "tokens.h"

#include "printer.h"
#include "logger.h"

#include "statement.h"

#include "state.h"

#include "instructions.h"

core::IOperand::IOperand(OperType type, std::string const & type_str, uint32_t width)
{
    this->type = type;
    this->type_str = type_str;
    this->width = width;
    this->value = 0;
}

core::IInstruction::IInstruction(std::string const & name, std::vector<IOperand *> const & operands)
{
    this->name = name;
    this->operands = operands;
}

core::IInstruction::IInstruction(IInstruction const & that)
{
    this->name = that.name;
    for(IOperand * op : that.operands) {
        this->operands.push_back(op->clone());
    }
}

core::IInstruction::~IInstruction(void)
{
    for(uint32_t i = 0; i < operands.size(); i += 1) {
        delete operands[i];
    }
}

uint32_t core::IInstruction::getNumOperands(void) const
{
    uint32_t ret = 0;
    for(IOperand * operand : operands) {
        if(operand->type != OPER_TYPE_FIXED) {
            ret += 1;
        }
    }
    return ret;
}

void core::IInstruction::assignOperands(uint32_t encoded_inst)
{
    uint32_t cur_pos = 15;
    for(IOperand * op : operands) {
        if(op->type != OPER_TYPE_FIXED) {
            op->value = utils::getBits(encoded_inst, cur_pos, cur_pos - op->width + 1);
        }
        cur_pos -= op->width;
    }
}

std::string core::IInstruction::toFormatString(void) const
{
    std::stringstream assembly;
    assembly << name << " ";
    std::string prefix = "";
    for(IOperand * operand : operands) {
        if(operand->type != OPER_TYPE_FIXED) {
            assembly << prefix << operand->type_str;
            prefix = ", ";
        }
    }
    return assembly.str();
}

std::string core::IInstruction::toValueString(void) const
{
    std::stringstream assembly;
    assembly << name << " ";
    std::string prefix = "";
    for(IOperand * operand : operands) {
        if(operand->type != OPER_TYPE_FIXED) {
            std::string oper_str;
            if(operand->type == OPER_TYPE_NUM || operand->type == OPER_TYPE_LABEL) {
                if((operand->type == OPER_TYPE_NUM && ((NumOperand *) operand)->sext) ||
                    operand->type == OPER_TYPE_LABEL)
                {
                    oper_str = "#" + std::to_string((int32_t) utils::sextTo32(operand->value, operand->width));
                } else {
                    oper_str = "#" + std::to_string(operand->value);
                }
            } else if(operand->type == OPER_TYPE_REG) {
                oper_str = "r" + std::to_string(operand->value);
            }
            assembly << prefix << oper_str;
            prefix = ", ";
        }
    }
    return assembly.str();
}

bool core::IOperand::isEqualType(OperType other) const
{
    return type == other;
}

core::InstructionHandler::InstructionHandler(void)
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
    instructions.push_back(new GETCInstruction());
    instructions.push_back(new OUTInstruction());
    instructions.push_back(new PUTSInstruction());
    instructions.push_back(new INInstruction());
    instructions.push_back(new PUTSPInstruction());
    instructions.push_back(new HALTInstruction());
}

core::InstructionHandler::~InstructionHandler(void)
{
    for(uint32_t i = 0; i < instructions.size(); i += 1) {
        delete instructions[i];
    }
}

uint32_t core::FixedOperand::encode(Token const * oper, uint32_t oper_count, std::map<std::string, uint32_t> const & regs,
    std::map<std::string, uint32_t> const & symbols, AssemblerLogger & logger)
{
    (void) oper;
    (void) oper_count;
    (void) regs;
    (void) symbols;
    (void) logger;

    return value & ((1 << width) - 1);
}

uint32_t core::RegOperand::encode(Token const * oper, uint32_t oper_count, std::map<std::string, uint32_t> const & regs,
    std::map<std::string, uint32_t> const & symbols, AssemblerLogger & logger)
{
    (void) symbols;

    uint32_t token_val = regs.at(std::string(oper->str)) & ((1 << width) - 1);

    logger.printf(PRINT_TYPE_EXTRA, true, "%d.%d: reg %s => %s", oper->row_num, oper_count, oper->str.c_str(),
        utils::udecToBin(token_val, width).c_str());

    return token_val;
}

uint32_t core::NumOperand::encode(Token const * oper, uint32_t oper_count, std::map<std::string, uint32_t> const & regs,
    std::map<std::string, uint32_t> const & symbols, AssemblerLogger & logger)
{
    (void) regs;
    (void) symbols;

    uint32_t token_val = oper->num & ((1 << width) - 1);

    if(sext) {
        if((int32_t) oper->num < -(1 << (width - 1)) || (int32_t) oper->num > ((1 << (width - 1)) - 1)) {
            logger.printfMessage(PRINT_TYPE_WARNING, oper, "immediate %d truncated to %d", oper->num,
                utils::sextTo32(token_val, width));
            logger.newline();
        }
    } else {
        if(oper->num > ((1 << width) - 1)) {
            logger.printfMessage(PRINT_TYPE_WARNING, oper, "immediate %d truncated to %u",
                oper->num, token_val);
            logger.newline();
        }
    }

    logger.printf(PRINT_TYPE_EXTRA, true, "%d.%d: imm %d => %s", oper->row_num, oper_count, oper->num,
        utils::udecToBin(token_val, width).c_str());

    return token_val;
}

uint32_t core::LabelOperand::encode(Token const * oper, uint32_t oper_count, std::map<std::string, uint32_t> const & regs,
    std::map<std::string, uint32_t> const & symbols, AssemblerLogger & logger)
{
    (void) regs;

    auto search = symbols.find(oper->str);
    if(search == symbols.end()) {
        logger.printfMessage(PRINT_TYPE_ERROR, oper, "unknown label \'%s\'", oper->str.c_str());
        logger.newline();
        throw utils::exception("unknown label");
    }

    uint32_t token_val = (((int32_t) search->second) - (oper->pc + 1)) & ((1 << width) - 1);

    logger.printf(PRINT_TYPE_EXTRA, true, "%d.%d: label %s (0x%0.4x) => %s", oper->row_num, oper_count,
        oper->str.c_str(), search->second, utils::udecToBin((uint32_t) token_val, width).c_str());

    return token_val;
}

std::vector<core::IStateChange const *> core::ADDRInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr1_val = utils::sextTo32(state.regs[operands[2]->value], 16);
    uint32_t sr2_val = utils::sextTo32(state.regs[operands[4]->value], 16);
    uint32_t result = (sr1_val + sr2_val) & 0xffff;

    return std::vector<IStateChange const *> {
        new PSRStateChange(utils::computePSRCC(result, state.psr)),
        new RegStateChange(dr, result)
    };
}

std::vector<core::IStateChange const *> core::ADDIInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr1_val = utils::sextTo32(state.regs[operands[2]->value], 16);
    uint32_t imm_val = utils::sextTo32(operands[4]->value, operands[4]->width);
    uint32_t result = (sr1_val + imm_val) & 0xffff;

    return std::vector<IStateChange const *> {
        new PSRStateChange(utils::computePSRCC(result, state.psr)),
        new RegStateChange(dr, result)
    };
}

std::vector<core::IStateChange const *> core::ANDRInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr1_val = utils::sextTo32(state.regs[operands[2]->value], 16);
    uint32_t sr2_val = utils::sextTo32(state.regs[operands[4]->value], 16);
    uint32_t result = (sr1_val & sr2_val) & 0xffff;

    return std::vector<IStateChange const *> {
        new PSRStateChange(utils::computePSRCC(result, state.psr)),
        new RegStateChange(dr, result)
    };
}

std::vector<core::IStateChange const *> core::ANDIInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr1_val = utils::sextTo32(state.regs[operands[2]->value], 16);
    uint32_t imm_val = utils::sextTo32(operands[4]->value, operands[4]->width);
    uint32_t result = (sr1_val & imm_val) & 0xffff;

    return std::vector<IStateChange const *> {
        new PSRStateChange(utils::computePSRCC(result, state.psr)),
        new RegStateChange(dr, result)
    };
}

std::vector<core::IStateChange const *> core::BRInstruction::execute(MachineState const & state)
{
    std::vector<IStateChange const *> ret;
    uint32_t addr = utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);

    if((operands[1]->value & (state.psr & 0x0007)) != 0) {
        ret.push_back(new PCStateChange(addr));
    }

    return ret;
}

std::vector<core::IStateChange const *> core::JMPInstruction::execute(MachineState const & state)
{
    return std::vector<IStateChange const *> {
        new PCStateChange(state.regs[operands[2]->value] & 0xffff)
    };
}

std::vector<core::IStateChange const *> core::JSRInstruction::execute(MachineState const & state)
{
    return std::vector<IStateChange const *> {
        new RegStateChange(7, state.pc & 0xffff),
        new PCStateChange(utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width))
    };
}

std::vector<core::IStateChange const *> core::JSRRInstruction::execute(MachineState const & state)
{
    return std::vector<IStateChange const *> {
        new RegStateChange(7, state.pc & 0xffff),
        new PCStateChange(state.regs[operands[3]->value])
    };
}

std::vector<core::IStateChange const *> core::LDInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t addr = utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);

    bool change_mem;
    IStateChange * change;
    uint32_t value = state.readMem(addr, change_mem, change);

    std::vector<IStateChange const *> ret {
        new PSRStateChange(utils::computePSRCC(value, state.psr)),
        new RegStateChange(dr, value)
    };

    if(change_mem) {
        ret.push_back(change);
    }

    return ret;
}

std::vector<core::IStateChange const *> core::LDIInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t addr1 = utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);

    bool change_mem1;
    IStateChange * change1;
    uint32_t addr2 = state.readMem(addr1, change_mem1, change1);

    bool change_mem2;
    IStateChange * change2;
    uint32_t value = state.readMem(addr2, change_mem2, change2);

    std::vector<IStateChange const *> ret {
        new PSRStateChange(utils::computePSRCC(value, state.psr)),
        new RegStateChange(dr, value)
    };

    // TODO: what if the changes are the same?
    if(change_mem1) {
        ret.push_back(change1);
    }

    if(change_mem2) {
        ret.push_back(change2);
    }


    return ret;
}

std::vector<core::IStateChange const *> core::LDRInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t addr = utils::computeBasePlusSOffset(state.regs[operands[2]->value], operands[3]->value,
            operands[3]->width);

    bool change_mem;
    IStateChange * change;
    uint32_t value = state.readMem(addr, change_mem, change);

    std::vector<IStateChange const *> ret {
        new PSRStateChange(utils::computePSRCC(value, state.psr)),
        new RegStateChange(dr, value)
    };

    if(change_mem) {
        ret.push_back(change);
    }

    return ret;
}

std::vector<core::IStateChange const *> core::LEAInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t addr = utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);

    return std::vector<IStateChange const *> {
        new PSRStateChange(utils::computePSRCC(addr, state.psr)),
        new RegStateChange(dr, addr)
    };
}

std::vector<core::IStateChange const *> core::NOTInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr_val = utils::sextTo32(state.regs[operands[2]->value], operands[2]->width);
    uint32_t result = (~sr_val) & 0xffff;

    return std::vector<IStateChange const *> {
        new PSRStateChange(utils::computePSRCC(result, state.psr)),
        new RegStateChange(dr, result)
    };
}

std::vector<core::IStateChange const *> core::RTIInstruction::execute(MachineState const & state)
{
    if((state.pc & 0x8000) == 0x0000) {
        bool pc_change_mem;
        IStateChange * pc_change;
        uint32_t pc_value = state.readMem(state.regs[6], pc_change_mem, pc_change);

        bool psr_change_mem;
        IStateChange * psr_change;
        uint32_t psr_value = state.readMem(state.regs[6] + 1, psr_change_mem, psr_change);

        std::vector<IStateChange const *> ret {
            new PCStateChange(pc_value),
            new PSRStateChange(psr_value)
        };

        if(pc_change_mem) {
            ret.push_back(pc_change);
        }

        if(psr_change_mem) {
            ret.push_back(psr_change);
        }

        return ret;
    }

    // TODO: trigger exception
    return std::vector<IStateChange const *> {};
}

std::vector<core::IStateChange const *> core::STInstruction::execute(MachineState const & state)
{
    uint32_t addr = utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);
    uint32_t value = state.regs[operands[1]->value] & 0xffff;

    return std::vector<IStateChange const *> {
        new MemWriteStateChange(addr, value)
    };
}

std::vector<core::IStateChange const *> core::STIInstruction::execute(MachineState const & state)
{
    uint32_t addr1 = utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);
    uint32_t addr2 = state.mem[addr1].getValue();
    uint32_t value = state.regs[operands[1]->value] & 0xffff;

    return std::vector<IStateChange const *> {
        new MemWriteStateChange(addr2, value)
    };
}

std::vector<core::IStateChange const *> core::STRInstruction::execute(MachineState const & state)
{
    uint32_t addr = utils::computeBasePlusSOffset(state.regs[operands[2]->value], operands[3]->value,
        operands[3]->width);
    uint32_t value = state.regs[operands[1]->value] & 0xffff;

    return std::vector<IStateChange const *> {
        new MemWriteStateChange(addr, value)
    };
}

std::vector<core::IStateChange const *> core::TRAPInstruction::execute(MachineState const & state)
{
    return std::vector<IStateChange const *> {
        new PSRStateChange(state.psr & 0x7fff),
        new RegStateChange(7, state.pc & 0xffff),
        new PCStateChange(state.mem[operands[2]->value].getValue() & 0xffff)
    };
}
