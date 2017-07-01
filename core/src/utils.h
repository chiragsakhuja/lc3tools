#ifndef UTILS_H
#define UTILS_H

namespace core {
    std::string udecToBin(uint32_t value, uint32_t num_bits);
    uint32_t sextTo32(uint32_t value, uint32_t num_bits);
};

#endif
