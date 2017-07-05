#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include "utils.h"

#include "object_file_utils.h"

using namespace utils;

ObjectFileReader::ObjectFileReader(std::string const & filename)
{
    file.open(filename, std::ios::binary);

    if(! file) {
        throw core::exception(core::ssprintf("could not load \'%s\'", filename.c_str()));
    } 

    file_stream = std::istreambuf_iterator<char>(file);

    first_read = true;
}

ObjectFileReader::~ObjectFileReader(void)
{
    if(file) {
        file.close();
    }
}

ObjectFileStatement ObjectFileReader::readStatement(void)
{
    uint32_t value = *file_stream;
    ++file_stream;
    value = (value << 8) | (*file_stream);
    ++file_stream;

    ObjectFileStatement ret(value, first_read);
    first_read = false;

    return ret;
}

ObjectFileStatement::ObjectFileStatement(uint32_t value, bool orig)
{
    this->value = value;
    this->orig = orig;
}
