/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include <nan.h>

#ifndef _PRINT_LEVEL
    #define _PRINT_LEVEL 4
#endif

#include <algorithm>

#define API_VER 2
#include "interface.h"
#include "ui_printer.h"
#include "ui_inputter.h"

utils::UIPrinter *printer = nullptr;
utils::UIInputter *inputter = nullptr;
lc3::as *as = nullptr;
lc3::conv *conv = nullptr;
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
        } catch(std::exception const & e) {
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
    try {
        printer = new utils::UIPrinter();
        inputter = new utils::UIInputter();
        as = new lc3::as(*printer, _PRINT_LEVEL, false);
        conv = new lc3::conv(*printer, _PRINT_LEVEL);
        sim = new lc3::sim(*printer, *inputter, _PRINT_LEVEL);
    } catch(std::exception const & e) {
        Nan::ThrowError(e.what());
    }
}

NAN_METHOD(Shutdown)
{
    delete sim;
    delete conv;
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

    Nan::Utf8String str(info[0].As<v8::String>());

    std::string bin_filename((char const *) (*str));

    try {
        auto ret = conv->convertBin(bin_filename);
        if(! ret) {
            Nan::ThrowError("conversion failed");
        }
    } catch(std::exception const & e) {
        Nan::ThrowError(e.what());
    }
}

NAN_METHOD(Assemble)
{
    if(!info[0]->IsString()) {
        Nan::ThrowError("Must provide filename as a string argument");
        return;
    }

    Nan::Utf8String str(info[0].As<v8::String>());

    std::string asm_filename((char const *) (*str));

    try {
        auto ret = as->assemble(asm_filename);
        if(! ret) {
            Nan::ThrowError("assembly failed");
        }
    } catch(std::exception const & e) {
        Nan::ThrowError(e.what());
    }
}

NAN_METHOD(SetEnableLiberalAsm)
{
    if(info.Length() != 1) {
        Nan::ThrowError("Requires 1 argument");
        return;
    }

    if(! info[0]->IsBoolean()) {
        Nan::ThrowError("Must provide setting as a bool argument");
        return;
    }

    try {
        as->setEnableLiberalAsm(Nan::To<bool>(info[0]).FromJust());
    } catch(std::exception const & e) {
        Nan::ThrowError(e.what());
    }
}

NAN_METHOD(LoadObjectFile)
{
    if(!info[0]->IsString()) {
        Nan::ThrowError("Must provide filename as a string argument");
        return;
    }

    Nan::Utf8String str(info[0].As<v8::String>());
    std::string filename((char const *) *str);

    try {
        sim->loadObjFile(filename);
    } catch(std::exception const & e) {
        Nan::ThrowError(e.what());
    }
}

NAN_METHOD(RestartMachine)
{
    try {
        sim->setup();
    } catch(std::exception const & e) {
        Nan::ThrowError(e.what());
    }
}

NAN_METHOD(ReinitializeMachine)
{
    try {
        sim->zeroState();
    } catch(std::exception const & e) {
        Nan::ThrowError(e.what());
    }
}

NAN_METHOD(RandomizeMachine)
{
    try {
        sim->randomizeState();
    } catch(std::exception const & e) {
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
        []() {
          try {
            sim->setRunInstLimit(0);
            sim->run();
          } catch(std::exception const & e) {
            Nan::ThrowError(e.what());
          }
        },
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
        []() {
          try {
            sim->stepIn();
          } catch(std::exception const & e) {
            Nan::ThrowError(e.what());
          }
        },
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
        []() {
          try {
            sim->stepOut();
          } catch(std::exception const & e) {
            Nan::ThrowError(e.what());
          }
        },
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
        []() {
          try {
            sim->stepOver();
          } catch(std::exception const & e) {
            Nan::ThrowError(e.what());
          }
        },
        new Nan::Callback(info[0].As<v8::Function>())
    ));
}

NAN_METHOD(Pause)
{
    try {
        sim->pause();
    } catch(std::exception const & e) {
        Nan::ThrowError(e.what());
    }
}

NAN_METHOD(GetRegValue)
{
    uint32_t ret_val = 0;

    if(!info[0]->IsString()) {
        Nan::ThrowError("Must provide register name as a string argument");
        return;
    }

    Nan::Utf8String str(info[0].As<v8::String>());
    std::string reg_name((char const *) *str);
    std::transform(reg_name.begin(), reg_name.end(), reg_name.begin(), ::tolower);

    try {
        if(reg_name[0] == 'r') {
            uint32_t reg_num = reg_name[1] - '0';
            if(reg_num > 7) {
                Nan::ThrowError("GPR must be R0 through R7");
                return;
            }
            ret_val = sim->readReg(reg_num);
        } else if(reg_name == "ir") {
            ret_val = sim->readMem(sim->readPC());
        } else if(reg_name == "psr") {
            ret_val = sim->readPSR();
        } else if(reg_name == "pc") {
            ret_val =  sim->readPC();
        } else if(reg_name == "mcr") {
            ret_val = sim->readMCR();
        }
    } catch(std::exception const & e) {
        Nan::ThrowError(e.what());
    }

    auto ret = Nan::New<v8::Number>(ret_val);
    info.GetReturnValue().Set(ret);
}

NAN_METHOD(SetRegValue)
{
    if(info.Length() != 2) {
        Nan::ThrowError("Requires 2 arguments");
        return;
    }

    if(! info[0]->IsString()) {
        Nan::ThrowError("First argument must be register name as a string");
        return;
    }

    if(! info[1]->IsNumber()) {
        Nan::ThrowError("Second argument must be value as a number");
        return;
    }

    Nan::Utf8String str(info[0].As<v8::String>());
    std::string reg_name((char const *) *str);
    std::transform(reg_name.begin(), reg_name.end(), reg_name.begin(), ::tolower);
    uint32_t value = Nan::To<uint32_t>(info[1]).FromJust();

    try {
        if(reg_name[0] == 'r') {
            uint32_t reg_num = reg_name[1] - '0';
            if(reg_num > 7) {
                Nan::ThrowError("GPR must be R0 through R7");
                return;
            }
            sim->writeReg(reg_num, value);
        } else if(reg_name == "psr") {
            sim->writePSR(value);
        } else if(reg_name == "pc") {
            sim->writePC(value);
        } else if(reg_name == "mcr") {
            sim->writeMCR(value);
        }
    } catch(std::exception const & e) {
        Nan::ThrowError(e.what());
    }
}

NAN_METHOD(GetMemValue)
{
    if(info.Length() != 1) {
        Nan::ThrowError("Requires 1 argument");
        return;
    }

    if(! info[0]->IsNumber()) {
        Nan::ThrowError("Must provide memory address as an numerical argument");
        return;
    }

    uint32_t addr = Nan::To<uint32_t>(info[0]).FromJust();
    try {
        auto ret = Nan::New<v8::Number>(sim->readMem(addr));
        info.GetReturnValue().Set(ret);
    } catch(std::exception const & e) {
        Nan::ThrowError(e.what());
    }
}

NAN_METHOD(SetMemValue)
{
    if(info.Length() != 2) {
        Nan::ThrowError("Requires 2 arguments");
        return;
    }

    if(! info[0]->IsNumber()) {
        Nan::ThrowError("First argument must be a memory address as a number");
        return;
    }

    if(! info[1]->IsNumber()) {
        Nan::ThrowError("Second argument must be value as a number");
        return;
    }

    uint32_t addr = Nan::To<uint32_t>(info[0]).FromJust();
    uint32_t value = Nan::To<uint32_t>(info[1]).FromJust();
    try {
        sim->writeMem(addr, value);
        sim->setMemLine(addr, "");
    } catch(std::exception const & e) {
        Nan::ThrowError(e.what());
    }

}

NAN_METHOD(GetMemLine)
{
    if(info.Length() != 1) {
        Nan::ThrowError("Requires 1 argument");
        return;
    }

    if(! info[0]->IsNumber()) {
        Nan::ThrowError("Must provide memory address as a numerical argument");
        return;
    }

    uint32_t addr = Nan::To<uint32_t>(info[0]).FromJust();
    try {
        auto ret = Nan::New<v8::String>(sim->getMemLine(addr)).ToLocalChecked();
        info.GetReturnValue().Set(ret);
    } catch(std::exception const & e) {
        Nan::ThrowError(e.what());
    }
}

NAN_METHOD(SetMemLine)
{
    if(info.Length() != 2) {
        Nan::ThrowError("Requires 2 arguments");
        return;
    }

    if(! info[0]->IsNumber()) {
        Nan::ThrowError("Must provide memory address as a numerical argument");
        return;
    }

    if(! info[1]->IsString()) {
        Nan::ThrowError("Second argument must be string");
        return;
    }

    uint32_t addr = Nan::To<uint32_t>(info[0]).FromJust();
    Nan::Utf8String str(info[1].As<v8::String>());
    std::string line((char const *) *str);

    try {
        sim->setMemLine(addr, line);
    } catch(std::exception const & e) {
        Nan::ThrowError(e.what());
    }
}

NAN_METHOD(SetIgnorePrivilege)
{
    if(info.Length() != 1) {
        Nan::ThrowError("Requires 1 argument");
        return;
    }

    if(! info[0]->IsBoolean()) {
        Nan::ThrowError("Must provide setting as a bool argument");
        return;
    }

    try {
        sim->setIgnorePrivilege(Nan::To<bool>(info[0]).FromJust());
    } catch(std::exception const & e) {
        Nan::ThrowError(e.what());
    }
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

    Nan::Utf8String str(info[0].As<v8::String>());
    std::string c((char const *) *str);
    if(c.size() != 1) {
        Nan::ThrowError("String must be a single character");
        return;
    }

    try {
        inputter->addInput(c[0]);
    } catch(std::exception const & e) {
        Nan::ThrowError(e.what());
    }
}

NAN_METHOD(GetOutput)
{
    try {
        std::vector<std::string> const & output = printer->getOutputBuffer();
        std::string joined = "";
        for(std::string const & str : output) { joined += str; }
        auto ret = Nan::New<v8::String>(joined).ToLocalChecked();
        info.GetReturnValue().Set(ret);
    } catch(std::exception const & e) {
        Nan::ThrowError(e.what());
    }
}

NAN_METHOD(ClearOutput)
{
    printer->clearOutputBuffer();
}

NAN_METHOD(SetBreakpoint)
{
    if(info.Length() != 1) {
        Nan::ThrowError("Requires 1 argument");
        return;
    }

    if(! info[0]->IsNumber()) {
        Nan::ThrowError("Must provide memory address as a numerical argument");
        return;
    }

    uint32_t addr = Nan::To<uint32_t>(info[0]).FromJust();
    try {
        sim->setBreakpoint(addr);
    } catch(std::exception const & e) {
        Nan::ThrowError(e.what());
    }
}

NAN_METHOD(RemoveBreakpoint)
{
    if(info.Length() != 1) {
        Nan::ThrowError("Requires 1 argument");
        return;
    }

    if(! info[0]->IsNumber()) {
        Nan::ThrowError("Must provide memory address as a numerical argument");
        return;
    }

    uint32_t addr = Nan::To<uint32_t>(info[0]).FromJust();
    try {
        sim->removeBreakpoint(addr);
    } catch(std::exception const & e) {
        Nan::ThrowError(e.what());
    }
}

NAN_METHOD(GetInstExecCount)
{
    try {
        auto ret = Nan::New<v8::Number>(sim->getInstExecCount());
        info.GetReturnValue().Set(ret);
    } catch(std::exception const & e) {
        Nan::ThrowError(e.what());
    }
}

NAN_MODULE_INIT(Initialize)
{
    NAN_EXPORT(target, Init);
    NAN_EXPORT(target, Shutdown);

    NAN_EXPORT(target, ConvertBin);
    NAN_EXPORT(target, Assemble);
    NAN_EXPORT(target, SetEnableLiberalAsm);
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
    NAN_EXPORT(target, SetIgnorePrivilege);

    NAN_EXPORT(target, ClearInput);
    NAN_EXPORT(target, AddInput);
    NAN_EXPORT(target, GetOutput);
    NAN_EXPORT(target, ClearOutput);

    NAN_EXPORT(target, SetBreakpoint);
    NAN_EXPORT(target, RemoveBreakpoint);

    NAN_EXPORT(target, GetInstExecCount);
}

NODE_MODULE(lc3interface, Initialize);
