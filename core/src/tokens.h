#ifndef TOKENS_H
#define TOKENS_H

class Token
{
private:
    Token(void);

public:
    std::string str;
    int num;

    int type;
    int num_operands;
    int pc;
    Token * opers;
    Token * next;

#ifdef _ENABLE_TESTING
    int encoding;
#endif

    // used for errors
    int row_num, col_num;
    int length;

    Token(std::string const & str);
    Token(int val);

    bool checkPseudoType(std::string const & pseudo) const;
    void print(std::ostream & out, int indent_level) const;

#ifdef _ENABLE_DEBUG
    friend std::ostream & operator<<(std::ostream &, Token const &);
#endif
};

#ifdef _ENABLE_DEBUG
std::ostream & operator<<(std::ostream & out, Token const & x);
#endif

#endif
