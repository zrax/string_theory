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

ST::string_stream &ST::string_stream::operator<<(long num)
{
    char buffer[24];
    snprintf(buffer, sizeof(buffer), "%ld", num);
    return append(buffer);
}

ST::string_stream &ST::string_stream::operator<<(unsigned long num)
{
    char buffer[24];
    snprintf(buffer, sizeof(buffer), "%lu", num);
    return append(buffer);
}

#if defined(ST_HAVE_INT64) && !defined(ST_INT64_IS_LONG)
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
