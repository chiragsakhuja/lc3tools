#include <nan.h>

#ifndef _PRINT_LEVEL
    #define _PRINT_LEVEL 4
#endif

#include "core.h"
#include "ui_printer.h"

namespace utils
{
    class UIInputter : public IInputter
    {
    public:
        void beginInput(void) {}
        bool getChar(char & c) { return false; }
        void endInput(void) {}
    };
};

utils::UIPrinter printer;
utils::UIInputter inputter;
core::lc3 interface(printer, inputter);

uint32_t getRegValue(std::string const & reg_name)
{
    core::MachineState const & state = interface.getMachineState();
    if(reg_name[0] == 'r') {
        uint32_t reg_num = reg_name[1] - '0';
        if(reg_num > 7) {
            throw std::runtime_error("Register must be R0 through R7");
        }
        return state.regs[reg_num];
    } else if(reg_name == "ir") {
        return state.mem[state.pc].getValue();
    } else if(reg_name == "psr") {
        return state.psr;
    } else if(reg_name == "pc") {
        return state.pc;
    } else {
        return 0;
    }
}

NAN_METHOD(InitializeSimulator) { interface.initializeSimulator(); }
NAN_METHOD(ClearOutputBuffer) { printer.clearOutputBuffer(); }

NAN_METHOD(GetRegValueDec)
{
    if(!info[0]->IsString()) {
        Nan::ThrowError("Argument must be a string");
        return;
    }

    v8::String::Utf8Value str(info[0]->ToString());
    std::string reg_name((char const *) *str);

    uint32_t value = 0;
    try {
        value = getRegValue(reg_name);
    } catch(std::exception const & e) {
        Nan::ThrowError(e.what());
        return;
    }

    std::string formatted = std::to_string(value);
    auto ret = Nan::New<v8::String>(formatted).ToLocalChecked();
    info.GetReturnValue().Set(ret);
}

NAN_METHOD(GetRegValueHex)
{
    if(!info[0]->IsString()) {
        Nan::ThrowError("Argument must be a string");
        return;
    }

    v8::String::Utf8Value str(info[0]->ToString());
    std::string reg_name((char const *) *str);

    uint32_t value = 0;
    try {
        value = getRegValue(reg_name);
    } catch(std::exception const & e) {
        Nan::ThrowError(e.what());
        return;
    }

    std::string formatted = utils::ssprintf("0x%0.4X", value);
    auto ret = Nan::New<v8::String>(formatted).ToLocalChecked();
    info.GetReturnValue().Set(ret);
}

NAN_METHOD(Assemble)
{
    if(!info[0]->IsString()) {
        Nan::ThrowError("Argument must be a string");
        return;
    }

    v8::String::Utf8Value str(info[0]->ToString());

    std::string asm_filename((char const *) (*str));
    std::string obj_filename(asm_filename.substr(0, asm_filename.find_last_of('.')) + ".obj");

    try {
        interface.assemble(asm_filename, obj_filename);
    } catch(utils::exception const & e) {
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

NAN_MODULE_INIT(Initialize)
{
    NAN_EXPORT(target, Assemble);
    NAN_EXPORT(target, GetOutput);
    NAN_EXPORT(target, InitializeSimulator);
    NAN_EXPORT(target, GetRegValueHex);
    NAN_EXPORT(target, GetRegValueDec);
    NAN_EXPORT(target, ClearOutputBuffer);
}

NODE_MODULE(wrapper, Initialize);
