#ifndef INPUTTER_H
#define INPUTTER_H

namespace utils
{
    class IInputter
    {
    public:
        IInputter(void) = default;
        virtual ~IInputter(void) = default;

        virtual void beginInput(void) = 0;
        virtual bool getChar(char & c) = 0;
        virtual void endInput(void) = 0;
    };
};

#endif
