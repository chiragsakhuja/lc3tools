#include "decoder.h"
#include "isa.h"
#include "state.h"
#include "uop.h"

using namespace lc3::core;

PIMicroOp IMicroOp::insert(PIMicroOp new_next)
{
    if(next == nullptr) {
        next = new_next;
    } else {
        PIMicroOp cur = next;
        while(cur->next != nullptr) {
            cur = cur->next;
        }
        cur->next = new_next;
    }

    return next;
}

std::string IMicroOp::regToString(uint16_t reg_id) const
{
    if(reg_id < 8) {
        return lc3::utils::ssprintf("R%d", reg_id);
    } else {
        return lc3::utils::ssprintf("T%d", reg_id - 8);
    }
}

void FetchMicroOp::handleMicroOp(MachineState & state)
{
    if(isAccessViolation(state.readPC(), state)) {
        PIMicroOp msg = std::make_shared<PrintMessageMicroOp>("illegal memory access (ACV)");
        std::pair<PIMicroOp, PIMicroOp> handle_exception_chain = buildSystemModeEnter(INTEX_TABLE_START, 0x2,
            lc3::utils::getBits(state.readPSR(), 10, 8));
        PIMicroOp callback = std::make_shared<CallbackMicroOp>(CallbackType::EX_ENTER);
        PIMicroOp func_trace = std::make_shared<PushFuncTypeMicroOp>(FuncType::EXCEPTION);

        msg->insert(handle_exception_chain.first);
        handle_exception_chain.second->insert(callback);
        callback->insert(func_trace);

        next = msg;
    } else {
        uint16_t value = std::get<0>(state.readMem(state.readPC()));
        state.writeIR(value);
    }
}

std::string FetchMicroOp::toString(MachineState const & state) const
{
    return lc3::utils::ssprintf("IR:0x%0.4hx <= M[0x%0.4hx]:0x%0.4hx", state.readIR(), state.readPC(),
        std::get<0>(state.readMem(state.readPC())));
}

void DecodeMicroOp::handleMicroOp(MachineState & state)
{
    lc3::optional<PIInstruction> inst = decoder.decode(state.readIR());
    if(inst) {
        insert((*inst)->buildMicroOps(state));
        state.writeDecodedIR(*inst);
    } else {
        PIMicroOp msg = std::make_shared<PrintMessageMicroOp>("unknown opcode");
        PIMicroOp dec_pc = std::make_shared<PCAddImmMicroOp>(-1);
        std::pair<PIMicroOp, PIMicroOp> handle_exception_chain = buildSystemModeEnter(INTEX_TABLE_START, 0x1,
            lc3::utils::getBits(state.readPSR(), 10, 8));
        PIMicroOp callback = std::make_shared<CallbackMicroOp>(CallbackType::EX_ENTER);
        PIMicroOp func_trace = std::make_shared<PushFuncTypeMicroOp>(FuncType::EXCEPTION);

        msg->insert(dec_pc);
        dec_pc->insert(handle_exception_chain.first);
        handle_exception_chain.second->insert(callback);
        callback->insert(func_trace);

        next = msg;
    }
}

std::string DecodeMicroOp::toString(MachineState const & state) const
{
    lc3::optional<PIInstruction> inst = decoder.decode(state.readIR());
    if(inst) {
        (*inst)->buildMicroOps(state);
        return lc3::utils::ssprintf("dIR <= %s", (*inst)->toValueString().c_str());
    } else {
        return "dIR <= Illegal instruction";
    }
}

void PCWriteRegMicroOp::handleMicroOp(MachineState & state)
{
    uint16_t value = state.readReg(reg_id);
    state.writePC(value);
}

std::string PCWriteRegMicroOp::toString(MachineState const & state) const
{
    return lc3::utils::ssprintf("PC:0x%0.4hx <= %s:0x%0.4hx", state.readPC(), regToString(reg_id).c_str(),
        state.readReg(reg_id));
}

void PSRWriteRegMicroOp::handleMicroOp(MachineState & state)
{
    uint16_t value = state.readReg(reg_id);
    state.writePSR(value);
}

std::string PSRWriteRegMicroOp::toString(MachineState const & state) const
{
    return lc3::utils::ssprintf("PSR:0x%0.4hx <= %s:0x%0.4hx", state.readPSR(), regToString(reg_id).c_str(),
        state.readReg(reg_id));
}

void PCAddImmMicroOp::handleMicroOp(MachineState & state)
{
    uint16_t value = state.readPC() + amnt;
    state.writePC(value);
}

std::string PCAddImmMicroOp::toString(MachineState const & state) const
{
    return lc3::utils::ssprintf("PC:0x%0.4hx <= (PC:0x%0.4hx + #%d):0x%0.4hx", state.readPC(), state.readPC(), amnt,
        state.readPC() + amnt);
}

void RegWriteImmMicroOp::handleMicroOp(MachineState & state)
{
    state.writeReg(reg_id, value);
}

std::string RegWriteImmMicroOp::toString(MachineState const & state) const
{
    return lc3::utils::ssprintf("%s:0x%0.4hx <= 0x%0.4hx", regToString(reg_id).c_str(), state.readReg(reg_id), value);
}

void RegWriteRegMicroOp::handleMicroOp(MachineState & state)
{
    state.writeReg(dst_id, state.readReg(src_id));
}

std::string RegWriteRegMicroOp::toString(MachineState const & state) const
{
    return lc3::utils::ssprintf("%s:0x%0.4hx <= %s:0x%0.4hx", regToString(dst_id).c_str(), state.readReg(dst_id),
        regToString(src_id).c_str(), state.readReg(src_id));
}

void RegWritePCMicroOp::handleMicroOp(MachineState & state)
{
    state.writeReg(reg_id, state.readPC());
}

std::string RegWritePCMicroOp::toString(MachineState const & state) const
{
    return lc3::utils::ssprintf("%s:0x%0.4hx <= PC:0x%0.4hx", regToString(reg_id).c_str(), state.readReg(reg_id),
        state.readPC());
}

void RegWritePSRMicroOp::handleMicroOp(MachineState & state)
{
    state.writeReg(reg_id, state.readPSR());
}

std::string RegWritePSRMicroOp::toString(MachineState const & state) const
{
    return lc3::utils::ssprintf("%s:0x%0.4hx <= PSR:0x%0.4hx", regToString(reg_id).c_str(), state.readReg(reg_id),
        state.readPSR());
}

void RegWriteSSPMicroOp::handleMicroOp(MachineState & state)
{
    state.writeReg(reg_id, state.readSSP());
}

std::string RegWriteSSPMicroOp::toString(MachineState const & state) const
{
    return lc3::utils::ssprintf("%s:0x%0.4hx <= SSP:0x%0.4hx", regToString(reg_id).c_str(), state.readReg(reg_id),
        state.readSSP());
}

void SSPWriteRegMicroOp::handleMicroOp(MachineState & state)
{
    state.writeSSP(state.readReg(reg_id));
}

std::string SSPWriteRegMicroOp::toString(MachineState const & state) const
{
    return lc3::utils::ssprintf("SSP:0x%0.4hx <= %s:0x%0.4hx", state.readSSP(), regToString(reg_id).c_str(),
        state.readReg(reg_id));
        
}

void RegAddImmMicroOp::handleMicroOp(MachineState & state)
{
    uint16_t value = state.readReg(src_id) + amnt;
    state.writeReg(dst_id, value);
}

std::string RegAddImmMicroOp::toString(MachineState const & state) const
{
    return lc3::utils::ssprintf("%s:0x%0.4hx <= (%s:0x%0.4hx + #%d):0x%0.4hx", regToString(dst_id).c_str(),
        state.readReg(dst_id), regToString(src_id).c_str(), state.readReg(src_id), amnt, state.readReg(src_id) + amnt);
}

void RegAddRegMicroOp::handleMicroOp(MachineState & state)
{
    uint16_t value = state.readReg(src_id1) + state.readReg(src_id2);
    state.writeReg(dst_id, value);
}

std::string RegAddRegMicroOp::toString(MachineState const & state) const
{
    return lc3::utils::ssprintf("%s:0x%0.4hx <= (%s:0x%0.4hx + %s:0x%0.4hx):0x%0.4hx", regToString(dst_id).c_str(),
        state.readReg(dst_id), regToString(src_id1).c_str(), state.readReg(src_id1), regToString(src_id2).c_str(),
        state.readReg(src_id2), state.readReg(src_id1) + state.readReg(src_id2));
}

void RegAndImmMicroOp::handleMicroOp(MachineState & state)
{
    uint16_t value = state.readReg(src_id) & amnt;
    state.writeReg(dst_id, value);
}

std::string RegAndImmMicroOp::toString(MachineState const & state) const
{
    return lc3::utils::ssprintf("%s:0x%0.4hx <= (%s:0x%0.4hx & #%d):0x%0.4hx", regToString(dst_id).c_str(),
        state.readReg(dst_id), regToString(src_id).c_str(), state.readReg(src_id), amnt, state.readReg(src_id) & amnt);
}

void RegAndRegMicroOp::handleMicroOp(MachineState & state)
{
    uint16_t value = state.readReg(src_id1) & state.readReg(src_id2);
    state.writeReg(dst_id, value);
}

std::string RegAndRegMicroOp::toString(MachineState const & state) const
{
    return lc3::utils::ssprintf("%s:0x%0.4hx <= (%s:0x%0.4hx & %s:0x%0.4hx):0x%0.4hx", regToString(dst_id).c_str(),
        state.readReg(dst_id), regToString(src_id1).c_str(), state.readReg(src_id1), regToString(src_id2).c_str(),
        state.readReg(src_id2), state.readReg(src_id1) & state.readReg(src_id2));
}

void RegNotMicroOp::handleMicroOp(MachineState & state)
{
    uint16_t value = ~state.readReg(src_id);
    state.writeReg(dst_id, value);
}

std::string RegNotMicroOp::toString(MachineState const & state) const
{
    return lc3::utils::ssprintf("%s:0x%0.4hx <= (~%s:0x%0.4hx):0x%0.4hx", regToString(dst_id).c_str(),
        state.readReg(dst_id), regToString(src_id).c_str(), state.readReg(src_id),
        ~state.readReg(src_id));
}

void MemReadMicroOp::handleMicroOp(MachineState & state)
{
    uint16_t addr = state.readReg(addr_reg_id);
    if(isAccessViolation(addr, state)) {
        PIMicroOp msg = std::make_shared<PrintMessageMicroOp>("illegal memory access (ACV)");
        PIMicroOp dec_pc = std::make_shared<PCAddImmMicroOp>(-1);
        std::pair<PIMicroOp, PIMicroOp> handle_exception_chain = buildSystemModeEnter(INTEX_TABLE_START, 0x2,
            lc3::utils::getBits(state.readPSR(), 10, 8));
        PIMicroOp callback = std::make_shared<CallbackMicroOp>(CallbackType::EX_ENTER);
        PIMicroOp func_trace = std::make_shared<PushFuncTypeMicroOp>(FuncType::EXCEPTION);

        msg->insert(dec_pc);
        dec_pc->insert(handle_exception_chain.first);
        handle_exception_chain.second->insert(callback);
        callback->insert(func_trace);

        next = msg;
    } else {
        std::pair<uint16_t, PIMicroOp> read_result = state.readMem(addr);

        uint16_t value = std::get<0>(read_result);
        PIMicroOp op = std::get<1>(read_result);

        if(op) {
            insert(op);
        }

        state.writeReg(dst_id, value);
    }
}

std::string MemReadMicroOp::toString(MachineState const & state) const
{
    return lc3::utils::ssprintf("%s:0x%0.4hx <= MEM[%s:0x%0.4hx]:0x%0.4hx", regToString(dst_id).c_str(),
        state.readReg(dst_id), regToString(addr_reg_id).c_str(), state.readReg(addr_reg_id),
        std::get<0>(state.readMem(state.readReg(addr_reg_id))));
}

void MemWriteImmMicroOp::handleMicroOp(MachineState & state)
{
    uint16_t addr = state.readReg(addr_reg_id);
    if(isAccessViolation(addr, state)) {
        PIMicroOp msg = std::make_shared<PrintMessageMicroOp>("illegal memory access (ACV)");
        PIMicroOp dec_pc = std::make_shared<PCAddImmMicroOp>(-1);
        std::pair<PIMicroOp, PIMicroOp> handle_exception_chain = buildSystemModeEnter(INTEX_TABLE_START, 0x2,
            lc3::utils::getBits(state.readPSR(), 10, 8));
        PIMicroOp callback = std::make_shared<CallbackMicroOp>(CallbackType::EX_ENTER);
        PIMicroOp func_trace = std::make_shared<PushFuncTypeMicroOp>(FuncType::EXCEPTION);

        msg->insert(dec_pc);
        dec_pc->insert(handle_exception_chain.first);
        handle_exception_chain.second->insert(callback);
        callback->insert(func_trace);

        next = msg;
    } else {
        PIMicroOp op = state.writeMem(addr, value);

        if(op) {
            insert(op);
        }
    }
}

std::string MemWriteImmMicroOp::toString(MachineState const & state) const
{
    return lc3::utils::ssprintf("MEM[%s:0x%0.4hx]:0x%0.4hx <= 0x%0.4hx", regToString(addr_reg_id).c_str(),
        state.readReg(addr_reg_id), std::get<0>(state.readMem(state.readReg(addr_reg_id))), value);
}

void MemWriteRegMicroOp::handleMicroOp(MachineState & state)
{
    uint16_t addr = state.readReg(addr_reg_id);
    if(isAccessViolation(addr, state)) {
        PIMicroOp msg = std::make_shared<PrintMessageMicroOp>("illegal memory access (ACV)");
        PIMicroOp dec_pc = std::make_shared<PCAddImmMicroOp>(-1);
        std::pair<PIMicroOp, PIMicroOp> handle_exception_chain = buildSystemModeEnter(INTEX_TABLE_START, 0x2,
            lc3::utils::getBits(state.readPSR(), 10, 8));
        PIMicroOp callback = std::make_shared<CallbackMicroOp>(CallbackType::EX_ENTER);
        PIMicroOp func_trace = std::make_shared<PushFuncTypeMicroOp>(FuncType::EXCEPTION);

        msg->insert(dec_pc);
        dec_pc->insert(handle_exception_chain.first);
        handle_exception_chain.second->insert(callback);
        callback->insert(func_trace);

        next = msg;
    } else {
        PIMicroOp op = state.writeMem(addr, state.readReg(src_id));

        if(op) {
            insert(op);
        }
    }
}

std::string MemWriteRegMicroOp::toString(MachineState const & state) const
{
    return lc3::utils::ssprintf("MEM[%s:0x%0.4hx]:0x%0.4hx <= %s:0x%0.4hx", regToString(addr_reg_id).c_str(),
        state.readReg(addr_reg_id), std::get<0>(state.readMem(state.readReg(addr_reg_id))), regToString(src_id).c_str(),
        state.readReg(src_id));
}

void CCUpdateRegMicroOp::handleMicroOp(MachineState & state)
{
    uint16_t psr_value = state.readPSR();
    char cc_char = getCCChar(state.readReg(reg_id));
    if(cc_char == 'N') {
        state.writePSR((psr_value & 0xFFF8) | 0x0004);
    } else if(cc_char == 'Z') {
        state.writePSR((psr_value & 0xFFF8) | 0x0002);
    } else {
        state.writePSR((psr_value & 0xFFF8) | 0x0001);
    }
}

std::string CCUpdateRegMicroOp::toString(MachineState const & state) const
{
    char cur_cc_char;
    uint16_t psr_value = state.readPSR();
    if((psr_value & 0x0004) != 0) {
        cur_cc_char = 'N';
    } else if((psr_value & 0x0002) != 0) {
        cur_cc_char = 'Z';
    } else {
        cur_cc_char = 'P';
    }

    return lc3::utils::ssprintf("CC:%c <= ComputeCC(0x%0.4hx):%c", cur_cc_char, state.readReg(reg_id),
        getCCChar(state.readReg(reg_id)));
}

char CCUpdateRegMicroOp::getCCChar(uint16_t value) const
{
    if((value & 0x8000) != 0) {
        return 'N';
    } else if(value == 0) {
        return 'Z';
    } else {
        return 'P';
    }
}

void BranchMicroOp::handleMicroOp(MachineState & state)
{
    bool result = pred(state);

    if(result) {
        next = true_next;
    } else {
        next = false_next;
    }
}

std::string BranchMicroOp::toString(MachineState const & state) const
{
    return lc3::utils::ssprintf("uBEN <= (%s):%s", msg.c_str(), pred(state) ? "true" : "false");
}

PIMicroOp BranchMicroOp::insert(PIMicroOp new_next)
{
    if(true_next) { true_next->insert(new_next); }
    if(false_next) { false_next->insert(new_next); }

    return new_next;
}

void CallbackMicroOp::handleMicroOp(MachineState & state)
{
    state.addPendingCallback(type);
}

std::string CallbackMicroOp::toString(MachineState const & state) const
{
    (void) state;

    return lc3::utils::ssprintf("callbacks <= %s", callbackTypeToString(type).c_str());
}

void PushInterruptTypeMicroOp::handleMicroOp(MachineState & state)
{
    state.enqueueInterrupt(type);
}

std::string PushInterruptTypeMicroOp::toString(MachineState const & state) const
{
    (void) state;

    return lc3::utils::ssprintf("interrupts <= %s", interruptTypeToString(type).c_str());
}

void PopInterruptTypeMicroOp::handleMicroOp(MachineState & state)
{
    state.dequeueInterrupt();
}

std::string PopInterruptTypeMicroOp::toString(MachineState const & state) const
{
    (void) state;

    return lc3::utils::ssprintf("interrupts <= interrupts.removeFront()");
}

void PushFuncTypeMicroOp::handleMicroOp(MachineState & state)
{
    state.pushFuncTraceType(type);
}

std::string PushFuncTypeMicroOp::toString(MachineState const & state) const
{
    (void) state;

    return lc3::utils::ssprintf("traceStack <= %s", funcTypeToString(type).c_str());
}

void PopFuncTypeMicroOp::handleMicroOp(MachineState & state)
{
    state.popFuncTraceType();
}

std::string PopFuncTypeMicroOp::toString(MachineState const & state) const
{
    (void) state;

    return lc3::utils::ssprintf("traceStack <= traceStack.removeTop()");
}

void PrintMessageMicroOp::handleMicroOp(MachineState & state)
{
    (void) state;
}

std::string PrintMessageMicroOp::toString(MachineState const & state) const
{
    (void) state;

    return msg;
}
