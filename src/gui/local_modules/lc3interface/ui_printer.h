/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#ifndef UI_PRINTER
#define UI_PRINTER

namespace utils
{
    class UIPrinter : public lc3::utils::IPrinter
    {
    private:
        std::mutex output_buffer_mutex;
        std::vector<std::string> output_buffer;
        uint32_t pending_colors;

    public:
        UIPrinter(void) : pending_colors(0) {}

        virtual void setColor(lc3::utils::PrintColor color) override;
        virtual void print(std::string const & string) override;
        virtual void newline(void) override;

        std::vector<std::string> getAndClearOutputBuffer(void);
    };
};

#endif
