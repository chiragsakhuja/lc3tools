#include <cstdint>
#include <stdexcept>
#include <string>

#include "utils.h"

std::string utils::udecToBin(uint32_t value, uint32_t num_bits)
{
    char * bits = new char[num_bits + 1];
    for(uint32_t i = 0; i < num_bits; i += 1) {
        bits[num_bits - i - 1] = (value & 1) + '0';
        value >>= 1;
    }
    bits[num_bits] = 0;

    return std::string(bits);
}

uint32_t utils::sextTo32(uint32_t value, uint32_t num_bits)
{
    uint32_t extension = ~((1 << num_bits) - 1);
    if((value >> (num_bits - 1)) & 1) {
        return extension | value;
    } else {
        return value;
    }
}

uint32_t utils::getBit(uint32_t value, uint32_t pos)
{
    return (value >> pos) & 1;
}

uint32_t utils::getBits(uint32_t value, uint32_t end, uint32_t start)
{
    return (value >> start) & ((1 << (end - start + 1)) - 1);
}

uint32_t utils::computePSRCC(uint32_t value, uint32_t psr)
{
    uint32_t cc = 0;
    if(value == 0) {
        cc = 2;
    } else if((value & 0x8000) != 0) {
        cc = 4;
    } else {
        cc = 1;
    }

    return (psr & 0xFFF8) | cc;
}

uint32_t utils::computeBasePlusSOffset(uint32_t base, uint32_t signed_off, uint32_t width)
{
    return (utils::sextTo32(base, (uint32_t) 16) + utils::sextTo32(signed_off, width)) & 0xffff;
}
