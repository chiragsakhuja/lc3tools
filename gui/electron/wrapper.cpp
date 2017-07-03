#include <nan.h>

#include <array>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#ifndef _PRINT_LEVEL
    #define _PRINT_LEVEL 4
#endif


#include "../../core/src/tokens.h"

#include "../../core/src/printer.h"
#include "../../core/src/logger.h"
#include "ui_printer.h"

#include "../../core/src/state.h"

#include "../../core/src/instructions.h"
#include "../../core/src/instruction_encoder.h"

#include "../../core/src/assembler.h"

utils::IPrinter * printer = nullptr;
core::Assembler * assembler = nullptr;

NAN_METHOD(Assemble)
{
    if(!info[0]->IsString()) {
        Nan::ThrowError("Argument must be a string");
        return;
    }

    v8::String::Utf8Value str(info[0]->ToString());
    assembler->genObjectFile((char const *) (*str));
}

NAN_METHOD(GetOutput)
{
    std::vector<std::string> const & output = ((utils::UIPrinter *) printer)->getOutputBuffer();
    std::string joined = "";
    for(std::string const & str : output) { joined += str; }
    auto ret = Nan::New<v8::String>(joined).ToLocalChecked();
    info.GetReturnValue().Set(ret);
}

NAN_MODULE_INIT(Initialize)
{
    NAN_EXPORT(target, Assemble);
    NAN_EXPORT(target, GetOutput);

    printer = new utils::UIPrinter();
    assembler = new core::Assembler(true, *printer);
}

NODE_MODULE(wrapper, Initialize);
