#include <nan.h>

#ifndef _PRINT_LEVEL
    #define _PRINT_LEVEL 4
#endif

#include <algorithm>

#include "device_regs.h"
#include "interface.h"
#include "ui_printer.h"
#include "ui_inputter.h"

utils::UIPrinter *printer = nullptr;
utils::UIInputter *inputter = nullptr;
lc3::as *as = nullptr;
lc3::sim *sim = nullptr;

class SimulatorAsyncWorker : public Nan::AsyncWorker
{
public:
    SimulatorAsyncWorker(Nan::Callback * callback) : Nan::AsyncWorker(callback) {}

    void Execute(void)
    {
        try {
            sim->run();
        } catch(lc3::utils::exception const & e) {
            this->SetErrorMessage(e.what());
        }
    }

    void HandleOKCallback(void) {
        Nan::HandleScope scope;
        v8::Local<v8::Value> argv[] = {
            Nan::Null()
        };
        Nan::Call(callback->GetFunction(), Nan::GetCurrentContext()->Global(), 1, argv);
    }

    void HandleErrorCallback(void) {
        Nan::HandleScope scope;
        v8::Local<v8::Value> argv[] = {
            Nan::New(this->ErrorMessage()).ToLocalChecked()
        };
        Nan::Call(callback->GetFunction(), Nan::GetCurrentContext()->Global(), 1, argv);
    }
};

NAN_METHOD(Init)
{
    if(!info[0]->IsString()) {
        Nan::ThrowError("Must provide OS path as a string argument");
        return;
    }

    v8::String::Utf8Value str(info[0]->ToString());
    std::string os_path((char const *) *str);

    try {
        printer = new utils::UIPrinter();
        inputter = new utils::UIInputter();
        as = new lc3::as(*printer);
        sim = new lc3::sim(*printer, *inputter, _PRINT_LEVEL, os_path);
    } catch(lc3::utils::exception const & e) {
        Nan::ThrowError(e.what());
    }
}

NAN_METHOD(Shutdown)
{
    delete sim;
    delete as;
    delete inputter;
    delete printer;
}

NAN_METHOD(Assemble)
{
    if(!info[0]->IsString()) {
        Nan::ThrowError("Must provide filename as a string argument");
        return;
    }

    v8::String::Utf8Value str(info[0]->ToString());

    std::string asm_filename((char const *) (*str));

    try {
        as->assemble(asm_filename);
    } catch(lc3::utils::exception const & e) {
        Nan::ThrowError(e.what());
    }
}

NAN_METHOD(LoadObjectFile)
{
    if(!info[0]->IsString()) {
        Nan::ThrowError("Must provide filename as a string argument");
        return;
    }

    v8::String::Utf8Value str(info[0]->ToString());
    std::string filename((char const *) *str);

    try {
        sim->loadObjectFile(filename);
    } catch(lc3::utils::exception const & e) {
        Nan::ThrowError(e.what());
    }
}

NAN_METHOD(Run)
{
    if(!info[0]->IsFunction()) {
        Nan::ThrowError("Must provide callback as an argument");
        return;
    }

    Nan::AsyncQueueWorker(new SimulatorAsyncWorker(
        new Nan::Callback(info[0].As<v8::Function>())
    ));
}

NAN_METHOD(Pause)
{
    sim->pause();
}

NAN_METHOD(ClearInput)
{
    inputter->clearInput();
}

NAN_METHOD(AddInput)
{
    if(!info[0]->IsString()) {
        Nan::ThrowError("Must provide character as as a string argument");
        return;
    }

    v8::String::Utf8Value str(info[0]->ToString());
    std::string c((char const *) *str);
    if(c.size() != 1) {
        Nan::ThrowError("String must be a single character");
        return;
    }

    inputter->addInput(c[0]);
}

NAN_METHOD(GetRegValue)
{
    uint32_t ret_val = 0;

    if(!info[0]->IsString()) {
        Nan::ThrowError("Must provide register name as a string argument");
        return;
    }

    v8::String::Utf8Value str(info[0]->ToString());
    std::string reg_name((char const *) *str);
    std::transform(reg_name.begin(), reg_name.end(), reg_name.begin(), ::tolower);

    lc3::core::MachineState const & state = sim->getMachineState();
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
    } else if(reg_name == "mcr") {
        ret_val = state.mem[MCR].getValue();
    }

    auto ret = Nan::New<v8::Number>(ret_val);
    info.GetReturnValue().Set(ret);
}

NAN_METHOD(GetMemValue)
{
    if(!info[0]->IsNumber()) {
        Nan::ThrowError("Must provide memory address as an numerical argument");
        return;
    }

    uint32_t addr = (uint32_t) info[0]->NumberValue();
    lc3::core::MachineState const & state = sim->getMachineState();
    auto ret = Nan::New<v8::Number>(state.mem[addr].getValue());
    info.GetReturnValue().Set(ret);
}

NAN_METHOD(GetMemLine)
{
    if(!info[0]->IsNumber()) {
        Nan::ThrowError("Must provide memory address as a numerical argument");
        return;
    }

    uint32_t addr = (uint32_t) info[0]->NumberValue();
    lc3::core::MachineState const & state = sim->getMachineState();
    auto ret = Nan::New<v8::String>(state.mem[addr].getLine()).ToLocalChecked();
    info.GetReturnValue().Set(ret);
}

NAN_METHOD(GetOutput)
{
    std::vector<std::string> const & output = printer->getOutputBuffer();
    std::string joined = "";
    for(std::string const & str : output) { joined += str; }
    auto ret = Nan::New<v8::String>(joined).ToLocalChecked();
    info.GetReturnValue().Set(ret);
}

NAN_METHOD(ClearOutput)
{
    printer->clearOutputBuffer();
}

NAN_METHOD(GetInstExecCount)
{
    auto ret = Nan::New<v8::Number>(sim->getInstExecCount());
    info.GetReturnValue().Set(ret);
}

NAN_MODULE_INIT(Initialize)
{
    NAN_EXPORT(target, Init);
    NAN_EXPORT(target, Shutdown);

    NAN_EXPORT(target, Assemble);
    NAN_EXPORT(target, LoadObjectFile);

    NAN_EXPORT(target, Run);
    NAN_EXPORT(target, Pause);
    NAN_EXPORT(target, ClearInput);
    NAN_EXPORT(target, AddInput);

    NAN_EXPORT(target, GetRegValue);
    NAN_EXPORT(target, GetMemValue);
    NAN_EXPORT(target, GetMemLine);
    NAN_EXPORT(target, GetOutput);
    NAN_EXPORT(target, ClearOutput);

    NAN_EXPORT(target, GetInstExecCount);
}

NODE_MODULE(wrapper, Initialize);
