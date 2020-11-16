/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

namespace lc3
{
    template<typename T>
    class optional
    {
    private:
        bool valid;
        T value;

    public:
        optional(void) : valid(false) {}

        optional(T const & value) : valid(true), value(value) {}
        optional(T && other) : valid(true) { std::swap(value, other); }

        optional(optional const & other) = default;
        optional(optional && other) = default;

        optional & operator=(T const & other)
        {
            valid = true;
            value = other;
            return *this;
        }

        optional & operator=(T && other)
        {
            valid = true;
            std::swap(value, other);
            return *this;
        }

        optional & operator=(optional const & other) = default;
        optional & operator=(optional && other) = default;

        T & operator*(void) { return value; }
        T const & operator*(void) const { return value; }
        T * operator->(void) { return &value; }
        T const * operator->(void) const { return &value; }

        operator bool(void) const { return valid; }
        bool isValid(void) const { return valid; }
    };

    template<typename T>
    class future
    {
    private:
        bool valid;
        T value;

    public:
        future(void) : valid(false) {}

        future(future const & other) = default;
        future(future && other) = default;

        future & operator=(T const & value)
        {
            valid = true;
            this->value = value;
            return *this;
        }

        future & operator=(T && value)
        {
            valid = true;
            std::swap(this->value, value);
            return *this;
        }

        future & operator=(future const & other) = default;
        future & operator=(future && other) = default;

        T & get(void)
        {
            if(! valid) {
                throw std::runtime_error("accessing invalid future");
            }

            return value;
        }

        T const & get(void) const
        {
            if(! valid) {
                throw std::runtime_error("accessing invalid future");
            }

            return value;
        }
    };

    namespace utils
    {
        std::string getMagicHeader(void);
        std::string getVersionString(void);

        std::string udecToBin(uint32_t value, uint32_t num_bits);
        uint32_t sextTo32(uint32_t value, uint32_t num_bits);
        uint16_t sextTo16(uint16_t value, uint32_t num_bits);
        uint32_t getBit(uint32_t value, uint32_t pos);
        uint32_t getBits(uint32_t value, uint32_t end, uint32_t start);
        std::string toLower(std::string const & str);

        template<typename ... Args>
        std::string ssprintf(std::string const & format, Args ... args)
        {
            int len = std::snprintf(nullptr, 0, format.c_str(), args...);
            char * c_str = new char[len + 1];

            std::snprintf(c_str, len + 1, format.c_str(), args...);

            std::string ret(c_str);
            delete[] c_str;

            return ret;
        }

        class exception : public std::runtime_error
        {
        public:
            exception(void) : exception("") {}
            exception(std::string const & msg) : std::runtime_error(msg) {}
            virtual const char * what(void) const noexcept override { return std::runtime_error::what(); }
        };

        class fatal_exception : public std::runtime_error
        {
        public:
            fatal_exception(std::string const & msg) : std::runtime_error(msg) {}
            virtual const char * what(void) const noexcept override { return std::runtime_error::what(); }
        };
    };
};

#endif
