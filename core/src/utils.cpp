#include <cstdint>
#include <string>
#include "utils.h"

std::string core::udecToBin(uint32_t value, uint32_t num_bits)
{
    char * bits = new char[num_bits + 1];
    for(uint32_t i = 0; i < num_bits; i += 1) {
        bits[num_bits - i - 1] = (value & 1) + '0';
        value >>= 1;
    }
    bits[num_bits] = 0;

    return std::string(bits);
}

uint32_t core::sextTo32(uint32_t value, uint32_t num_bits)
{
    uint32_t extension = ~((1 << num_bits) - 1);
    if((value >> (num_bits - 1)) & 1) {
        return extension | value;
    } else {
        return value;
    }
}

