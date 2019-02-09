#include <nan.h>

#ifndef _PRINT_LEVEL
    #define _PRINT_LEVEL 4
#endif

#include <algorithm>

#include "interface.h"
#include "ui_printer.h"
#include "ui_inputter.h"

utils::UIPrinter *printer = nullptr;
utils::UIInputter *inputter = nullptr;
lc3::as *as = nullptr;
lc3::sim *sim = nullptr;

class SimulatorAsyncWorker : public Nan::AsyncWorker
{
private:
    std::function<void(void)> run_function;

public:
    SimulatorAsyncWorker(std::function<void(void)> run_function, Nan::Callback * callback) :
        Nan::AsyncWorker(callback), run_function(run_function) {}

    void Execute(void)
    {
        try {
            run_function();
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
        sim = new lc3::sim(*printer, *inputter, os_path, _PRINT_LEVEL);
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

NAN_METHOD(ConvertBin)
{
    if(!info[0]->IsString()) {
        Nan::ThrowError("Must provide filename as a string argument");
        return;
    }

    v8::String::Utf8Value str(info[0]->ToString());

    std::string bin_filename((char const *) (*str));

    try {
        as->convertBin(bin_filename);
    } catch(lc3::utils::exception const & e) {
        Nan::ThrowError(e.what());
    }
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

NAN_METHOD(RestartMachine)
{
    try {
        sim->restart();
    } catch(lc3::utils::exception const & e) {
        Nan::ThrowError(e.what());
    }
}

NAN_METHOD(ReinitializeMachine)
{
    try {
        sim->reinitialize();
    } catch(lc3::utils::exception const & e) {
        Nan::ThrowError(e.what());
    }
}

NAN_METHOD(RandomizeMachine)
{
    try {
        sim->randomize();
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
        []() { sim->run(); },
        new Nan::Callback(info[0].As<v8::Function>())
    ));
}

NAN_METHOD(StepIn)
{
    if(!info[0]->IsFunction()) {
        Nan::ThrowError("Must provide callback as an argument");
        return;
    }

    Nan::AsyncQueueWorker(new SimulatorAsyncWorker(
        []() { sim->stepIn(); },
        new Nan::Callback(info[0].As<v8::Function>())
    ));
}

NAN_METHOD(StepOut)
{
    if(!info[0]->IsFunction()) {
        Nan::ThrowError("Must provide callback as an argument");
        return;
    }

    Nan::AsyncQueueWorker(new SimulatorAsyncWorker(
        []() { sim->stepOut(); },
        new Nan::Callback(info[0].As<v8::Function>())
    ));
}

NAN_METHOD(StepOver)
{
    if(!info[0]->IsFunction()) {
        Nan::ThrowError("Must provide callback as an argument");
        return;
    }

    Nan::AsyncQueueWorker(new SimulatorAsyncWorker(
        []() { sim->stepOver(); },
        new Nan::Callback(info[0].As<v8::Function>())
    ));
}

NAN_METHOD(Pause)
{
    sim->pause();
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
        ret_val = sim->getPSR();
    } else if(reg_name == "pc") {
        ret_val =  sim->getPC();
    } else if(reg_name == "mcr") {
        ret_val = sim->getMCR();
    }

    auto ret = Nan::New<v8::Number>(ret_val);
    info.GetReturnValue().Set(ret);
}

NAN_METHOD(SetRegValue)
{
    if(!info[0]->IsString()) {
        Nan::ThrowError("First argument must be register name as a string");
        return;
    }

    if(!info[1]->IsNumber()) {
        Nan::ThrowError("Second argument must be value as a number");
        return;
    }

    v8::String::Utf8Value str(info[0]->ToString());
    std::string reg_name((char const *) *str);
    std::transform(reg_name.begin(), reg_name.end(), reg_name.begin(), ::tolower);
    uint32_t value = (uint32_t) info[1]->NumberValue();

    lc3::core::MachineState & state = sim->getMachineState();
    if(reg_name[0] == 'r') {
        uint32_t reg_num = reg_name[1] - '0';
        if(reg_num > 7) {
            Nan::ThrowError("GPR must be R0 through R7");
            return;
        }
        state.regs[reg_num] = value;
    } else if(reg_name == "psr") {
        sim->setPSR(value);
    } else if(reg_name == "pc") {
        sim->setPC(value);
    } else if(reg_name == "mcr") {
        sim->setMCR(value);
    }
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

NAN_METHOD(SetMemValue)
{
    if(!info[0]->IsNumber()) {
        Nan::ThrowError("First argument must be a memory address as a number");
        return;
    }

    if(!info[1]->IsNumber()) {
        Nan::ThrowError("Second argument must be value as a number");
        return;
    }

    uint32_t addr = (uint32_t) info[0]->NumberValue();
    uint32_t value = (uint32_t) info[1]->NumberValue();
    lc3::core::MachineState & state = sim->getMachineState();
    state.mem[addr].setValue(value);
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

NAN_METHOD(SetMemLine)
{
    if(!info[0]->IsNumber()) {
        Nan::ThrowError("Must provide memory address as a numerical argument");
        return;
    }

    if(!info[1]->IsString()) {
        Nan::ThrowError("Second argument must be string");
        return;
    }

    uint32_t addr = (uint32_t) info[0]->NumberValue();
    v8::String::Utf8Value str(info[1]->ToString());
    std::string line((char const *) *str);

    lc3::core::MachineState & state = sim->getMachineState();
    sim->setMemLine(addr, line);
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

NAN_METHOD(SetBreakpoint)
{
    if(!info[0]->IsNumber()) {
        Nan::ThrowError("Must provide memory address as a numerical argument");
        return;
    }

    uint32_t addr = (uint32_t) info[0]->NumberValue();
    sim->setBreakpoint(addr);
}

NAN_METHOD(RemoveBreakpoint)
{
    if(!info[0]->IsNumber()) {
        Nan::ThrowError("Must provide memory address as a numerical argument");
        return;
    }

    uint32_t addr = (uint32_t) info[0]->NumberValue();
    sim->removeBreakpointByAddr(addr);
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

    NAN_EXPORT(target, ConvertBin);
    NAN_EXPORT(target, Assemble);
    NAN_EXPORT(target, LoadObjectFile);

    NAN_EXPORT(target, RestartMachine);
    NAN_EXPORT(target, ReinitializeMachine);
    NAN_EXPORT(target, RandomizeMachine);

    NAN_EXPORT(target, Run);
    NAN_EXPORT(target, StepIn);
    NAN_EXPORT(target, StepOver);
    NAN_EXPORT(target, StepOut);
    NAN_EXPORT(target, Pause);

    NAN_EXPORT(target, GetRegValue);
    NAN_EXPORT(target, SetRegValue);
    NAN_EXPORT(target, GetMemValue);
    NAN_EXPORT(target, SetMemValue);
    NAN_EXPORT(target, GetMemLine);
    NAN_EXPORT(target, SetMemLine);

    NAN_EXPORT(target, ClearInput);
    NAN_EXPORT(target, AddInput);
    NAN_EXPORT(target, GetOutput);
    NAN_EXPORT(target, ClearOutput);

    NAN_EXPORT(target, SetBreakpoint);
    NAN_EXPORT(target, RemoveBreakpoint);

    NAN_EXPORT(target, GetInstExecCount);
}

NODE_MODULE(wrapper, Initialize);
