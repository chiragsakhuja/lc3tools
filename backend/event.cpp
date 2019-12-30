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
    return utils::ssprintf("Processing M[0x%0.4x]:0x%0.4x (%s)", state.readPC(),
        std::get<0>(state.readMem(state.readPC())), state.getMemLine(state.readPC()).c_str());
}

void LoadObjFileEvent::handleEvent(MachineState & state)
{
    uint32_t fill_pc = 0;
    uint32_t offset = 0;
    bool first_orig_set = false;

    // Verify header.
    std::string expected_header = utils::getMagicHeader();
    char * header = new char[expected_header.size()];
    if(buffer.read(header, expected_header.size())) {
        for(uint32_t i = 0; i < expected_header.size(); i += 1) {
            if(header[i] != expected_header[i]) {
                //logger.printf(PrintType::P_ERROR, true, "invalid header (is this a .obj file?); try re-assembling");
                throw utils::exception("invalid header (is this a .obj file?); try re-assembling");
            }
        }
    } else {
        //logger.printf(PrintType::P_ERROR, true, "could not read header");
        throw utils::exception("could not read header");
    }
    delete[] header;

    // Verify version number matches current version number.
    std::string expected_version = utils::getVersionString();
    char * version = new char[expected_version.size()];
    if(buffer.read(version, expected_version.size())) {
        for(uint32_t i = 0; i < expected_version.size(); i += 1) {
            if(version[i] != expected_version[i]) {
                //logger.printf(PrintType::P_ERROR, true, "mismatched version numbers; try re-assembling");
                throw utils::exception("mismatched version numbers; try re-assembling");
            }
        }
    } else {
        //logger.printf(PrintType::P_ERROR, true, "could not version number; try re-assembling");
        throw utils::exception("could not read version number; try re-assembling");
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
                state.writePC(mem.getValue());
                first_orig_set = true;
            }
            fill_pc = mem.getValue();
            offset = 0;
        } else {
            /*
             *logger.printf(lc3::utils::PrintType::P_DEBUG, true, "0x%0.4x: %s (0x%0.4x)", fill_pc + offset,
             *    statement.getLine().c_str(), statement.getValue());
             */
            state.writeMem(fill_pc + offset, mem.getValue());
            state.setMemLine(fill_pc + offset, mem.getLine());
            offset += 1;
        }

    }
}

std::string LoadObjFileEvent::toString(MachineState const & state) const
{
    (void) state;

    return utils::ssprintf("Loading %s into memory", filename.c_str());
}
