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

#include "st_assert.h"

#include <cstdio>

static void _default_assert_handler(const char *condition_str,
                                    const char *filename, int line,
                                    const char *message)
{
    (void)condition_str;

    fprintf(stderr, "%s:%d: %s\n", filename, line, message);
    abort();
}

ST::assert_handler_t _ST_PRIVATE::_assert_handler = _default_assert_handler;

void ST::set_assert_handler(assert_handler_t handler)
{
    _ST_PRIVATE::_assert_handler = handler;
}
