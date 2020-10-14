/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <cstdint>
#include <string>

namespace lc3
{
namespace core
{
    enum class ExceptionType
    {
          PRIVILEGE_MODE
        , ILLEGAL_OPCODE
        , ACV
        , INVALID
    };

    enum class InterruptType
    {
          KEYBOARD
        , INVALID
    };

    uint8_t getExceptionVector(ExceptionType type);

    uint8_t getInterruptVector(InterruptType type);
    uint8_t getInterruptPriority(InterruptType type);
    std::string interruptTypeToString(InterruptType type);
};
};

#endif
