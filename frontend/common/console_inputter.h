/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#ifndef CONSOLE_INPUTTER_H
#define CONSOLE_INPUTTER_H

#include "inputter.h"

namespace lc3 {
    class ConsoleInputter : public utils::IInputter
    {
    public:
        ConsoleInputter(void) = default;
        ~ConsoleInputter(void) = default;

        virtual void beginInput(void) override;
        virtual bool getChar(char & c) override;
        virtual void endInput(void) override;

    private:
#if !(defined(WIN32) || defined(_WIN32) || defined(__WIN32))
        int kbhit(void);
#endif
    };
};

#endif
