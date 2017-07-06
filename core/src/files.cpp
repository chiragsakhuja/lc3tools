#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "utils.h"

#include "files.h"

using namespace utils;

ObjectFileReader::ObjectFileReader(std::string const & filename) :
    first_read(true)
{
    file.open(filename);
    if(! file.is_open()) {
        throw core::exception(core::ssprintf("could not open file \'%s\'", filename.c_str()));
    }
}

ObjectFileStatement ObjectFileReader::readStatement(void)
{
    uint32_t value = (*file_stream) & 0xff;
    ++file_stream;
    value = (value << 8) | ((*file_stream) & 0xff);
    ++file_stream;

    ObjectFileStatement ret(value & 0xffff, first_read);
    first_read = false;

    return ret;
}

ObjectFileStatement::ObjectFileStatement(uint32_t value, bool orig)
{
    this->value = value;
    this->orig = orig;
}
