/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include "func_type.h"

std::string lc3::core::funcTypeToString(FuncType type)
{
    switch(type) {
        case FuncType::SUBROUTINE: return "subroutine";
        case FuncType::TRAP: return "trap";
        case FuncType::EXCEPTION: return "exception";
        case FuncType::INTERRUPT: return "interrupt";
        default: return "unknown";
    }
}
