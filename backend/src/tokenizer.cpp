#include <stdexcept>

#include "tokenizer.h"

lc3::core::asmbl::Tokenizer::Tokenizer(std::string const & filename) : filename(filename), file_opened(false),
    get_new_line(true), return_newline(false), row(-1), col(0), done(false)
{}

lc3::core::asmbl::Tokenizer::~Tokenizer(void)
{
    if(file_opened) {
        file.close();
    }
}

lc3::core::asmbl::Tokenizer & lc3::core::asmbl::Tokenizer::operator>>(Token & token)
{
    // if this is the first time stream is used, open the file
    if(! file_opened) {
        file.open(filename);
        // If the file cannot be opened, consider stream to be empty
        if(! file.is_open()) {
            done = true;
            return *this;
        }
        file_opened = true;
    }

    // check if we need to fetch a new line i.e. previous line is done being tokenized
    if(get_new_line) {
        // check if an end-of-statement token should be returned
        if(return_newline) {
            return_newline = false;
            token.type = TokenType::EOS;
            return *this;
        }

        col = 0;
        row += 1;

        // get another line and check if we've reached the end of the file
        if(! std::getline(file, line)) {
            done = true;
            return *this;
        }

        // remove everything following a comment and any remaining trailing whitespace
        size_t search_pos = line.find(';');
        if(search_pos != std::string::npos) {
            line = line.substr(0, search_pos);
        }
        search_pos = line.find_last_not_of(" \t");
        if(search_pos != std::string::npos) {
            line = line.substr(0, search_pos + 1);
        } else {
            // if this line had nothing on it after trimming, get a new line
            get_new_line = true;
            return_newline = false;
            operator>>(token);
            return *this;
        }

        get_new_line = false;
    }

    // move until we're past delimiters
    std::string delims =  ",: \t";
    while(col < line.size() && delims.find(line[col]) != std::string::npos) {
        col += 1;
    }

    // check if we reached the end of the line (meaning the previous token we returned was the last one on the line)
    // note that this ignores trailing delimiters
    if(col >= line.size()) {
        get_new_line = true;
        return_newline = true;
        operator>>(token);
        return *this;
    }

    // get the length of the token
    uint32_t len = 0;
    // if we see a token starting with a ", then continue until the closing " or the end of the line
    if(line[col] == '"') {
        delims = '"';
        col += 1;
    }

    while(col + len < line.size() && delims.find(line[col + len]) == std::string::npos) {
        len += 1;
    }

    int32_t token_num_val = 0;
    if(convertStringToNum(line.substr(col, len), token_num_val)) {
        token.type = TokenType::NUM;
        token.num = token_num_val;
    } else {
        token.type = TokenType::STRING;
        token.str = line.substr(col, len);
    }
    token.col = col;
    token.row = row;
    token.len = len;
    token.line = line;
    col += len + 1;

    return *this;
}

bool lc3::core::asmbl::Tokenizer::convertStringToNum(std::string const & str, int32_t & val) const
{
    char const * c_str = str.c_str();
#ifdef LIBERAL_CONSTANTS
    if(c_str[0] == '0' && c_str[1] != '\0') { c_str += 1; }
#endif

    try {
        if(c_str[0] == 'B' || c_str[0] == 'b' || c_str[0] == 'X' || c_str[0] == 'x' || c_str[0] == '#') {
            std::string conv = std::string(c_str + 1);
            switch(c_str[0]) {
                case 'B':
                case 'b': val = std::stoi(conv, nullptr, 2) ; break;
                case 'X':
                case 'x': val = std::stoi(conv, nullptr, 16); break;
                case '#': val = std::stoi(conv)             ; break;
                default : val = std::stoi(c_str)            ; break;
            }
            return true;
        } else {
#ifdef LIBERAL_CONSTANTS
            val = std::stoi(c_str);
            return true;
#endif
        }
        return false;
    } catch(std::invalid_argument const & e) {
        return false;
    }
}

bool lc3::core::asmbl::Tokenizer::isDone(void) const
{
    return done;
}

bool lc3::core::asmbl::Tokenizer::operator!(void) const
{
    return ! isDone();
}

lc3::core::asmbl::Tokenizer::operator bool(void) const
{
    return operator!();
}
