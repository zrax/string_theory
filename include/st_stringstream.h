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

#ifndef _ST_STRINGSTREAM_H
#define _ST_STRINGSTREAM_H

#include "st_string.h"

namespace ST
{
    class ST_EXPORT string_stream
    {
        ST_DISABLE_COPY(string_stream)

    public:
        string_stream() noexcept
            : m_chars(m_stack), m_alloc(ST_STACK_STRING_LEN), m_size() { }

        ~string_stream() noexcept
        {
            if (is_heap())
                delete[] m_chars;
        }

        string_stream(string_stream &&move) noexcept
            : m_alloc(move.m_alloc), m_size(move.m_size)
        {
            m_chars = is_heap() ? move.m_chars : m_stack;
            std::char_traits<char>::copy(m_stack, move.m_stack, ST_STACK_STRING_LEN);
            move.m_alloc = 0;
        }

        string_stream &operator=(string_stream &&move) noexcept
        {
            if (is_heap())
                delete[] m_chars;

            m_alloc = move.m_alloc;
            m_size = move.m_size;
            m_chars = is_heap() ? move.m_chars : m_stack;
            std::char_traits<char>::copy(m_stack, move.m_stack, ST_STACK_STRING_LEN);
            move.m_alloc = 0;
            return *this;
        }

        string_stream &append(const char *data, size_t size = ST_AUTO_SIZE);

        string_stream &append_char(char ch, size_t count = 1);

        string_stream &operator<<(const char *text)
        {
            return append(text);
        }

        string_stream &operator<<(int num);
        string_stream &operator<<(unsigned int num);
        string_stream &operator<<(long num);
        string_stream &operator<<(unsigned long num);
        string_stream &operator<<(long long num);
        string_stream &operator<<(unsigned long long num);

        string_stream &operator<<(float num);
        string_stream &operator<<(double num);

        string_stream &operator<<(char ch)
        {
            return append_char(ch);
        }

        string_stream &operator<<(const string &text)
        {
            return append(text.c_str(), text.size());
        }

        const char *raw_buffer() const noexcept { return m_chars; }
        size_t size() const noexcept { return m_size; }

        string to_string(bool utf8_encoded = true,
                         utf_validation_t validation = ST_DEFAULT_VALIDATION) const
        {
            if (utf8_encoded)
                return string::from_utf8(raw_buffer(), size(), validation);
            else
                return string::from_latin_1(raw_buffer(), size());
        }

        void truncate(size_t size = 0) noexcept
        {
            if (size < m_size)
                m_size = size;
        }

        void erase(size_t count) noexcept
        {
            if (count < m_size)
                m_size -= count;
            else
                m_size = 0;
        }

    private:
        char  *m_chars;
        size_t m_alloc, m_size;
        char   m_stack[ST_STACK_STRING_LEN];

        bool is_heap() const noexcept
        {
            return m_alloc > ST_STACK_STRING_LEN;
        }
    };
}

#endif // _ST_STRINGSTREAM_H
