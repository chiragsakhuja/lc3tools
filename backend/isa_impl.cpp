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
    if((getOperand(1)->getValue() & (state.readPSR() & 0x0007)) != 0) {
        return std::make_shared<PCAddImmMicroOp>(lc3::utils::sextTo16(getOperand(2)->getValue(),
            getOperand(2)->getWidth()));
    }

    return nullptr;
}

PIMicroOp JMPInstruction::buildMicroOps(MachineState const & state) const
{
    (void) state;

    return std::make_shared<PCWriteRegMicroOp>(getOperand(2)->getValue());
}

PIMicroOp JSRInstruction::buildMicroOps(MachineState const & state) const
{
    (void) state;

    PIMicroOp link = std::make_shared<RegWritePCMicroOp>(7);
    PIMicroOp jump = std::make_shared<PCAddImmMicroOp>(lc3::utils::sextTo16(getOperand(2)->getValue(),
        getOperand(2)->getWidth()));
    // TODO: Add subroutine enter callback

    link->insert(jump);
    return link;
}

PIMicroOp JSRRInstruction::buildMicroOps(MachineState const & state) const
{
    (void) state;

    PIMicroOp link = std::make_shared<RegWritePCMicroOp>(7);
    PIMicroOp jump = std::make_shared<PCWriteRegMicroOp>(getOperand(2)->getValue());
    // TODO: Add subroutine enter callback

    link->insert(jump);
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
    PIMicroOp set_cc = std::make_shared<CCUpdateRegMicroOp>(8);

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
    // TODO: Fill in code
    return nullptr;
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
    // TODO: Fill in code
    return nullptr;
}
