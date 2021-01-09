/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include "framework_common.h"

bool endsWith(std::string const & search, std::string const & suffix)
{
    if(suffix.size() > search.size()) { return false; }
    return std::equal(suffix.rbegin(), suffix.rend(), search.rbegin());
}

void BufferedPrinter::print(std::string const & string)
{
    std::copy(string.begin(), string.end(), std::back_inserter(display_buffer));
    if(print_output) {
        std::cout << string;
    }
}

void BufferedPrinter::newline(void)
{
    display_buffer.push_back('\n');
    if(print_output) {
        std::cout << "\n";
    }
}

void StringInputter::setString(std::string const & source)
{
    this->source = source;
    this->pos = 0;
}

void StringInputter::setCharDelay(uint32_t inst_count)
{
    this->reset_inst_delay = inst_count;
    this->cur_inst_delay = inst_count;
}

void StringInputter::setStringAfter(std::string const & source, uint32_t inst_count)
{
    setString(source);
    setCharDelay(inst_count);
}

bool StringInputter::getChar(char & c)
{
    if(cur_inst_delay > 0) {
        --cur_inst_delay;
        return false;
    }

    if(pos == source.size()) {
        return false;
    }

    c = source[pos];
    ++pos;
    cur_inst_delay = reset_inst_delay;
    return true;
}

