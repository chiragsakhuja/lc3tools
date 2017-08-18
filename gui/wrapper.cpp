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
core::lc3 lc3interface(printer, inputter);

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
        lc3interface.assemble(asm_filename, obj_filename);
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
}

NODE_MODULE(wrapper, Initialize);
