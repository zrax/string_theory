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

#ifndef _ST_STRING_H
#define _ST_STRING_H

#include <vector>
#include "st_charbuffer.h"

#ifdef ST_HAVE_INT64
#   include <cstdint>
#endif

/* This can be set globally for your project in order to change the default
 * behavior for verification of unicode data during string conversions. */
#ifndef ST_DEFAULT_VALIDATION
#   define ST_DEFAULT_VALIDATION ST::check_validity
#endif

#define ST_AUTO_SIZE    (static_cast<size_t>(-1))
#define ST_WHITESPACE   " \t\r\n"

namespace ST
{
    int _lower_char(int ch);
    int _upper_char(int ch);

    ST_STRONG_ENUM(case_sensitivity_t)
    {
        case_sensitive,
        case_insensitive
    };
    ST_ENUM_CONSTANT(case_sensitivity_t, case_sensitive);
    ST_ENUM_CONSTANT(case_sensitivity_t, case_insensitive);

    ST_STRONG_ENUM(utf_validation_t)
    {
        assume_valid,       //! Don't do any checking
        substitute_invalid, //! Replace invalid sequences with a substitute
        check_validity,     //! Throw a ST::unicode_error for invalid sequences
        assert_validity     //! call ST_ASSERT for invalid sequences
    };
    ST_ENUM_CONSTANT(utf_validation_t, assume_valid);
    ST_ENUM_CONSTANT(utf_validation_t, substitute_invalid);
    ST_ENUM_CONSTANT(utf_validation_t, check_validity);
    ST_ENUM_CONSTANT(utf_validation_t, assert_validity);

    class ST_EXPORT string
    {
    public:
        static const string null;

    private:
        char_buffer m_buffer;

        void _convert_from_utf8(const char *utf8, size_t size,
                                utf_validation_t validation);
        void _convert_from_utf16(const char16_t *utf16, size_t size,
                                 utf_validation_t validation);
        void _convert_from_utf32(const char32_t *utf32, size_t size,
                                 utf_validation_t validation);
        void _convert_from_wchar(const wchar_t *wstr, size_t size,
                                 utf_validation_t validation);
        void _convert_from_latin_1(const char *astr, size_t size);

    public:
        string() ST_NOEXCEPT { }

        string(const char *cstr, size_t size = ST_AUTO_SIZE,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = ST::char_buffer::strlen(cstr);
            _convert_from_utf8(cstr, size, validation);
        }

        string(const wchar_t *wstr, size_t size = ST_AUTO_SIZE,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = ST::wchar_buffer::strlen(wstr);
            _convert_from_wchar(wstr, size, validation);
        }

        string(const string &copy) ST_NOEXCEPT
            : m_buffer(copy.m_buffer) { }

#ifdef ST_HAVE_RVALUE_MOVE
        string(string &&move) ST_NOEXCEPT
            : m_buffer(std::move(move.m_buffer)) { }
#endif

        string(const char_buffer &init,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            set(init, validation);
        }

#ifdef ST_HAVE_RVALUE_MOVE
        string(char_buffer &&init,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            set(std::move(init), validation);
        }
#endif

        string(const utf16_buffer &init,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_utf16(init.data(), init.size(), validation);
        }

        string(const utf32_buffer &init,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_utf32(init.data(), init.size(), validation);
        }

        string(const wchar_buffer &init,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_wchar(init.data(), init.size(), validation);
        }

        void set(const char *cstr, size_t size = ST_AUTO_SIZE,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = ST::char_buffer::strlen(cstr);
            _convert_from_utf8(cstr, size, validation);
        }

        void set(const wchar_t *wstr, size_t size = ST_AUTO_SIZE,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = ST::wchar_buffer::strlen(wstr);
            _convert_from_wchar(wstr, size, validation);
        }

        void set(const string &copy) ST_NOEXCEPT
        {
            m_buffer = copy.m_buffer;
        }

#ifdef ST_HAVE_RVALUE_MOVE
        void set(string &&move) ST_NOEXCEPT
        {
            m_buffer = std::move(move.m_buffer);
        }
#endif

        void set(const char_buffer &init,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION);

#ifdef ST_HAVE_RVALUE_MOVE
        void set(char_buffer &&init,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION);
#endif

        void set(const utf16_buffer &init,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_utf16(init.data(), init.size(), validation);
        }

        void set(const utf32_buffer &init,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_utf32(init.data(), init.size(), validation);
        }

        void set(const wchar_buffer &init,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_wchar(init.data(), init.size(), validation);
        }

        string &operator=(const char *cstr)
        {
            set(cstr);
            return *this;
        }

        string &operator=(const wchar_t *wstr)
        {
            set(wstr);
            return *this;
        }

        string &operator=(const string &copy) ST_NOEXCEPT
        {
            m_buffer = copy.m_buffer;
            return *this;
        }

#ifdef ST_HAVE_RVALUE_MOVE
        string &operator=(string &&move) ST_NOEXCEPT
        {
            m_buffer = std::move(move.m_buffer);
            return *this;
        }
#endif

        string &operator=(const char_buffer &init)
        {
            set(init);
            return *this;
        }

#ifdef ST_HAVE_RVALUE_MOVE
        string &operator=(char_buffer &&init)
        {
            set(std::move(init));
            return *this;
        }
#endif

        string &operator=(const utf16_buffer &init)
        {
            set(init);
            return *this;
        }

        string &operator=(const utf32_buffer &init)
        {
            set(init);
            return *this;
        }

        string &operator=(const wchar_buffer &init)
        {
            set(init);
            return *this;
        }

        string &operator+=(const char *cstr);
        string &operator+=(const wchar_t *wstr);

        string &operator+=(const string &other);

        static inline string from_literal(const char *literal, size_t size)
        {
            string str;
            str.set(ST::char_buffer(literal, size), ST::assume_valid);
            return str;
        }

        static inline string from_utf8(const char *utf8,
                                       size_t size = ST_AUTO_SIZE,
                                       utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = ST::char_buffer::strlen(utf8);

            string str;
            str._convert_from_utf8(utf8, size, validation);
            return str;
        }

        static inline string from_utf16(const char16_t *utf16,
                                        size_t size = ST_AUTO_SIZE,
                                        utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = ST::utf16_buffer::strlen(utf16);

            string str;
            str._convert_from_utf16(utf16, size, validation);
            return str;
        }

        static inline string from_utf32(const char32_t *utf32,
                                        size_t size = ST_AUTO_SIZE,
                                        utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = ST::utf32_buffer::strlen(utf32);

            string str;
            str._convert_from_utf32(utf32, size, validation);
            return str;
        }

        static inline string from_wchar(const wchar_t *wstr,
                                        size_t size = ST_AUTO_SIZE,
                                        utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = ST::wchar_buffer::strlen(wstr);

            string str;
            str._convert_from_wchar(wstr, size, validation);
            return str;
        }

        static inline string from_latin_1(const char *astr,
                                          size_t size = ST_AUTO_SIZE)
        {
            if (size == ST_AUTO_SIZE)
                size = ST::char_buffer::strlen(astr);

            string str;
            str._convert_from_latin_1(astr, size);
            return str;
        }

        const char *c_str(const char *substitute = "") const ST_NOEXCEPT
        {
            return is_empty() ? substitute : m_buffer.data();
        }

        char char_at(size_t position) const ST_NOEXCEPT
        {
            return c_str()[position];
        }

        char_buffer to_utf8() const ST_NOEXCEPT { return m_buffer; }

        utf16_buffer to_utf16() const;
        utf32_buffer to_utf32() const;
        wchar_buffer to_wchar() const;
        char_buffer to_latin_1(utf_validation_t validation = substitute_invalid) const;

        size_t size() const ST_NOEXCEPT { return m_buffer.size(); }

        bool is_empty() const ST_NOEXCEPT { return m_buffer.size() == 0; }

        int to_int(int base = 0) const ST_NOEXCEPT;
        unsigned int to_uint(int base = 0) const ST_NOEXCEPT;
        float to_float() const ST_NOEXCEPT;
        double to_double() const ST_NOEXCEPT;

#ifdef ST_HAVE_INT64
        int64_t to_int64(int base = 0) const ST_NOEXCEPT;
        uint64_t to_uint64(int base = 0) const ST_NOEXCEPT;
#endif

        bool to_bool() const ST_NOEXCEPT;

        int compare(const string &str, case_sensitivity_t cs = case_sensitive)
            const ST_NOEXCEPT;
        int compare(const char *str, case_sensitivity_t cs = case_sensitive)
            const ST_NOEXCEPT;

        int compare_n(const string &str, size_t count,
                      case_sensitivity_t cs = case_sensitive) const ST_NOEXCEPT;
        int compare_n(const char *str, size_t count,
                      case_sensitivity_t cs = case_sensitive) const ST_NOEXCEPT;

        int compare_i(const string &str) const ST_NOEXCEPT
        {
            return compare(str, case_insensitive);
        }

        int compare_i(const char *str) const ST_NOEXCEPT
        {
            return compare(str, case_insensitive);
        }

        int compare_ni(const string &str, size_t count) const ST_NOEXCEPT
        {
            return compare_n(str, count, case_insensitive);
        }

        int compare_ni(const char *str, size_t count) const ST_NOEXCEPT
        {
            return compare_n(str, count, case_insensitive);
        }

        bool operator<(const string &other) const ST_NOEXCEPT
        {
            return compare(other) < 0;
        }

        bool operator==(const string &other) const ST_NOEXCEPT
        {
            return compare(other) == 0;
        }

        bool operator==(const char *other) const ST_NOEXCEPT
        {
            return compare(other) == 0;
        }

        bool operator!=(const string &other) const ST_NOEXCEPT
        {
            return compare(other) != 0;
        }

        bool operator!=(const char *other) const ST_NOEXCEPT
        {
            return compare(other) != 0;
        }

        ST_ssize_t find(char ch, case_sensitivity_t cs = case_sensitive)
            const ST_NOEXCEPT;

        ST_ssize_t find(const char *substr, case_sensitivity_t cs = case_sensitive)
            const ST_NOEXCEPT;

        ST_ssize_t find(const string &substr, case_sensitivity_t cs = case_sensitive)
            const ST_NOEXCEPT
        {
            return find(substr.c_str(), cs);
        }

        ST_ssize_t find_last(char ch, case_sensitivity_t cs = case_sensitive)
            const ST_NOEXCEPT;

        ST_ssize_t find_last(const char *substr, case_sensitivity_t cs = case_sensitive)
            const ST_NOEXCEPT;

        ST_ssize_t find_last(const string &substr, case_sensitivity_t cs = case_sensitive)
            const ST_NOEXCEPT
        {
            return find_last(substr.c_str(), cs);
        }

        string trim_left(const char *charset = ST_WHITESPACE) const;
        string trim_right(const char *charset = ST_WHITESPACE) const;
        string trim(const char *charset = ST_WHITESPACE) const;

        string substr(ST_ssize_t start, size_t size = ST_AUTO_SIZE) const;

        string left(size_t size) const
        {
            return substr(0, size);
        }

        string right(size_t size) const
        {
            return substr(this->size() - size, size);
        }

        bool starts_with(const ST::string &prefix, case_sensitivity_t cs = case_sensitive) const;
        bool ends_with(const ST::string &suffix, case_sensitivity_t cs = case_sensitive) const;

        bool starts_with(const char *prefix, case_sensitivity_t cs = case_sensitive) const;
        bool ends_with(const char *suffix, case_sensitivity_t cs = case_sensitive) const;

        string before_first(const char *sep) const;
        string before_first(char sep) const;
        string after_first(const char *sep) const;
        string after_first(char sep) const;
        string before_last(const char *sep) const;
        string before_last(char sep) const;
        string after_last(const char *sep) const;
        string after_last(char sep) const;

        string replace(const char *from, const char *to,
                       utf_validation_t validation = ST_DEFAULT_VALIDATION) const;

        string to_upper() const;
        string to_lower() const;

        std::vector<string> split(const char *splitter,
                                  size_t max_splits = ST_AUTO_SIZE) const;
        std::vector<string> split(char split_char,
                                  size_t max_splits = ST_AUTO_SIZE) const;

        std::vector<string> tokenize(const char *delims = ST_WHITESPACE) const;

        static string fill(size_t count, char c);
    };

    struct ST_EXPORT hash
    {
        size_t operator()(const string &str) const ST_NOEXCEPT;

    protected:
        virtual unsigned char fetch_char(const string &str, size_t index)
            const ST_NOEXCEPT
        {
            return static_cast<unsigned char>(str.char_at(index));
        }
    };

    struct ST_EXPORT hash_i : public hash
    {
    protected:
        unsigned char fetch_char(const string &str, size_t index)
            const ST_NOEXCEPT ST_OVERRIDE
        {
            return static_cast<unsigned char>(_lower_char(str.char_at(index)));
        }
    };

    struct ST_EXPORT less_i
    {
        bool operator()(const string &left, const string &right)
            const ST_NOEXCEPT
        {
            return left.compare_i(right) < 0;
        }
    };

    struct ST_EXPORT equal_i
    {
        bool operator()(const string &left, const string &right)
        const ST_NOEXCEPT
        {
            return left.compare_i(right) == 0;
        }
    };
}

ST_EXPORT ST::string operator+(const ST::string &left, const ST::string &right);
ST_EXPORT ST::string operator+(const ST::string &left, const char *right);
ST_EXPORT ST::string operator+(const char *left, const ST::string &right);

ST_EXPORT inline ST::string operator+(const ST::string &left, const wchar_t *right)
{
    return operator+(left, ST::string(right));
}

ST_EXPORT inline ST::string operator+(const wchar_t *left, const ST::string &right)
{
    return operator+(ST::string(left), right);
}

#define ST_LITERAL(str) \
    ST::string::from_literal("" str "", sizeof(str) - 1)

#ifdef ST_HAVE_USER_LITERALS
inline ST::string operator"" _st(const char *str, size_t size)
{
    return ST::string::from_literal(str, size);
}

inline ST::string operator"" _st(const wchar_t *str, size_t size)
{
    return ST::string::from_wchar(str, size, ST::assume_valid);
}

inline ST::string operator"" _st(const char16_t *str, size_t size)
{
    return ST::string::from_utf16(str, size, ST::assume_valid);
}

inline ST::string operator"" _st(const char32_t *str, size_t size)
{
    return ST::string::from_utf32(str, size, ST::assume_valid);
}
#endif

#endif // _ST_STRING_H
