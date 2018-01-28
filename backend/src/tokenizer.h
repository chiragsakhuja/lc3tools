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
        Tokenizer(std::string const & filename);
        ~Tokenizer(void);

        Tokenizer & operator>>(Token & token);
        bool isDone(void) const;
        bool operator!() const;
        explicit operator bool() const;

    private:
        std::string filename;
        bool file_opened;
        bool get_new_line;
        bool return_newline;
        std::ifstream file;
        std::string line;
        uint32_t row, col;
        bool done;

        bool convertStringToNum(std::string const & str, int32_t & val) const;
    };
};
};
};

#endif
