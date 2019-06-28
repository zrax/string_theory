/*  Copyright (c) 2018 Michael Hansen

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

#ifndef _ST_FORMAT_NUMERIC_H
#define _ST_FORMAT_NUMERIC_H

#include <limits>

namespace _ST_PRIVATE
{
    ST_EXPORT size_t format_double(char *buffer, size_t size, double value, char format);
}

namespace ST
{
    template <typename uint_T>
    class uint_formatter
    {
        typedef std::numeric_limits<uint_T> uint_limits;
        static_assert(uint_limits::is_integer && !uint_limits::is_signed,
                      "uint_formatter can only be used for unsigned integral types");

    public:
        uint_formatter() noexcept : m_start(nullptr) { }

        void format(uint_T value, int radix, bool upper_case = false) noexcept
        {
            m_buffer[uint_limits::digits] = 0;
            m_start = &m_buffer[uint_limits::digits];
            if (value == 0) {
                *--m_start = '0';
                return;
            }

            while (value) {
                unsigned int digit = (value % radix);
                value /= radix;
                --m_start;

                if (digit < 10)
                    *m_start = '0' + digit;
                else if (upper_case)
                    *m_start = 'A' + digit - 10;
                else
                    *m_start = 'a' + digit - 10;
            }
        }

        const char *text() const noexcept { return m_start; }

        size_t size() const noexcept
        {
            return m_buffer + uint_limits::digits - m_start;
        }

    private:
        char m_buffer[uint_limits::digits + 1];
        char *m_start;
    };

    template <typename float_T>
    class float_formatter
    {
        static_assert(!std::numeric_limits<float_T>::is_integer,
                      "float_formatter can only be used for floating point types");

    public:
        float_formatter() noexcept : m_size() { }

        void format(float_T value, char format)
        {
            static const char valid_formats[] = "efgEFG";
            if (!std::char_traits<char>::find(valid_formats, sizeof(valid_formats) - 1, format))
                throw ST::bad_format("Unsupported floating-point format specifier");

            m_size = _ST_PRIVATE::format_double(m_buffer, sizeof(m_buffer), value, format);
        }

        const char *text() const noexcept { return m_buffer; }
        size_t size() const noexcept { return m_size; }

    private:
        char m_buffer[64];
        size_t m_size;
    };
}

#endif // _ST_FORMAT_NUMERIC_H
