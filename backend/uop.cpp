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

std::string IMicroOp::regToString(uint16_t reg_id) const
{
    if(reg_id < 8) {
        return utils::ssprintf("R%d", reg_id);
    } else {
        return utils::ssprintf("T%d", reg_id - 8);
    }
}

void FetchMicroOp::handleMicroOp(MachineState & state)
{
    uint16_t value = std::get<0>(state.readMem(state.readPC()));
    state.writeIR(value);
}

std::string FetchMicroOp::toString(MachineState const & state) const
{
    return utils::ssprintf("IR:0x%0.4hx <= M[0x%0.4hx]:0x%0.4hx", state.readIR(), state.readPC(),
        std::get<0>(state.readMem(state.readPC())));
}

void DecodeMicroOp::handleMicroOp(MachineState & state)
{
    lc3::optional<PIInstruction> inst = decoder.decode(state.readIR());
    if(inst) {
        insert((*inst)->buildMicroOps(state));
        state.writeDecodedIR(*inst);
    } else {
        // TODO: Handle illegal instruction
    }
}

std::string DecodeMicroOp::toString(MachineState const & state) const
{
    // TODO: Print out as dIR <= ...
    return utils::ssprintf("Decoding IR:0x%0.4hx", state.readIR());
}

void PCWriteRegMicroOp::handleMicroOp(MachineState & state)
{
    uint16_t value = state.readReg(reg_id);
    state.writePC(value);
}

std::string PCWriteRegMicroOp::toString(MachineState const & state) const
{
    return utils::ssprintf("PC:0x%0.4hx <= %s:0x%0.4hx", state.readPC(), regToString(reg_id).c_str(),
        state.readReg(reg_id));
}

void PCAddImmMicroOp::handleMicroOp(MachineState & state)
{
    uint16_t value = state.readPC() + amnt;
    state.writePC(value);
}

std::string PCAddImmMicroOp::toString(MachineState const & state) const
{
    return utils::ssprintf("PC:0x%0.4hx <= (PC:0x%0.4hx + #%d):0x%0.4hx", state.readPC(), state.readPC(), amnt,
        state.readPC() + amnt);
}

void RegWriteRegMicroOp::handleMicroOp(MachineState & state)
{
    state.writeReg(dst_id, state.readReg(src_id));
}

std::string RegWriteRegMicroOp::toString(MachineState const & state) const
{
    return utils::ssprintf("%s:0x%0.4hx <= %s:0x%0.4hx", regToString(dst_id).c_str(), state.readReg(dst_id),
        regToString(src_id).c_str(), state.readReg(src_id));
}

void RegWritePCMicroOp::handleMicroOp(MachineState & state)
{
    state.writeReg(reg_id, state.readPC());
}

std::string RegWritePCMicroOp::toString(MachineState const & state) const
{
    return utils::ssprintf("%s:0x%0.4hx <= PC:0x%0.4hx", regToString(reg_id).c_str(), state.readReg(reg_id),
        state.readPC());
}

void RegAddImmMicroOp::handleMicroOp(MachineState & state)
{
    uint16_t value = state.readReg(src_id) + amnt;
    state.writeReg(dst_id, value);
}

std::string RegAddImmMicroOp::toString(MachineState const & state) const
{
    return utils::ssprintf("%s:0x%0.4hx <= (%s:0x%0.4hx + #%d):0x%0.4hx", regToString(dst_id).c_str(),
        state.readReg(dst_id), regToString(src_id).c_str(), state.readReg(src_id), amnt, state.readReg(src_id) + amnt);
}

void RegAddRegMicroOp::handleMicroOp(MachineState & state)
{
    uint16_t value = state.readReg(src_id1) + state.readReg(src_id2);
    state.writeReg(dst_id, value);
}

std::string RegAddRegMicroOp::toString(MachineState const & state) const
{
    return utils::ssprintf("%s:0x%0.4hx <= (%s:0x%0.4hx + %s:0x%0.4hx):0x%0.4hx", regToString(dst_id).c_str(),
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
    return utils::ssprintf("%s:0x%0.4hx <= (%s:0x%0.4hx & #%d):0x%0.4hx", regToString(dst_id).c_str(),
        state.readReg(dst_id), regToString(src_id).c_str(), state.readReg(src_id), amnt, state.readReg(src_id) & amnt);
}

void RegAndRegMicroOp::handleMicroOp(MachineState & state)
{
    uint16_t value = state.readReg(src_id1) & state.readReg(src_id2);
    state.writeReg(dst_id, value);
}

std::string RegAndRegMicroOp::toString(MachineState const & state) const
{
    return utils::ssprintf("%s:0x%0.4hx <= (%s:0x%0.4hx & %s:0x%0.4hx):0x%0.4hx", regToString(dst_id).c_str(),
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
    return utils::ssprintf("%s:0x%0.4hx <= (~%s:0x%0.4hx):0x%0.4hx", regToString(dst_id).c_str(),
        state.readReg(dst_id), regToString(src_id).c_str(), state.readReg(src_id),
        ~state.readReg(src_id));
}

void MemReadMicroOp::handleMicroOp(MachineState & state)
{
    std::pair<uint16_t, PIMicroOp> read_result = state.readMem(state.readReg(addr_reg_id));

    uint16_t value = std::get<0>(read_result);
    PIMicroOp op = std::get<1>(read_result);

    if(op) {
        insert(op);
    }

    state.writeReg(dst_id, value);
}

std::string MemReadMicroOp::toString(MachineState const & state) const
{
    return utils::ssprintf("%s:0x%0.4hx <= MEM[%s:0x%0.4hx]:0x%0.4hx", regToString(dst_id).c_str(),
        state.readReg(dst_id), regToString(addr_reg_id).c_str(), state.readReg(addr_reg_id),
        std::get<0>(state.readMem(state.readReg(addr_reg_id))));
}

void MemWriteImmMicroOp::handleMicroOp(MachineState & state)
{
    PIMicroOp op = state.writeMemImm(state.readReg(addr_reg_id), value);

    if(op) {
        insert(op);
    }
}

std::string MemWriteImmMicroOp::toString(MachineState const & state) const
{
    return utils::ssprintf("MEM[%s:0x%0.4hx]:0x%0.4hx <= 0x%0.4hx", regToString(addr_reg_id).c_str(),
        std::get<0>(state.readMem(state.readReg(addr_reg_id))), value);
}

void MemWriteRegMicroOp::handleMicroOp(MachineState & state)
{
    PIMicroOp op = state.writeMemImm(state.readReg(addr_reg_id), state.readReg(src_id));

    if(op) {
        insert(op);
    }
}

std::string MemWriteRegMicroOp::toString(MachineState const & state) const
{
    return utils::ssprintf("MEM[%s:0x%0.4hx]:0x%0.4hx <= %s:0x%0.4hx", regToString(addr_reg_id).c_str(),
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

    return utils::ssprintf("CC:%c <= ComputeCC(0x%0.4hx):%c", cur_cc_char, state.readReg(reg_id),
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
