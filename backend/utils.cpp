/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>

#include "utils.h"

std::string lc3::utils::getMagicHeader(void) { return "\x1c\x30\x15\xc0\x01"; }
std::string lc3::utils::getVersionString(void) { return "\x01\x01"; }

std::string lc3::utils::udecToBin(uint32_t value, uint32_t num_bits)
{
    char * bits = new char[num_bits + 1];
    for(uint32_t i = 0; i < num_bits; i += 1) {
        bits[num_bits - i - 1] = (value & 1) + '0';
        value >>= 1;
    }
    bits[num_bits] = 0;

    return std::string(bits);
}

uint32_t lc3::utils::sextTo32(uint32_t value, uint32_t num_bits)
{
    uint32_t extension = ~((1 << num_bits) - 1);
    if((value >> (num_bits - 1)) & 1) {
        return extension | value;
    } else {
        return value;
    }
}

uint16_t lc3::utils::sextTo16(uint16_t value, uint32_t num_bits)
{
    return static_cast<uint16_t>(sextTo32(static_cast<uint32_t>(value), num_bits) & 0xFFFF);
}

uint32_t lc3::utils::getBit(uint32_t value, uint32_t pos)
{
    return (value >> pos) & 1;
}

uint32_t lc3::utils::getBits(uint32_t value, uint32_t end, uint32_t start)
{
    return (value >> start) & ((1 << (end - start + 1)) - 1);
}

uint32_t lc3::utils::computePSRCC(uint32_t value, uint32_t psr)
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

uint32_t lc3::utils::computeBasePlusSOffset(uint32_t base, uint32_t signed_off, uint32_t width)
{
    return (utils::sextTo32(base, (uint32_t) 16) + lc3::utils::sextTo32(signed_off, width)) & 0xffff;
}

std::string lc3::utils::toLower(std::string const & str)
{
    std::string ret = str;
    std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);
    return ret;
}
