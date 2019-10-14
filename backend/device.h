/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#ifndef DEVICE_H
#define DEVICE_H

#include <functional>
#include <mutex>
#include <unordered_map>

#include "device_regs.h"

namespace lc3
{
namespace core
{

    extern std::mutex g_io_lock;

struct State;

class Device
{
public:
    using read_callback_t  = std::function<void(uint32_t, State &)>;
    using write_callback_t  = std::function<void(uint32_t, State &)>;

    virtual std::unordered_map<uint32_t, read_callback_t> getReadAddrMap(void) const = 0;
    virtual std::unordered_map<uint32_t, write_callback_t> getWriteAddrMap(void) const = 0;
};

class KeyboardDevice : public Device
{
public:
    virtual std::unordered_map<uint32_t, read_callback_t> getReadAddrMap(void) const override
    {
        return { { KBDR, readKBDR } };
    }

    virtual std::unordered_map<uint32_t, write_callback_t> getWriteAddrMap(void) const override
    {
        return { { KBSR, writeKBSR } };
    };

    static void readKBDR(uint32_t value, State & state)
    {
        std::lock_guard<std::mutex> guard(g_io_lock);
        state[KBSR] &= 0x7FFF;
 /*    if(addr == KBSR || addr == KBDR) {
 *        std::lock_guard<std::mutex> guard(g_io_lock);
 *        value = readMemRaw(addr);
 *        if(addr == KBDR) {
 *            change_mem = true;
 *            change = std::make_shared<MemWriteEvent>(KBSR, readMemRaw(KBSR) & 0x7FFF);
 *        } else if(addr == KBSR && (value & 0x8000) == 0) {
 *            change_mem = true;
 *            change = std::make_shared<CallbackEvent>(wait_for_input_callback_v, wait_for_input_callback);
 *        }
 *    } else {
 *        value = readMemRaw(addr);
 *    }
 */
    }

    static void writeKBSR(uint32_t data, State & state)
    {
    }
};

};
};

#endif
