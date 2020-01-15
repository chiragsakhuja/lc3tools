#include "event.h"
#include "uop.h"
#include "state.h"

#include "utils.h"

using namespace lc3::core;

void AtomicInstProcessEvent::handleEvent(MachineState & state)
{
    (void) state;

    PIMicroOp fetch = std::make_shared<FetchMicroOp>();
    PIMicroOp inc_pc = std::make_shared<PCAddImmMicroOp>(1);
    PIMicroOp decode = std::make_shared<DecodeMicroOp>(decoder);

    fetch->insert(inc_pc);
    inc_pc->insert(decode);

    uops = fetch;
}

std::string AtomicInstProcessEvent::toString(MachineState const & state) const
{
    return lc3::utils::ssprintf("Processing M[0x%0.4x]:0x%0.4x (%s)", state.readPC(),
        std::get<0>(state.readMem(state.readPC())), state.getMemLine(state.readPC()).c_str());
}

void StartupEvent::handleEvent(MachineState & state)
{
    uint16_t reset_pc = state.readResetPC();
    state.writePC(reset_pc == 0x0000 ? RESET_PC : reset_pc);
    state.writeMCR(0x8000);

    // Clear privilege bit and condition codes.
    uint16_t psr_value = state.readPSR();
    psr_value &= 0x7FF8;
    if(USER_START <= reset_pc && reset_pc <= USER_END) {
        // Set privilege bit to user mode and condition codes to Z.
        psr_value |= 0x8002;
        state.writePSR(psr_value);
        state.writeSSP(USER_START);
    } else {
        // Set condition codes to Z.
        psr_value |= 0x0002;
        state.writePSR(psr_value);
        // Don't initialize stack pointer if in program starts in system mode.
    }
}

std::string StartupEvent::toString(MachineState const & state) const
{
    (void) state;

    return "Starting machine";
}

void LoadObjFileEvent::handleEvent(MachineState & state)
{
    using namespace lc3::utils;

    uint32_t fill_pc = 0;
    uint32_t offset = 0;
    bool first_orig_set = false;

    // Verify header.
    std::string expected_header = lc3::utils::getMagicHeader();
    char * header = new char[expected_header.size()];
    if(buffer.read(header, expected_header.size())) {
        for(uint32_t i = 0; i < expected_header.size(); i += 1) {
            if(header[i] != expected_header[i]) {
                logger.printf(PrintType::P_ERROR, true, "invalid header (is this a .obj file?); try re-assembling");
                throw lc3::utils::exception("invalid header (is this a .obj file?); try re-assembling");
            }
        }
    } else {
        //logger.printf(PrintType::P_ERROR, true, "could not read header");
        throw lc3::utils::exception("could not read header");
    }
    delete[] header;

    // Verify version number matches current version number.
    std::string expected_version = lc3::utils::getVersionString();
    char * version = new char[expected_version.size()];
    if(buffer.read(version, expected_version.size())) {
        for(uint32_t i = 0; i < expected_version.size(); i += 1) {
            if(version[i] != expected_version[i]) {
                logger.printf(PrintType::P_ERROR, true, "mismatched version numbers; try re-assembling");
                throw lc3::utils::exception("mismatched version numbers; try re-assembling");
            }
        }
    } else {
        logger.printf(PrintType::P_ERROR, true, "could not version number; try re-assembling");
        throw lc3::utils::exception("could not read version number; try re-assembling");
    }
    delete[] version;

    while(! buffer.eof()) {
        MemLocation mem;
        buffer >> mem;

        if(buffer.eof()) {
            break;
        }

        if(mem.isOrig()) {
            if(! first_orig_set) {
                state.writeResetPC(mem.getValue());
                first_orig_set = true;
            }
            fill_pc = mem.getValue();
            offset = 0;
        } else {
            logger.printf(lc3::utils::PrintType::P_DEBUG, true, "0x%0.4x: %s (0x%0.4x)", fill_pc + offset,
                mem.getLine().c_str(), mem.getValue());
            state.writeMem(fill_pc + offset, mem.getValue());
            state.setMemLine(fill_pc + offset, mem.getLine());
            offset += 1;
        }

    }
}

std::string LoadObjFileEvent::toString(MachineState const & state) const
{
    (void) state;

    return lc3::utils::ssprintf("Loading %s into memory", filename.c_str());
}

void DeviceUpdateEvent::handleEvent(MachineState & state)
{
    (void) state;

    uops = device->tick();
}

std::string DeviceUpdateEvent::toString(MachineState const & state) const
{
    (void) state;

    return lc3::utils::ssprintf("Updating %s", device->getName().c_str());
}

void CheckForInterruptEvent::handleEvent(MachineState & state)
{
    InterruptType interrupt = state.peekInterrupt();
    if(interrupt != InterruptType::INVALID && (getInterruptPriority(interrupt) > ((state.readPSR() & 0x0700) >> 8))) {
        std::pair<PIMicroOp, PIMicroOp> handle_interrupt_chain = buildSystemModeEnter(INTEX_TABLE_START,
            getInterruptVector(interrupt), getInterruptPriority(interrupt)
        );
        PIMicroOp dequeue_interrupt = std::make_shared<PopInterruptType>();

        handle_interrupt_chain.second->insert(dequeue_interrupt);

        uops = handle_interrupt_chain.first;
    }
}

std::string CheckForInterruptEvent::toString(MachineState const & state) const
{
    InterruptType interrupt = state.peekInterrupt();
    if(interrupt != InterruptType::INVALID && (getInterruptPriority(interrupt) > ((state.readPSR() & 0x0700) >> 8))) {
        return lc3::utils::ssprintf("Handling %s interrupt", interruptTypeToString(interrupt).c_str());
    } else {
        return "No interrupt of higher priority pending";
    }
}

void CallbackEvent::handleEvent(MachineState & state)
{
    func(type, state);
}

std::string CallbackEvent::toString(MachineState const & state) const
{
    (void) state;

    return lc3::utils::ssprintf("Triggering %s callback", callbackTypeToString(type).c_str());
}
