#ifndef UI_PRINTER
#define UI_PRINTER

namespace utils
{
    class UIPrinter : public IPrinter
    {
    private:
        std::vector<std::string> output_buffer;
        uint32_t pending_colors;
    public:
        UIPrinter(void);

        virtual void setColor(int color) override;
        virtual void print(std::string const & string) override;
        virtual void newline(void) override;

        std::vector<std::string> const & getOutputBuffer(void) { return output_buffer; }
        void clearOutputBuffer(void) { output_buffer.clear(); }
    };
};

utils::UIPrinter::UIPrinter(void)
{
    pending_colors = 0;
}

void utils::UIPrinter::setColor(int color)
{
    if(color == PRINT_COLOR_RESET) {
        while(pending_colors != 0) {
            output_buffer.push_back("</span>");
            pending_colors -= 1;
        }
    } else {
        std::string format = "<span class=\"console-text-";
        switch(color)
        {
            case PRINT_COLOR_RED      : format += "red"    ; break;
            case PRINT_COLOR_YELLOW   : format += "yellow" ; break;
            case PRINT_COLOR_GREEN    : format += "green"  ; break;
            case PRINT_COLOR_MAGENTA  : format += "magenta"; break;
            case PRINT_COLOR_BLUE     : format += "blue"   ; break;
            case PRINT_COLOR_GRAY     : format += "gray"   ; break;
            case PRINT_COLOR_BOLD     : format += "bold"   ; break;
            default                   : break;
        }
        format += "\">";
        output_buffer.push_back(format);
        pending_colors += 1;
    }
}

void utils::UIPrinter::print(std::string const & string)
{
    output_buffer.push_back(string);
}

void utils::UIPrinter::newline(void)
{
    output_buffer.push_back("</br>");
}

#endif
