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

#ifndef _ST_STRINGSTREAM_H
#define _ST_STRINGSTREAM_H

#include "st_string.h"

namespace ST
{
    class ST_EXPORT string_stream
    {
        ST_DISABLE_COPY(string_stream)

    public:
        string_stream() : m_alloc(ST_STACK_STRING_LEN), m_size() { }

        ~string_stream()
        {
            if (is_heap())
                delete[] m_buffer;
        }

#ifdef ST_HAVE_RVALUE_MOVE
        string_stream(string_stream &&move) ST_NOEXCEPT
        {
            _ST_PRIVATE::_copy_buffer(m_stack, move.m_stack, sizeof(m_stack));
            move.m_alloc = 0;
        }

        string_stream &operator=(string_stream &&move) ST_NOEXCEPT
        {
            _ST_PRIVATE::_copy_buffer(m_stack, move.m_stack, sizeof(m_stack));
            m_alloc = move.m_alloc;
            m_size = move.m_size;
            move.m_alloc = 0;
            return *this;
        }
#endif

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

#if defined(ST_HAVE_INT64) && !defined(ST_INT64_IS_LONG)
        string_stream &operator<<(int64_t num);
        string_stream &operator<<(uint64_t num);
#endif

        string_stream &operator<<(float num)
        {
            return operator<<(static_cast<double>(num));
        }

        string_stream &operator<<(double num);

        string_stream &operator<<(char ch)
        {
            return append_char(ch);
        }

        string_stream &operator<<(const string &text)
        {
            return append(text.c_str(), text.size());
        }

        const char *raw_buffer() const ST_NOEXCEPT
        {
            return is_heap() ? m_buffer : m_stack;
        }

        size_t size() const ST_NOEXCEPT { return m_size; }

        string to_string(bool utf8_encoded = true,
                         utf_validation_t validation = assert_validity) const
        {
            if (utf8_encoded)
                return string::from_utf8(raw_buffer(), size(), validation);
            else
                return string::from_latin_1(raw_buffer(), size());
        }

        void truncate() { m_size = 0; }

    private:
        union
        {
            char *m_buffer;
            char  m_stack[ST_STACK_STRING_LEN];
        };
        size_t m_alloc, m_size;

        bool is_heap() const ST_NOEXCEPT
        {
            return m_alloc > ST_STACK_STRING_LEN;
        }
    };
}

#endif
