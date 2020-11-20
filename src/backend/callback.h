/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#ifndef CALLBACK_H
#define CALLBACK_H

#include <cstdint>
#include <functional>
#include <string>

namespace lc3
{
namespace core
{
    class MachineState;

    using CallbackTypeUnderlying = int64_t;

    enum class CallbackType : CallbackTypeUnderlying
    {
          BREAKPOINT = -3
        , PRE_INST = -2
        , INT_ENTER = -1
        , EX_ENTER = 1
        , EX_EXIT = 2
        , INT_EXIT = 3
        , SUB_ENTER = 4
        , SUB_EXIT = 5
        , INPUT_REQUEST = 6
        , INPUT_POLL = 7
        , POST_INST = 8
        , INVALID
    };

    std::string callbackTypeToString(CallbackType type);
    CallbackTypeUnderlying callbackTypeToUnderlying(CallbackType type);
};
};

namespace std
{
    template<>
    struct hash<lc3::core::CallbackType> {
        size_t operator()(lc3::core::CallbackType const & x) const {
            return std::hash<size_t>()(static_cast<size_t>(x));
        }
    };
};

#endif
