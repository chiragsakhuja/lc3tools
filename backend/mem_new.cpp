#include "mem_new.h"

std::istream & lc3::core::operator>>(std::istream & in, MemLocation & out)
{
    uint16_t value;
    bool is_orig;

    in.read(reinterpret_cast<char *>(&value), 2);
    in.read(reinterpret_cast<char *>(&is_orig), 1);
    out.setValue(value);
    out.setIsOrig(is_orig);

    uint32_t num_chars;
    in.read(reinterpret_cast<char *>(&num_chars), 4);
#ifdef _ENABLE_DEBUG_ASM
    std::cout << "value: " << lc3::utils::ssprintf("0x%04x", out.getValue()) << "\n";
    std::cout << "orig: " << out.isOrig() << "\n";
    std::cout << "num_chars: " << num_chars << "\n";
#endif
    if(num_chars > 0) {
        char * chars = new char[num_chars + 1];
        in.read(chars, num_chars);
        chars[num_chars] = 0;
        out.setLine(std::string(chars));
#ifdef _ENABLE_DEBUG_ASM
        std::cout << "line: " << out.getLine() << "\n";
#endif
    }

    return in;
}
