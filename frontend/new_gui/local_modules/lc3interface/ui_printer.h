#ifndef UI_PRINTER
#define UI_PRINTER

namespace utils
{
    class UIPrinter : public lc3::utils::IPrinter
    {
    private:
        std::vector<std::string> output_buffer;
        uint32_t pending_colors;

    public:
        UIPrinter(void) : pending_colors(0) {}

        virtual void setColor(lc3::utils::PrintColor color) override;
        virtual void print(std::string const & string) override;
        virtual void newline(void) override;

        std::vector<std::string> const & getOutputBuffer(void) { return output_buffer; }
        void clearOutputBuffer(void) { output_buffer.clear(); }
    };
};

void utils::UIPrinter::setColor(lc3::utils::PrintColor color)
{
    using namespace lc3::utils;

    if(color == PrintColor::RESET) {
        while(pending_colors != 0) {
            output_buffer.push_back("</span>");
            pending_colors -= 1;
        }
    } else {
        std::string format = "<span class=\"text-";
        switch(color)
        {
            case PrintColor::RED      : format += "red"    ; break;
            case PrintColor::YELLOW   : format += "yellow" ; break;
            case PrintColor::GREEN    : format += "green"  ; break;
            case PrintColor::MAGENTA  : format += "magenta"; break;
            case PrintColor::BLUE     : format += "blue"   ; break;
            case PrintColor::GRAY     : format += "gray"   ; break;
            case PrintColor::BOLD     : format += "bold"   ; break;
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
