#include "helper.h"

class BufferedPrinter : public utils::IPrinter
{
public:
    std::vector<char> display_buffer;

    void setColor(int color) {}
    void print(std::string const & string)
    {
        std::copy(string.begin(), string.end(), std::back_inserter(display_buffer));
    }

    void newline(void) { display_buffer.push_back('\n'); }

};

class FileInputter : public utils::IInputter
{
public:
    void beginInput(void) {}
    bool getChar(char & c)
    {
        return false;
    }
    void endInput(void) {}
};

int main(int argc, char ** argv)
{
    BufferedPrinter printer;
    FileInputter inputter;

    coreInit(printer, inputter);
    coreShutdown();

    return 0;
}
