#ifndef STATEMENT_H
#define STATEMENT_H

#include <cstdint>
#include <string>

#include "tokenizer.h"

namespace lc3
{
namespace core
{
    class OldStatement
    {
    public:
        OldStatement(void) = default;
        OldStatement(uint16_t value, bool orig, std::string const & line) : value(value), orig(orig), line(line) {}

        uint16_t getValue(void) const { return value; }
        void setValue(uint16_t value) { this->value = value; }
        bool isOrig(void) const { return orig; }
        std::string getLine(void) const { return line; }

        friend std::ostream & operator<<(std::ostream & out, OldStatement const & in);
        friend std::istream & operator>>(std::istream & in, OldStatement & out);

    private:
        uint16_t value;
        bool orig;
        std::string line;
    };

    class MemEntry
    {
    public:
        MemEntry(void) = default;
    };
};
};

#endif
