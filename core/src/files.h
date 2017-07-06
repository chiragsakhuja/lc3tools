#ifndef OBJECT_FILE_UTILS_H
#define OBJECT_FILE_UTILS_H

namespace utils {
    class ObjectFileStatement
    {
    public:
        ObjectFileStatement(uint32_t value, bool orig);

        uint32_t getValue(void) { return value; }
        bool isOrig(void) { return orig; }

    private:
        uint32_t value;
        bool orig;
    };

    class ObjectFileReader : public std::ifstream
    {
    public:
        ObjectFileReader(std::string const & filename);
        ~ObjectFileReader(void) { if(file.is_open()) { file.close(); } }

        ObjectFileStatement readStatement(void);
        bool atEnd(void) { return file_stream == std::istreambuf_iterator<char>(); }

    private:
        std::ifstream file;
        std::istreambuf_iterator<char> file_stream;
        bool first_read;
    };
};

#endif
