#include "uop.h"

using namespace lc3::core;

void FetchEvent::handleEvent(MachineState & state)
{
    state.writeIR(state.getMemValue(state.readPC()));
}

std::string FetchEvent::toString(MachineState const & state) const
{
    return utils::ssprintf("IR:0x%0.4x <= M[0x%0.4x]:0x%0.4x", state.readIR(), state.readPC(),
        state.getMemValue(state.readPC()));
}

void PCAddImmEvent::handleEvent(MachineState & state)
{
    uint16_t value = state.readPC() + amnt;
    state.writePC(value);
    if(result) {
        *result = value;
    }
}

std::string PCAddImmEvent::toString(MachineState const & state) const
{
    return utils::ssprintf("PC:0x%0.4x <= (PC:0x%0.4x + #%d):0x%0.4x", state.readPC(), state.readPC(), amnt,
        state.readPC() + amnt);
}

void RegAddImmEvent::handleEvent(MachineState & state)
{
    uint16_t value = state.readReg(reg_id) + amnt;
    state.writeReg(reg_id, value);
    if(result) {
        *result = value;
    }
}

std::string RegAddImmEvent::toString(MachineState const & state) const
{
    return utils::ssprintf("R%d:0x%0.4x <= (R%d:0x%0.4x + #%d):0x%0.4x", reg_id, state.readReg(reg_id), reg_id,
        state.readReg(reg_id), amnt, state.readReg(reg_id) + amnt);
}

void RegAddRegEvent::handleEvent(MachineState & state)
{
    uint16_t value = state.readReg(reg_id1) + state.readReg(reg_id2);
    state.writeReg(reg_id1, value);
    if(result) {
        *result = value;
    }
}

std::string RegAddRegEvent::toString(MachineState const & state) const
{
    return utils::ssprintf("R%d:0x%0.4x <= (R%d:0x%0.4x + R%d:0x%0.4x):0x%0.4x", reg_id1, state.readReg(reg_id1),
        reg_id1, state.readReg(reg_id1), reg_id2, state.readReg(reg_id2),
        state.readReg(reg_id1) + state.readReg(reg_id2));
}

void MemReadEvent::handleEvent(MachineState & state)
{
    uint16_t value = state.getMemValue(mem_addr);
    state.writeReg(reg_id, value);
    if(result) {
        *result = value;
    }
}

std::string MemReadEvent::toString(MachineState const & state) const
{
    return utils::ssprintf("R%d:0x%0.4x <= MEM[0x%0.4x]:0x%0.4x", reg_id, state.readReg(reg_id), mem_addr,
        state.getMemValue(mem_addr));
}

void MemWriteImmEvent::handleEvent(MachineState & state)
{
    state.setMemValue(addr, value);
}

std::string MemWriteImmEvent::toString(MachineState const & state) const
{
    return utils::ssprintf("MEM[0x%0.4x]:0x%0.4x <= 0x%0.4x", addr, state.getMemValue(addr), value);
}

void CCUpdateEvent::handleEvent(MachineState & state)
{
    if(prev_result) {
        uint16_t prev_value = prev_result->get();
        uint16_t psr_value = state.getMemValue(PSR);
        char cc_char = getCCChar(prev_value);
        if(cc_char == 'N') {
            state.setMemValue(PSR, (psr_value & 0xFFF8) | 0x0004);
        } else if(cc_char == 'Z') {
            state.setMemValue(PSR, (psr_value & 0xFFF8) | 0x0002);
        } else {
            state.setMemValue(PSR, (psr_value & 0xFFF8) | 0x0001);
        }
    }
}

std::string CCUpdateEvent::toString(MachineState const & state) const
{
    if(prev_result) {
        char cur_cc_char;
        uint16_t prev_value = prev_result->get();
        uint16_t psr_value = state.getMemValue(PSR);
        if((psr_value & 0x0004) != 0) {
            cur_cc_char = 'N';
        } else if((psr_value & 0x0002) != 0) {
            cur_cc_char = 'Z';
        } else {
            cur_cc_char = 'P';
        }

        return utils::ssprintf("CC:%c <= CC(0x%0.4x):%c", cur_cc_char, prev_value, getCCChar(prev_value));
    }

    return "invalid CC change";
}

char CCUpdateEvent::getCCChar(uint16_t value) const
{
    if((value & 0x8000) != 0) {
        return 'N';
    } else if(value == 0) {
        return 'Z';
    } else {
        return 'P';
    }
}
