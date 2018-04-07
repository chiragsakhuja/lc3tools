#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <stdexcept>
#include <string>

namespace lc3
{
namespace utils
{
    std::string udecToBin(uint32_t value, uint32_t num_bits);
    uint32_t sextTo32(uint32_t value, uint32_t num_bits);
    uint32_t getBit(uint32_t value, uint32_t pos);
    uint32_t getBits(uint32_t value, uint32_t end, uint32_t start);
    uint32_t computePSRCC(uint32_t value, uint32_t psr);
    uint32_t computeBasePlusSOffset(uint32_t base, uint32_t signed_off, uint32_t width);

    template<typename ... Args>
    std::string ssprintf(std::string const & format, Args ... args)
    {
        int len = std::snprintf(nullptr, 0, format.c_str(), args...);
        char * c_str = new char[len + 1];

        std::snprintf(c_str, len + 1, format.c_str(), args...);

        std::string ret(c_str);
        delete[] c_str;

        return ret;
    }

    class exception : public std::runtime_error
    {
    public:
        exception(void) : exception("") {}
        exception(std::string const & msg) : std::runtime_error(msg) {}
        virtual const char * what(void) const noexcept override { return std::runtime_error::what(); }
    };

    class fatal_exception : public std::runtime_error
    {
    public:
        fatal_exception(std::string const & msg) : std::runtime_error(msg) {}
        virtual const char * what(void) const noexcept override { return std::runtime_error::what(); }
    };
};
};

#endif
