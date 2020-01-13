#ifndef CALLBACK_H
#define CALLBACK_H

#include <functional>

namespace lc3
{
namespace core
{
    class MachineState;

    using CallbackTypeUnderlying = int64_t;

    enum class CallbackType : CallbackTypeUnderlying
    {
          PRE_INST = -2
        , POST_INST = 1
        , SUB_ENTER = 6
        , SUB_EXIT = 7
        , EX_ENTER = 2
        , EX_EXIT = 3
        , INT_ENTER = 4
        , INT_EXIT = 5
        , BREAKPOINT = -3
    };

    std::string callbackTypeToString(CallbackType type);
    CallbackTypeUnderlying callbackTypeToUnderlying(CallbackType type);
};
};

#endif
