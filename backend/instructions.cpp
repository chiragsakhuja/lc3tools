/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
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
            op->value = lc3::utils::getBits(encoded_inst, cur_pos, cur_pos - op->width + 1);
        }
        cur_pos -= op->width;
    }
}

std::string IInstruction::toFormatString(void) const
{
    std::stringstream assembly;
    assembly << name;
    if(getNumOperands() > 0) {
        assembly << " ";
    }
    std::string prefix = "";
    for(PIOperand operand : operands) {
        if(operand->type != OperType::FIXED) {
            assembly << prefix << operand->type_str;
            prefix = ", ";
        }
    }
    return assembly.str();
}

std::string IInstruction::toValueString(void) const
{
    std::stringstream assembly;
    assembly << name;
    if(getNumOperands() > 0) {
        assembly << " ";
    }
    std::string prefix = "";
    for(PIOperand operand : operands) {
        if(operand->type != OperType::FIXED) {
            std::string oper_str;
            if(operand->type == OperType::NUM || operand->type == OperType::LABEL) {
                if((operand->type == OperType::NUM && std::static_pointer_cast<NumOperand>(operand)->sext) ||
                    operand->type == OperType::LABEL)
                {
                    oper_str = "#" + std::to_string((int32_t) lc3::utils::sextTo32(operand->value, operand->width));
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

std::vector<PIEvent> IInstruction::buildSysCallEnterHelper(MachineState const & state, uint32_t vector_id,
    MachineState::SysCallType sys_call_type, std::function<uint32_t(uint32_t)> computeNewPSRValue)
{
    bool change_mem;
    PIEvent change;
    uint32_t vector = state.readMemEvent(vector_id, change_mem, change);

    bool current_psr_change_mem;
    PIEvent current_psr_change;
    uint32_t current_psr_value = state.readMemEvent(PSR, current_psr_change_mem, current_psr_change);
    uint32_t new_psr_value = computeNewPSRValue(current_psr_value) & 0xffff;

    bool bsp_change_mem;
    PIEvent bsp_change;
    uint32_t sp = state.readMemEvent(BSP, bsp_change_mem, bsp_change);
    if((current_psr_value & 0x8000) == 0x0000) {
        sp = state.regs[6];
    }

    uint32_t top_of_stack = (sp - 2) & 0xffff;
    if(top_of_stack > MMIO_START) {
        state.logger.printf(lc3::utils::PrintType::P_ERROR, true, "triple fault: invalid top of stack address 0x%0.4x",
            top_of_stack);
        throw utils::exception("triple fault: invalid top of stack address");
    }

    std::vector<PIEvent> ret;
    if(((current_psr_value & 0x8000) ^ (new_psr_value & 0x8000)) == 0x8000) {
        ret.emplace_back(std::make_shared<SwapSPEvent>());
    }

    ret.emplace_back(std::make_shared<RegEvent>(6, sp - 1));
    ret.emplace_back(std::make_shared<MemWriteEvent>(sp - 1, current_psr_value));
    ret.emplace_back(std::make_shared<RegEvent>(6, sp - 2));
    ret.emplace_back(std::make_shared<MemWriteEvent>(sp - 2, state.pc));
    ret.emplace_back(std::make_shared<PSREvent>(new_psr_value));
    ret.emplace_back(std::make_shared<PCEvent>(vector & 0xffff));

    if(change_mem) {
        ret.push_back(change);
    }

    if(current_psr_change_mem) {
        ret.push_back(current_psr_change);
    }

    if(bsp_change_mem) {
        ret.push_back(bsp_change);
    }

    if(sys_call_type == MachineState::SysCallType::TRAP) {
        ret.push_back(std::make_shared<CallbackEvent>(state.sub_enter_callback_v, state.sub_enter_callback));
    } else {
        ret.push_back(std::make_shared<CallbackEvent>(state.interrupt_enter_callback_v, state.interrupt_enter_callback));
    }
    ret.push_back(std::make_shared<PushSysCallTypeEvent>(sys_call_type));

    return ret;
}

std::vector<PIEvent> IInstruction::buildSysCallExitHelper(MachineState const & state,
    MachineState::SysCallType sys_call_type)
{
    bool pc_change_mem;
    PIEvent pc_change;
    uint32_t pc_value = state.readMemEvent(state.regs[6], pc_change_mem, pc_change);

    bool new_psr_change_mem;
    PIEvent new_psr_change;
    uint32_t new_psr_value = state.readMemEvent(state.regs[6] + 1, new_psr_change_mem, new_psr_change);

    bool current_psr_change_mem;
    PIEvent current_psr_change;
    uint32_t current_psr_value = state.readMemEvent(PSR, current_psr_change_mem, current_psr_change);

    bool bsp_change_mem;
    PIEvent bsp_change;
    uint32_t sp = state.readMemEvent(BSP, bsp_change_mem, bsp_change);
    if((current_psr_value & 0x8000) == 0x0000) {
        sp = state.regs[6];
    }

    uint32_t bottom_of_stack = (sp + 2) & 0xffff;
    if(bottom_of_stack >= MMIO_START) {
        state.logger.printf(lc3::utils::PrintType::P_ERROR, true, "triple fault: invalid bottom of stack address 0x%0.4x",
            bottom_of_stack);
        throw utils::exception("triple fault: invalid bottom of stack address");
    }

    if((current_psr_value & 0x8000) == 0x8000) {
        // if an RTI is encountered in user mode, trigger an access violation exception
        return buildSysCallEnterHelper(state, INTEX_TABLE_START + 0, MachineState::SysCallType::INT);
    }

    std::vector<PIEvent> ret {
        std::make_shared<PCEvent>(pc_value),
        std::make_shared<RegEvent>(6, sp + 1),
        std::make_shared<PSREvent>(new_psr_value),
        std::make_shared<RegEvent>(6, sp + 2),
    };

    if(((current_psr_value & 0x8000) ^ (new_psr_value & 0x8000)) == 0x8000) {
        ret.emplace_back(std::make_shared<SwapSPEvent>());
    }

    if(pc_change_mem) {
        ret.push_back(pc_change);
    }

    if(new_psr_change_mem) {
        ret.push_back(new_psr_change);
    }

    if(current_psr_change_mem) {
        ret.push_back(current_psr_change);
    }

    if(bsp_change_mem) {
        ret.push_back(bsp_change);
    }

    if(sys_call_type == MachineState::SysCallType::TRAP) {
        ret.push_back(std::make_shared<CallbackEvent>(state.sub_exit_callback_v, state.sub_exit_callback));
    } else {
        ret.push_back(std::make_shared<CallbackEvent>(state.interrupt_exit_callback_v, state.interrupt_exit_callback));
    }
    ret.push_back(std::make_shared<PopSysCallTypeEvent>());

    return ret;
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

    instructions.push_back(std::make_shared<ADDRegInstruction>());
    instructions.push_back(std::make_shared<ADDImmInstruction>());
    instructions.push_back(std::make_shared<ANDRegInstruction>());
    instructions.push_back(std::make_shared<ANDImmInstruction>());
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
    instructions.push_back(std::make_shared<PUTCInstruction>());
    instructions.push_back(std::make_shared<PUTSInstruction>());
    instructions.push_back(std::make_shared<INInstruction>());
    instructions.push_back(std::make_shared<PUTSPInstruction>());
    instructions.push_back(std::make_shared<HALTInstruction>());
}

lc3::optional<uint32_t> FixedOperand::encode(asmbl::StatementNew const & statement, asmbl::StatementPiece const & piece,
    SymbolTable const & regs, SymbolTable const & symbols, lc3::utils::AssemblerLogger & logger)
{
    (void) statement;
    (void) piece;
    (void) regs;
    (void) symbols;
    (void) logger;

    return value & ((1 << width) - 1);
}

lc3::optional<uint32_t> RegOperand::encode(asmbl::StatementNew const & statement, asmbl::StatementPiece const & piece,
    SymbolTable const & regs, SymbolTable const & symbols, lc3::utils::AssemblerLogger & logger)
{
    (void) statement;
    (void) symbols;

    uint32_t token_val = regs.at(utils::toLower(piece.str)) & ((1 << width) - 1);

    logger.printf(lc3::utils::PrintType::P_EXTRA, true, "  reg %s := %s",
        piece.str.c_str(), lc3::utils::udecToBin(token_val, width).c_str());

    return token_val;
}

lc3::optional<uint32_t> NumOperand::encode(asmbl::StatementNew const & statement, asmbl::StatementPiece const & piece,
    SymbolTable const & regs, SymbolTable const & symbols, lc3::utils::AssemblerLogger & logger)
{
    using namespace lc3::utils;

    (void) regs;
    (void) symbols;

    auto ret = getNum(statement, piece, this->width, this->sext, logger, true);

    if(! ret) {
        throw lc3::utils::exception("invalid immediate");
    }

    logger.printf(lc3::utils::PrintType::P_EXTRA, true, "  imm %d := %s", piece.num,
        lc3::utils::udecToBin(*ret, width).c_str());

    return *ret;
}

lc3::optional<uint32_t> LabelOperand::encode(asmbl::StatementNew const & statement, asmbl::StatementPiece const & piece,
    SymbolTable const & regs, SymbolTable const & symbols, lc3::utils::AssemblerLogger & logger)
{
    using namespace lc3::utils;
    using namespace lc3::core::asmbl;

    (void) regs;

    if(piece.type == StatementPiece::Type::NUM) {
        return NumOperand(this->width, true).encode(statement, piece, regs, symbols, logger);
    } else {
        auto search = symbols.find(utils::toLower(piece.str));
        if(search == symbols.end()) {
            logger.asmPrintf(PrintType::P_ERROR, statement, piece, "could not find label");
            logger.newline();
            return {};
        }

        asmbl::StatementPiece num_piece = piece;
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

std::vector<PIEvent> ADDRegInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr1_val = lc3::utils::sextTo32(state.regs[operands[2]->value], 16);
    uint32_t sr2_val = lc3::utils::sextTo32(state.regs[operands[4]->value], 16);
    uint32_t result = (sr1_val + sr2_val) & 0xffff;

    bool psr_change_mem;
    PIEvent psr_change;
    uint32_t psr_value = state.readMemEvent(PSR, psr_change_mem, psr_change);

    std::vector<PIEvent> ret {
        std::make_shared<PSREvent>(lc3::utils::computePSRCC(result, psr_value)),
        std::make_shared<RegEvent>(dr, result)
    };

    if(psr_change_mem) {
        ret.push_back(psr_change);
    }

    return ret;
}

std::vector<PIEvent> ADDImmInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr1_val = lc3::utils::sextTo32(state.regs[operands[2]->value], 16);
    uint32_t imm_val = lc3::utils::sextTo32(operands[4]->value, operands[4]->width);
    uint32_t result = (sr1_val + imm_val) & 0xffff;

    bool psr_change_mem;
    PIEvent psr_change;
    uint32_t psr_value = state.readMemEvent(PSR, psr_change_mem, psr_change);

    std::vector<PIEvent> ret {
        std::make_shared<PSREvent>(lc3::utils::computePSRCC(result, psr_value)),
        std::make_shared<RegEvent>(dr, result)
    };

    if(psr_change_mem) {
        ret.push_back(psr_change);
    }

    return ret;
}

std::vector<PIEvent> ANDRegInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr1_val = lc3::utils::sextTo32(state.regs[operands[2]->value], 16);
    uint32_t sr2_val = lc3::utils::sextTo32(state.regs[operands[4]->value], 16);
    uint32_t result = (sr1_val & sr2_val) & 0xffff;

    bool psr_change_mem;
    PIEvent psr_change;
    uint32_t psr_value = state.readMemEvent(PSR, psr_change_mem, psr_change);

    std::vector<PIEvent> ret {
        std::make_shared<PSREvent>(lc3::utils::computePSRCC(result, psr_value)),
        std::make_shared<RegEvent>(dr, result)
    };

    if(psr_change_mem) {
        ret.push_back(psr_change);
    }

    return ret;
}

std::vector<PIEvent> ANDImmInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr1_val = lc3::utils::sextTo32(state.regs[operands[2]->value], 16);
    uint32_t imm_val = lc3::utils::sextTo32(operands[4]->value, operands[4]->width);
    uint32_t result = (sr1_val & imm_val) & 0xffff;

    bool psr_change_mem;
    PIEvent psr_change;
    uint32_t psr_value = state.readMemEvent(PSR, psr_change_mem, psr_change);

    std::vector<PIEvent> ret {
        std::make_shared<PSREvent>(lc3::utils::computePSRCC(result, psr_value)),
        std::make_shared<RegEvent>(dr, result)
    };

    if(psr_change_mem) {
        ret.push_back(psr_change);
    }

    return ret;
}

std::vector<PIEvent> BRInstruction::execute(MachineState const & state)
{
    std::vector<PIEvent> ret;
    uint32_t addr = lc3::utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);

    bool psr_change_mem;
    PIEvent psr_change;
    uint32_t psr_value = state.readMemEvent(PSR, psr_change_mem, psr_change);

    if((operands[1]->value & (psr_value & 0x0007)) != 0) {
        ret.push_back(std::make_shared<PCEvent>(addr));
    }

    if(psr_change_mem) {
        ret.push_back(psr_change);
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
        std::make_shared<PCEvent>(lc3::utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width)),
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
    uint32_t addr = lc3::utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);

    bool psr_change_mem;
    PIEvent psr_change;
    uint32_t psr_value = state.readMemEvent(PSR, psr_change_mem, psr_change);

    if(! state.ignore_privilege && ((addr <= SYSTEM_END || addr >= MMIO_START) && (psr_value & 0x8000) != 0x0000)) {
        return buildSysCallEnterHelper(state, INTEX_TABLE_START + 0, MachineState::SysCallType::INT);
    }

    bool change_mem;
    PIEvent change;
    uint32_t value = state.readMemEvent(addr, change_mem, change);


    std::vector<PIEvent> ret {
        std::make_shared<PSREvent>(lc3::utils::computePSRCC(value, psr_value)),
        std::make_shared<RegEvent>(dr, value)
    };

    if(change_mem) {
        ret.push_back(change);
    }

    if(psr_change_mem) {
        ret.push_back(psr_change);
    }

    return ret;
}

std::vector<PIEvent> LDIInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t addr1 = lc3::utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);

    bool change_mem1;
    PIEvent change1;
    uint32_t addr2 = state.readMemEvent(addr1, change_mem1, change1);

    bool change_mem2;
    PIEvent change2;
    uint32_t value = state.readMemEvent(addr2, change_mem2, change2);

    bool psr_change_mem;
    PIEvent psr_change;
    uint32_t psr_value = state.readMemEvent(PSR, psr_change_mem, psr_change);

    if(! state.ignore_privilege && (((addr1 <= SYSTEM_END || addr1 >= MMIO_START)
        || (addr2 <= SYSTEM_END || addr2 >= MMIO_START)) && (psr_value & 0x8000) != 0x0000))
    {
        return buildSysCallEnterHelper(state, INTEX_TABLE_START + 0, MachineState::SysCallType::INT);
    }

    std::vector<PIEvent> ret {
        std::make_shared<PSREvent>(lc3::utils::computePSRCC(value, psr_value)),
        std::make_shared<RegEvent>(dr, value)
    };

    // TODO: what if the changes are the same?
    if(change_mem1) {
        ret.push_back(change1);
    }

    if(change_mem2) {
        ret.push_back(change2);
    }

    if(psr_change_mem) {
        ret.push_back(psr_change);
    }

    return ret;
}

std::vector<PIEvent> LDRInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t addr = lc3::utils::computeBasePlusSOffset(state.regs[operands[2]->value], operands[3]->value,
            operands[3]->width);

    bool psr_change_mem;
    PIEvent psr_change;
    uint32_t psr_value = state.readMemEvent(PSR, psr_change_mem, psr_change);

    if(! state.ignore_privilege && ((addr <= SYSTEM_END || addr >= MMIO_START) && (psr_value & 0x8000) != 0x0000)) {
        return buildSysCallEnterHelper(state, INTEX_TABLE_START + 0, MachineState::SysCallType::INT);
    }

    bool change_mem;
    PIEvent change;
    uint32_t value = state.readMemEvent(addr, change_mem, change);

    std::vector<PIEvent> ret {
        std::make_shared<PSREvent>(lc3::utils::computePSRCC(value, psr_value)),
        std::make_shared<RegEvent>(dr, value)
    };

    if(change_mem) {
        ret.push_back(change);
    }

    if(psr_change_mem) {
        ret.push_back(psr_change);
    }

    return ret;
}

std::vector<PIEvent> LEAInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t addr = lc3::utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);

    std::vector<PIEvent> ret {
        std::make_shared<RegEvent>(dr, addr)
    };

    return ret;
}

std::vector<PIEvent> NOTInstruction::execute(MachineState const & state)
{
    uint32_t dr = operands[1]->value;
    uint32_t sr_val = lc3::utils::sextTo32(state.regs[operands[2]->value], 16);
    uint32_t result = (~sr_val) & 0xffff;

    bool psr_change_mem;
    PIEvent psr_change;
    uint32_t psr_value = state.readMemEvent(PSR, psr_change_mem, psr_change);

    std::vector<PIEvent> ret {
        std::make_shared<PSREvent>(lc3::utils::computePSRCC(result, psr_value)),
        std::make_shared<RegEvent>(dr, result)
    };

    if(psr_change_mem) {
        ret.push_back(psr_change);
    }

    return ret;
}

std::vector<PIEvent> RTIInstruction::execute(MachineState const & state)
{
    if(state.sys_call_types.size() > 0) {
        return buildSysCallExitHelper(state, state.sys_call_types.top());
    } else {
        // If there are no entries in the sys_call_types stack, that means RTI
        // is being used before a TRAP or interrupt.
        uint32_t psr_value = state.readMemRaw(PSR);

        std::vector<PIEvent> ret;
        if(! state.ignore_privilege && (psr_value & 0x8000) != 0x0000) {
            // If in user mode, illegal RTI was used.
            return buildSysCallEnterHelper(state, INTEX_TABLE_START + 0, MachineState::SysCallType::INT);
        } else {
            // If in system mode, pretend like it was returning from a TRAP.
            return buildSysCallExitHelper(state, MachineState::SysCallType::TRAP);
        }
    }
}

std::vector<PIEvent> STInstruction::execute(MachineState const & state)
{
    uint32_t addr = lc3::utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);
    uint32_t value = state.regs[operands[1]->value] & 0xffff;

    bool psr_change_mem;
    PIEvent psr_change;
    uint32_t psr_value = state.readMemEvent(PSR, psr_change_mem, psr_change);

    if(! state.ignore_privilege && ((addr <= SYSTEM_END || addr >= MMIO_START) && (psr_value & 0x8000) != 0x0000)) {
        return buildSysCallEnterHelper(state, INTEX_TABLE_START + 0, MachineState::SysCallType::INT);
    }

    std::vector<PIEvent> ret {
        std::make_shared<MemWriteEvent>(addr, value)
    };

    if(psr_change_mem) {
        ret.push_back(psr_change);
    }

    return ret;
}

std::vector<PIEvent> STIInstruction::execute(MachineState const & state)
{
    uint32_t addr1 = lc3::utils::computeBasePlusSOffset(state.pc, operands[2]->value, operands[2]->width);

    bool change_mem;
    PIEvent change;
    uint32_t addr2 = state.readMemEvent(addr1, change_mem, change);

    bool psr_change_mem;
    PIEvent psr_change;
    uint32_t psr_value = state.readMemEvent(PSR, psr_change_mem, psr_change);

    if(! state.ignore_privilege && (((addr1 <= SYSTEM_END || addr1 >= MMIO_START)
        || (addr2 <= SYSTEM_END || addr2 >= MMIO_START)) && (psr_value & 0x8000) != 0x0000))
    {
        return buildSysCallEnterHelper(state, INTEX_TABLE_START + 0, MachineState::SysCallType::INT);
    }

    uint32_t value = state.regs[operands[1]->value] & 0xffff;

    std::vector<PIEvent> ret {
        std::make_shared<MemWriteEvent>(addr2, value)
    };

    if(change_mem) {
        ret.push_back(change);
    }

    if(psr_change_mem) {
        ret.push_back(psr_change);
    }

    return ret;
}

std::vector<PIEvent> STRInstruction::execute(MachineState const & state)
{
    uint32_t addr = lc3::utils::computeBasePlusSOffset(state.regs[operands[2]->value], operands[3]->value,
        operands[3]->width);
    uint32_t value = state.regs[operands[1]->value] & 0xffff;

    bool psr_change_mem;
    PIEvent psr_change;
    uint32_t psr_value = state.readMemEvent(PSR, psr_change_mem, psr_change);

    if(! state.ignore_privilege && ((addr <= SYSTEM_END || addr >= MMIO_START) && (psr_value & 0x8000) != 0x0000)) {
        return buildSysCallEnterHelper(state, INTEX_TABLE_START + 0, MachineState::SysCallType::INT);
    }

    std::vector<PIEvent> ret {
        std::make_shared<MemWriteEvent>(addr, value)
    };

    if(psr_change_mem) {
        ret.push_back(psr_change);
    }

    return ret;
}

std::vector<PIEvent> TRAPInstruction::execute(MachineState const & state)
{
    return buildSysCallEnterHelper(state, operands[2]->value, MachineState::SysCallType::TRAP);
}
