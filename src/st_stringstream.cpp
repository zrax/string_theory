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
#include "st_format_simple.h"

#include <cstdio>

#define EXPAND_SS_BUFFER(added_size)                                \
    if (m_size + added_size > m_alloc) {                            \
        size_t big_size = m_alloc;                                  \
        do {                                                        \
            big_size *= 2;                                          \
        } while (m_size + added_size > big_size);                   \
                                                                    \
        char *bigger = new char[big_size];                          \
        std::char_traits<char>::copy(bigger, m_chars, m_alloc);     \
        if (is_heap())                                              \
            delete[] m_chars;                                       \
        m_chars = bigger;                                           \
        m_alloc = big_size;                                         \
    }

ST::string_stream &ST::string_stream::append(const char *data, size_t size)
{
    if (size == 0)
        return *this;

    if (size == ST_AUTO_SIZE)
        size = data ? std::char_traits<char>::length(data) : 0;

    EXPAND_SS_BUFFER(size)

    std::char_traits<char>::move(m_chars + m_size, data, size);
    m_size += size;
    return *this;
}

ST::string_stream &ST::string_stream::append_char(char ch, size_t count)
{
    if (count == 0)
        return *this;

    EXPAND_SS_BUFFER(count)

    if (count == 1)
        *(m_chars + m_size) = ch;
    else
        std::char_traits<char>::assign(m_chars + m_size, count, ch);
    m_size += count;
    return *this;
}

ST::string_stream &ST::string_stream::operator<<(int num)
{
    ST::uint_formatter<unsigned int> formatter;
    formatter.format(safe_abs(num), 10, false);
    if (num < 0)
        append_char('-');
    return append(formatter.text(), formatter.size());
}

ST::string_stream &ST::string_stream::operator<<(unsigned int num)
{
    ST::uint_formatter<unsigned int> formatter;
    formatter.format(num, 10, false);
    return append(formatter.text(), formatter.size());
}

ST::string_stream &ST::string_stream::operator<<(long num)
{
    ST::uint_formatter<unsigned long> formatter;
    formatter.format(safe_abs(num), 10, false);
    if (num < 0)
        append_char('-');
    return append(formatter.text(), formatter.size());
}

ST::string_stream &ST::string_stream::operator<<(unsigned long num)
{
    ST::uint_formatter<unsigned long> formatter;
    formatter.format(num, 10, false);
    return append(formatter.text(), formatter.size());
}

ST::string_stream &ST::string_stream::operator<<(long long num)
{
    ST::uint_formatter<unsigned long long> formatter;
    formatter.format(safe_abs(num), 10, false);
    if (num < 0)
        append_char('-');
    return append(formatter.text(), formatter.size());
}

ST::string_stream &ST::string_stream::operator<<(unsigned long long num)
{
    ST::uint_formatter<unsigned long long> formatter;
    formatter.format(num, 10, false);
    return append(formatter.text(), formatter.size());
}

ST::string_stream &ST::string_stream::operator<<(float num)
{
    ST::float_formatter<float> formatter;
    formatter.format(num, 'g');
    return append(formatter.text(), formatter.size());
}

ST::string_stream &ST::string_stream::operator<<(double num)
{
    ST::float_formatter<double> formatter;
    formatter.format(num, 'g');
    return append(formatter.text(), formatter.size());
}
