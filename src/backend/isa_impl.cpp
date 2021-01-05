/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include "isa.h"

using namespace lc3::core;

ISAHandler::ISAHandler(void)
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

PIMicroOp ADDRegInstruction::buildMicroOps(MachineState const & state) const
{
    (void) state;

    uint16_t dst_id = getOperand(1)->getValue();
    PIMicroOp compute = std::make_shared<RegAddRegMicroOp>(dst_id, getOperand(2)->getValue(),
        getOperand(4)->getValue());
    PIMicroOp set_cc = std::make_shared<CCUpdateRegMicroOp>(dst_id);

    compute->insert(set_cc);
    return compute;
}

PIMicroOp ADDImmInstruction::buildMicroOps(MachineState const & state) const
{
    (void) state;

    uint16_t dst_id = getOperand(1)->getValue();
    PIMicroOp compute = std::make_shared<RegAddImmMicroOp>(dst_id, getOperand(2)->getValue(),
        lc3::utils::sextTo16(getOperand(4)->getValue(), getOperand(4)->getWidth()));
    PIMicroOp set_cc = std::make_shared<CCUpdateRegMicroOp>(dst_id);

    compute->insert(set_cc);
    return compute;
}

PIMicroOp ANDRegInstruction::buildMicroOps(MachineState const & state) const
{
    (void) state;

    uint16_t dst_id = getOperand(1)->getValue();
    PIMicroOp compute = std::make_shared<RegAndRegMicroOp>(dst_id, getOperand(2)->getValue(),
        getOperand(4)->getValue());
    PIMicroOp set_cc =std::make_shared<CCUpdateRegMicroOp>(dst_id);

    compute->insert(set_cc);
    return compute;
}

PIMicroOp ANDImmInstruction::buildMicroOps(MachineState const & state) const
{
    (void) state;

    uint16_t dst_id = getOperand(1)->getValue();
    PIMicroOp compute = std::make_shared<RegAndImmMicroOp>(dst_id, getOperand(2)->getValue(),
        lc3::utils::sextTo16(getOperand(4)->getValue(), getOperand(4)->getWidth()));
    PIMicroOp set_cc = std::make_shared<CCUpdateRegMicroOp>(dst_id);

    compute->insert(set_cc);
    return compute;
}

PIMicroOp BRInstruction::buildMicroOps(MachineState const & state) const
{
    (void) state;

    PIMicroOp jump = std::make_shared<PCAddImmMicroOp>(lc3::utils::sextTo16(getOperand(2)->getValue(),
        getOperand(2)->getWidth()));

    return std::make_shared<BranchMicroOp>([this](MachineState const & state) {
        return (getOperand(1)->getValue() & lc3::utils::getBits(state.readPSR(), 2, 0)) != 0;
    }, "(N&n) | (Z&z) | (P&p)", jump, nullptr);
}

PIMicroOp JMPInstruction::buildMicroOps(MachineState const & state) const
{
    (void) state;

    uint16_t reg_id = getOperand(2)->getValue();
    PIMicroOp jump = std::make_shared<PCWriteRegMicroOp>(reg_id);
    PIMicroOp callback = std::make_shared<CallbackMicroOp>(CallbackType::SUB_EXIT);
    PIMicroOp func_trace = std::make_shared<PopFuncTypeMicroOp>();

    jump->insert(std::make_shared<BranchMicroOp>([this](MachineState const & state) {
        return (getOperand(2)->getValue() == 7 && state.peekFuncTraceType() == FuncType::SUBROUTINE);
    }, "funcTrace.top() == subroutine", callback, nullptr));
    callback->insert(func_trace);

    return jump;
}

PIMicroOp JSRInstruction::buildMicroOps(MachineState const & state) const
{
    (void) state;

    PIMicroOp link = std::make_shared<RegWritePCMicroOp>(7);
    PIMicroOp jump = std::make_shared<PCAddImmMicroOp>(lc3::utils::sextTo16(getOperand(2)->getValue(),
        getOperand(2)->getWidth()));
    PIMicroOp callback = std::make_shared<CallbackMicroOp>(CallbackType::SUB_ENTER);
    PIMicroOp func_trace = std::make_shared<PushFuncTypeMicroOp>(FuncType::SUBROUTINE);

    link->insert(jump);
    jump->insert(callback);
    callback->insert(func_trace);
    return link;
}

PIMicroOp JSRRInstruction::buildMicroOps(MachineState const & state) const
{
    (void) state;

    PIMicroOp link = std::make_shared<RegWritePCMicroOp>(7);
    PIMicroOp jump = std::make_shared<PCWriteRegMicroOp>(getOperand(3)->getValue());
    PIMicroOp callback = std::make_shared<CallbackMicroOp>(CallbackType::SUB_ENTER);
    PIMicroOp func_trace = std::make_shared<PushFuncTypeMicroOp>(FuncType::SUBROUTINE);

    link->insert(jump);
    jump->insert(callback);
    callback->insert(func_trace);
    return link;
}

PIMicroOp LDInstruction::buildMicroOps(MachineState const & state) const
{
    (void) state;

    uint16_t dst_id = getOperand(1)->getValue();
    PIMicroOp write_pc = std::make_shared<RegWritePCMicroOp>(8);
    PIMicroOp compute_addr = std::make_shared<RegAddImmMicroOp>(8, 8,
        lc3::utils::sextTo16(getOperand(2)->getValue(), getOperand(2)->getWidth()));
    PIMicroOp load = std::make_shared<MemReadMicroOp>(dst_id, 8);
    PIMicroOp set_cc = std::make_shared<CCUpdateRegMicroOp>(dst_id);

    write_pc->insert(compute_addr);
    compute_addr->insert(load);
    load->insert(set_cc);
    return write_pc;
}

PIMicroOp LDIInstruction::buildMicroOps(MachineState const & state) const
{
    (void) state;

    uint16_t dst_id = getOperand(1)->getValue();
    PIMicroOp write_pc = std::make_shared<RegWritePCMicroOp>(8);
    PIMicroOp compute_addr = std::make_shared<RegAddImmMicroOp>(8, 8,
        lc3::utils::sextTo16(getOperand(2)->getValue(), getOperand(2)->getWidth()));
    PIMicroOp load1 = std::make_shared<MemReadMicroOp>(8, 8);
    PIMicroOp load2 = std::make_shared<MemReadMicroOp>(dst_id, 8);
    PIMicroOp set_cc = std::make_shared<CCUpdateRegMicroOp>(dst_id);

    write_pc->insert(compute_addr);
    compute_addr->insert(load1);
    load1->insert(load2);
    load2->insert(set_cc);
    return write_pc;
}

PIMicroOp LDRInstruction::buildMicroOps(MachineState const & state) const
{
    (void) state;

    uint16_t dst_id = getOperand(1)->getValue();
    uint16_t base_id = getOperand(2)->getValue();
    PIMicroOp write_base = std::make_shared<RegWriteRegMicroOp>(8, base_id);
    PIMicroOp compute_addr = std::make_shared<RegAddImmMicroOp>(8, 8,
        lc3::utils::sextTo16(getOperand(3)->getValue(), getOperand(3)->getWidth()));
    PIMicroOp load = std::make_shared<MemReadMicroOp>(dst_id, 8);
    PIMicroOp set_cc = std::make_shared<CCUpdateRegMicroOp>(dst_id);

    write_base->insert(compute_addr);
    compute_addr->insert(load);
    load->insert(set_cc);
    return write_base;
}

PIMicroOp LEAInstruction::buildMicroOps(MachineState const & state) const
{
    (void) state;

    uint16_t dst_id = getOperand(1)->getValue();
    PIMicroOp write_pc = std::make_shared<RegWritePCMicroOp>(8);
    PIMicroOp compute_addr = std::make_shared<RegAddImmMicroOp>(dst_id, 8,
        lc3::utils::sextTo16(getOperand(2)->getValue(), getOperand(2)->getWidth()));

    write_pc->insert(compute_addr);
    return write_pc;
}

PIMicroOp NOTInstruction::buildMicroOps(MachineState const & state) const
{
    (void) state;

    uint16_t dst_id = getOperand(1)->getValue();
    PIMicroOp compute = std::make_shared<RegNotMicroOp>(dst_id, getOperand(2)->getValue());
    PIMicroOp set_cc = std::make_shared<CCUpdateRegMicroOp>(dst_id);

    compute->insert(set_cc);
    return compute;
}

PIMicroOp RTIInstruction::buildMicroOps(MachineState const & state) const
{
    PIMicroOp msg = std::make_shared<PrintMessageMicroOp>("privilege violation");
    PIMicroOp dec_pc = std::make_shared<PCAddImmMicroOp>(-1);
    std::pair<PIMicroOp, PIMicroOp> handle_exception_chain = buildSystemModeEnter(INTEX_TABLE_START, 0x0,
        lc3::utils::getBits(state.readPSR(), 10, 8));
    PIMicroOp ex_callback = std::make_shared<CallbackMicroOp>(CallbackType::EX_ENTER);
    PIMicroOp ex_func_trace = std::make_shared<PushFuncTypeMicroOp>(FuncType::EXCEPTION);

    PIMicroOp load_pc = std::make_shared<MemReadMicroOp>(8, 6);
    PIMicroOp write_pc = std::make_shared<PCWriteRegMicroOp>(8);
    PIMicroOp dec_sp1 = std::make_shared<RegAddImmMicroOp>(6, 6, 1);
    PIMicroOp load_psr = std::make_shared<MemReadMicroOp>(8, 6);
    PIMicroOp write_psr = std::make_shared<PSRWriteRegMicroOp>(8);
    PIMicroOp dec_sp2 = std::make_shared<RegAddImmMicroOp>(6, 6, 1);
    PIMicroOp save_cur_sp = std::make_shared<RegWriteRegMicroOp>(9, 6);
    PIMicroOp write_ssp = std::make_shared<RegWriteSSPMicroOp>(6);
    PIMicroOp write_cur_sp = std::make_shared<SSPWriteRegMicroOp>(9);

    PIMicroOp callback = nullptr;
    switch(state.peekFuncTraceType()) {
        case FuncType::TRAP: callback = std::make_shared<CallbackMicroOp>(CallbackType::SUB_EXIT); break;
        case FuncType::INTERRUPT: callback = std::make_shared<CallbackMicroOp>(CallbackType::INT_EXIT); break;
        case FuncType::EXCEPTION: callback = std::make_shared<CallbackMicroOp>(CallbackType::EX_EXIT); break;
        default: break;
    }
    PIMicroOp func_trace = std::make_shared<PopFuncTypeMicroOp>();

    PIMicroOp start = std::make_shared<BranchMicroOp>([](MachineState const & state) {
        return lc3::utils::getBit(state.readPSR(), 15) == 0;
    }, "PSR[15] == 0", load_pc, msg);

    msg->insert(dec_pc);
    dec_pc->insert(handle_exception_chain.first);
    handle_exception_chain.second->insert(ex_callback);
    ex_callback->insert(ex_func_trace);

    load_pc->insert(write_pc);
    write_pc->insert(dec_sp1);
    dec_sp1->insert(load_psr);
    load_psr->insert(write_psr);
    write_psr->insert(dec_sp2);
    dec_sp2->insert(std::make_shared<BranchMicroOp>([](MachineState const & state) {
        return lc3::utils::getBit(state.readPSR(), 15) == 0;
    }, "PSR[15] == 0", callback, save_cur_sp));

    save_cur_sp->insert(write_ssp);
    write_ssp->insert(write_cur_sp);
    if(callback != nullptr) {
        write_cur_sp->insert(callback);
        callback->insert(func_trace);
    }

    return start;
}

PIMicroOp STInstruction::buildMicroOps(MachineState const & state) const
{
    (void) state;

    uint16_t src_id = getOperand(1)->getValue();
    PIMicroOp write_pc = std::make_shared<RegWritePCMicroOp>(8);
    PIMicroOp compute_addr = std::make_shared<RegAddImmMicroOp>(8, 8,
        lc3::utils::sextTo16(getOperand(2)->getValue(), getOperand(2)->getWidth()));
    PIMicroOp store = std::make_shared<MemWriteRegMicroOp>(8, src_id);

    write_pc->insert(compute_addr);
    compute_addr->insert(store);
    return write_pc;
}

PIMicroOp STIInstruction::buildMicroOps(MachineState const & state) const
{
    (void) state;

    uint16_t src_id = getOperand(1)->getValue();
    PIMicroOp write_pc = std::make_shared<RegWritePCMicroOp>(8);
    PIMicroOp compute_addr = std::make_shared<RegAddImmMicroOp>(8, 8,
        lc3::utils::sextTo16(getOperand(2)->getValue(), getOperand(2)->getWidth()));
    PIMicroOp load = std::make_shared<MemReadMicroOp>(8, 8);
    PIMicroOp store = std::make_shared<MemWriteRegMicroOp>(8, src_id);

    write_pc->insert(compute_addr);
    compute_addr->insert(load);
    load->insert(store);
    return write_pc;
}

PIMicroOp STRInstruction::buildMicroOps(MachineState const & state) const
{
    (void) state;

    uint16_t src_id = getOperand(1)->getValue();
    uint16_t base_id = getOperand(2)->getValue();
    PIMicroOp write_base = std::make_shared<RegWriteRegMicroOp>(8, base_id);
    PIMicroOp compute_addr = std::make_shared<RegAddImmMicroOp>(8, 8,
        lc3::utils::sextTo16(getOperand(3)->getValue(), getOperand(3)->getWidth()));
    PIMicroOp store = std::make_shared<MemWriteRegMicroOp>(8, src_id);

    write_base->insert(compute_addr);
    compute_addr->insert(store);
    return write_base;
}

PIMicroOp TRAPInstruction::buildMicroOps(MachineState const & state) const
{
    std::pair<PIMicroOp, PIMicroOp> handle_trap_chain = buildSystemModeEnter(TRAP_TABLE_START,
        static_cast<uint8_t>(getOperand(2)->getValue()), (state.readPSR() & 0x0700) >> 8);
    PIMicroOp callback = std::make_shared<CallbackMicroOp>(CallbackType::SUB_ENTER);
    PIMicroOp func_trace = std::make_shared<PushFuncTypeMicroOp>(FuncType::TRAP);

    handle_trap_chain.second->insert(callback);
    callback->insert(func_trace);

    return handle_trap_chain.first;
}

bool lc3::core::isAccessViolation(uint16_t addr, MachineState const & state)
{
    if(state.getIgnorePrivilege()) {
        return false;
    } else {
        return lc3::utils::getBit(state.readPSR(), 15) == 1 && (addr <= SYSTEM_END || addr >= MMIO_START);
    }
}

std::pair<PIMicroOp, PIMicroOp> lc3::core::buildSystemModeEnter(uint16_t table_start, uint8_t vec, uint8_t priority)
{
    PIMicroOp save_cur_sp = std::make_shared<RegWriteRegMicroOp>(8, 6);
    PIMicroOp write_ssp = std::make_shared<RegWriteSSPMicroOp>(6);
    PIMicroOp write_cur_sp = std::make_shared<SSPWriteRegMicroOp>(8);
    PIMicroOp dec_sp1 = std::make_shared<RegAddImmMicroOp>(6, 6, -1);
    PIMicroOp write_psr = std::make_shared<RegWritePSRMicroOp>(9);
    PIMicroOp copy_psr = std::make_shared<RegWriteRegMicroOp>(10, 9);
    PIMicroOp clear_priority = std::make_shared<RegAndImmMicroOp>(10, 10, 0xF1FF);
    PIMicroOp set_priority = std::make_shared<RegAddImmMicroOp>(10, 10, (priority & 0x7) << 8);
    PIMicroOp set_priv = std::make_shared<RegAndImmMicroOp>(10, 10, 0x7FFF);
    PIMicroOp change_priv = std::make_shared<PSRWriteRegMicroOp>(10);
    PIMicroOp store_psr = std::make_shared<MemWriteRegMicroOp>(6, 9);
    PIMicroOp dec_sp2 = std::make_shared<RegAddImmMicroOp>(6, 6, -1);
    PIMicroOp write_pc = std::make_shared<RegWritePCMicroOp>(9);
    PIMicroOp store_pc = std::make_shared<MemWriteRegMicroOp>(6, 9);
    PIMicroOp write_table_start = std::make_shared<RegWriteImmMicroOp>(11, table_start);
    PIMicroOp add_table_offset = std::make_shared<RegAddImmMicroOp>(11, 11, vec);
    PIMicroOp load_table = std::make_shared<MemReadMicroOp>(11, 11);
    PIMicroOp jump = std::make_shared<PCWriteRegMicroOp>(11);

    PIMicroOp start = std::make_shared<BranchMicroOp>([](MachineState const & state) {
        return lc3::utils::getBit(state.readPSR(), 15) == 1;
    }, "PSR[15] == 1", save_cur_sp, dec_sp1);

    save_cur_sp->insert(write_ssp);
    write_ssp->insert(write_cur_sp);
    write_cur_sp->insert(dec_sp1);

    dec_sp1->insert(write_psr);
    write_psr->insert(copy_psr);
    copy_psr->insert(clear_priority);
    clear_priority->insert(set_priority);
    set_priority->insert(std::make_shared<BranchMicroOp>([](MachineState const & state) {
        return lc3::utils::getBit(state.readPSR(), 15) == 1;
    }, "PSR[15] == 1", set_priv, store_psr));

    set_priv->insert(change_priv);
    change_priv->insert(store_psr);

    store_psr->insert(dec_sp2);
    dec_sp2->insert(write_pc);
    write_pc->insert(store_pc);
    store_pc->insert(write_table_start);
    write_table_start->insert(add_table_offset);
    add_table_offset->insert(load_table);
    load_table->insert(jump);

    return std::make_pair(start, jump);
}
