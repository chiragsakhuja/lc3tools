#include <cassert>
#include <random>

#include "device_regs.h"
#include "interface.h"

constexpr char const * getOSStr(void);

lc3::sim::sim(utils::IPrinter & printer, utils::IInputter & inputter, uint32_t print_level, bool propagate_exceptions) :
    printer(printer), simulator(*this, printer, inputter, print_level), propagate_exceptions(propagate_exceptions)
{
    simulator.registerPreInstructionCallback(lc3::sim::preInstructionCallback);
    simulator.registerPostInstructionCallback(lc3::sim::postInstructionCallback);
    simulator.registerInterruptEnterCallback(lc3::sim::interruptEnterCallback);
    simulator.registerInterruptExitCallback(lc3::sim::interruptExitCallback);
    simulator.registerSubEnterCallback(lc3::sim::subEnterCallback);
    simulator.registerSubExitCallback(lc3::sim::subExitCallback);
    simulator.registerInputPollCallback(lc3::sim::inputPollCallback);
    if(propagate_exceptions) {
        loadOS();
    } else {
        try {
            loadOS();
        } catch(utils::exception const & e) {
#ifdef _ENABLE_DEBUG
            printer.print("caught exception: " + std::string(e.what()) + "\n");
#endif
        }
    }
    restart();
}

bool lc3::sim::loadObjectFile(std::string const & obj_filename)
{
    if(propagate_exceptions) {
        simulator.loadObjectFile(obj_filename);
    } else {
        try {
            simulator.loadObjectFile(obj_filename);
        } catch(utils::exception const & e) {
#ifdef _ENABLE_DEBUG
            printer.print("caught exception: " + std::string(e.what()) + "\n");
#endif
            return false;
        }
    }
    return true;
}

void lc3::sim::reinitialize(void)
{
    simulator.reinitialize();
    loadOS();
}

void lc3::sim::loadOS(void)
{
    as assembler(printer, simulator.getPrintLevel());

    std::stringstream src_buffer;
    src_buffer << getOSStr();
    std::stringstream obj_buffer = assembler.assembleBuffer(src_buffer);
    simulator.loadObjectFileFromBuffer(obj_buffer);
    getMachineState().pc = RESET_PC;
}

void lc3::sim::randomize(void)
{
    std::random_device dev;
    std::mt19937 gen(dev());
    std::uniform_int_distribution<> dis(0x0000, 0xffff);

    for(uint32_t i = 0x3000; i < 0xfe00; i += 1) {
        setMem(i, dis(gen));
    }

    for(uint32_t i = 0; i <= 7; i += 1) {
        setReg(i, dis(gen));
    }

    restart();
}

void lc3::sim::restart(void)
{
    uint32_t mcr = getMem(MCR);
    setMem(MCR, mcr | 0x8000);
}

void lc3::sim::setRunInstLimit(uint32_t inst_limit)
{
    target_inst_count = inst_exec_count + inst_limit;
    counted_run = true;
    step_out_run = false;
}

bool lc3::sim::run(void)
{
    restart();
    if(propagate_exceptions) {
        simulator.simulate();
    } else {
        try {
            simulator.simulate();
        } catch(utils::exception const & e) {
#ifdef _ENABLE_DEBUG
            printer.print("caught exception: " + std::string(e.what()) + "\n");
#endif
            return false;
        }
    }
    return true;
}

bool lc3::sim::runUntilHalt(void)
{
    until_halt_run = true;
    until_input_run = false;
    return run();
}

bool lc3::sim::runUntilInputPoll(void)
{
    until_halt_run = false;
    until_input_run = true;
    return run();
}

void lc3::sim::pause(void)
{
    simulator.pause();
}

bool lc3::sim::stepIn(void)
{
    setRunInstLimit(1);
    return run();
}

bool lc3::sim::stepOver(void)
{
    counted_run = false;
    step_out_run = true;
    until_halt_run = false;
    until_input_run = false;

    // this will immediately be incremented by the sub enter callback if it is about to enter a subroutine
    sub_depth = 0;
    return run();
}

bool lc3::sim::stepOut(void)
{
    counted_run = false;
    step_out_run = true;
    until_halt_run = false;
    until_input_run = false;
    // act like we are already in a subroutine
    sub_depth = 1;
    return run();
}

lc3::core::MachineState & lc3::sim::getMachineState(void) { return simulator.getMachineState(); }

lc3::core::MachineState const & lc3::sim::getMachineState(void) const { return simulator.getMachineState(); }

uint32_t lc3::sim::getInstExecCount(void) const { return inst_exec_count; }

std::vector<lc3::Breakpoint> const & lc3::sim::getBreakpoints(void) const { return breakpoints; }

uint32_t lc3::sim::getReg(uint32_t id) const
{
#ifdef _ENABLE_DEBUG
    assert(id <= 7);
#else
    id &= 0x7;
#endif
    return getMachineState().regs[id];
}

uint32_t lc3::sim::getMem(uint32_t addr) const
{
#ifdef _ENABLE_DEBUG
    assert(addr <= 0xffff);
#else
    addr &= 0xffff;
#endif
    return getMachineState().readMemRaw(addr);
}

std::string lc3::sim::getMemLine(uint32_t addr) const
{
#ifdef _ENABLE_DEBUG
    assert(addr <= 0xffff);
#else
    addr &= 0xffff;
#endif
    return getMachineState().mem[addr].getLine();
}

uint32_t lc3::sim::getPC(void) const { return getMachineState().pc; }

uint32_t lc3::sim::getPSR(void) const { return getMem(PSR); }

uint32_t lc3::sim::getMCR(void) const { return getMem(MCR); }

char lc3::sim::getCC(void) const
{
    uint32_t value = getPSR() & 0x7;
#ifdef _ENABLE_DEBUG
    assert(value == 0x4 || value == 0x2 || value == 0x1);
#else
    if(value == 0x4 || value == 0x2 || value == 0x1) {
        value = 0x1;
    }
#endif
    if(value == 0x4) { return 'N'; }
    else if(value == 0x2) { return 'Z'; }
    else { return 'P'; }
}

void lc3::sim::setReg(uint32_t id, uint32_t value)
{
#ifdef _ENABLE_DEBUG
    assert(id <= 7);
    assert(value <= 0xffff);
#else
    id &= 0x7;
    value &= 0xffff;
#endif
    getMachineState().regs[id] = value;
}

void lc3::sim::setMem(uint32_t addr, uint32_t value)
{
#ifdef _ENABLE_DEBUG
    assert(addr <= 0xffff);
    assert(value <= 0xffff);
#else
    addr &= 0xffff;
    value &= 0xffff;
#endif
    getMachineState().writeMemSafe(addr, value);
    getMachineState().mem[addr].setLine("");
}

void lc3::sim::setMemString(uint32_t addr, std::string const & value)
{
#ifdef _ENABLE_DEBUG
    assert(addr <= 0xffff);
#else
    addr &= 0xffff;
#endif
    for(uint32_t i = 0; i < value.size(); i += 1) {
        getMachineState().writeMemRaw(addr + i, static_cast<uint32_t>(value[i]));
        getMachineState().mem[addr + i].setLine(std::string(1, value[i]));
    }
    getMachineState().writeMemRaw(addr + value.size(), 0);
    getMachineState().mem[addr + value.size()].setLine(value);
}

void lc3::sim::setMemLine(uint32_t addr, std::string const & value)
{
#ifdef _ENABLE_DEBUG
    assert(addr <= 0xffff);
#else
    addr &= 0xffff;
#endif
    getMachineState().mem[addr].setLine(value);
}

void lc3::sim::setPC(uint32_t value)
{
#ifdef _ENABLE_DEBUG
    assert(value <= 0xfe00u);
#else
    value = std::min(value & 0xffff, 0xfe00u);
#endif
    getMachineState().pc = value;
    while(! getMachineState().sys_call_types.empty()) {
        getMachineState().sys_call_types.pop();
    }
    sub_depth = 0;

    // re-enable system
    restart();
}

void lc3::sim::setPSR(uint32_t value)
{
#ifdef _ENABLE_DEBUG
    assert((value & ((~0x8707) & 0xffff)) == 0x0000);
#else
    value &= 0xffff;
#endif
    setMem(PSR, value);
}

void lc3::sim::setMCR(uint32_t value)
{
#ifdef _ENABLE_DEBUG
    assert((value & ((~0x8000) & 0xffff)) == 0x0000);
#else
    value &= 0xffff;
#endif
    setMem(MCR, value);
}

void lc3::sim::setCC(char value)
{
    value |= 0x20;
#ifdef _ENABLE_DEBUG
    assert(value == 'n' || value == 'z' || value == 'p');
#else
    if(! (value == 'n' || value == 'z' || value == 'p')) { return; }
#endif
    uint32_t new_value = 0;
    if(value == 'n') { new_value = 0x4; }
    else if(value == 'z') { new_value = 0x2; }
    else { new_value = 0x1; }
    uint32_t psr = getPSR();
    setPSR((psr & 0xfff8) | new_value);
}

lc3::Breakpoint lc3::sim::setBreakpoint(uint32_t addr)
{
#ifdef _ENABLE_DEBUG
    assert(addr <= 0xffff);
#else
    addr &= 0xffff;
#endif
    Breakpoint bp(breakpoint_id, addr, this);
    breakpoints.push_back(bp);
    breakpoint_id += 1;
    return bp;
}

bool lc3::sim::removeBreakpointByID(uint32_t id)
{
    auto it = breakpoints.begin();
    bool found = false;
    for(; it != breakpoints.end(); ++it) {
        if(it->id == id) {
            found = true;
            break;
        }
    }

    if(found) {
        breakpoints.erase(it);
    }

    return found;
}

bool lc3::sim::removeBreakpointByAddr(uint32_t addr)
{
#ifdef _ENABLE_DEBUG
    assert(addr <= 0xffff);
#else
    addr &= 0xffff;
#endif
    auto it = breakpoints.begin();
    bool found = false;
    for(; it != breakpoints.end(); ++it) {
        if(it->loc == addr) {
            found = true;
            break;
        }
    }

    if(found) {
        breakpoints.erase(it);
    }

    return found;
}

void lc3::sim::registerPreInstructionCallback(callback_func_t func)
{
    pre_instruction_callback_v = true;
    pre_instruction_callback = func;
}

void lc3::sim::registerPostInstructionCallback(callback_func_t func)
{
    post_instruction_callback_v = true;
    post_instruction_callback = func;
}

void lc3::sim::registerInterruptEnterCallback(callback_func_t func)
{
    interrupt_enter_callback_v = true;
    interrupt_enter_callback = func;
}

void lc3::sim::registerInterruptExitCallback(callback_func_t func)
{
    interrupt_exit_callback_v = true;
    interrupt_exit_callback = func;
}

void lc3::sim::registerSubEnterCallback(callback_func_t func)
{
    sub_enter_callback_v = true;
    sub_enter_callback = func;
}


void lc3::sim::registerSubExitCallback(callback_func_t func)
{
    sub_exit_callback_v = true;
    sub_exit_callback = func;
}

void lc3::sim::registerBreakpointCallback(breakpoint_callback_func_t func)
{
    breakpoint_callback_v = true;
    breakpoint_callback = func;
}

lc3::utils::IPrinter const & lc3::sim::getPrinter(void) const { return printer; }
void lc3::sim::setPrintLevel(uint32_t print_level) { simulator.setPrintLevel(print_level); }
void lc3::sim::setPropagateExceptions(void) { propagate_exceptions = true; }
void lc3::sim::clearPropagateExceptions(void) { propagate_exceptions = false; }

void lc3::sim::preInstructionCallback(lc3::sim & sim_inst, lc3::core::MachineState & state)
{
    for(auto const & x : sim_inst.breakpoints) {
        if(state.pc == x.loc) {
            if(sim_inst.breakpoint_callback_v) {
                sim_inst.breakpoint_callback(state, x);
            }
            sim_inst.step_out_run = false;
            sim_inst.pause();
            break;
        }
    }

    if(sim_inst.until_halt_run && state.readMemRaw(state.pc) == 0xf025) {
        sim_inst.counted_run = false;
        sim_inst.step_out_run = false;
        sim_inst.until_halt_run = false;
        sim_inst.until_input_run = false;
        sim_inst.pause();
    }

    if(sim_inst.pre_instruction_callback_v) {
        sim_inst.pre_instruction_callback(state);
    }
}

void lc3::sim::postInstructionCallback(lc3::sim & sim_inst, core::MachineState & state)
{
    sim_inst.inst_exec_count += 1;
    if((sim_inst.counted_run && sim_inst.inst_exec_count == sim_inst.target_inst_count) ||
        (sim_inst.step_out_run && sim_inst.sub_depth <= 0))
    {
        sim_inst.counted_run = false;
        sim_inst.step_out_run = false;
        sim_inst.pause();
    }

    if(sim_inst.post_instruction_callback_v) {
        sim_inst.post_instruction_callback(state);
    }
}

void lc3::sim::interruptEnterCallback(lc3::sim & sim_inst, core::MachineState & state)
{
    sim_inst.sub_depth += 1;
    if(sim_inst.interrupt_enter_callback_v) {
        sim_inst.interrupt_enter_callback(state);
    }
}

void lc3::sim::interruptExitCallback(lc3::sim & sim_inst, core::MachineState & state)
{
    sim_inst.sub_depth -= 1;
    if(sim_inst.interrupt_exit_callback_v) {
        sim_inst.interrupt_exit_callback(state);
    }
}

void lc3::sim::subEnterCallback(lc3::sim & sim_inst, core::MachineState & state)
{
    sim_inst.sub_depth += 1;
    if(sim_inst.sub_enter_callback_v) {
        sim_inst.sub_enter_callback(state);
    }
}

void lc3::sim::subExitCallback(lc3::sim & sim_inst, core::MachineState & state)
{
    sim_inst.sub_depth -= 1;
    if(sim_inst.sub_exit_callback_v) {
        sim_inst.sub_exit_callback(state);
    }
}

void lc3::sim::inputPollCallback(lc3::sim & sim_inst, core::MachineState & state)
{
    if(sim_inst.until_input_run) {
        sim_inst.step_out_run = false;
        sim_inst.until_halt_run = false;
        sim_inst.until_input_run = false;
        sim_inst.pause();
    }

    if(sim_inst.input_poll_callback_v) {
        sim_inst.input_poll_callback(state);
    }
}

std::pair<bool, std::string> lc3::as::assemble(std::string const & asm_filename)
{
    std::string obj_filename(asm_filename.substr(0, asm_filename.find_last_of('.')) + ".obj");
    if(propagate_exceptions) {
        assembler.assemble(asm_filename, obj_filename);
    } else {
        try {
            assembler.assemble(asm_filename, obj_filename);
        } catch(utils::exception const & e) {
#ifdef _ENABLE_DEBUG
            printer.print("caught exception: " + std::string(e.what()) + "\n");
#endif
            return std::make_pair(false, obj_filename);
        }
    }
    return std::make_pair(true, obj_filename);
}

std::stringstream lc3::as::assembleBuffer(std::istream & buffer)
{
    return assembler.assembleBuffer(buffer);
}

std::pair<bool, std::string> lc3::conv::convertBin(std::string const & bin_filename)
{
    std::string obj_filename(bin_filename.substr(0, bin_filename.find_last_of('.')) + ".obj");
    if(propagate_exceptions) {
        converter.convertBin(bin_filename, obj_filename);
    } else {
        try {
            converter.convertBin(bin_filename, obj_filename);
        } catch(utils::exception const & e) {
#ifdef _ENABLE_DEBUG
            printer.print("caught exception: " + std::string(e.what()) + "\n");
#endif
            return std::make_pair(false, obj_filename);
        }
    }
    return std::make_pair(true, obj_filename);
}

void lc3::as::setPropagateExceptions(void) { propagate_exceptions = true; }
void lc3::as::clearPropagateExceptions(void) { propagate_exceptions = false; }


constexpr char const * getOSStr(void)
{
    char const * lc3os_src = R"LC3OS(
        .ORIG x0000

    ; the TRAP vector table
        .FILL BAD_TRAP    ; x00
        .FILL BAD_TRAP    ; x01
        .FILL BAD_TRAP    ; x02
        .FILL BAD_TRAP    ; x03
        .FILL BAD_TRAP    ; x04
        .FILL BAD_TRAP    ; x05
        .FILL BAD_TRAP    ; x06
        .FILL BAD_TRAP    ; x07
        .FILL BAD_TRAP    ; x08
        .FILL BAD_TRAP    ; x09
        .FILL BAD_TRAP    ; x0A
        .FILL BAD_TRAP    ; x0B
        .FILL BAD_TRAP    ; x0C
        .FILL BAD_TRAP    ; x0D
        .FILL BAD_TRAP    ; x0E
        .FILL BAD_TRAP    ; x0F
        .FILL BAD_TRAP    ; x10
        .FILL BAD_TRAP    ; x11
        .FILL BAD_TRAP    ; x12
        .FILL BAD_TRAP    ; x13
        .FILL BAD_TRAP    ; x14
        .FILL BAD_TRAP    ; x15
        .FILL BAD_TRAP    ; x16
        .FILL BAD_TRAP    ; x17
        .FILL BAD_TRAP    ; x18
        .FILL BAD_TRAP    ; x19
        .FILL BAD_TRAP    ; x1A
        .FILL BAD_TRAP    ; x1B
        .FILL BAD_TRAP    ; x1C
        .FILL BAD_TRAP    ; x1D
        .FILL BAD_TRAP    ; x1E
        .FILL BAD_TRAP    ; x1F
        .FILL TRAP_GETC   ; x20
        .FILL TRAP_OUT    ; x21
        .FILL TRAP_PUTS   ; x22
        .FILL TRAP_IN     ; x23
        .FILL TRAP_PUTSP  ; x24
        .FILL TRAP_HALT   ; x25
        .FILL BAD_TRAP    ; x26
        .FILL BAD_TRAP    ; x27
        .FILL BAD_TRAP    ; x28
        .FILL BAD_TRAP    ; x29
        .FILL BAD_TRAP    ; x2A
        .FILL BAD_TRAP    ; x2B
        .FILL BAD_TRAP    ; x2C
        .FILL BAD_TRAP    ; x2D
        .FILL BAD_TRAP    ; x2E
        .FILL BAD_TRAP    ; x2F
        .FILL BAD_TRAP    ; x30
        .FILL BAD_TRAP    ; x31
        .FILL BAD_TRAP    ; x32
        .FILL BAD_TRAP    ; x33
        .FILL BAD_TRAP    ; x34
        .FILL BAD_TRAP    ; x35
        .FILL BAD_TRAP    ; x36
        .FILL BAD_TRAP    ; x37
        .FILL BAD_TRAP    ; x38
        .FILL BAD_TRAP    ; x39
        .FILL BAD_TRAP    ; x3A
        .FILL BAD_TRAP    ; x3B
        .FILL BAD_TRAP    ; x3C
        .FILL BAD_TRAP    ; x3D
        .FILL BAD_TRAP    ; x3E
        .FILL BAD_TRAP    ; x3F
        .FILL BAD_TRAP    ; x40
        .FILL BAD_TRAP    ; x41
        .FILL BAD_TRAP    ; x42
        .FILL BAD_TRAP    ; x43
        .FILL BAD_TRAP    ; x44
        .FILL BAD_TRAP    ; x45
        .FILL BAD_TRAP    ; x46
        .FILL BAD_TRAP    ; x47
        .FILL BAD_TRAP    ; x48
        .FILL BAD_TRAP    ; x49
        .FILL BAD_TRAP    ; x4A
        .FILL BAD_TRAP    ; x4B
        .FILL BAD_TRAP    ; x4C
        .FILL BAD_TRAP    ; x4D
        .FILL BAD_TRAP    ; x4E
        .FILL BAD_TRAP    ; x4F
        .FILL BAD_TRAP    ; x50
        .FILL BAD_TRAP    ; x51
        .FILL BAD_TRAP    ; x52
        .FILL BAD_TRAP    ; x53
        .FILL BAD_TRAP    ; x54
        .FILL BAD_TRAP    ; x55
        .FILL BAD_TRAP    ; x56
        .FILL BAD_TRAP    ; x57
        .FILL BAD_TRAP    ; x58
        .FILL BAD_TRAP    ; x59
        .FILL BAD_TRAP    ; x5A
        .FILL BAD_TRAP    ; x5B
        .FILL BAD_TRAP    ; x5C
        .FILL BAD_TRAP    ; x5D
        .FILL BAD_TRAP    ; x5E
        .FILL BAD_TRAP    ; x5F
        .FILL BAD_TRAP    ; x60
        .FILL BAD_TRAP    ; x61
        .FILL BAD_TRAP    ; x62
        .FILL BAD_TRAP    ; x63
        .FILL BAD_TRAP    ; x64
        .FILL BAD_TRAP    ; x65
        .FILL BAD_TRAP    ; x66
        .FILL BAD_TRAP    ; x67
        .FILL BAD_TRAP    ; x68
        .FILL BAD_TRAP    ; x69
        .FILL BAD_TRAP    ; x6A
        .FILL BAD_TRAP    ; x6B
        .FILL BAD_TRAP    ; x6C
        .FILL BAD_TRAP    ; x6D
        .FILL BAD_TRAP    ; x6E
        .FILL BAD_TRAP    ; x6F
        .FILL BAD_TRAP    ; x70
        .FILL BAD_TRAP    ; x71
        .FILL BAD_TRAP    ; x72
        .FILL BAD_TRAP    ; x73
        .FILL BAD_TRAP    ; x74
        .FILL BAD_TRAP    ; x75
        .FILL BAD_TRAP    ; x76
        .FILL BAD_TRAP    ; x77
        .FILL BAD_TRAP    ; x78
        .FILL BAD_TRAP    ; x79
        .FILL BAD_TRAP    ; x7A
        .FILL BAD_TRAP    ; x7B
        .FILL BAD_TRAP    ; x7C
        .FILL BAD_TRAP    ; x7D
        .FILL BAD_TRAP    ; x7E
        .FILL BAD_TRAP    ; x7F
        .FILL BAD_TRAP    ; x80
        .FILL BAD_TRAP    ; x81
        .FILL BAD_TRAP    ; x82
        .FILL BAD_TRAP    ; x83
        .FILL BAD_TRAP    ; x84
        .FILL BAD_TRAP    ; x85
        .FILL BAD_TRAP    ; x86
        .FILL BAD_TRAP    ; x87
        .FILL BAD_TRAP    ; x88
        .FILL BAD_TRAP    ; x89
        .FILL BAD_TRAP    ; x8A
        .FILL BAD_TRAP    ; x8B
        .FILL BAD_TRAP    ; x8C
        .FILL BAD_TRAP    ; x8D
        .FILL BAD_TRAP    ; x8E
        .FILL BAD_TRAP    ; x8F
        .FILL BAD_TRAP    ; x90
        .FILL BAD_TRAP    ; x91
        .FILL BAD_TRAP    ; x92
        .FILL BAD_TRAP    ; x93
        .FILL BAD_TRAP    ; x94
        .FILL BAD_TRAP    ; x95
        .FILL BAD_TRAP    ; x96
        .FILL BAD_TRAP    ; x97
        .FILL BAD_TRAP    ; x98
        .FILL BAD_TRAP    ; x99
        .FILL BAD_TRAP    ; x9A
        .FILL BAD_TRAP    ; x9B
        .FILL BAD_TRAP    ; x9C
        .FILL BAD_TRAP    ; x9D
        .FILL BAD_TRAP    ; x9E
        .FILL BAD_TRAP    ; x9F
        .FILL BAD_TRAP    ; xA0
        .FILL BAD_TRAP    ; xA1
        .FILL BAD_TRAP    ; xA2
        .FILL BAD_TRAP    ; xA3
        .FILL BAD_TRAP    ; xA4
        .FILL BAD_TRAP    ; xA5
        .FILL BAD_TRAP    ; xA6
        .FILL BAD_TRAP    ; xA7
        .FILL BAD_TRAP    ; xA8
        .FILL BAD_TRAP    ; xA9
        .FILL BAD_TRAP    ; xAA
        .FILL BAD_TRAP    ; xAB
        .FILL BAD_TRAP    ; xAC
        .FILL BAD_TRAP    ; xAD
        .FILL BAD_TRAP    ; xAE
        .FILL BAD_TRAP    ; xAF
        .FILL BAD_TRAP    ; xB0
        .FILL BAD_TRAP    ; xB1
        .FILL BAD_TRAP    ; xB2
        .FILL BAD_TRAP    ; xB3
        .FILL BAD_TRAP    ; xB4
        .FILL BAD_TRAP    ; xB5
        .FILL BAD_TRAP    ; xB6
        .FILL BAD_TRAP    ; xB7
        .FILL BAD_TRAP    ; xB8
        .FILL BAD_TRAP    ; xB9
        .FILL BAD_TRAP    ; xBA
        .FILL BAD_TRAP    ; xBB
        .FILL BAD_TRAP    ; xBC
        .FILL BAD_TRAP    ; xBD
        .FILL BAD_TRAP    ; xBE
        .FILL BAD_TRAP    ; xBF
        .FILL BAD_TRAP    ; xC0
        .FILL BAD_TRAP    ; xC1
        .FILL BAD_TRAP    ; xC2
        .FILL BAD_TRAP    ; xC3
        .FILL BAD_TRAP    ; xC4
        .FILL BAD_TRAP    ; xC5
        .FILL BAD_TRAP    ; xC6
        .FILL BAD_TRAP    ; xC7
        .FILL BAD_TRAP    ; xC8
        .FILL BAD_TRAP    ; xC9
        .FILL BAD_TRAP    ; xCA
        .FILL BAD_TRAP    ; xCB
        .FILL BAD_TRAP    ; xCC
        .FILL BAD_TRAP    ; xCD
        .FILL BAD_TRAP    ; xCE
        .FILL BAD_TRAP    ; xCF
        .FILL BAD_TRAP    ; xD0
        .FILL BAD_TRAP    ; xD1
        .FILL BAD_TRAP    ; xD2
        .FILL BAD_TRAP    ; xD3
        .FILL BAD_TRAP    ; xD4
        .FILL BAD_TRAP    ; xD5
        .FILL BAD_TRAP    ; xD6
        .FILL BAD_TRAP    ; xD7
        .FILL BAD_TRAP    ; xD8
        .FILL BAD_TRAP    ; xD9
        .FILL BAD_TRAP    ; xDA
        .FILL BAD_TRAP    ; xDB
        .FILL BAD_TRAP    ; xDC
        .FILL BAD_TRAP    ; xDD
        .FILL BAD_TRAP    ; xDE
        .FILL BAD_TRAP    ; xDF
        .FILL BAD_TRAP    ; xE0
        .FILL BAD_TRAP    ; xE1
        .FILL BAD_TRAP    ; xE2
        .FILL BAD_TRAP    ; xE3
        .FILL BAD_TRAP    ; xE4
        .FILL BAD_TRAP    ; xE5
        .FILL BAD_TRAP    ; xE6
        .FILL BAD_TRAP    ; xE7
        .FILL BAD_TRAP    ; xE8
        .FILL BAD_TRAP    ; xE9
        .FILL BAD_TRAP    ; xEA
        .FILL BAD_TRAP    ; xEB
        .FILL BAD_TRAP    ; xEC
        .FILL BAD_TRAP    ; xED
        .FILL BAD_TRAP    ; xEE
        .FILL BAD_TRAP    ; xEF
        .FILL BAD_TRAP    ; xF0
        .FILL BAD_TRAP    ; xF1
        .FILL BAD_TRAP    ; xF2
        .FILL BAD_TRAP    ; xF3
        .FILL BAD_TRAP    ; xF4
        .FILL BAD_TRAP    ; xF5
        .FILL BAD_TRAP    ; xF6
        .FILL BAD_TRAP    ; xF7
        .FILL BAD_TRAP    ; xF8
        .FILL BAD_TRAP    ; xF9
        .FILL BAD_TRAP    ; xFA
        .FILL BAD_TRAP    ; xFB
        .FILL BAD_TRAP    ; xFC
        .FILL BAD_TRAP    ; xFD
        .FILL BAD_TRAP    ; xFE
        .FILL BAD_TRAP    ; xFF

    ; the interrupt vector table
        .FILL EX_PRIV   ; x00
        .FILL EX_ILL    ; x01
        .FILL BAD_INT    ; x02
        .FILL BAD_INT    ; x03
        .FILL BAD_INT    ; x04
        .FILL BAD_INT    ; x05
        .FILL BAD_INT    ; x06
        .FILL BAD_INT    ; x07
        .FILL BAD_INT    ; x08
        .FILL BAD_INT    ; x09
        .FILL BAD_INT    ; x0A
        .FILL BAD_INT    ; x0B
        .FILL BAD_INT    ; x0C
        .FILL BAD_INT    ; x0D
        .FILL BAD_INT    ; x0E
        .FILL BAD_INT    ; x0F
        .FILL BAD_INT    ; x10
        .FILL BAD_INT    ; x11
        .FILL BAD_INT    ; x12
        .FILL BAD_INT    ; x13
        .FILL BAD_INT    ; x14
        .FILL BAD_INT    ; x15
        .FILL BAD_INT    ; x16
        .FILL BAD_INT    ; x17
        .FILL BAD_INT    ; x18
        .FILL BAD_INT    ; x19
        .FILL BAD_INT    ; x1A
        .FILL BAD_INT    ; x1B
        .FILL BAD_INT    ; x1C
        .FILL BAD_INT    ; x1D
        .FILL BAD_INT    ; x1E
        .FILL BAD_INT    ; x1F
        .FILL BAD_INT    ; x20
        .FILL BAD_INT    ; x21
        .FILL BAD_INT    ; x22
        .FILL BAD_INT    ; x23
        .FILL BAD_INT    ; x24
        .FILL BAD_INT    ; x25
        .FILL BAD_INT    ; x26
        .FILL BAD_INT    ; x27
        .FILL BAD_INT    ; x28
        .FILL BAD_INT    ; x29
        .FILL BAD_INT    ; x2A
        .FILL BAD_INT    ; x2B
        .FILL BAD_INT    ; x2C
        .FILL BAD_INT    ; x2D
        .FILL BAD_INT    ; x2E
        .FILL BAD_INT    ; x2F
        .FILL BAD_INT    ; x30
        .FILL BAD_INT    ; x31
        .FILL BAD_INT    ; x32
        .FILL BAD_INT    ; x33
        .FILL BAD_INT    ; x34
        .FILL BAD_INT    ; x35
        .FILL BAD_INT    ; x36
        .FILL BAD_INT    ; x37
        .FILL BAD_INT    ; x38
        .FILL BAD_INT    ; x39
        .FILL BAD_INT    ; x3A
        .FILL BAD_INT    ; x3B
        .FILL BAD_INT    ; x3C
        .FILL BAD_INT    ; x3D
        .FILL BAD_INT    ; x3E
        .FILL BAD_INT    ; x3F
        .FILL BAD_INT    ; x40
        .FILL BAD_INT    ; x41
        .FILL BAD_INT    ; x42
        .FILL BAD_INT    ; x43
        .FILL BAD_INT    ; x44
        .FILL BAD_INT    ; x45
        .FILL BAD_INT    ; x46
        .FILL BAD_INT    ; x47
        .FILL BAD_INT    ; x48
        .FILL BAD_INT    ; x49
        .FILL BAD_INT    ; x4A
        .FILL BAD_INT    ; x4B
        .FILL BAD_INT    ; x4C
        .FILL BAD_INT    ; x4D
        .FILL BAD_INT    ; x4E
        .FILL BAD_INT    ; x4F
        .FILL BAD_INT    ; x50
        .FILL BAD_INT    ; x51
        .FILL BAD_INT    ; x52
        .FILL BAD_INT    ; x53
        .FILL BAD_INT    ; x54
        .FILL BAD_INT    ; x55
        .FILL BAD_INT    ; x56
        .FILL BAD_INT    ; x57
        .FILL BAD_INT    ; x58
        .FILL BAD_INT    ; x59
        .FILL BAD_INT    ; x5A
        .FILL BAD_INT    ; x5B
        .FILL BAD_INT    ; x5C
        .FILL BAD_INT    ; x5D
        .FILL BAD_INT    ; x5E
        .FILL BAD_INT    ; x5F
        .FILL BAD_INT    ; x60
        .FILL BAD_INT    ; x61
        .FILL BAD_INT    ; x62
        .FILL BAD_INT    ; x63
        .FILL BAD_INT    ; x64
        .FILL BAD_INT    ; x65
        .FILL BAD_INT    ; x66
        .FILL BAD_INT    ; x67
        .FILL BAD_INT    ; x68
        .FILL BAD_INT    ; x69
        .FILL BAD_INT    ; x6A
        .FILL BAD_INT    ; x6B
        .FILL BAD_INT    ; x6C
        .FILL BAD_INT    ; x6D
        .FILL BAD_INT    ; x6E
        .FILL BAD_INT    ; x6F
        .FILL BAD_INT    ; x70
        .FILL BAD_INT    ; x71
        .FILL BAD_INT    ; x72
        .FILL BAD_INT    ; x73
        .FILL BAD_INT    ; x74
        .FILL BAD_INT    ; x75
        .FILL BAD_INT    ; x76
        .FILL BAD_INT    ; x77
        .FILL BAD_INT    ; x78
        .FILL BAD_INT    ; x79
        .FILL BAD_INT    ; x7A
        .FILL BAD_INT    ; x7B
        .FILL BAD_INT    ; x7C
        .FILL BAD_INT    ; x7D
        .FILL BAD_INT    ; x7E
        .FILL BAD_INT    ; x7F
        .FILL BAD_INT    ; x80
        .FILL BAD_INT    ; x81
        .FILL BAD_INT    ; x82
        .FILL BAD_INT    ; x83
        .FILL BAD_INT    ; x84
        .FILL BAD_INT    ; x85
        .FILL BAD_INT    ; x86
        .FILL BAD_INT    ; x87
        .FILL BAD_INT    ; x88
        .FILL BAD_INT    ; x89
        .FILL BAD_INT    ; x8A
        .FILL BAD_INT    ; x8B
        .FILL BAD_INT    ; x8C
        .FILL BAD_INT    ; x8D
        .FILL BAD_INT    ; x8E
        .FILL BAD_INT    ; x8F
        .FILL BAD_INT    ; x90
        .FILL BAD_INT    ; x91
        .FILL BAD_INT    ; x92
        .FILL BAD_INT    ; x93
        .FILL BAD_INT    ; x94
        .FILL BAD_INT    ; x95
        .FILL BAD_INT    ; x96
        .FILL BAD_INT    ; x97
        .FILL BAD_INT    ; x98
        .FILL BAD_INT    ; x99
        .FILL BAD_INT    ; x9A
        .FILL BAD_INT    ; x9B
        .FILL BAD_INT    ; x9C
        .FILL BAD_INT    ; x9D
        .FILL BAD_INT    ; x9E
        .FILL BAD_INT    ; x9F
        .FILL BAD_INT    ; xA0
        .FILL BAD_INT    ; xA1
        .FILL BAD_INT    ; xA2
        .FILL BAD_INT    ; xA3
        .FILL BAD_INT    ; xA4
        .FILL BAD_INT    ; xA5
        .FILL BAD_INT    ; xA6
        .FILL BAD_INT    ; xA7
        .FILL BAD_INT    ; xA8
        .FILL BAD_INT    ; xA9
        .FILL BAD_INT    ; xAA
        .FILL BAD_INT    ; xAB
        .FILL BAD_INT    ; xAC
        .FILL BAD_INT    ; xAD
        .FILL BAD_INT    ; xAE
        .FILL BAD_INT    ; xAF
        .FILL BAD_INT    ; xB0
        .FILL BAD_INT    ; xB1
        .FILL BAD_INT    ; xB2
        .FILL BAD_INT    ; xB3
        .FILL BAD_INT    ; xB4
        .FILL BAD_INT    ; xB5
        .FILL BAD_INT    ; xB6
        .FILL BAD_INT    ; xB7
        .FILL BAD_INT    ; xB8
        .FILL BAD_INT    ; xB9
        .FILL BAD_INT    ; xBA
        .FILL BAD_INT    ; xBB
        .FILL BAD_INT    ; xBC
        .FILL BAD_INT    ; xBD
        .FILL BAD_INT    ; xBE
        .FILL BAD_INT    ; xBF
        .FILL BAD_INT    ; xC0
        .FILL BAD_INT    ; xC1
        .FILL BAD_INT    ; xC2
        .FILL BAD_INT    ; xC3
        .FILL BAD_INT    ; xC4
        .FILL BAD_INT    ; xC5
        .FILL BAD_INT    ; xC6
        .FILL BAD_INT    ; xC7
        .FILL BAD_INT    ; xC8
        .FILL BAD_INT    ; xC9
        .FILL BAD_INT    ; xCA
        .FILL BAD_INT    ; xCB
        .FILL BAD_INT    ; xCC
        .FILL BAD_INT    ; xCD
        .FILL BAD_INT    ; xCE
        .FILL BAD_INT    ; xCF
        .FILL BAD_INT    ; xD0
        .FILL BAD_INT    ; xD1
        .FILL BAD_INT    ; xD2
        .FILL BAD_INT    ; xD3
        .FILL BAD_INT    ; xD4
        .FILL BAD_INT    ; xD5
        .FILL BAD_INT    ; xD6
        .FILL BAD_INT    ; xD7
        .FILL BAD_INT    ; xD8
        .FILL BAD_INT    ; xD9
        .FILL BAD_INT    ; xDA
        .FILL BAD_INT    ; xDB
        .FILL BAD_INT    ; xDC
        .FILL BAD_INT    ; xDD
        .FILL BAD_INT    ; xDE
        .FILL BAD_INT    ; xDF
        .FILL BAD_INT    ; xE0
        .FILL BAD_INT    ; xE1
        .FILL BAD_INT    ; xE2
        .FILL BAD_INT    ; xE3
        .FILL BAD_INT    ; xE4
        .FILL BAD_INT    ; xE5
        .FILL BAD_INT    ; xE6
        .FILL BAD_INT    ; xE7
        .FILL BAD_INT    ; xE8
        .FILL BAD_INT    ; xE9
        .FILL BAD_INT    ; xEA
        .FILL BAD_INT    ; xEB
        .FILL BAD_INT    ; xEC
        .FILL BAD_INT    ; xED
        .FILL BAD_INT    ; xEE
        .FILL BAD_INT    ; xEF
        .FILL BAD_INT    ; xF0
        .FILL BAD_INT    ; xF1
        .FILL BAD_INT    ; xF2
        .FILL BAD_INT    ; xF3
        .FILL BAD_INT    ; xF4
        .FILL BAD_INT    ; xF5
        .FILL BAD_INT    ; xF6
        .FILL BAD_INT    ; xF7
        .FILL BAD_INT    ; xF8
        .FILL BAD_INT    ; xF9
        .FILL BAD_INT    ; xFA
        .FILL BAD_INT    ; xFB
        .FILL BAD_INT    ; xFC
        .FILL BAD_INT    ; xFD
        .FILL BAD_INT    ; xFE
        .FILL BAD_INT    ; xFF


    OS_START    ; machine starts executing at x0200
        LD R6,OS_SP            ; set system stack pointer
        LEA R0,OS_START_MSG    ; print a welcome message
        PUTS
        LDI R0,OS_PSR          ; go to user mode
        LD R1, MASK_HI
        NOT R0,R0
        AND R0,R0,R1
        NOT R0,R0
        ; normally this part would be in user space and the
        ; loader would set the PC
        STI R0,OS_PSR
        HALT

    OS_START_MSG    .STRINGZ ""

    OS_KBSR    .FILL xFE00
    OS_KBDR    .FILL xFE02
    OS_DSR     .FILL xFE04
    OS_DDR     .FILL xFE06
    OS_PSR     .FILL xFFFC
    OS_MCR     .FILL xFFFE
    OS_SP      .FILL x3000
    MASK_HI    .FILL x7FFF
    LOW_8_BITS .FILL x00FF
    TOUT_R1    .BLKW #1
    TIN_R7     .BLKW #1
    OS_R0      .BLKW #1
    OS_R1      .BLKW #1
    OS_R2      .BLKW #1
    OS_R3      .BLKW #1
    OS_R7      .BLKW #1


    TRAP_GETC
        LDI R0,OS_KBSR        ; wait for a keystroke
        BRzp TRAP_GETC
        LDI R0,OS_KBDR        ; read it and return
        RTI

    TRAP_OUT
        ST R1,TOUT_R1        ; save R1
    TRAP_OUT_WAIT
        LDI R1,OS_DSR        ; wait for the display to be ready
        BRzp TRAP_OUT_WAIT
        STI R0,OS_DDR        ; write the character and return
        LD R1,TOUT_R1        ; restore R1
        RTI

    TRAP_PUTS
        ST R0,OS_R0          ; save R0, R1, and R7
        ST R1,OS_R1
        ST R7,OS_R7
        ADD R1,R0,#0         ; move string pointer (R0) into R1
    TRAP_PUTS_LOOP
        LDR R0,R1,#0         ; write characters in string using OUT
        BRz TRAP_PUTS_DONE
        OUT
        ADD R1,R1,#1
        BRnzp TRAP_PUTS_LOOP
    TRAP_PUTS_DONE
        LD R0,OS_R0          ; restore R0, R1, and R7
        LD R1,OS_R1
        LD R7,OS_R7
        RTI

    TRAP_IN
        ST R7,TIN_R7       ; save R7 (no need to save R0, since we
                           ; overwrite later
        LEA R0,TRAP_IN_MSG ; prompt for input
        PUTS
        GETC               ; read a character
        OUT                ; echo back to monitor
        ST R0,OS_R0        ; save the character
        AND R0,R0,#0       ; write a linefeed, too
        ADD R0,R0,#10
        OUT
        LD R0,OS_R0        ; restore the character
        LD R7,TIN_R7       ; restore R7
        RTI

    TRAP_PUTSP
        ; NOTE: This trap will end when it sees any NUL, even in
        ; packed form, despite the P&P second edition's requirement
        ; of a double NUL.

        ST R0,OS_R0         ; save R0, R1, R2, R3, and R7
        ST R1,OS_R1
        ST R2,OS_R2
        ST R3,OS_R3
        ST R7,OS_R7
        ADD R1,R0,#0        ; move string pointer (R0) into R1
    TRAP_PUTSP_LOOP
        LDR R2,R1,#0        ; read the next two characters
        LD R0,LOW_8_BITS    ; use mask to get low byte
        AND R0,R0,R2        ; if low byte is NUL, quit printing
        BRz TRAP_PUTSP_DONE
        OUT                 ; otherwise print the low byte
        AND R0,R0,#0        ; shift high byte into R0
        ADD R3,R0,#8
    TRAP_PUTSP_S_LOOP
        ADD R0,R0,R0        ; shift R0 left
        ADD R2,R2,#0        ; move MSB from R2 into R0
        BRzp TRAP_PUTSP_MSB_0
        ADD R0,R0,#1
    TRAP_PUTSP_MSB_0
        ADD R2,R2,R2        ; shift R2 left
        ADD R3,R3,#-1
        BRp TRAP_PUTSP_S_LOOP
        ADD R0,R0,#0        ; if high byte is NUL, quit printing
        BRz TRAP_PUTSP_DONE
        OUT                 ; otherwise print the low byte
        ADD R1,R1,#1        ; and keep going
        BRnzp TRAP_PUTSP_LOOP
    TRAP_PUTSP_DONE
        LD R0,OS_R0         ; restore R0, R1, R2, R3, and R7
        LD R1,OS_R1
        LD R2,OS_R2
        LD R3,OS_R3
        LD R7,OS_R7
        RTI

    TRAP_HALT
        ; an infinite loop of lowering OS_MCR's MSB
        LEA R0,TRAP_HALT_MSG ; give a warning
        PUTS
        LDI R0,OS_MCR        ; halt the machine
        LD R1,MASK_HI
        AND R0,R0,R1
        STI R0,OS_MCR
        BRnzp TRAP_HALT      ; HALT again...

    BAD_TRAP
        ; print an error message, then HALT
        LEA R0,BAD_TRAP_MSG    ; give an error message
        PUTS
        HALT                   ; execute HALT

    EX_PRIV
        ; print an error message, then HALT
        LEA R0,EX_PRIV_MSG     ; give an error message
        PUTS
        HALT                   ; execute HALT

    EX_ILL
        ; print an error message, then HALT
        LEA R0,EX_ILL_MSG      ; give an error message
        PUTS
        HALT                   ; execute HALT

    BAD_INT        RTI

    TRAP_IN_MSG    .STRINGZ "\nInput a character> "
    TRAP_HALT_MSG  .STRINGZ "\n\n--- Halting the LC-3 ---\n\n"
    EX_PRIV_MSG    .STRINGZ "\n\n--- Access violation ---\n\n"
    EX_ILL_MSG     .STRINGZ "\n\n--- Illegal opcode ---\n\n"
    BAD_TRAP_MSG   .STRINGZ "\n\n--- Undefined trap executed ---\n\n"

        .END
    )LC3OS";

    return lc3os_src;
};
