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

#ifndef _ST_CHARBUFFER_H
#define _ST_CHARBUFFER_H

#include "st_config.h"

#include <cstddef>
#ifdef ST_HAVE_RVALUE_MOVE
#  include <utility>    // For std::move
#endif

namespace _ST_PRIVATE
{
    // Avoid including <cstring> in this header...
    ST_EXPORT void _zero_buffer(void *buffer, size_t size);
    ST_EXPORT void _fill_buffer(void *buffer, int ch, size_t count);
    ST_EXPORT void _copy_buffer(void *dest, const void *src, size_t size);
    ST_EXPORT int _compare_buffer(const void *left, const void *right, size_t size);
}

namespace ST
{
    // For optimized construction of empty objects
    struct ST_EXPORT null_t
    {
        null_t() { }
    };
    static const null_t null;

    template <typename char_T>
    class ST_EXPORT buffer
    {
    private:
        union
        {
            char_T *m_ref;
            char_T m_data[ST_SHORT_STRING_LEN];
        };
        size_t m_size;

        inline bool is_reffed() const ST_NOEXCEPT
        {
            return m_size >= ST_SHORT_STRING_LEN;
        }

        struct _scope_deleter
        {
            // Useful for deleting the buffer at the *end* of a function,
            // even though we must capture it at the beginning, in case
            // the user does something silly like assign a buffer to itself.
            char_T *m_buffer;

            _scope_deleter(buffer<char_T> *self)
            {
                m_buffer = self->is_reffed() ? self->m_ref : ST_NULLPTR;
            }

            ~_scope_deleter()
            {
                delete[] m_buffer;
            }
        };

    public:
        buffer() ST_NOEXCEPT
            : m_size()
        {
            _ST_PRIVATE::_zero_buffer(m_data, sizeof(m_data));
        }

        buffer(const null_t &) ST_NOEXCEPT
            : m_size()
        {
            _ST_PRIVATE::_zero_buffer(m_data, sizeof(m_data));
        }

        buffer(const buffer<char_T> &copy)
            : m_size(copy.m_size)
        {
            _ST_PRIVATE::_zero_buffer(m_data, sizeof(m_data));
            if (is_reffed()) {
                m_ref = new char_T[m_size + 1];
                _ST_PRIVATE::_copy_buffer(m_ref, copy.m_ref, m_size * sizeof(char_T));
                m_ref[m_size] = 0;
            } else {
                _ST_PRIVATE::_copy_buffer(m_data, copy.m_data, sizeof(m_data));
            }
        }

#ifdef ST_HAVE_RVALUE_MOVE
        buffer(buffer<char_T> &&move) ST_NOEXCEPT
            : m_size(std::move(move.m_size))
        {
            _ST_PRIVATE::_copy_buffer(m_data, move.m_data, sizeof(m_data));
            move.m_size = 0;
        }
#endif

        buffer(const char_T *data, size_t size)
            : m_size(size)
        {
            _ST_PRIVATE::_zero_buffer(m_data, sizeof(m_data));
            char_T *buffer = is_reffed() ? (m_ref = new char_T[m_size + 1]) : m_data;
            _ST_PRIVATE::_copy_buffer(buffer, data, m_size * sizeof(char_T));
            buffer[m_size] = 0;
        }

        ~buffer<char_T>() ST_NOEXCEPT
        {
            if (is_reffed())
                delete[] m_ref;
        }

        buffer<char_T> &operator=(const null_t &) ST_NOEXCEPT
        {
            _scope_deleter unref(this);
            m_size = 0;
            _ST_PRIVATE::_zero_buffer(m_data, sizeof(m_data));
            return *this;
        }

        buffer<char_T> &operator=(const buffer<char_T> &copy)
        {
            _scope_deleter unref(this);
            m_size = copy.m_size;
            if (is_reffed()) {
                m_ref = new char_T[m_size + 1];
                _ST_PRIVATE::_copy_buffer(m_ref, copy.m_ref, m_size * sizeof(char_T));
                m_ref[m_size] = 0;
            } else {
                _ST_PRIVATE::_copy_buffer(m_data, copy.m_data, sizeof(m_data));
            }
            return *this;
        }

#ifdef ST_HAVE_RVALUE_MOVE
        buffer<char_T> &operator=(buffer<char_T> &&move) ST_NOEXCEPT
        {
            _scope_deleter unref(this);
            m_size = std::move(move.m_size);
            _ST_PRIVATE::_copy_buffer(m_data, move.m_data, sizeof(m_data));
            move.m_size = 0;
            return *this;
        }
#endif

        bool operator==(const null_t &) const ST_NOEXCEPT
        {
            return is_empty();
        }

        bool operator==(const buffer<char_T> &other) const ST_NOEXCEPT
        {
            if (other.size() != size())
                return false;
            return _ST_PRIVATE::_compare_buffer(data(), other.data(), size() * sizeof(char_T)) == 0;
        }

        bool operator!=(const null_t &) const ST_NOEXCEPT
        {
            return !is_empty();
        }

        bool operator!=(const buffer<char_T> &other) const ST_NOEXCEPT
        {
            return !operator==(other);
        }

        const char_T *data() const ST_NOEXCEPT
        {
            return is_reffed() ? m_ref : m_data;
        }

        size_t size() const ST_NOEXCEPT { return m_size; }
        bool is_empty() const ST_NOEXCEPT { return m_size == 0; }

        operator const char_T *() const ST_NOEXCEPT { return data(); }

        char_T *create_writable_buffer(size_t size)
        {
            if (is_reffed())
                delete[] m_ref;

            m_size = size;
            if (is_reffed())
                return m_ref = new char_T[m_size + 1];
            else
                return m_data;
        }

        static inline size_t strlen(const char_T *buffer)
        {
            if (!buffer)
                return 0;

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

    template <typename char_T>
    bool operator==(const null_t &, const buffer<char_T> &right) ST_NOEXCEPT
    {
        return right.is_empty();
    }

    template <typename char_T>
    bool operator!=(const null_t &, const buffer<char_T> &right) ST_NOEXCEPT
    {
        return !right.is_empty();
    }
}

#endif // _ST_CHARBUFFER_H
