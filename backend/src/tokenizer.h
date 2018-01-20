#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <cstdint>
#include <fstream>
#include <string>

struct AsmToken
{
    AsmToken(void);

    enum class TokenType {
          INVALID
        , NUM
        , STRING
        , EOS

        , INST
        , REG
        , PSEUDO
        , LABEL
    } type;

    std::string str;
    int32_t num;

    uint32_t row, col, len;
};

class AsmTokenizer
{
public:
    AsmTokenizer(std::string const & filename);
    ~AsmTokenizer(void);

    AsmTokenizer & operator>>(AsmToken & token);
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

#endif
