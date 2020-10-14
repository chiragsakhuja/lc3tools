/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#ifndef FUNC_TYPE_H
#define FUNC_TYPE_H

#include <string>

namespace lc3
{
namespace core
{
    enum class FuncType
    {
          SUBROUTINE
        , TRAP
        , EXCEPTION
        , INTERRUPT
        , INVALID
    };

    std::string funcTypeToString(FuncType type);
};
};

#endif
