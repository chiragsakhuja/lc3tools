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
          PRE_INST = -1
        , POST_INST = 8
        , SUB_ENTER = 5
        , SUB_EXIT = 6
        , EX_ENTER = 1
        , EX_EXIT = 2
        , INT_ENTER = 3
        , INT_EXIT = 4
        , BREAKPOINT = -2
        , INPUT_POLL = 7
        , INVALID
    };

    std::string callbackTypeToString(CallbackType type);
    CallbackTypeUnderlying callbackTypeToUnderlying(CallbackType type);
};
};

#endif
