#ifndef UTILS_H
#define UTILS_H

namespace core {
    std::string udecToBin(uint32_t value, uint32_t num_bits);
    uint32_t sextTo32(uint32_t value, uint32_t num_bits);
    uint32_t getBit(uint32_t value, uint32_t pos);
    uint32_t getBits(uint32_t value, uint32_t end, uint32_t start);
    uint32_t computePSRCC(uint32_t value, uint32_t psr);
    uint32_t computeBasePlusSOffset(uint32_t base, uint32_t signed_off, uint32_t width);
};

#endif
