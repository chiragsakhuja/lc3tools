/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include <stdexcept>

#include "tokenizer.h"

lc3::core::asmbl::Tokenizer::Tokenizer(std::istream & buffer, bool enable_liberal_asm)
    : buffer(buffer), get_new_line(true), return_new_line(false), row(-1), col(0), done(false),
      enable_liberal_asm(enable_liberal_asm)
{ }

std::istream & lc3::core::asmbl::Tokenizer::getline(std::istream & is, std::string & t) const
{
    t.clear();

    std::istream::sentry se(is, true);
    std::streambuf * sb = is.rdbuf();

    while(true) {
        int c = sb->sbumpc();
        switch(c) {
            case '\n': return is;
            case '\r':
                if(sb->sgetc() == '\n') {
                    sb->sbumpc();
                }
                return is;
            case std::streambuf::traits_type::eof():
                if(t.empty()) {
                    is.setstate(std::ios::eofbit);
                }
                return is;
            default:
                t += (char) c;
        }
    }

    return is;
}

lc3::core::asmbl::Tokenizer & lc3::core::asmbl::Tokenizer::operator>>(Token & token)
{
    if(done) {
        return *this;
    }

    if(get_new_line) {
        if(return_new_line) {
            return_new_line = false;
            token.type = Token::Type::EOL;
            return *this;
        }

        col = 0;
        row += 1;

        // Mark as done if we've reached EOF.
        if(getline(buffer, line).eof()) {
            done = true;
            return *this;
        }

        // Ignore comments directly in tokenizer.
        // The ; may be embedded within quotes in a .stringz, so we cannot blindly ignore after ;.
        bool in_string = false;
        uint64_t comment_idx = line.size();
        for(uint64_t i = 0; i < line.size(); ++i) {
            if(line[i] == '"') {
                in_string = ! in_string;
            }
            if(line[i] == ';' && ! in_string) {
                comment_idx = i;
                break;
            }
        }
        line = line.substr(0, comment_idx);

        size_t search = line.find_last_not_of(" \t");
        if(search != std::string::npos) {
            // Ignore trailing whitespace.
            line = line.substr(0, search + 1);
        } else {
            // If here, that means the line had nothing but ' ' or '\t' on it (i.e. empty line), so ignore.
            get_new_line = true;
            return_new_line = false;
            return operator>>(token);
        }

        get_new_line = false;
    }

    // Ignore delimeters entirely.
    std::string delims = ",: \t";
    while(col < line.size() && delims.find(line[col]) != std::string::npos) {
        col += 1;
    }

    // If there's nothing left on this line, get a new line (but first return EOL).
    if(col >= line.size()) {
        get_new_line = true;
        return_new_line = true;
        return operator>>(token);
    }

    // If we've made it here, we have a valid token. First find the length.
    uint32_t len = 0;
    bool found_string = false;
    if(line[col] == '"' && (col == 0 || line[col - 1] != '\\')) {
        // If token begins with an non-escaped quotation mark, the length goes on until the matching non-escaped
        // quotation mark (or EOL if non exists).
        col += 1;    // Consume first non-escaped quotation mark.
        while(col + len < line.size() && ! (line[col + len] == '"' && line[col + len - 1] != '\\')) {
            len += 1;
        }
        found_string = true;
    } else {
        while(col + len < line.size() && delims.find(line[col + len]) == std::string::npos) {
            len += 1;
        }
    }

    // Attempt to convert token into numeric value. If possible, mark as NUM. Otherwise, mark as STRING.
    int32_t token_num_val = 0;
    if(! found_string && convertStringToNum(line.substr(col, len), token_num_val)) {
        token.type = Token::Type::NUM;
        token.num = token_num_val;
    } else {
        token.type = Token::Type::STRING;
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
    if(enable_liberal_asm) {
        if(c_str[0] == '0' && c_str[1] != '\0') { c_str += 1; }
    }

    try {
        uint32_t base;
        switch(c_str[0]) {
            case 'B':
            case 'b': c_str += 1; base = 2;  break;
            case 'X':
            case 'x': c_str += 1; base = 16; break;
            case '#': c_str += 1; base = 10; break;
            default: base = 10; break;
        }

        bool negative = false;
        if(c_str[0] == '-') {
            c_str += 1;
            negative = true;
        }

        std::string conv{c_str};
        if(isValidNumString(conv, base)) {
            val = std::stoi(conv, nullptr, base);
            if(negative) {
                val *= -1;
            }
            return true;
        }

        return false;
    } catch(std::invalid_argument const & e) {
        (void) e;
        return false;
    }
}

bool lc3::core::asmbl::Tokenizer::isValidNumString(std::string const & str, uint32_t base) const
{
    if(base == 16) {
        for(char c : str) {
            if(! std::isxdigit(c)) {
                return false;
            }
        }
    } else if(base == 10) {
        for(char c : str) {
            if(! std::isdigit(c)) {
                return false;
            }
        }
    } else if(base == 2) {
        for(char c : str) {
            if(c != '0' && c != '1') {
                return false;
            }
        }
    }
    return true;
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
