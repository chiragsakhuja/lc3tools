#include <cstring>

#include "mem.h"

std::ostream & lc3::core::operator<<(std::ostream & out, lc3::core::MemLocation const & in)
{
#ifdef _ENABLE_DEBUG_ASM
    out << lc3::utils::ssprintf("0x%0.4x", in.value) << " " << in.orig << " " << in.line << "\n";
    return out;
#else
    // TODO: this is extrememly unportable, namely because it relies on the endianness not changing
    // encoding (2 bytes), then orig bool (1 byte), then number of characters (4 bytes), then actual line (N bytes,
    // not null terminated)
    uint32_t num_bytes = (uint32_t) (2 + 1 + 4 + in.line.size());
    char * bytes = new char[num_bytes];
    std::memcpy(bytes, (char *) (&in.value), 2);
    std::memcpy(bytes + 2, (char *) (&in.is_orig), 1);
    uint32_t num_chars = (uint32_t) in.line.size();
    std::memcpy(bytes + 3, (char *) (&num_chars), 4);
    char const * data = in.line.data();
    std::memcpy(bytes + 7, data, num_chars);
    out.write(bytes, num_bytes);
#ifdef _ENABLE_DEBUG_ASM
    std::cout << in.line << ": ";
    for (uint32_t i = 0; i < num_bytes; i += 1) {
        std::cout << lc3::utils::ssprintf("0x%02x ", bytes[i]);
    }
    std::cout << "\n";
#endif
    delete[] bytes;
    return out;
#endif
}

std::istream & lc3::core::operator>>(std::istream & in, MemLocation & out)
{
    uint16_t value;
    bool is_orig;

    in.read(reinterpret_cast<char *>(&value), 2);
    in.read(reinterpret_cast<char *>(&is_orig), 1);
    out.value = value;
    out.is_orig = is_orig;

    uint32_t num_chars;
    in.read(reinterpret_cast<char *>(&num_chars), 4);
#ifdef _ENABLE_DEBUG_ASM
    std::cout << "value: " << lc3::utils::ssprintf("0x%04x", out.value) << "\n";
    std::cout << "orig: " << out.is_orig << "\n";
    std::cout << "num_chars: " << num_chars << "\n";
#endif
    if(num_chars > 0) {
        char * chars = new char[num_chars + 1];
        in.read(chars, num_chars);
        chars[num_chars] = 0;
        out.line = std::string(chars);
#ifdef _ENABLE_DEBUG_ASM
        std::cout << "line: " << out.line << "\n";
#endif
    }

    return in;
}
