#ifndef CONSOLE_INPUTTER_H
#define CONSOLE_INPUTTER_H

namespace utils {
    class ConsoleInputter : public IInputter
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
