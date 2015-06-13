#ifndef TOKENS_H
#define TOKENS_H

#include <string>

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
    Token *args;
    Token *next;

    // used for errors
    int rowNum, colNum;
    int length;

    Token(std::string *str, int type);
    Token(int val, int type);
    ~Token();
};

#endif
