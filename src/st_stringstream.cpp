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

#include "st_stringstream.h"

#include <cstring>
#include <cstdio>

#define EXPAND_SS_BUFFER(added_size)                                \
    char *bufp = is_heap() ? m_buffer : m_stack;                    \
                                                                    \
    if (m_size + added_size > m_alloc) {                            \
        size_t big_size = m_alloc;                                  \
        do {                                                        \
            big_size *= 2;                                          \
        } while (m_size + added_size > big_size);                   \
                                                                    \
        char *bigger = new char[big_size];                          \
        _ST_PRIVATE::_copy_buffer(bigger, raw_buffer(), m_alloc);   \
        if (is_heap())                                              \
            delete[] m_buffer;                                      \
        m_buffer = bufp = bigger;                                   \
        m_alloc = big_size;                                         \
    }

ST::string_stream &ST::string_stream::append(const char *data, size_t size)
{
    if (size == 0)
        return *this;

    if (size == ST_AUTO_SIZE)
        size = strlen(data);

    EXPAND_SS_BUFFER(size)

    _ST_PRIVATE::_copy_buffer(bufp + m_size, data, size);
    m_size += size;
    return *this;
}

ST::string_stream &ST::string_stream::append_char(char ch, size_t count)
{
    if (count == 0)
        return *this;

    EXPAND_SS_BUFFER(count)

    if (count == 1)
        *(bufp + m_size) = ch;
    else
        _ST_PRIVATE::_fill_buffer(bufp + m_size, ch, count);
    m_size += count;
    return *this;
}

ST::string_stream &ST::string_stream::operator<<(int num)
{
    char buffer[12];
    snprintf(buffer, sizeof(buffer), "%d", num);
    return append(buffer);
}

ST::string_stream &ST::string_stream::operator<<(unsigned int num)
{
    char buffer[12];
    snprintf(buffer, sizeof(buffer), "%u", num);
    return append(buffer);
}

#ifdef ST_HAVE_INT64
ST::string_stream &ST::string_stream::operator<<(int64_t num)
{
    char buffer[24];
    snprintf(buffer, sizeof(buffer), "%lld", static_cast<long long>(num));
    return append(buffer);
}

ST::string_stream &ST::string_stream::operator<<(uint64_t num)
{
    char buffer[24];
    snprintf(buffer, sizeof(buffer), "%llu", static_cast<unsigned long long>(num));
    return append(buffer);
}
#endif

ST::string_stream &ST::string_stream::operator<<(double num)
{
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%f", num);
    return *this;
}
