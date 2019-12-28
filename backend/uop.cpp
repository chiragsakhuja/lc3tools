#include "decoder.h"
#include "isa.h"
#include "state.h"
#include "uop.h"

using namespace lc3::core;

PIMicroOp IMicroOp::insert(PIMicroOp new_next)
{
    if(next) {
        PIMicroOp cur_next = next;
        next = new_next;
        new_next->next = cur_next;
    } else {
        next = new_next;
    }

    return next;
}

void FetchMicroOp::handleMicroOp(MachineState & state)
{
    uint16_t value = std::get<0>(state.readMem(state.readPC()));
    state.writeIR(value);
    if(result) {
        *result = value;
    }
}

std::string FetchMicroOp::toString(MachineState const & state) const
{
    return utils::ssprintf("IR:0x%0.4x <= M[0x%0.4x]:0x%0.4x", state.readIR(), state.readPC(),
        std::get<0>(state.readMem(state.readPC())));
}

void DecodeMicroOp::handleMicroOp(MachineState & state)
{
    lc3::optional<PIInstruction> inst = decoder.decode(state.readIR());
    if(inst) {
        insert((*inst)->buildMicroOps(state));
    } else {
        // TODO: Handle illegal instruction
    }
}

std::string DecodeMicroOp::toString(MachineState const & state) const
{
    return utils::ssprintf("Decoding IR:0x%0.4x", state.readIR());
}

void PCAddImmMicroOp::handleMicroOp(MachineState & state)
{
    uint16_t value = state.readPC() + amnt;
    state.writePC(value);
    if(result) {
        *result = value;
    }
}

std::string PCAddImmMicroOp::toString(MachineState const & state) const
{
    return utils::ssprintf("PC:0x%0.4x <= (PC:0x%0.4x + #%d):0x%0.4x", state.readPC(), state.readPC(), amnt,
        state.readPC() + amnt);
}

void RegAddImmMicroOp::handleMicroOp(MachineState & state)
{
    uint16_t value = state.readReg(reg_id) + amnt;
    state.writeReg(reg_id, value);
    if(result) {
        *result = value;
    }
}

std::string RegAddImmMicroOp::toString(MachineState const & state) const
{
    return utils::ssprintf("R%d:0x%0.4x <= (R%d:0x%0.4x + #%d):0x%0.4x", reg_id, state.readReg(reg_id), reg_id,
        state.readReg(reg_id), amnt, state.readReg(reg_id) + amnt);
}

void RegAddRegMicroOp::handleMicroOp(MachineState & state)
{
    uint16_t value = state.readReg(reg_id1) + state.readReg(reg_id2);
    state.writeReg(reg_id1, value);
    if(result) {
        *result = value;
    }
}

std::string RegAddRegMicroOp::toString(MachineState const & state) const
{
    return utils::ssprintf("R%d:0x%0.4x <= (R%d:0x%0.4x + R%d:0x%0.4x):0x%0.4x", reg_id1, state.readReg(reg_id1),
        reg_id1, state.readReg(reg_id1), reg_id2, state.readReg(reg_id2),
        state.readReg(reg_id1) + state.readReg(reg_id2));
}

void MemReadMicroOp::handleMicroOp(MachineState & state)
{
    std::pair<uint16_t, PIMicroOp> read_result = state.readMem(mem_addr);
    uint16_t value = std::get<0>(read_result);
    PIMicroOp op = std::get<1>(read_result);

    if(op) {
        insert(op);
    }

    state.writeReg(reg_id, value);
    if(result) {
        *result = value;
    }
}

std::string MemReadMicroOp::toString(MachineState const & state) const
{
    return utils::ssprintf("R%d:0x%0.4x <= MEM[0x%0.4x]:0x%0.4x", reg_id, state.readReg(reg_id), mem_addr,
        std::get<0>(state.readMem(mem_addr)));
}

void MemWriteImmMicroOp::handleMicroOp(MachineState & state)
{
    PIMicroOp op = state.writeMemImm(addr, value);

    if(op) {
        insert(op);
    }
}

std::string MemWriteImmMicroOp::toString(MachineState const & state) const
{
    return utils::ssprintf("MEM[0x%0.4x]:0x%0.4x <= 0x%0.4x", addr, std::get<0>(state.readMem(addr)), value);
}

void CCUpdateMicroOp::handleMicroOp(MachineState & state)
{
    if(prev_result) {
        uint16_t prev_value = prev_result->get();
        uint16_t psr_value = std::get<0>(state.readMem(PSR));
        char cc_char = getCCChar(prev_value);
        if(cc_char == 'N') {
            state.writeMemImm(PSR, (psr_value & 0xFFF8) | 0x0004);
        } else if(cc_char == 'Z') {
            state.writeMemImm(PSR, (psr_value & 0xFFF8) | 0x0002);
        } else {
            state.writeMemImm(PSR, (psr_value & 0xFFF8) | 0x0001);
        }
    }
}

std::string CCUpdateMicroOp::toString(MachineState const & state) const
{
    if(prev_result) {
        char cur_cc_char;
        uint16_t prev_value = prev_result->get();
        uint16_t psr_value = std::get<0>(state.readMem(PSR));
        if((psr_value & 0x0004) != 0) {
            cur_cc_char = 'N';
        } else if((psr_value & 0x0002) != 0) {
            cur_cc_char = 'Z';
        } else {
            cur_cc_char = 'P';
        }

        return utils::ssprintf("CC:%c <= ComputeCC(0x%0.4x):%c", cur_cc_char, prev_value, getCCChar(prev_value));
    }

    return "invalid CC change";
}

char CCUpdateMicroOp::getCCChar(uint16_t value) const
{
    if((value & 0x8000) != 0) {
        return 'N';
    } else if(value == 0) {
        return 'Z';
    } else {
        return 'P';
    }
}
