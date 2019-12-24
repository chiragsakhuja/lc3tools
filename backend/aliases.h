/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#ifndef ALIASES_H
#define ALIASES_H

#include <map>
#include <memory>
#include <string>

namespace lc3
{
namespace core
{
    class IOperand;
    class IInstruction;
    class IEvent;
    class IDevice;

    using PIOperand = std::shared_ptr<IOperand>;
    using PIInstruction = std::shared_ptr<IInstruction>;
    using PIEvent = std::shared_ptr<IEvent>;
    using PIDevice = std::shared_ptr<IDevice>;

    using SymbolTable = std::map<std::string, uint32_t>;
};
};

#endif
