#ifndef SIM_H
#define SIM_H

namespace core
{
    struct MachineState
    {
        std::vector<uint16_t> mem;
        std::array<uint32_t, 8> regs;
        uint32_t pc;
        uint32_t psr;
    };

    class Simulator
    {
    public:
        Simulator(bool log_enable, utils::Printer & printer);
        ~Simulator(void);

        void loadObjectFile(std::string const & filename);
        void simulate(void);
    private:
        std::function<void(void)> preInstructionCallback;
        MachineState state;

        Logger * logger;
        bool log_enable;
    };
};

#endif
