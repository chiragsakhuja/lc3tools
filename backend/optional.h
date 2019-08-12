#ifndef OPTIONAL_H
#define OPTIONAL_H

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
        value = other.value;
        return *this;
    }

    optional & operator=(T && other)
    {
        valid = true;
        std::swap(value, other.value);
        return *this;
    }

    optional & operator=(optional const & other) = default;
    optional & operator=(optional && other) = default;

    T & operator*(void) { return value; }
    T const & operator*(void) const { return value; }

    operator bool(void) const { return valid; }
};
};

#endif
