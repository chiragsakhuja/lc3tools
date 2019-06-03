#ifndef CONVERTER_H
#define CONVERTER_H

#include "logger.h"
#include "printer.h"

namespace lc3
{
namespace core
{
    class Converter
    {
    public:
        Converter(lc3::utils::IPrinter & printer, uint32_t print_level) : logger(printer, print_level) {}
        void convertBin(std::string const & bin_filename, std::string const & obj_filename);

    private:
        lc3::utils::Logger logger;
    };
};
};

#endif
