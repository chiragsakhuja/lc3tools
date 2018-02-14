#include <cstring>
#include <fstream>

#include "mem.h"
#ifdef _ENABLE_DEBUG_ASM
    #include "utils.h"
#endif

namespace lc3
{
namespace core
{
    std::ostream & operator<<(std::ostream & out, lc3::core::MemEntry const & in)
    {
#ifdef _ENABLE_DEBUG_ASM
        out << lc3::utils::ssprintf("0x%0.4x", in.value) << " " << in.orig << " " << in.line << "\n";
        return out;
#else
        // TODO: this is extrememly unportable, namely because it relies on the endianness not changing
        // size of num_bytes + value + orig + line + nullptr
        uint32_t num_bytes = 2 + 1 + 4 + in.line.size();
        char * bytes = new char[num_bytes];
        std::memcpy(bytes, (char *) (&in.value), 2);
        std::memcpy(bytes + 2, (char *) (&in.orig), 1);
        uint32_t num_chars = in.line.size();
        std::memcpy(bytes + 3, (char *) (&num_chars), 4);
        char const * data = in.line.data();
        std::memcpy(bytes + 7, data, num_chars);
        out.write(bytes, num_bytes);
        delete[] bytes;
        return out;
#endif
    }

    std::istream & operator>>(std::istream & in, lc3::core::MemEntry & out)
    {
        in.read((char *) (&out.value), 2);
        in.read((char *) (&out.orig), 1);
        uint32_t num_chars;
        in.read((char *) (&num_chars), 4);
        char * chars = new char[num_chars + 1];
        in.read(chars, num_chars);
        chars[num_chars] = 0;
        out.line = std::string(chars);
        return in;
    }
};
};
