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
#include "st_format_numeric.h"

namespace ST
{
    class string_stream
    {
        ST_DISABLE_COPY(string_stream)

    public:
        string_stream() noexcept
            : m_chars(m_stack), m_alloc(ST_STACK_STRING_SIZE), m_size() { }

        ~string_stream() noexcept
        {
            if (is_heap())
                delete[] m_chars;
        }

        string_stream(string_stream &&move) noexcept
            : m_alloc(move.m_alloc), m_size(move.m_size)
        {
            m_chars = is_heap() ? move.m_chars : m_stack;
            std::char_traits<char>::copy(m_stack, move.m_stack, ST_STACK_STRING_SIZE);
            move.m_alloc = 0;
        }

        string_stream &operator=(string_stream &&move) noexcept
        {
            if (is_heap())
                delete[] m_chars;

            m_alloc = move.m_alloc;
            m_size = move.m_size;
            m_chars = is_heap() ? move.m_chars : m_stack;
            std::char_traits<char>::copy(m_stack, move.m_stack, ST_STACK_STRING_SIZE);
            move.m_alloc = 0;
            return *this;
        }

        string_stream &append(const char *data, size_t size = ST_AUTO_SIZE)
        {
            if (size == 0)
                return *this;

            if (size == ST_AUTO_SIZE)
                size = data ? std::char_traits<char>::length(data) : 0;

            expand_buffer(size);

            std::char_traits<char>::move(m_chars + m_size, data, size);
            m_size += size;
            return *this;
        }

        string_stream &append_char(char ch, size_t count = 1)
        {
            if (count == 0)
                return *this;

            expand_buffer(count);

            std::char_traits<char>::assign(m_chars + m_size, count, ch);
            m_size += count;
            return *this;
        }

        string_stream &operator<<(const char *text)
        {
            return append(text);
        }

        string_stream &operator<<(const wchar_t *text)
        {
            if (text) {
                const auto size = std::char_traits<wchar_t>::length(text);
                ST::char_buffer utf8 = ST::wchar_to_utf8(text, size);
                return append(utf8.data(), utf8.size());
            }
            return *this;
        }

        string_stream &operator<<(const char16_t *text)
        {
            if (text) {
                const auto size = std::char_traits<char16_t>::length(text);
                ST::char_buffer utf8 = ST::utf16_to_utf8(text, size);
                return append(utf8.data(), utf8.size());
            }
            return *this;
        }

        string_stream &operator<<(const char32_t *text)
        {
            if (text) {
                const auto size = std::char_traits<char32_t>::length(text);
                ST::char_buffer utf8 = ST::utf32_to_utf8(text, size);
                return append(utf8.data(), utf8.size());
            }
            return *this;
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        string_stream &operator<<(const char8_t *text)
        {
            return append(reinterpret_cast<const char *>(text));
        }
#endif

        string_stream &operator<<(int num)
        {
            ST::uint_formatter<unsigned int> formatter;
            formatter.format(std::abs(num), 10, false);
            if (num < 0)
                append_char('-');
            return append(formatter.text(), formatter.size());
        }

        string_stream &operator<<(unsigned int num)
        {
            ST::uint_formatter<unsigned int> formatter;
            formatter.format(num, 10, false);
            return append(formatter.text(), formatter.size());
        }

        string_stream &operator<<(long num)
        {
            ST::uint_formatter<unsigned long> formatter;
            formatter.format(std::abs(num), 10, false);
            if (num < 0)
                append_char('-');
            return append(formatter.text(), formatter.size());
        }

        string_stream &operator<<(unsigned long num)
        {
            ST::uint_formatter<unsigned long> formatter;
            formatter.format(num, 10, false);
            return append(formatter.text(), formatter.size());
        }

        string_stream &operator<<(long long num)
        {
            ST::uint_formatter<unsigned long long> formatter;
            formatter.format(std::abs(num), 10, false);
            if (num < 0)
                append_char('-');
            return append(formatter.text(), formatter.size());
        }

        string_stream &operator<<(unsigned long long num)
        {
            ST::uint_formatter<unsigned long long> formatter;
            formatter.format(num, 10, false);
            return append(formatter.text(), formatter.size());
        }

        string_stream &operator<<(float num)
        {
            ST::float_formatter<float> formatter;
            formatter.format(num, 'g');
            return append(formatter.text(), formatter.size());
        }

        string_stream &operator<<(double num)
        {
            ST::float_formatter<double> formatter;
            formatter.format(num, 'g');
            return append(formatter.text(), formatter.size());
        }

        string_stream &operator<<(char ch)
        {
            return append_char(ch);
        }

        string_stream &operator<<(const string &text)
        {
            return append(text.c_str(), text.size());
        }

#if defined(ST_ENABLE_STL_STRINGS)

        string_stream &operator<<(const std::string &text)
        {
            return append(text.c_str(), text.size());
        }

        string_stream &operator<<(const std::wstring &text)
        {
            ST::char_buffer utf8 = ST::wchar_to_utf8(text.c_str(), text.size());
            return append(utf8.data(), utf8.size());
        }

        string_stream &operator<<(const std::u16string &text)
        {
            ST::char_buffer utf8 = ST::utf16_to_utf8(text.c_str(), text.size());
            return append(utf8.data(), utf8.size());
        }

        string_stream &operator<<(const std::u32string &text)
        {
            ST::char_buffer utf8 = ST::utf32_to_utf8(text.c_str(), text.size());
            return append(utf8.data(), utf8.size());
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        string_stream &operator<<(const std::u8string &text)
        {
            return append(reinterpret_cast<const char*>(text.c_str()), text.size());
        }
#endif

#ifdef ST_HAVE_CXX17_STRING_VIEW
        string_stream &operator<<(const std::string_view &text)
        {
            return append(text.data(), text.size());
        }

        string_stream &operator<<(const std::wstring_view &text)
        {
            ST::char_buffer utf8 = ST::wchar_to_utf8(text.data(), text.size());
            return append(utf8.data(), utf8.size());
        }

        string_stream &operator<<(const std::u16string_view &text)
        {
            ST::char_buffer utf8 = ST::utf16_to_utf8(text.data(), text.size());
            return append(utf8.data(), utf8.size());
        }

        string_stream &operator<<(const std::u32string_view &text)
        {
            ST::char_buffer utf8 = ST::utf32_to_utf8(text.data(), text.size());
            return append(utf8.data(), utf8.size());
        }
#endif

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        string_stream &operator<<(const std::u8string_view &text)
        {
            return append(reinterpret_cast<const char*>(text.data()), text.size());
        }
#endif

#ifdef ST_HAVE_EXPERIMENTAL_STRING_VIEW
        string_stream &operator<<(const std::experimental::string_view &text)
        {
            return append(text.data(), text.size());
        }

        string_stream &operator<<(const std::experimental::wstring_view &text)
        {
            ST::char_buffer utf8 = ST::wchar_to_utf8(text.data(), text.size());
            return append(utf8.data(), utf8.size());
        }

        string_stream &operator<<(const std::experimental::u16string_view &text)
        {
            ST::char_buffer utf8 = ST::utf16_to_utf8(text.data(), text.size());
            return append(utf8.data(), utf8.size());
        }

        string_stream &operator<<(const std::experimental::u32string_view &text)
        {
            ST::char_buffer utf8 = ST::utf32_to_utf8(text.data(), text.size());
            return append(utf8.data(), utf8.size());
        }
#endif

#endif // defined(ST_ENABLE_STL_STRINGS)

#if defined(ST_ENABLE_STL_FILESYSTEM)

#ifdef ST_HAVE_CXX17_FILESYSTEM
        string_stream &operator<<(const std::filesystem::path &path)
        {
            auto u8path = path.u8string();
            return append(reinterpret_cast<const char*>(u8path.c_str()), u8path.size());
        }
#endif

#ifdef ST_HAVE_EXPERIMENTAL_FILESYSTEM
        string_stream& operator<<(const std::experimental::filesystem::path& path)
        {
            auto u8path = path.u8string();
            return append(reinterpret_cast<const char *>(u8path.c_str()), u8path.size());
        }
#endif

#endif // defined(ST_ENABLE_STL_FILESYSTEM)

        ST_NODISCARD
        const char *raw_buffer() const noexcept { return m_chars; }

        ST_NODISCARD
        size_t size() const noexcept { return m_size; }

        ST_NODISCARD
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
        char   m_stack[ST_STACK_STRING_SIZE];

        ST_NODISCARD
        bool is_heap() const noexcept
        {
            return m_alloc > ST_STACK_STRING_SIZE;
        }

        void expand_buffer(size_t added_size)
        {
            if (m_size + added_size > m_alloc) {
                size_t big_size = m_alloc;
                do {
                    big_size *= 2;
                } while (m_size + added_size > big_size);

                char *bigger = new char[big_size];
                std::char_traits<char>::copy(bigger, m_chars, m_alloc);
                if (is_heap())
                    delete[] m_chars;
                m_chars = bigger;
                m_alloc = big_size;
            }
        }
    };
}

#endif // _ST_STRINGSTREAM_H
