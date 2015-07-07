#ifndef TOKENS_H
#define TOKENS_H

class Token
{
private:
    Token();

public:
    std::string str;
    int num;

    int type;
    int numOperands;
    int pc;
    Token *opers;
    Token *next;

#ifdef _ENABLE_TESTING
    int encoding;
#endif

    // used for errors
    int rowNum, colNum;
    int length;

    Token(const std::string& str);
    Token(int val);

    bool checkPseudoType(const std::string& pseudo) const;
};

#endif
