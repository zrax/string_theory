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

#ifndef _ST_CHARBUFFER_H
#define _ST_CHARBUFFER_H

#include "st_config.h"

#include <cstddef>
#ifdef ST_HAVE_RVALUE_MOVE
#  include <utility>    // For std::move
#endif

namespace ST
{
    // Avoid including <cstring> in this header...
    extern void _zero_buffer(void *buffer, size_t size);
    extern void _fill_buffer(void *buffer, int ch, size_t count);
    extern void _copy_buffer(void *dest, const void *src, size_t size);

    template<typename char_T>
    class buffer
    {
    private:
        struct _sref
        {
            unsigned int m_refs;
            const char_T *m_data;

            _sref(const char_T *data) ST_NOEXCEPT
                : m_refs(1), m_data(data)
            { }

            inline void add_ref() ST_NOEXCEPT { ++m_refs; }
            inline void dec_ref() ST_NOEXCEPT
            {
                if (--m_refs == 0) {
                    delete[] m_data;
                    delete this;
                }
            }
        };

        union
        {
            _sref *m_ref;
            char_T m_data[ST_SHORT_STRING_LEN];
        };
        size_t m_size;

        inline bool is_reffed() const ST_NOEXCEPT
        {
            return m_size >= ST_SHORT_STRING_LEN;
        }

    public:
        buffer() ST_NOEXCEPT
            : m_size()
        {
            _zero_buffer(m_data, sizeof(m_data));
        }

        buffer(const buffer<char_T> &copy) ST_NOEXCEPT
            : m_size(copy.m_size)
        {
            _copy_buffer(m_data, copy.m_data, sizeof(m_data));
            if (is_reffed())
                m_ref->add_ref();
        }

#ifdef ST_HAVE_RVALUE_MOVE
        buffer(buffer<char_T> &&move) ST_NOEXCEPT
            : m_size(std::move(m_size))
        {
            _copy_buffer(m_data, move.m_data, sizeof(m_data));
            move.m_size = 0;
        }
#endif

        buffer(const char_T *data, size_t size)
            : m_size(size)
        {
            _zero_buffer(m_data, sizeof(m_data));
            char_T *copy_data = is_reffed() ? new char_T[size + 1] : m_data;
            _copy_buffer(copy_data, data, size * sizeof(char_T));
            copy_data[size] = 0;

            if (is_reffed())
                m_ref = new _sref(copy_data);
        }

        ~buffer<char_T>() ST_NOEXCEPT
        {
            if (is_reffed())
                m_ref->dec_ref();
        }

        buffer<char_T> &operator=(const buffer<char_T> &copy) ST_NOEXCEPT
        {
            if (copy.is_reffed())
                copy.m_ref->add_ref();
            if (is_reffed())
                m_ref->dec_ref();

            _copy_buffer(m_data, copy.m_data, sizeof(m_data));
            m_size = copy.m_size;
            return *this;
        }

#ifdef ST_HAVE_RVALUE_MOVE
        buffer<char_T> &operator=(buffer<char_T> &&move) ST_NOEXCEPT
        {
            if (is_reffed())
                m_ref->dec_ref();

            _copy_buffer(m_data, move.m_data, sizeof(m_data));
            m_size = std::move(move.m_size);
            move.m_size = 0;
            return *this;
        }
#endif

        const char_T *data() const ST_NOEXCEPT
        {
            return is_reffed() ? m_ref->m_data : m_data;
        }

        size_t size() const ST_NOEXCEPT { return m_size; }

        operator const char_T *() const ST_NOEXCEPT { return data(); }

        char_T *create_writable_buffer(size_t size)
        {
            if (is_reffed())
                m_ref->dec_ref();

            m_size = size;
            if (is_reffed()) {
                char_T *writable = new char_T[m_size + 1];
                m_ref = new _sref(writable);
                return writable;
            } else {
                return m_data;
            }
        }

        static size_t strlen(const char_T *buffer)
        {
            size_t length = 0;
            for ( ; *buffer++; ++length)
                ;
            return length;
        }
    };

    typedef buffer<char>        char_buffer;
    typedef buffer<wchar_t>     wchar_buffer;
    typedef buffer<char16_t>    utf16_buffer;
    typedef buffer<char32_t>    utf32_buffer;
} // namespace ST

#endif // _ST_CHARBUFFER_H
