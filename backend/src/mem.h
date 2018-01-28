#ifndef MEM_H
#define MEM_H

#include <iostream>

namespace lc3
{
namespace core
{
    class MemEntry
    {
    public:
        MemEntry(void) = default;
        MemEntry(uint16_t value, bool orig, std::string const & line) : value(value), orig(orig), line(line) {}

        uint16_t getValue(void) const { return value; }
        void setValue(uint16_t value) { this->value = value; }
        bool isOrig(void) const { return orig; }
        std::string getLine(void) const { return line; }

        friend std::ostream & operator<<(std::ostream & out, MemEntry const & in);
        friend std::istream & operator>>(std::istream & in, MemEntry & out);

    private:
        uint16_t value;
        bool orig;
        std::string line;
    };
};
};

#endif
