#include <nan.h>

#include <vector>
#include <string>

#include "core/src/asm/assembler.h"
#include "core/src/common/printer.h"
#include "ui_printer.h"

utils::Printer * printer = nullptr;
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
