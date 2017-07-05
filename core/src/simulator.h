#ifndef SIM_H
#define SIM_H

namespace core
{
    class Simulator
    {
    public:
        Simulator(bool log_enable, utils::IPrinter & printer);
        ~Simulator(void) = default;

        void loadObjectFile(std::string const & obj_file);
        void simulate(void);
        void reset(void);
    private:
        InstructionDecoder decoder;
        std::function<void(void)> preInstructionCallback;
        MachineState state;

        Logger logger;
        bool log_enable;
    };
};

#endif
