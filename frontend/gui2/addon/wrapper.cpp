#include <nan.h>

#ifndef _PRINT_LEVEL
    #define _PRINT_LEVEL 4
#endif

#include "interface.h"
#include "ui_printer.h"

namespace utils
{
    class UIInputter : public lc3::utils::IInputter
    {
    public:
        void beginInput(void) {}
        bool getChar(char & c) { return false; }
        void endInput(void) {}
    };
};

utils::UIPrinter printer;
utils::UIInputter inputter;
lc3::as as(printer);
lc3::sim sim(printer, inputter);

NAN_METHOD(GetRegValue)
{
    uint32_t ret_val = 0;

    if(!info[0]->IsString()) {
        Nan::ThrowError("Argument must be a string");
        return;
    }

    v8::String::Utf8Value str(info[0]->ToString());
    std::string reg_name((char const *) *str);

    lc3::core::MachineState const & state = sim.getMachineState();
    if(reg_name[0] == 'r') {
        uint32_t reg_num = reg_name[1] - '0';
        if(reg_num > 7) {
            Nan::ThrowError("GPR must be R0 through R7");
            return;
        }
        ret_val = state.regs[reg_num];
    } else if(reg_name == "ir") {
        ret_val = state.mem[state.pc].getValue();
    } else if(reg_name == "psr") {
        ret_val = state.psr;
    } else if(reg_name == "pc") {
        ret_val =  state.pc;
    }

    auto ret = Nan::New<v8::Number>(ret_val);
    info.GetReturnValue().Set(ret);
}

NAN_METHOD(GetMemValue)
{
    if(!info[0]->IsNumber()) {
        Nan::ThrowError("Argument must be a string");
        return;
    }

    uint32_t addr = (uint32_t) info[0]->NumberValue();
    lc3::core::MachineState const & state = sim.getMachineState();
    auto ret = Nan::New<v8::Number>(state.mem[addr].getValue());
    info.GetReturnValue().Set(ret);
}

NAN_METHOD(GetMemLine)
{
    if(!info[0]->IsNumber()) {
        Nan::ThrowError("Argument must be a string");
        return;
    }

    uint32_t addr = (uint32_t) info[0]->NumberValue();
    lc3::core::MachineState const & state = sim.getMachineState();
    auto ret = Nan::New<v8::String>(state.mem[addr].getLine()).ToLocalChecked();
    info.GetReturnValue().Set(ret);
}

NAN_METHOD(InitializeSimulator) { }
NAN_METHOD(ClearOutputBuffer) { printer.clearOutputBuffer(); }

NAN_METHOD(Assemble)
{
    if(!info[0]->IsString()) {
        Nan::ThrowError("Argument must be a string");
        return;
    }

    v8::String::Utf8Value str(info[0]->ToString());

    std::string asm_filename((char const *) (*str));

    try {
        as.assemble(asm_filename);
    } catch(lc3::utils::exception const & e) {
        Nan::ThrowError(e.what());
    }
}

NAN_METHOD(GetOutput)
{
    std::vector<std::string> const & output = printer.getOutputBuffer();
    std::string joined = "";
    for(std::string const & str : output) { joined += str; }
    auto ret = Nan::New<v8::String>(joined).ToLocalChecked();
    info.GetReturnValue().Set(ret);
}

NAN_METHOD(ClearOutput)
{
    printer.clearOutputBuffer();
}

NAN_MODULE_INIT(Initialize)
{
    NAN_EXPORT(target, Assemble);
    NAN_EXPORT(target, GetOutput);
    NAN_EXPORT(target, InitializeSimulator);
    NAN_EXPORT(target, GetRegValue);
    NAN_EXPORT(target, GetMemValue);
    NAN_EXPORT(target, GetMemLine);
    NAN_EXPORT(target, ClearOutputBuffer);
}

NODE_MODULE(wrapper, Initialize);
