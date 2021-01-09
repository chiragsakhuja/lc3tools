/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include <cstdint>
#include <string>

#include "inputter.h"
#include "interface.h"
#include "printer.h"

class BufferedPrinter : public lc3::utils::IPrinter
{
public:
    BufferedPrinter(bool print_output) : print_output(print_output) {}

    virtual void setColor(lc3::utils::PrintColor color) override { (void) color; }
    virtual void print(std::string const & string) override;
    virtual void newline(void) override;
    void clear(void) { display_buffer.clear(); }
    std::vector<char> const & getBuffer(void) const { return display_buffer; }

private:
    bool print_output;
#if (! defined API_VER) || API_VER == 1
public:
#endif
    std::vector<char> display_buffer;
};

class StringInputter : public lc3::utils::IInputter
{
public:
    StringInputter(void) : StringInputter("") {}
    StringInputter(std::string const & source) : pos(0), reset_inst_delay(0), cur_inst_delay(0) { setString(source); }

    void setString(std::string const & source);
    void setCharDelay(uint32_t inst_count);
    void setStringAfter(std::string const & source, uint32_t inst_count);    // Really just here for legacy reasons :(
    virtual void beginInput(void) override {}
    virtual bool getChar(char & c) override;
    virtual void endInput(void) override {}
    virtual bool hasRemaining(void) const override { return pos == source.size(); }

private:
    std::string source;
    uint32_t pos;
    uint32_t reset_inst_delay, cur_inst_delay;
};

bool endsWith(std::string const & search, std::string const & suffix);
