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

#include "st_charbuffer.h"

#include <cstring>

ST_STATIC_ASSERT(ST_SHORT_STRING_LEN >= sizeof(void *),
                 "ST_SHORT_STRING_LEN must be at least as large as a pointer")
ST_STATIC_ASSERT(ST_STACK_STRING_LEN >= sizeof(void *),
                 "ST_STACK_STRING_LEN must be at least as large as a pointer")

void ST::_zero_buffer(void *buffer, size_t size)
{
    memset(buffer, 0, size);
}

void ST::_fill_buffer(void *buffer, int ch, size_t count)
{
    memset(buffer, ch, count);
}

void ST::_copy_buffer(void *dest, const void *src, size_t size)
{
    memcpy(dest, src, size);
}
