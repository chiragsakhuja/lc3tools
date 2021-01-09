/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#ifndef DEVICE_REGS_H
#define DEVICE_REGS_H

#include <cstdint>

static constexpr uint16_t SYSTEM_START = 0x0000;
static constexpr uint16_t SYSTEM_END = 0x2FFF;
static constexpr uint16_t USER_START = 0x3000;
static constexpr uint16_t USER_END = 0xFDFF;
static constexpr uint16_t MMIO_START = 0xFE00;
static constexpr uint16_t MMIO_END = 0xFFFF;

static constexpr uint16_t RESET_PC = 0x0200;
static constexpr uint16_t SYSTEM_STACK_POINTER = 0x3000;
static constexpr uint16_t TRAP_TABLE_START = 0x0000;
static constexpr uint16_t INTEX_TABLE_START = 0x0100;
static constexpr uint16_t KBSR = 0xFE00;
static constexpr uint16_t KBDR = 0xFE02;
static constexpr uint16_t DSR = 0xFE04;
static constexpr uint16_t DDR = 0xFE06;
static constexpr uint16_t BSP = 0xFFFA;
static constexpr uint16_t PSR = 0xFFFC;
static constexpr uint16_t MCR = 0xFFFE;

#endif
