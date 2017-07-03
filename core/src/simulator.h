#ifndef SIM_H
#define SIM_H

namespace core
{
    class Simulator
    {
    public:
        Simulator(bool log_enable, utils::IPrinter & printer);
        ~Simulator(void) = default;

        void loadObjectFile(std::string const & filename);
        void simulate(void);
    private:
        InstructionDecoder decoder;
        std::function<void(void)> preInstructionCallback;
        MachineState state;

        Logger logger;
        bool log_enable;
    };
};

#endif
