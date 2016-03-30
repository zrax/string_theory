/*  This file is part of string_theory.

    string_theory is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    string_theory is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with string_theory.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef _ST_ASSERT_H
#define _ST_ASSERT_H

#include "st_config.h"

#include <functional>
#include <stdexcept>

namespace ST
{
    typedef std::function<void (const char *condition_str,
                                const char *filename, int line,
                                const char *message)> assert_handler_t;

    ST_EXPORT void set_assert_handler(assert_handler_t handler);

    class ST_EXPORT unicode_error : public std::runtime_error
    {
    public:
        explicit unicode_error(const char *message)
            : std::runtime_error(message)
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

#endif
