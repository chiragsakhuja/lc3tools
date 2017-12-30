#ifndef INPUTTER_H
#define INPUTTER_H

namespace lc3::utils
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

    class NullInputter : public IInputter
    {
    public:
        virtual void beginInput(void) override {}
        virtual bool getChar(char &) override { return false; }
        virtual void endInput(void) override {}
    };
};

#endif
