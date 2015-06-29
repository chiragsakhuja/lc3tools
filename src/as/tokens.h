#ifndef TOKENS_H
#define TOKENS_H

union DataType
{
    std::string *str;
    int num;
};

class Token
{
private:
    Token();

public:
    DataType data;
    int type;
    int numOperands;
    int pc;
    Token *opers;
    Token *next;

    // used for errors
    int rowNum, colNum;
    int length;

    Token(std::string *str);
    Token(int val);
    ~Token();

    bool checkPseudoType(const std::string& pseudo) const;
};

#endif
