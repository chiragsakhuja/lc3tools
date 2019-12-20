#ifndef MEM_NEW_H
#define MEM_NEW_H

#include <string>

namespace lc3
{
namespace core
{
    class MemLocation
    {
    private:
        uint16_t value;
        std::string line;

    public:
        MemLocation(void) : MemLocation(0x0000, "") {}
        MemLocation(uint16_t value, std::string const & line) : value(value), line(line) {}

        uint16_t getValue(void) const { return value; }
        std::string const & getLine(void) const { return line; }
        void setValue(uint16_t value) { this->value = value; }
        void setLine(std::string const & line) { this->line = line; }
    };
};
};

#endif
