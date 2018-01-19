#include <stdexcept>

#include "tokenizer.h"

AsmToken::AsmToken(void) : type(AsmToken::TokenType::INVALID) {}

AsmTokenizer::AsmTokenizer(std::string const & filename) : filename(filename), file_opened(false), get_new_line(true),
    return_newline(false), found_on_line(false), row(-1), col(0), done(false)
{}

AsmTokenizer::~AsmTokenizer(void)
{
    if(file_opened) {
        file.close();
    }
}

AsmTokenizer & AsmTokenizer::operator>>(AsmToken & token)
{
    // If this is the first time stream is used, open the file
    if(! file_opened) {
        file.open(filename);
        // If the file cannot be opened, consider stream to be empty
        if(! file.is_open()) {
            done = true;
            return *this;
        }
        file_opened = true;
    }

    // Check if previous line is done being tokenized
    if(get_new_line) {
        // An end-of-statement token should be returned between statements
        if(return_newline) {
            return_newline = false;
            token.type = AsmToken::TokenType::EOS;
            return *this;
        }

        col = 0;
        row += 1;

        // Get another line and check if we've reached the end of the file
        if(! std::getline(file, line)) {
            done = true;
            return *this;
        }

        // Remove everything following a comment and any remaining trailing whitespace
        size_t search_pos = line.find(';');
        if(search_pos != std::string::npos) {
            line = line.substr(0, search_pos);
        }
        search_pos = line.find_last_not_of(" \t");
        if(search_pos != std::string::npos) {
            line = line.substr(0, search_pos + 1);
        } else {
            // If this line had nothing on it after trimming, get a new line
            get_new_line = true;
            return_newline = false;
            operator>>(token);
            return *this;
        }

        get_new_line = false;
    }

    // Move until we're past delimiters
    std::string delims =  ",: \t";
    while(col < line.size() && delims.find(line[col]) != std::string::npos) {
        col += 1;
    }

    // Check if we reached the end of the line (meaning the previous token we returned was the last one on the line)
    // Note that this ignores trailing delimiters
    if(col >= line.size()) {
        get_new_line = true;
        return_newline = true;
        operator>>(token);
        return *this;
    }

    // Get the length of the token
    uint32_t len = 0;
    while(col + len < line.size() && delims.find(line[col + len]) == std::string::npos) {
        len += 1;
    }

    int32_t token_num_val = 0;
    if(convertStringToNum(line.substr(col, len), token_num_val)) {
        token.type = AsmToken::TokenType::NUM;
        token.num = token_num_val;
    } else {
        token.type = AsmToken::TokenType::STRING;
        token.str = line.substr(col, len);
    }
    token.col = col;
    token.row = row;
    token.len = len;
    col += len + 1;

    return *this;
}

bool AsmTokenizer::convertStringToNum(std::string const & str, int32_t & val) const
{
    char const * c_str = str.c_str();
    if(c_str[0] == '0' && c_str[1] != '\0') { c_str += 1; }

    try {
        if(c_str[0] == 'b' || c_str[0] == 'x' || c_str[0] == '#') {
            std::string conv = std::string(c_str + 1);
            switch(c_str[0]) {
                case 'b': val = std::stoi(conv, nullptr, 2) ; break;
                case 'x': val = std::stoi(conv, nullptr, 16); break;
                case '#': val = std::stoi(conv)             ; break;
                default : val = std::stoi(c_str)            ; break;
            }
            return true;
        } else {
            val = std::stoi(c_str);
            return true;
        }
        return true;
    } catch(std::invalid_argument const & e) {
        return false;
    }
}

bool AsmTokenizer::operator!(void) const
{
    return ! done;
}

AsmTokenizer::operator bool(void) const
{
    return operator!();
}

std::ostream & operator<<(std::ostream & out, AsmToken const & x)
{
    if(x.type == AsmToken::TokenType::STRING) {
        out << x.str << " (string)";
    } else if(x.type == AsmToken::TokenType::NUM) {
        out << x.num << " (num)";
    } else if(x.type == AsmToken::TokenType::EOS) {
        out << "EOS\n";
        return out;
    } else {
        out << "invalid token";
    }

    out << " " << (x.row + 1) << ":" << (x.col + 1) << "+" << x.len << "\n";
    return out;
}
