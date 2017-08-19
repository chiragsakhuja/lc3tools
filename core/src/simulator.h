#ifndef SIM_H
#define SIM_H

namespace core
{
    class Simulator
    {
    public:
        Simulator(bool log_enable, utils::IPrinter & printer, utils::IInputter & inputter);
        ~Simulator(void) = default;

        void loadObjectFile(std::string const & obj_file);
        void loadOS(void);
        void simulate(void);
        void reset(void);

        void registerPreInstructionCallback(std::function<void(MachineState & state)> func);
        void registerPostInstructionCallback(std::function<void(MachineState & state)> func);
        void registerInterruptEnterCallback(std::function<void(MachineState & state)> func);
        void registerInterruptExitCallback(std::function<void(MachineState & state)> func);

        MachineState & getMachineState(void) { return state; }
    private:
        InstructionDecoder decoder;
        bool pre_instruction_callback_v;
        bool post_instruction_callback_v;
        bool interrupt_enter_callback_v;;
        bool interrupt_exit_callback_v;;
        std::function<void(MachineState & state)> pre_instruction_callback;
        std::function<void(MachineState & state)> post_instruction_callback;
        std::function<void(MachineState & state)> interrupt_enter_callback;
        std::function<void(MachineState & state)> interrupt_exit_callback;

        MachineState state;

        Logger logger;
        utils::IInputter & inputter;

        std::atomic<bool> collecting_input;

        std::vector<core::IStateChange const *> executeInstruction(void);
        std::vector<core::IStateChange const *> checkAndSetupInterrupts(bool & interrupt_triggered);
        void executeChangeChain(std::vector<core::IStateChange const *> & changes);
        void updateDevices(void);
        void handleInput(void);
    };
};

#endif
