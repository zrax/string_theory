/*  Copyright (c) 2016 Michael Hansen

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE. */

#ifndef _ST_ASSERT_H
#define _ST_ASSERT_H

#include "st_config.h"

#include <functional>
#include <stdexcept>

#ifdef _MSC_VER
#   pragma warning(push)
#   pragma warning(disable: 4275)
#endif

namespace ST
{
    typedef std::function<void (const char *condition_str,
                                const char *filename, int line,
                                const char *message)> assert_handler_t;

    ST_EXPORT void set_assert_handler(assert_handler_t handler) noexcept;
    ST_EXPORT void set_default_assert_handler() noexcept;

    class ST_EXPORT unicode_error : public std::runtime_error
    {
    public:
        explicit unicode_error(const char *message) noexcept
            : std::runtime_error(message)
        { }
    };

    class ST_EXPORT codec_error : public std::runtime_error
    {
    public:
        explicit codec_error(const char *message) noexcept
            : std::runtime_error(message)
        { }
    };

    class ST_EXPORT bad_format : public std::invalid_argument
    {
    public:
        explicit bad_format(const char *message) noexcept
            : std::invalid_argument(message)
        { }
    };
}

namespace _ST_PRIVATE
{
    ST_EXPORT extern ST::assert_handler_t _assert_handler;
}

#define ST_ASSERT(condition, message) \
    do { \
        if (!(condition) && _ST_PRIVATE::_assert_handler) \
            _ST_PRIVATE::_assert_handler(#condition, __FILE__, __LINE__, message); \
    } while (0)

#ifdef _MSC_VER
#   pragma warning(pop)
#endif

#endif // _ST_ASSERT_H
