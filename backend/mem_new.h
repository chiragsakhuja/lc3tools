#ifndef MEM_NEW_H
#define MEM_NEW_H

#include <string>
#include <iostream>

namespace lc3
{
namespace core
{
    class MemLocation
    {
    public:
        MemLocation(void) : MemLocation(0x0000, "") {}
        MemLocation(uint16_t value, std::string const & line) : value(value), line(line), is_orig(false) {}

        uint16_t getValue(void) const { return value; }
        std::string const & getLine(void) const { return line; }
        bool isOrig(void) const { return is_orig; }
        void setValue(uint16_t value) { this->value = value; }
        void setLine(std::string const & line) { this->line = line; }
        void setIsOrig(bool is_orig) { this->is_orig = is_orig; }

    private:
        uint16_t value;
        std::string line;
        bool is_orig;
    };

    std::istream & operator>>(std::istream & in, MemLocation & out);
};
};

#endif
