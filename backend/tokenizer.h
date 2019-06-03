#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <cstdint>
#include <fstream>
#include <string>

#include "asm_types.h"

namespace lc3
{
namespace core
{
namespace asmbl
{
    class Tokenizer
    {
    public:
        Tokenizer(std::istream & buffer);
        ~Tokenizer(void) = default;

        Tokenizer & operator>>(Token & token);
        bool isDone(void) const;
        bool operator!() const;
        explicit operator bool() const;

    private:
        std::istream & buffer;
        bool get_new_line;
        bool return_new_line;
        std::string line;
        uint32_t row, col;
        bool done;

        bool convertStringToNum(std::string const & str, int32_t & val) const;
    };
};
};
};

#endif
