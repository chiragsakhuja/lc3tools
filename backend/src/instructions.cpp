#include <sstream>

#include "instructions.h"

using namespace lc3::core;

IOperand::IOperand(OperType type, std::string const & type_str, uint32_t width)
{
    this->type = type;
    this->type_str = type_str;
    this->width = width;
    this->value = 0;
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

uint32_t IInstruction::getNumOperands(void) const
{
    uint32_t ret = 0;
    for(PIOperand operand : operands) {
        if(operand->type != OperType::FIXED) {
            ret += 1;
        }
    }
    return ret;
}

void IInstruction::assignOperands(uint32_t encoded_inst)
{
    uint32_t cur_pos = 15;
    for(PIOperand op : operands) {
        if(op->type != OperType::FIXED) {
            op->value = utils::getBits(encoded_inst, cur_pos, cur_pos - op->width + 1);
        }
        cur_pos -= op->width;
    }
}

std::string IInstruction::toFormatString(void) const
{
    std::stringstream assembly;
    assembly << name;
    if(operands.size() > 0) {
        assembly << " ";
        std::string prefix = "";
        for(PIOperand operand : operands) {
            if(operand->type != OperType::FIXED) {
                assembly << prefix << operand->type_str;
                prefix = ", ";
            }
        }
    }
    return assembly.str();
}

std::string IInstruction::toValueString(void) const
{
    std::stringstream assembly;
    assembly << name << " ";
    std::string prefix = "";
    for(PIOperand operand : operands) {
        if(operand->type != OperType::FIXED) {
            std::string oper_str;
            if(operand->type == OperType::NUM || operand->type == OperType::LABEL) {
                if((operand->type == OperType::NUM && (std::static_pointer_cast<NumOperand>(operand))->sext) ||
                    operand->type == OperType::LABEL)
                {
                    oper_str = "#" + std::to_string((int32_t) utils::sextTo32(operand->value, operand->width));
                } else {
                    oper_str = "#" + std::to_string(operand->value);
                }
            } else if(operand->type == OperType::REG) {
                oper_str = "r" + std::to_string(operand->value);
            }
            assembly << prefix << oper_str;
            prefix = ", ";
        }
    }
    return assembly.str();
}

bool IOperand::isEqualType(OperType other) const
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

    instructions.push_back(std::make_shared<ADDRInstruction>());
    instructions.push_back(std::make_shared<ADDIInstruction>());
    instructions.push_back(std::make_shared<ANDRInstruction>());
    instructions.push_back(std::make_shared<ANDIInstruction>());
    instructions.push_back(std::make_shared<BRInstruction>());
    instructions.push_back(std::make_shared<BRnInstruction>());
    instructions.push_back(std::make_shared<BRzInstruction>());
    instructions.push_back(std::make_shared<BRpInstruction>());
    instructions.push_back(std::make_shared<BRnzInstruction>());
    instructions.push_back(std::make_shared<BRzpInstruction>());
    instructions.push_back(std::make_shared<BRnpInstruction>());
    instructions.push_back(std::make_shared<BRnzpInstruction>());
    instructions.push_back(std::make_shared<NOP0Instruction>());
    instructions.push_back(std::make_shared<NOP1Instruction>());
    instructions.push_back(std::make_shared<JMPInstruction>());
    instructions.push_back(std::make_shared<JSRInstruction>());
    instructions.push_back(std::make_shared<JSRRInstruction>());
    instructions.push_back(std::make_shared<LDInstruction>());
    instructions.push_back(std::make_shared<LDIInstruction>());
    instructions.push_back(std::make_shared<LDRInstruction>());
    instructions.push_back(std::make_shared<LEAInstruction>());
    instructions.push_back(std::make_shared<NOTInstruction>());
    instructions.push_back(std::make_shared<RETInstruction>());
    instructions.push_back(std::make_shared<RTIInstruction>());
    instructions.push_back(std::make_shared<STInstruction>());
    instructions.push_back(std::make_shared<STIInstruction>());
    instructions.push_back(std::make_shared<STRInstruction>());
    instructions.push_back(std::make_shared<TRAPInstruction>());
    instructions.push_back(std::make_shared<GETCInstruction>());
    instructions.push_back(std::make_shared<OUTInstruction>());
    instructions.push_back(std::make_shared<PUTSInstruction>());
    instructions.push_back(std::make_shared<INInstruction>());
    instructions.push_back(std::make_shared<PUTSPInstruction>());
    instructions.push_back(std::make_shared<HALTInstruction>());
}

uint32_t FixedOperand::encode(OldToken const * oper, uint32_t oper_count,
    std::map<std::string, uint32_t> const & regs, std::map<std::string, uint32_t> const & symbols,
    lc3::utils::AssemblerLogger & logger)
{
    (void) oper;
    (void) oper_count;
    (void) regs;
    (void) symbols;
    (void) logger;

    return value & ((1 << width) - 1);
}

uint32_t RegOperand::encode(OldToken const * oper, uint32_t oper_count, 
    std::map<std::string, uint32_t> const & regs, std::map<std::string, uint32_t> const & symbols,
    lc3::utils::AssemblerLogger & logger)
{
    (void) symbols;

    uint32_t token_val = regs.at(std::string(oper->str)) & ((1 << width) - 1);

    logger.printf(lc3::utils::PrintType::EXTRA, true, "%d.%d: reg %s => %s", oper->row_num, oper_count,
        oper->str.c_str(), utils::udecToBin(token_val, width).c_str());

    return token_val;
}

uint32_t NumOperand::encode(OldToken const * oper, uint32_t oper_count,
    std::map<std::string, uint32_t> const & regs, std::map<std::string, uint32_t> const & symbols,
    lc3::utils::AssemblerLogger & logger)
{
    (void) regs;
    (void) symbols;

    uint32_t token_val = oper->num & ((1 << width) - 1);

    if(sext) {
        if((int32_t) oper->num < -(1 << (width - 1)) || (int32_t) oper->num > ((1 << (width - 1)) - 1)) {
            logger.printfMessage(lc3::utils::PrintType::WARNING, oper, "immediate %d truncated to %d",
                oper->num, utils::sextTo32(token_val, width));
            logger.newline();
        }
    } else {
        if(oper->num > ((1 << width) - 1)) {
            logger.printfMessage(lc3::utils::PrintType::WARNING, oper, "immediate %d truncated to %u",
                oper->num, token_val);
            logger.newline();
        }
    }

    logger.printf(lc3::utils::PrintType::EXTRA, true, "%d.%d: imm %d => %s", oper->row_num, oper_count,
        oper->num, utils::udecToBin(token_val, width).c_str());

    return token_val;
}

uint32_t LabelOperand::encode(OldToken const * oper, uint32_t oper_count,
    std::map<std::string, uint32_t> const & regs, std::map<std::string, uint32_t> const & symbols,
    lc3::utils::AssemblerLogger & logger)
{
    (void) regs;

    auto search = symbols.find(oper->str);
    if(search == symbols.end()) {
        logger.printfMessage(lc3::utils::PrintType::ERROR, oper, "unknown label \'%s\'", oper->str.c_str());
        logger.newline();
        throw utils::exception("unknown label");
    }

    uint32_t token_val = (((int32_t) search->second) - (oper->pc + 1)) & ((1 << width) - 1);

    logger.printf(lc3::utils::PrintType::EXTRA, true, "%d.%d: label %s (0x%0.4x) => %s", oper->row_num,
        oper_count, oper->str.c_str(), search->second, utils::udecToBin((uint32_t) token_val, width).c_str());

    return token_val;
}

std::vector<PIEvent> ADDRInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr1_val = utils::sextTo32(state.regs[operands[2]->value], 16);
    uint32_t sr2_val = utils::sextTo32(state.regs[operands[4]->value], 16);
    uint32_t result = (sr1_val + sr2_val) & 0xffff;

    return std::vector<PIEvent> {
        std::make_shared<PSREvent>(utils::computePSRCC(result, state.psr)),
        std::make_shared<RegEvent>(dr, result)
    };
}

std::vector<PIEvent> ADDIInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr1_val = utils::sextTo32(state.regs[operands[2]->value], 16);
    uint32_t imm_val = utils::sextTo32(operands[4]->value, operands[4]->width);
    uint32_t result = (sr1_val + imm_val) & 0xffff;

    return std::vector<PIEvent> {
        std::make_shared<PSREvent>(utils::computePSRCC(result, state.psr)),
        std::make_shared<RegEvent>(dr, result)
    };
}

std::vector<PIEvent> ANDRInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr1_val = utils::sextTo32(state.regs[operands[2]->value], 16);
    uint32_t sr2_val = utils::sextTo32(state.regs[operands[4]->value], 16);
    uint32_t result = (sr1_val & sr2_val) & 0xffff;

    return std::vector<PIEvent> {
        std::make_shared<PSREvent>(utils::computePSRCC(result, state.psr)),
        std::make_shared<RegEvent>(dr, result)
    };
}

std::vector<PIEvent> ANDIInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr1_val = utils::sextTo32(state.regs[operands[2]->value], 16);
    uint32_t imm_val = utils::sextTo32(operands[4]->value, operands[4]->width);
    uint32_t result = (sr1_val & imm_val) & 0xffff;

    return std::vector<PIEvent> {
        std::make_shared<PSREvent>(utils::computePSRCC(result, state.psr)),
        std::make_shared<RegEvent>(dr, result)
    };
}

std::vector<PIEvent> BRInstruction::execute(MachineState const & state)
{
    std::vector<PIEvent> ret;
    uint32_t addr = utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);

    if((operands[1]->value & (state.psr & 0x0007)) != 0) {
        ret.push_back(std::make_shared<PCEvent>(addr));
    }

    return ret;
}

std::vector<PIEvent> JMPInstruction::execute(MachineState const & state)
{
    std::vector<PIEvent> ret {
        std::make_shared<PCEvent>(state.regs[operands[2]->value] & 0xffff)
    };
    if(operands[2]->value == 7) {
        ret.push_back(std::make_shared<CallbackEvent>(state.sub_exit_callback_v, state.sub_exit_callback));
    }
    return ret;
}

std::vector<PIEvent> JSRInstruction::execute(MachineState const & state)
{
    return std::vector<PIEvent> {
        std::make_shared<RegEvent>(7, state.pc & 0xffff),
        std::make_shared<PCEvent>(utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width)),
        std::make_shared<CallbackEvent>(state.sub_enter_callback_v, state.sub_enter_callback)
    };
}

std::vector<PIEvent> JSRRInstruction::execute(MachineState const & state)
{
    return std::vector<PIEvent> {
        std::make_shared<RegEvent>(7, state.pc & 0xffff),
        std::make_shared<PCEvent>(state.regs[operands[3]->value]),
        std::make_shared<CallbackEvent>(state.sub_enter_callback_v, state.sub_enter_callback)
    };
}

std::vector<PIEvent> LDInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t addr = utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);

    bool change_mem;
    PIEvent change;
    uint32_t value = state.readMem(addr, change_mem, change);

    std::vector<PIEvent> ret {
        std::make_shared<PSREvent>(utils::computePSRCC(value, state.psr)),
        std::make_shared<RegEvent>(dr, value)
    };

    if(change_mem) {
        ret.push_back(change);
    }

    return ret;
}

std::vector<PIEvent> LDIInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t addr1 = utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);

    bool change_mem1;
    PIEvent change1;
    uint32_t addr2 = state.readMem(addr1, change_mem1, change1);

    bool change_mem2;
    PIEvent change2;
    uint32_t value = state.readMem(addr2, change_mem2, change2);

    std::vector<PIEvent> ret {
        std::make_shared<PSREvent>(utils::computePSRCC(value, state.psr)),
        std::make_shared<RegEvent>(dr, value)
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

std::vector<PIEvent> LDRInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t addr = utils::computeBasePlusSOffset(state.regs[operands[2]->value], operands[3]->value,
            operands[3]->width);

    bool change_mem;
    PIEvent change;
    uint32_t value = state.readMem(addr, change_mem, change);

    std::vector<PIEvent> ret {
        std::make_shared<PSREvent>(utils::computePSRCC(value, state.psr)),
        std::make_shared<RegEvent>(dr, value)
    };

    if(change_mem) {
        ret.push_back(change);
    }

    return ret;
}

std::vector<PIEvent> LEAInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t addr = utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);

    return std::vector<PIEvent> {
        std::make_shared<PSREvent>(utils::computePSRCC(addr, state.psr)),
        std::make_shared<RegEvent>(dr, addr)
    };
}

std::vector<PIEvent> NOTInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr_val = utils::sextTo32(state.regs[operands[2]->value], operands[2]->width);
    uint32_t result = (~sr_val) & 0xffff;

    return std::vector<PIEvent> {
        std::make_shared<PSREvent>(utils::computePSRCC(result, state.psr)),
        std::make_shared<RegEvent>(dr, result)
    };
}

std::vector<PIEvent> RTIInstruction::execute(MachineState const & state)
{
    if((state.pc & 0x8000) == 0x0000) {
        bool pc_change_mem;
        PIEvent pc_change;
        uint32_t pc_value = state.readMem(state.regs[6], pc_change_mem, pc_change);

        bool psr_change_mem;
        PIEvent psr_change;
        uint32_t psr_value = state.readMem(state.regs[6] + 1, psr_change_mem, psr_change);

        std::vector<PIEvent> ret {
            std::make_shared<PCEvent>(pc_value),
            std::make_shared<PSREvent>(psr_value),
            std::make_shared<CallbackEvent>(state.interrupt_exit_callback_v, state.interrupt_exit_callback)
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
    return std::vector<PIEvent> {};
}

std::vector<PIEvent> STInstruction::execute(MachineState const & state)
{
    uint32_t addr = utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);
    uint32_t value = state.regs[operands[1]->value] & 0xffff;

    return std::vector<PIEvent> {
        std::make_shared<MemWriteEvent>(addr, value)
    };
}

std::vector<PIEvent> STIInstruction::execute(MachineState const & state)
{
    uint32_t addr1 = utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);
    uint32_t addr2 = state.mem[addr1].getValue();
    uint32_t value = state.regs[operands[1]->value] & 0xffff;

    return std::vector<PIEvent> {
        std::make_shared<MemWriteEvent>(addr2, value)
    };
}

std::vector<PIEvent> STRInstruction::execute(MachineState const & state)
{
    uint32_t addr = utils::computeBasePlusSOffset(state.regs[operands[2]->value], operands[3]->value,
        operands[3]->width);
    uint32_t value = state.regs[operands[1]->value] & 0xffff;

    return std::vector<PIEvent> {
        std::make_shared<MemWriteEvent>(addr, value)
    };
}

std::vector<PIEvent> TRAPInstruction::execute(MachineState const & state)
{
    return std::vector<PIEvent> {
        std::make_shared<PSREvent>(state.psr & 0x7fff),
        std::make_shared<RegEvent>(7, state.pc & 0xffff),
        std::make_shared<PCEvent>(state.mem[operands[2]->value].getValue() & 0xffff),
        std::make_shared<CallbackEvent>(state.sub_enter_callback_v, state.sub_enter_callback)
    };
}
