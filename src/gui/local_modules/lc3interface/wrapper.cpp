/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include <nan.h>

#ifndef DEFAULT_PRINT_LEVEL
    #define DEFAULT_PRINT_LEVEL 4
#endif

#include <algorithm>
#include <memory>
#include <mutex>

#define API_VER 2
#include "interface.h"
#include "ui_printer.h"
#include "ui_inputter.h"

utils::UIPrinter printer;
utils::UIInputter inputter;
std::shared_ptr<lc3::as> as = nullptr;
std::shared_ptr<lc3::conv> conv = nullptr;
std::shared_ptr<lc3::sim> sim = nullptr;
bool hit_breakpoint = false;

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
        as = std::make_shared<lc3::as>(printer, DEFAULT_PRINT_LEVEL, false);
        conv = std::make_shared<lc3::conv>(printer, DEFAULT_PRINT_LEVEL);
        sim = std::make_shared<lc3::sim>(printer, inputter, DEFAULT_PRINT_LEVEL);
        sim->registerCallback(lc3::core::CallbackType::BREAKPOINT,
            [](lc3::core::CallbackType, lc3::sim &) {
                hit_breakpoint = true;
            }
        );
    } catch(std::exception const & e) {
        Nan::ThrowError(e.what());
    }
}

NAN_METHOD(ConvertBin)
{
    if(info.Length() != 1) {
        Nan::ThrowError("Requires 1 argument");
        return;
    }

    if(! info[0]->IsString()) {
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
    if(info.Length() != 1) {
        Nan::ThrowError("Requires 1 argument");
        return;
    }

    if(! info[0]->IsString()) {
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
    if(info.Length() != 1) {
        Nan::ThrowError("Requires 1 argument");
        return;
    }

    if(! info[0]->IsString()) {
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
    if(info.Length() != 1) {
        Nan::ThrowError("Requires 1 argument");
        return;
    }

    if(! info[0]->IsFunction()) {
        Nan::ThrowError("Must provide callback as an argument");
        return;
    }

    hit_breakpoint = false;
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
    if(info.Length() != 1) {
        Nan::ThrowError("Requires 1 argument");
        return;
    }

    if(! info[0]->IsFunction()) {
        Nan::ThrowError("Must provide callback as an argument");
        return;
    }

    hit_breakpoint = false;
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
    if(info.Length() != 1) {
        Nan::ThrowError("Requires 1 argument");
        return;
    }

    if(! info[0]->IsFunction()) {
        Nan::ThrowError("Must provide callback as an argument");
        return;
    }

    hit_breakpoint = false;
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
    if(info.Length() != 1) {
        Nan::ThrowError("Requires 1 argument");
        return;
    }

    if(! info[0]->IsFunction()) {
        Nan::ThrowError("Must provide callback as an argument");
        return;
    }

    hit_breakpoint = false;
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
        sim->asyncInterrupt();
    } catch(std::exception const & e) {
        Nan::ThrowError(e.what());
    }
}

NAN_METHOD(GetRegValue)
{
    uint32_t ret_val = 0;

    if(info.Length() != 1) {
        Nan::ThrowError("Requires 1 argument");
        return;
    }

    if(! info[0]->IsString()) {
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
    inputter.clearInput();
}

NAN_METHOD(AddInput)
{
    if(info.Length() != 1) {
        Nan::ThrowError("Requires 1 argument");
        return;
    }

    if(! info[0]->IsString()) {
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
        inputter.addInput(c[0]);
    } catch(std::exception const & e) {
        Nan::ThrowError(e.what());
    }
}

NAN_METHOD(GetAndClearOutput)
{
    try {
        std::vector<std::string> const & output = printer.getAndClearOutputBuffer();
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
    try {
        printer.clearOutputBuffer();
    } catch(std::exception const & e) {
        Nan::ThrowError(e.what());
    }
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

NAN_METHOD(DidHitBreakpoint)
{
    try {
        auto ret = Nan::New<v8::Boolean>(hit_breakpoint);
        info.GetReturnValue().Set(ret);
    } catch(std::exception const & e) {
        Nan::ThrowError(e.what());
    }
}

NAN_MODULE_INIT(NanInit)
{
    NAN_EXPORT(target, Init);

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
    NAN_EXPORT(target, GetAndClearOutput);
    NAN_EXPORT(target, ClearOutput);

    NAN_EXPORT(target, SetBreakpoint);
    NAN_EXPORT(target, RemoveBreakpoint);

    NAN_EXPORT(target, GetInstExecCount);
    NAN_EXPORT(target, DidHitBreakpoint);
}

NODE_MODULE(lc3interface, NanInit);

void utils::UIPrinter::setColor(lc3::utils::PrintColor color)
{
    using namespace lc3::utils;

    if(color == PrintColor::RESET) {
        while(pending_colors != 0) {
            output_buffer.push_back("</span>");
            pending_colors -= 1;
        }
    } else {
        std::string format = "<span class=\"text-";
        switch(color)
        {
            case PrintColor::RED      : format += "red"    ; break;
            case PrintColor::YELLOW   : format += "yellow" ; break;
            case PrintColor::GREEN    : format += "green"  ; break;
            case PrintColor::MAGENTA  : format += "magenta"; break;
            case PrintColor::BLUE     : format += "blue"   ; break;
            case PrintColor::GRAY     : format += "gray"   ; break;
            case PrintColor::BOLD     : format += "bold"   ; break;
            default                   : break;
        }
        format += "\">";
        output_buffer.push_back(format);
        pending_colors += 1;
    }
}

std::vector<std::string> utils::UIPrinter::getAndClearOutputBuffer(void)
{
    std::lock_guard<std::mutex> const lock(output_buffer_mutex);
    std::vector<std::string> output_buffer_copy = output_buffer;
    output_buffer.clear();
    return output_buffer_copy;
}

void utils::UIPrinter::clearOutputBuffer(void)
{
    std::lock_guard<std::mutex> const lock(output_buffer_mutex);
    output_buffer.clear();
}

void utils::UIPrinter::print(std::string const & string)
{
    std::lock_guard<std::mutex> const lock(output_buffer_mutex);
    output_buffer.push_back(string);
}

void utils::UIPrinter::newline(void)
{
    std::lock_guard<std::mutex> const lock(output_buffer_mutex);
    output_buffer.push_back("\n");
}

bool utils::UIInputter::getChar(char & c)
{
    std::lock_guard<std::mutex> const lock(buffer_mutex);
    if(buffer.empty()) { return false; }

    c = buffer.front();
    buffer.erase(buffer.begin());
    return true;
}

void utils::UIInputter::clearInput(void)
{
    std::lock_guard<std::mutex> const lock(buffer_mutex);
    buffer.clear();
}

void utils::UIInputter::addInput(char c)
{
    std::lock_guard<std::mutex> const lock(buffer_mutex);
    buffer.push_back(c);
}
