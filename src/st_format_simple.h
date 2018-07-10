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

namespace ST
{
    template <typename uint_T>
    class uint_formatter
    {
    public:
        void format(uint_T value, int radix, bool upper_case = false)
        {
            m_buffer[std::numeric_limits<uint_T>::digits] = 0;
            m_start = &m_buffer[std::numeric_limits<uint_T>::digits];
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

        char *text() const { return m_start; }
        size_t size() const { return m_buffer + std::numeric_limits<uint_T>::digits - m_start; }

    private:
        char m_buffer[std::numeric_limits<uint_T>::digits + 1];
        char *m_start = ST_NULLPTR;
    };
}
