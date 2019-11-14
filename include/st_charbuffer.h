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

#include "st_assert.h"

#include <cstddef>
#include <iterator>
#include <string>       // Needed for char_traits
#include <utility>      // For std::move
#include <algorithm>    // For std::min

#if defined(ST_ENABLE_STL_STRINGS)
#   if defined(ST_HAVE_CXX17_STRING_VIEW)
#       include <string_view>
#   endif
#   if defined(ST_HAVE_EXPERIMENTAL_STRING_VIEW)
#       include <experimental/string_view>
#   endif
#endif

#define ST_AUTO_SIZE    (static_cast<size_t>(-1))

namespace ST
{
    // For optimized construction of empty objects
    struct null_t
    {
        constexpr null_t() noexcept { }
    };
    static constexpr null_t null;

    template <typename char_T>
    class buffer
    {
    public:
        // STL-compatible typedefs
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
        typedef char_T value_type;
        typedef value_type *pointer;
        typedef const value_type *const_pointer;
        typedef value_type &reference;
        typedef const value_type &const_reference;

        // This should satisfy ContiguousIterator if std::array is any indication
        typedef value_type *iterator;
        typedef const value_type *const_iterator;
        typedef std::reverse_iterator<iterator> reverse_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    private:
        char_T *m_chars;
        size_t m_size;
        char_T m_data[ST_SHORT_STRING_LEN];

        typedef std::char_traits<char_T> traits_t;

        inline bool is_reffed() const noexcept
        {
            return m_size >= ST_SHORT_STRING_LEN;
        }

    public:
        constexpr buffer() noexcept
            : m_chars(m_data), m_size(), m_data() { }

        constexpr buffer(const null_t &) noexcept
            : m_chars(m_data), m_size(), m_data() { }

        buffer(const buffer<char_T> &copy)
            : m_size(copy.m_size)
        {
            if (is_reffed()) {
                m_chars = new char_T[m_size + 1];
                traits_t::copy(m_chars, copy.m_chars, m_size);
                m_chars[m_size] = 0;
            } else {
                traits_t::copy(m_data, copy.m_data, ST_SHORT_STRING_LEN);
                m_chars = m_data;
            }
        }

        buffer(buffer<char_T> &&move) noexcept
            : m_size(move.m_size)
        {
            m_chars = is_reffed() ? move.m_chars : m_data;
            traits_t::copy(m_data, move.m_data, ST_SHORT_STRING_LEN);
            move.m_size = 0;
        }

        buffer(const char_T *data, size_t size)
            : m_size(size), m_data()
        {
            m_chars = is_reffed() ? new char_T[m_size + 1] : m_data;
            traits_t::move(m_chars, data, m_size);
            m_chars[m_size] = 0;
        }

        buffer(size_t count, char_T fill)
            : m_size(count), m_data()
        {
            m_chars = is_reffed() ? new char_T[m_size + 1] : m_data;
            traits_t::assign(m_chars, m_size, fill);
            m_chars[m_size] = 0;
        }

        ~buffer<char_T>() noexcept
        {
            if (is_reffed())
                delete[] m_chars;
        }

        buffer<char_T> &operator=(const null_t &) noexcept
        {
            if (is_reffed())
                delete[] m_chars;

            m_chars = m_data;
            m_size = 0;
            traits_t::assign(m_data, ST_SHORT_STRING_LEN, 0);
            return *this;
        }

        buffer<char_T> &operator=(const buffer<char_T> &copy)
        {
            if (this == &copy)
                return *this;

            if (is_reffed())
                delete[] m_chars;

            m_size = copy.m_size;
            if (is_reffed()) {
                m_chars = new char_T[m_size + 1];
                traits_t::copy(m_chars, copy.m_chars, m_size);
                m_chars[m_size] = 0;
            } else {
                traits_t::copy(m_data, copy.m_data, ST_SHORT_STRING_LEN);
                m_chars = m_data;
            }
            return *this;
        }

        buffer<char_T> &operator=(buffer<char_T> &&move) noexcept
        {
            std::swap(m_chars, move.m_chars);
            std::swap(m_size, move.m_size);
            traits_t::copy(m_data, move.m_data, ST_SHORT_STRING_LEN);
            if (!is_reffed())
                m_chars = m_data;
            return *this;
        }

        static int compare(const char_T *left, size_t lsize,
                           const char_T *right, size_t rsize) noexcept
        {
            const size_t cmplen = std::min<size_t>(lsize, rsize);
            const int cmp = traits_t::compare(left, right, cmplen);
            return cmp ? cmp : static_cast<int>(lsize - rsize);
        }

        static int compare(const char_T *left, size_t lsize,
                           const char_T *right, size_t rsize, size_t maxlen) noexcept
        {
            lsize = std::min<size_t>(lsize, maxlen);
            rsize = std::min<size_t>(rsize, maxlen);
            return compare(left, lsize, right, rsize);
        }

        int compare(const buffer<char_T> &other) const noexcept
        {
            return compare(data(), size(), other.data(), other.size());
        }

        int compare(const char_T *str) const noexcept
        {
            const size_t rsize = str ? traits_t::length(str) : 0;
            return compare(data(), size(), str ? str : "", rsize);
        }

        int compare_n(const buffer<char_T> &other, size_t count) const noexcept
        {
            return compare(data(), size(), other.data(), other.size(), count);
        }

        int compare_n(const char_T *str, size_t count) const noexcept
        {
            const size_t rsize = str ? std::char_traits<char>::length(str) : 0;
            return compare(data(), size(), str ? str : "", rsize, count);
        }

        bool operator==(const null_t &) const noexcept
        {
            return empty();
        }

        bool operator==(const buffer<char_T> &other) const noexcept
        {
            return compare(other) == 0;
        }

        bool operator!=(const null_t &) const noexcept
        {
            return !empty();
        }

        bool operator!=(const buffer<char_T> &other) const noexcept
        {
            return compare(other) != 0;
        }

        bool operator<(const buffer<char_T> &other) const noexcept
        {
            return compare(other) < 0;
        }

        char_T *data() noexcept { return m_chars; }
        const char_T *data() const noexcept { return m_chars; }

        const char_T *c_str() const noexcept { return m_chars; }
        const char_T *c_str(const char_T *substitute) const noexcept
        {
            return empty() ? substitute : m_chars;
        }

        size_t size() const noexcept { return m_size; }
        bool empty() const noexcept { return m_size == 0; }

        char_T &at(size_t index)
        {
            if (index >= size())
                throw std::out_of_range("Character index out of range");
            return m_chars[index];
        }

        const char_T &at(size_t index) const
        {
            if (index >= size())
                throw std::out_of_range("Character index out of range");
            return m_chars[index];
        }

        char_T &operator[](size_t index) noexcept
        {
            return m_chars[index];
        }

        const char_T &operator[](size_t index) const noexcept
        {
            return m_chars[index];
        }

        char_T &front() noexcept
        {
            return m_chars[0];
        }

        const char_T &front() const noexcept
        {
            return m_chars[0];
        }

        char_T &back() noexcept
        {
            return empty() ? m_chars[0] : m_chars[m_size - 1];
        }

        const char_T &back() const noexcept
        {
            return empty() ? m_chars[0] : m_chars[m_size - 1];
        }

        iterator begin() noexcept { return m_chars; }
        const_iterator begin() const noexcept { return m_chars; }
        const_iterator cbegin() const noexcept { return m_chars; }

        iterator end() noexcept { return m_chars + m_size; }
        const_iterator end() const noexcept { return m_chars + m_size; }
        const_iterator cend() const noexcept { return m_chars + m_size; }

        reverse_iterator rbegin() noexcept
        {
            return reverse_iterator(end());
        }

        const_reverse_iterator rbegin() const noexcept
        {
            return const_reverse_iterator(end());
        }

        const_reverse_iterator crbegin() const noexcept
        {
            return const_reverse_iterator(cend());
        }

        reverse_iterator rend() noexcept
        {
            return reverse_iterator(begin());
        }

        const_reverse_iterator rend() const noexcept
        {
            return const_reverse_iterator(begin());
        }

        const_reverse_iterator crend() const noexcept
        {
            return const_reverse_iterator(cbegin());
        }

        void allocate(size_t size)
        {
            if (is_reffed())
                delete[] m_chars;
            else
                traits_t::assign(m_data, ST_SHORT_STRING_LEN, 0);

            m_size = size;
            m_chars = is_reffed() ? new char_T[m_size + 1] : m_data;
            m_chars[m_size] = 0;
        }

        void allocate(size_t size, char_T fill)
        {
            allocate(size);
            traits_t::assign(m_chars, size, fill);
        }

        static inline size_t strlen(const char_T *buffer)
        {
            ST_ASSERT(buffer, "buffer<char_T>::strlen passed null buffer");
            return traits_t::length(buffer);
        }

#if defined(ST_ENABLE_STL_STRINGS)
        std::basic_string<char_T> to_std_string() const
        {
            return std::basic_string<char_T>(data(), size());
        }

#if defined(ST_HAVE_CXX17_STRING_VIEW)
        std::basic_string_view<char_T> view(size_t start = 0,
                                            size_t length = ST_AUTO_SIZE) const
        {
            if (length == ST_AUTO_SIZE)
                length = size() - start;
            return std::basic_string_view<char_T>(data() + start, length);
        }
#elif defined(ST_HAVE_EXPERIMENTAL_STRING_VIEW)
        std::experimental::basic_string_view<char_T> view(size_t start = 0,
                                                          size_t length = ST_AUTO_SIZE) const
        {
            if (length == ST_AUTO_SIZE)
                length = size() - start;
            return std::experimental::basic_string_view<char_T>(data() + start, length);
        }
#endif

#endif  /* defined(ST_ENABLE_STL_STRINGS) */
    };

    typedef buffer<char>        char_buffer;
    typedef buffer<wchar_t>     wchar_buffer;
    typedef buffer<char16_t>    utf16_buffer;
    typedef buffer<char32_t>    utf32_buffer;

    template <typename char_T>
    bool operator==(const null_t &, const buffer<char_T> &right) noexcept
    {
        return right.empty();
    }

    template <typename char_T>
    bool operator!=(const null_t &, const buffer<char_T> &right) noexcept
    {
        return !right.empty();
    }
}

#define ST_CHAR_LITERAL(str) \
    ST::char_buffer("" str "", sizeof(str) - 1)

#define ST_WCHAR_LITERAL(str) \
    ST::wchar_buffer(L"" str L"", (sizeof(str) / sizeof(wchar_t)) - 1)

#define ST_UTF16_LITERAL(str) \
    ST::utf16_buffer(u"" str u"", (sizeof(str) / sizeof(char16_t)) - 1)

#define ST_UTF32_LITERAL(str) \
    ST::utf32_buffer(U"" str U"", (sizeof(str) / sizeof(char32_t)) - 1)

namespace ST { namespace literals
{
    inline ST::char_buffer operator"" _stbuf(const char *str, size_t size)
    {
        return ST::char_buffer(str, size);
    }

    inline ST::utf16_buffer operator"" _stbuf(const char16_t *str, size_t size)
    {
        return ST::utf16_buffer(str, size);
    }

    inline ST::utf32_buffer operator"" _stbuf(const char32_t *str, size_t size)
    {
        return ST::utf32_buffer(str, size);
    }

    inline ST::wchar_buffer operator"" _stbuf(const wchar_t *str, size_t size)
    {
        return ST::wchar_buffer(str, size);
    }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
    inline ST::char_buffer operator"" _stbuf(const char8_t *str, size_t size)
    {
        return ST::char_buffer(reinterpret_cast<const char *>(str), size);
    }
#endif
}}

#endif // _ST_CHARBUFFER_H
