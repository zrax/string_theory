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
#include <functional>
#include "st_charbuffer.h"

#if !defined(ST_NO_STL_STRINGS)
#   if defined(ST_HAVE_CXX17_FILESYSTEM)
#       include <filesystem>
#   endif
#   if defined(ST_HAVE_EXPERIMENTAL_FILESYSTEM)
#       include <experimental/filesystem>
#   endif
#endif

#ifdef ST_HAVE_INT64
#   include <cstdint>
#endif

/* This can be set globally for your project in order to change the default
 * behavior for verification of unicode data during string conversions. */
#ifndef ST_DEFAULT_VALIDATION
#   define ST_DEFAULT_VALIDATION ST::check_validity
#endif

#define ST_WHITESPACE   " \t\r\n"

namespace ST
{
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

    class ST_EXPORT conversion_result
    {
        enum
        {
            result_ok = (1 << 0),
            result_full_match = (1 << 1)
        };

    public:
        explicit conversion_result() ST_NOEXCEPT : m_flags() { }

        bool ok() const ST_NOEXCEPT { return (m_flags & result_ok) != 0; }
        bool full_match() const ST_NOEXCEPT { return (m_flags & result_full_match) != 0; }

    private:
        int m_flags;
        friend class string;
    };

    class ST_EXPORT string
    {
    public:
        // STL-compatible typedefs
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
        typedef char_buffer::value_type value_type;
        typedef const value_type *const_pointer;
        typedef const value_type &const_reference;

        // This should satisfy ContiguousIterator if std::array is any indication
        typedef char_buffer::const_iterator const_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

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

        struct from_literal_t {};
        string(const from_literal_t &, const char *data, size_t size)
            : m_buffer(data, size) { }

        struct from_validated_t {};
        string(const from_validated_t &, const char *data, size_t size)
            : m_buffer(data, size) { }
        string(const from_validated_t &, const char_buffer &buffer)
            : m_buffer(buffer) { }
        string(const from_validated_t &, char_buffer &&buffer)
            : m_buffer(std::move(buffer)) { }

        friend ST_EXPORT ST::string operator+(const ST::string &left, const ST::string &right);

    public:
        string() ST_NOEXCEPT { }
        string(const null_t &) ST_NOEXCEPT { }

        string(const char *cstr, size_t size = ST_AUTO_SIZE,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = cstr ? std::char_traits<char>::length(cstr) : 0;
            _convert_from_utf8(cstr, size, validation);
        }

        string(const wchar_t *wstr, size_t size = ST_AUTO_SIZE,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = wstr ? std::char_traits<wchar_t>::length(wstr) : 0;
            _convert_from_wchar(wstr, size, validation);
        }

#ifdef ST_HAVE_CHAR_TYPES
        string(const char16_t *cstr, size_t size = ST_AUTO_SIZE,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = cstr ? std::char_traits<char16_t>::length(cstr) : 0;
            _convert_from_utf16(cstr, size, validation);
        }

        string(const char32_t *cstr, size_t size = ST_AUTO_SIZE,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = cstr ? std::char_traits<char32_t>::length(cstr) : 0;
            _convert_from_utf32(cstr, size, validation);
        }
#endif

        string(const string &copy)
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

#if !defined(ST_NO_STL_STRINGS)
        string(const std::string &init,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_utf8(init.c_str(), init.size(), validation);
        }

        string(const std::wstring &init,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_wchar(init.c_str(), init.size(), validation);
        }

#ifdef ST_HAVE_STD_USTRING_TYPES
        string(const std::u16string &init,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_utf16(init.c_str(), init.size(), validation);
        }

        string(const std::u32string &init,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_utf32(init.c_str(), init.size(), validation);
        }
#endif

#ifdef ST_HAVE_CXX17_STRING_VIEW
        string(const std::string_view &view,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_utf8(view.data(), view.size(), validation);
        }

        string(const std::wstring_view &view,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_wchar(view.data(), view.size(), validation);
        }

        string(const std::u16string_view &view,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_utf16(view.data(), view.size(), validation);
        }

        string(const std::u32string_view &view,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_utf32(view.data(), view.size(), validation);
        }
#endif

#ifdef ST_HAVE_EXPERIMENTAL_STRING_VIEW
        string(const std::experimental::string_view &view,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_utf8(view.data(), view.size(), validation);
        }

        string(const std::experimental::wstring_view &view,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_wchar(view.data(), view.size(), validation);
        }

        string(const std::experimental::u16string_view &view,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_utf16(view.data(), view.size(), validation);
        }

        string(const std::experimental::u32string_view &view,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_utf32(view.data(), view.size(), validation);
        }
#endif

#ifdef ST_HAVE_CXX17_FILESYSTEM
        string(const std::filesystem::path &path)
        {
            set(path);
        }
#endif

#ifdef ST_HAVE_EXPERIMENTAL_FILESYSTEM
        string(const std::experimental::filesystem::path &path)
        {
            set(path);
        }
#endif

#endif // !defined(ST_NO_STL_STRINGS)

        void set(const null_t &) ST_NOEXCEPT { m_buffer = null; }

        void set(const char *cstr, size_t size = ST_AUTO_SIZE,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = cstr ? std::char_traits<char>::length(cstr) : 0;
            _convert_from_utf8(cstr, size, validation);
        }

        void set(const wchar_t *wstr, size_t size = ST_AUTO_SIZE,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = wstr ? std::char_traits<wchar_t>::length(wstr) : 0;
            _convert_from_wchar(wstr, size, validation);
        }

#ifdef ST_HAVE_CHAR_TYPES
        void set(const char16_t *cstr, size_t size = ST_AUTO_SIZE,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = cstr ? std::char_traits<char16_t>::length(cstr) : 0;
            _convert_from_utf16(cstr, size, validation);
        }

        void set(const char32_t *cstr, size_t size = ST_AUTO_SIZE,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = cstr ? std::char_traits<char32_t>::length(cstr) : 0;
            _convert_from_utf32(cstr, size, validation);
        }
#endif

        void set(const string &copy)
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

#if !defined(ST_NO_STL_STRINGS)
        void set(const std::string &init,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_utf8(init.c_str(), init.size(), validation);
        }

        void set(const std::wstring &init,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_wchar(init.c_str(), init.size(), validation);
        }

#ifdef ST_HAVE_STD_USTRING_TYPES
        void set(const std::u16string &init,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_utf16(init.c_str(), init.size(), validation);
        }

        void set(const std::u32string &init,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_utf32(init.c_str(), init.size(), validation);
        }
#endif

#ifdef ST_HAVE_CXX17_STRING_VIEW
        void set(const std::string_view &view,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_utf8(view.data(), view.size(), validation);
        }

        void set(const std::wstring_view &view,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_wchar(view.data(), view.size(), validation);
        }

        void set(const std::u16string_view &view,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_utf16(view.data(), view.size(), validation);
        }

        void set(const std::u32string_view &view,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_utf32(view.data(), view.size(), validation);
        }
#endif

#ifdef ST_HAVE_EXPERIMENTAL_STRING_VIEW
        void set(const std::experimental::string_view &view,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_utf8(view.data(), view.size(), validation);
        }

        void set(const std::experimental::wstring_view &view,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_wchar(view.data(), view.size(), validation);
        }

        void set(const std::experimental::u16string_view &view,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_utf16(view.data(), view.size(), validation);
        }

        void set(const std::experimental::u32string_view &view,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _convert_from_utf32(view.data(), view.size(), validation);
        }
#endif

#ifdef ST_HAVE_CXX17_FILESYSTEM
        void set(const std::filesystem::path &path)
        {
            std::string path_utf8 = path.u8string();
            set_validated(path_utf8.c_str(), path_utf8.size());
        }
#endif

#ifdef ST_HAVE_EXPERIMENTAL_FILESYSTEM
        void set(const std::experimental::filesystem::path &path)
        {
            std::string path_utf8 = path.u8string();
            set_validated(path_utf8.c_str(), path_utf8.size());
        }
#endif

#endif // !defined(ST_NO_STL_STRINGS)

        inline void set_validated(const char *text, size_t size)
        {
            m_buffer = ST::char_buffer(text, size);
        }

        inline void set_validated(const char_buffer &buffer)
        {
            m_buffer = buffer;
        }

        inline void set_validated(char_buffer &&buffer)
        {
            m_buffer = std::move(buffer);
        }

        string &operator=(const null_t &) ST_NOEXCEPT
        {
            m_buffer = null;
            return *this;
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

#ifdef ST_HAVE_CHAR_TYPES
        string &operator=(const char16_t *cstr)
        {
            set(cstr);
            return *this;
        }

        string &operator=(const char32_t *cstr)
        {
            set(cstr);
            return *this;
        }
#endif

        string &operator=(const string &copy)
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

#if !defined(ST_NO_STL_STRINGS)
        string &operator=(const std::string &init)
        {
            set(init);
            return *this;
        }

        string &operator=(const std::wstring &init)
        {
            set(init);
            return *this;
        }

#ifdef ST_HAVE_STD_USTRING_TYPES
        string &operator=(const std::u16string &init)
        {
            set(init);
            return *this;
        }

        string &operator=(const std::u32string &init)
        {
            set(init);
            return *this;
        }
#endif

#ifdef ST_HAVE_CXX17_STRING_VIEW
        string &operator=(const std::string_view &view)
        {
            set(view);
            return *this;
        }

        string &operator=(const std::wstring_view &view)
        {
            set(view);
            return *this;
        }

        string &operator=(const std::u16string_view &view)
        {
            set(view);
            return *this;
        }

        string &operator=(const std::u32string_view &view)
        {
            set(view);
            return *this;
        }
#endif

#ifdef ST_HAVE_EXPERIMENTAL_STRING_VIEW
        string &operator=(const std::experimental::string_view &view)
        {
            set(view);
            return *this;
        }

        string &operator=(const std::experimental::wstring_view &view)
        {
            set(view);
            return *this;
        }

        string &operator=(const std::experimental::u16string_view &view)
        {
            set(view);
            return *this;
        }

        string &operator=(const std::experimental::u32string_view &view)
        {
            set(view);
            return *this;
        }
#endif

#ifdef ST_HAVE_CXX17_FILESYSTEM
        string &operator=(const std::filesystem::path &path)
        {
            set(path);
            return *this;
        }
#endif

#ifdef ST_HAVE_EXPERIMENTAL_FILESYSTEM
        string &operator=(const std::experimental::filesystem::path &path)
        {
            set(path);
            return *this;
        }
#endif

#endif // !defined(ST_NO_STL_STRINGS)

        string &operator+=(const char *cstr);
        string &operator+=(const wchar_t *wstr);

#ifdef ST_HAVE_CHAR_TYPES
        string &operator+=(const char16_t *cstr);
        string &operator+=(const char32_t *cstr);
#endif

        string &operator+=(const string &other);

        string &operator+=(char ch);
        string &operator+=(wchar_t ch);

#ifdef ST_HAVE_CHAR_TYPES
        string &operator+=(char16_t ch);
        string &operator+=(char32_t ch);
#endif

        static inline string from_literal(const char *literal, size_t size)
        {
            from_literal_t lit_marker;
            return string(lit_marker, literal, size);
        }

        static inline string from_validated(const char *text, size_t size)
        {
            from_validated_t valid_tag;
            return string(valid_tag, text, size);
        }

        static inline string from_validated(const char_buffer &buffer)
        {
            from_validated_t valid_tag;
            return string(valid_tag, buffer);
        }

        static inline string from_validated(char_buffer &&buffer)
        {
            from_validated_t valid_tag;
            return string(valid_tag, std::move(buffer));
        }

        static inline string from_utf8(const char *utf8,
                                       size_t size = ST_AUTO_SIZE,
                                       utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = utf8 ? std::char_traits<char>::length(utf8) : 0;

            string str;
            str._convert_from_utf8(utf8, size, validation);
            return str;
        }

        static inline string from_utf16(const char16_t *utf16,
                                        size_t size = ST_AUTO_SIZE,
                                        utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = utf16 ? std::char_traits<char16_t>::length(utf16) : 0;

            string str;
            str._convert_from_utf16(utf16, size, validation);
            return str;
        }

        static inline string from_utf32(const char32_t *utf32,
                                        size_t size = ST_AUTO_SIZE,
                                        utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = utf32 ? std::char_traits<char32_t>::length(utf32) : 0;

            string str;
            str._convert_from_utf32(utf32, size, validation);
            return str;
        }

        static inline string from_wchar(const wchar_t *wstr,
                                        size_t size = ST_AUTO_SIZE,
                                        utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = wstr ? std::char_traits<wchar_t>::length(wstr) : 0;

            string str;
            str._convert_from_wchar(wstr, size, validation);
            return str;
        }

        static inline string from_latin_1(const char *astr,
                                          size_t size = ST_AUTO_SIZE)
        {
            if (size == ST_AUTO_SIZE)
                size = astr ? std::char_traits<char>::length(astr) : 0;

            string str;
            str._convert_from_latin_1(astr, size);
            return str;
        }

        static inline string from_utf8(const char_buffer &utf8,
                                       utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            string str;
            str._convert_from_utf8(utf8.data(), utf8.size(), validation);
            return str;
        }

        static inline string from_utf16(const utf16_buffer &utf16,
                                        utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            string str;
            str._convert_from_utf16(utf16.data(), utf16.size(), validation);
            return str;
        }

        static inline string from_utf32(const utf32_buffer &utf32,
                                        utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            string str;
            str._convert_from_utf32(utf32.data(), utf32.size(), validation);
            return str;
        }

        static inline string from_wchar(const wchar_buffer &wstr,
                                        utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            string str;
            str._convert_from_wchar(wstr.data(), wstr.size(), validation);
            return str;
        }

        static inline string from_latin_1(const char_buffer &astr)
        {
            string str;
            str._convert_from_latin_1(astr.data(), astr.size());
            return str;
        }

#if !defined(ST_NO_STL_STRINGS)
        static inline string from_std_string(const std::string &sstr,
                                utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            string str;
            str._convert_from_utf8(sstr.c_str(), sstr.size(), validation);
            return str;
        }

        static inline string from_std_string(const std::wstring &wstr,
                                utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            string str;
            str._convert_from_wchar(wstr.c_str(), wstr.size(), validation);
            return str;
        }

        static inline string from_std_wstring(const std::wstring &wstr,
                                utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            return from_std_string(wstr, validation);
        }

#ifdef ST_HAVE_STD_USTRING_TYPES
        static inline string from_std_string(const std::u16string &ustr,
                                utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            string str;
            str._convert_from_utf16(ustr.c_str(), ustr.size(), validation);
            return str;
        }

        static inline string from_std_string(const std::u32string &ustr,
                                utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            string str;
            str._convert_from_utf32(ustr.c_str(), ustr.size(), validation);
            return str;
        }
#endif

#ifdef ST_HAVE_CXX17_STRING_VIEW
        static inline string from_std_string(const std::string_view &view,
                                utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            string str;
            str._convert_from_utf8(view.data(), view.size(), validation);
            return str;
        }

        static inline string from_std_string(const std::wstring_view &view,
                                utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            string str;
            str._convert_from_wchar(view.data(), view.size(), validation);
            return str;
        }

        static inline string from_std_wstring(const std::wstring_view &view,
                                utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            return from_std_string(view, validation);
        }

        static inline string from_std_string(const std::u16string_view &view,
                                utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            string str;
            str._convert_from_utf16(view.data(), view.size(), validation);
            return str;
        }

        static inline string from_std_string(const std::u32string_view &view,
                                utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            string str;
            str._convert_from_utf32(view.data(), view.size(), validation);
            return str;
        }
#endif

#ifdef ST_HAVE_EXPERIMENTAL_STRING_VIEW
        static inline string from_std_string(const std::experimental::string_view &view,
                                utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            string str;
            str._convert_from_utf8(view.data(), view.size(), validation);
            return str;
        }

        static inline string from_std_string(const std::experimental::wstring_view &view,
                                utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            string str;
            str._convert_from_wchar(view.data(), view.size(), validation);
            return str;
        }

        static inline string from_std_wstring(const std::experimental::wstring_view &view,
                                utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            return from_std_string(view, validation);
        }

        static inline string from_std_string(const std::experimental::u16string_view &view,
                                utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            string str;
            str._convert_from_utf16(view.data(), view.size(), validation);
            return str;
        }

        static inline string from_std_string(const std::experimental::u32string_view &view,
                                utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            string str;
            str._convert_from_utf32(view.data(), view.size(), validation);
            return str;
        }
#endif

#if defined(ST_HAVE_CXX17_FILESYSTEM)
        static inline string from_path(const std::filesystem::path &path)
        {
            string str;
            str.set(path);
            return str;
        }
#endif

#if defined(ST_HAVE_EXPERIMENTAL_FILESYSTEM)
        static inline string from_path(const std::experimental::filesystem::path &path)
        {
            string str;
            str.set(path);
            return str;
        }
#endif

#endif // !defined(ST_NO_STL_STRINGS)

        const char *c_str() const ST_NOEXCEPT
        {
            return m_buffer.data();
        }

        const char *c_str(const char *substitute) const ST_NOEXCEPT
        {
            return empty() ? substitute : m_buffer.data();
        }

        const char &at(size_t position) const
        {
            return m_buffer.at(position);
        }

        const char &operator[](size_t position) const ST_NOEXCEPT
        {
            return m_buffer.operator[](position);
        }

        ST_DEPRECATED_IN_2_0("replaced with at() or operator[] in string_theory 2.0")
        char char_at(size_t position) const ST_NOEXCEPT { return c_str()[position]; }

        const char &front() const ST_NOEXCEPT
        {
            return m_buffer.front();
        }

        const char &back() const ST_NOEXCEPT
        {
            return m_buffer.back();
        }

        const_iterator begin() const ST_NOEXCEPT { return m_buffer.begin(); }
        const_iterator cbegin() const ST_NOEXCEPT { return m_buffer.cbegin(); }
        const_iterator end() const ST_NOEXCEPT { return m_buffer.end(); }
        const_iterator cend() const ST_NOEXCEPT { return m_buffer.cend(); }

        const_reverse_iterator rbegin() const ST_NOEXCEPT { return m_buffer.rbegin(); }
        const_reverse_iterator crbegin() const ST_NOEXCEPT { return m_buffer.crbegin(); }
        const_reverse_iterator rend() const ST_NOEXCEPT { return m_buffer.rend(); }
        const_reverse_iterator crend() const ST_NOEXCEPT { return m_buffer.crend(); }

        char_buffer to_utf8() const ST_NOEXCEPT { return m_buffer; }

        utf16_buffer to_utf16() const;
        utf32_buffer to_utf32() const;
        wchar_buffer to_wchar() const;
        char_buffer to_latin_1(utf_validation_t validation = substitute_invalid) const;

#if !defined(ST_NO_STL_STRINGS)
        std::string to_std_string(bool utf8 = true,
                                  utf_validation_t validation = substitute_invalid) const
        {
            if (utf8)
                return std::string(c_str(), size());

            return to_latin_1(validation).to_std_string();
        }

        void to_std_string(std::string &result, bool utf8 = true,
                           utf_validation_t validation = substitute_invalid) const
        {
            result = to_std_string(utf8, validation);
        }

        std::wstring to_std_wstring() const
        {
            return to_wchar().to_std_string();
        }

        void to_std_string(std::wstring &result) const
        {
            result = to_std_wstring();
        }

#ifdef ST_HAVE_STD_USTRING_TYPES
        std::u16string to_std_u16string() const
        {
            return to_utf16().to_std_string();
        }

        void to_std_string(std::u16string &result) const
        {
            result = to_std_u16string();
        }

        std::u32string to_std_u32string() const
        {
            return to_utf32().to_std_string();
        }

        void to_std_string(std::u32string &result) const
        {
            result = to_std_u32string();
        }
#endif

#if defined(ST_HAVE_CXX17_FILESYSTEM)
        std::filesystem::path to_path() const
        {
            return std::filesystem::u8path(c_str(), c_str() + size());
        }
#elif defined(ST_HAVE_EXPERIMENTAL_FILESYSTEM)
        std::experimental::filesystem::path to_path() const
        {
            return std::experimental::filesystem::u8path(c_str(), c_str() + size());
        }
#endif

#if defined(ST_HAVE_CXX17_STRING_VIEW)
        std::string_view view(size_t start = 0, size_t length = ST_AUTO_SIZE) const
        {
            return m_buffer.view(start, length);
        }
#elif defined(ST_HAVE_EXPERIMENTAL_STRING_VIEW)
        std::experimental::string_view view(size_t start = 0, size_t length = ST_AUTO_SIZE) const
        {
            return m_buffer.view(start, length);
        }
#endif

#ifdef ST_HAVE_CXX17_STRING_VIEW
        operator std::string_view() const
        {
            return (std::string_view)m_buffer;
        }
#endif

#ifdef ST_HAVE_EXPERIMENTAL_STRING_VIEW
        operator std::experimental::string_view() const
        {
            return (std::experimental::string_view)m_buffer;
        }
#endif

#endif // !defined(ST_NO_STL_STRINGS)

        size_t size() const ST_NOEXCEPT { return m_buffer.size(); }
        bool empty() const ST_NOEXCEPT { return m_buffer.size() == 0; }

        ST_DEPRECATED_IN_2_0("replaced with empty() in string_theory 2.0")
        bool is_empty() const ST_NOEXCEPT { return empty(); }

        static string from_int(int value, int base = 10, bool upper_case = false);
        static string from_uint(unsigned int value, int base = 10, bool upper_case = false);
        static string from_float(float value, char format='g');
        static string from_double(double value, char format='g');

#ifdef ST_HAVE_INT64
        static string from_int64(int64_t value, int base = 10, bool upper_case = false);
        static string from_uint64(uint64_t value, int base = 10, bool upper_case = false);
#endif

        static string from_bool(bool value)
        {
            return value ? from_literal("true", 4)
                         : from_literal("false", 5);
        }

        int to_int(int base = 0) const ST_NOEXCEPT;
        int to_int(conversion_result &result, int base = 0) const ST_NOEXCEPT;
        unsigned int to_uint(int base = 0) const ST_NOEXCEPT;
        unsigned int to_uint(conversion_result &result, int base = 0) const ST_NOEXCEPT;
        float to_float() const ST_NOEXCEPT;
        float to_float(conversion_result &result) const ST_NOEXCEPT;
        double to_double() const ST_NOEXCEPT;
        double to_double(conversion_result &result) const ST_NOEXCEPT;

#ifdef ST_HAVE_INT64
        int64_t to_int64(int base = 0) const ST_NOEXCEPT;
        int64_t to_int64(conversion_result &result, int base = 0) const ST_NOEXCEPT;
        uint64_t to_uint64(int base = 0) const ST_NOEXCEPT;
        uint64_t to_uint64(conversion_result &result, int base = 0) const ST_NOEXCEPT;
#endif

        bool to_bool() const ST_NOEXCEPT;
        bool to_bool(conversion_result &result) const ST_NOEXCEPT;

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

        bool operator==(const null_t &) const ST_NOEXCEPT
        {
            return empty();
        }

        bool operator==(const string &other) const ST_NOEXCEPT
        {
            return compare(other) == 0;
        }

        bool operator==(const char *other) const ST_NOEXCEPT
        {
            return compare(other) == 0;
        }

        bool operator!=(const null_t &) const ST_NOEXCEPT
        {
            return !empty();
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
            const ST_NOEXCEPT
        {
            return find(0, ch, cs);
        }

        ST_ssize_t find(const char *substr, case_sensitivity_t cs = case_sensitive)
            const ST_NOEXCEPT
        {
            return find(0, substr, cs);
        }

        ST_ssize_t find(const string &substr, case_sensitivity_t cs = case_sensitive)
            const ST_NOEXCEPT
        {
            return find(0, substr.c_str(), cs);
        }

        ST_ssize_t find(size_t start, char ch, case_sensitivity_t cs = case_sensitive)
            const ST_NOEXCEPT;

        ST_ssize_t find(size_t start, const char *substr, case_sensitivity_t cs = case_sensitive)
            const ST_NOEXCEPT;

        ST_ssize_t find(size_t start, const string &substr, case_sensitivity_t cs = case_sensitive)
            const ST_NOEXCEPT
        {
            return find(start, substr.c_str(), cs);
        }

        ST_ssize_t find_last(char ch, case_sensitivity_t cs = case_sensitive)
            const ST_NOEXCEPT
        {
            return find_last(ST_AUTO_SIZE, ch, cs);
        }

        ST_ssize_t find_last(const char *substr, case_sensitivity_t cs = case_sensitive)
            const ST_NOEXCEPT
        {
            return find_last(ST_AUTO_SIZE, substr, cs);
        }

        ST_ssize_t find_last(const string &substr, case_sensitivity_t cs = case_sensitive)
            const ST_NOEXCEPT
        {
            return find_last(ST_AUTO_SIZE, substr.c_str(), cs);
        }

        ST_ssize_t find_last(size_t max, char ch, case_sensitivity_t cs = case_sensitive)
            const ST_NOEXCEPT;

        ST_ssize_t find_last(size_t max, const char *substr, case_sensitivity_t cs = case_sensitive)
            const ST_NOEXCEPT;

        ST_ssize_t find_last(size_t max, const string &substr, case_sensitivity_t cs = case_sensitive)
            const ST_NOEXCEPT
        {
            return find_last(max, substr.c_str(), cs);
        }

        bool contains(char ch, case_sensitivity_t cs = case_sensitive)
            const ST_NOEXCEPT
        {
            return find(ch, cs) >= 0;
        }

        bool contains(const char *substr, case_sensitivity_t cs = case_sensitive)
            const ST_NOEXCEPT
        {
            return find(substr, cs) >= 0;
        }

        bool contains(const string &substr, case_sensitivity_t cs = case_sensitive)
            const ST_NOEXCEPT
        {
            return find(substr, cs) >= 0;
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

        bool starts_with(const string &prefix, case_sensitivity_t cs = case_sensitive) const ST_NOEXCEPT;
        bool starts_with(const char *prefix, case_sensitivity_t cs = case_sensitive) const ST_NOEXCEPT;
        bool ends_with(const string &suffix, case_sensitivity_t cs = case_sensitive) const ST_NOEXCEPT;
        bool ends_with(const char *suffix, case_sensitivity_t cs = case_sensitive) const ST_NOEXCEPT;

        string before_first(char sep, case_sensitivity_t cs = case_sensitive) const;
        string before_first(const char *sep, case_sensitivity_t cs = case_sensitive) const;
        string before_first(const string &sep, case_sensitivity_t cs = case_sensitive) const;
        string after_first(char sep, case_sensitivity_t cs = case_sensitive) const;
        string after_first(const char *sep, case_sensitivity_t cs = case_sensitive) const;
        string after_first(const string &sep, case_sensitivity_t cs = case_sensitive) const;
        string before_last(char sep, case_sensitivity_t cs = case_sensitive) const;
        string before_last(const char *sep, case_sensitivity_t cs = case_sensitive) const;
        string before_last(const string &sep, case_sensitivity_t cs = case_sensitive) const;
        string after_last(char sep, case_sensitivity_t cs = case_sensitive) const;
        string after_last(const char *sep, case_sensitivity_t cs = case_sensitive) const;
        string after_last(const string &sep, case_sensitivity_t cs = case_sensitive) const;

        string replace(const char *from, const char *to,
                       case_sensitivity_t cs = case_sensitive,
                       utf_validation_t validation = ST_DEFAULT_VALIDATION) const
        {
            return replace(from ? string(from) : null, to ? string(to) : null, cs, validation);
        }

        string replace(const string &from, const char *to,
                       case_sensitivity_t cs = case_sensitive,
                       utf_validation_t validation = ST_DEFAULT_VALIDATION) const
        {
            return replace(from, to ? string(to) : null, cs, validation);
        }

        string replace(const char *from, const string &to,
                       case_sensitivity_t cs = case_sensitive,
                       utf_validation_t validation = ST_DEFAULT_VALIDATION) const
        {
            return replace(from ? string(from) : null, to, cs, validation);
        }

        string replace(const string &from, const string &to,
                       case_sensitivity_t cs = case_sensitive,
                       utf_validation_t validation = ST_DEFAULT_VALIDATION) const;

        string to_upper() const;
        string to_lower() const;

        std::vector<string> split(char split_char,
                                  size_t max_splits = ST_AUTO_SIZE,
                                  case_sensitivity_t cs = case_sensitive) const;
        std::vector<string> split(const char *splitter,
                                  size_t max_splits = ST_AUTO_SIZE,
                                  case_sensitivity_t cs = case_sensitive) const;
        std::vector<string> split(const string &splitter,
                                  size_t max_splits = ST_AUTO_SIZE,
                                  case_sensitivity_t cs = case_sensitive) const;

        std::vector<string> tokenize(const char *delims = ST_WHITESPACE) const;

        static string fill(size_t count, char c);
    };

    struct ST_EXPORT hash
    {
        size_t operator()(const string &str) const ST_NOEXCEPT;
    };

    struct ST_EXPORT hash_i
    {
        size_t operator()(const string &str) const ST_NOEXCEPT;
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

    ST_EXPORT string operator+(const string &left, const string &right);

    inline string operator+(const string &left, const char *right)
    {
        return operator+(left, string::from_utf8(right));
    }

    inline string operator+(const char *left, const string &right)
    {
        return operator+(string::from_utf8(left), right);
    }

    inline string operator+(const string &left, const wchar_t *right)
    {
        return operator+(left, string::from_wchar(right));
    }

    inline ST::string operator+(const wchar_t *left, const string &right)
    {
        return operator+(string::from_wchar(left), right);
    }

#ifdef ST_HAVE_CHAR_TYPES
    inline string operator+(const string &left, const char16_t *right)
    {
        return operator+(left, string::from_utf16(right));
    }

    inline string operator+(const char16_t *left, const string &right)
    {
        return operator+(string::from_utf16(left), right);
    }

    inline string operator+(const string &left, const char32_t *right)
    {
        return operator+(left, string::from_utf32(right));
    }

    inline string operator+(const char32_t *left, const string &right)
    {
        return operator+(string::from_utf32(left), right);
    }
#endif

    ST_EXPORT string operator+(const string &left, char right);
    ST_EXPORT string operator+(const string &left, wchar_t right);

#ifdef ST_HAVE_CHAR_TYPES
    ST_EXPORT string operator+(const string &left, char16_t right);
    ST_EXPORT string operator+(const string &left, char32_t right);
#endif

    ST_EXPORT string operator+(char left, const string &right);
    ST_EXPORT string operator+(wchar_t left, const string &right);

#ifdef ST_HAVE_CHAR_TYPES
    ST_EXPORT string operator+(char16_t left, const string &right);
    ST_EXPORT string operator+(char32_t left, const string &right);
#endif

    inline bool operator==(const null_t &, const string &right) ST_NOEXCEPT
    {
        return right.empty();
    }

    inline bool operator!=(const null_t &, const string &right) ST_NOEXCEPT
    {
        return !right.empty();
    }
}

namespace std
{
    template <>
    struct hash<ST::string>
    {
         inline size_t operator()(const ST::string &str) const ST_NOEXCEPT
         {
             return ST::hash()(str);
         }
    };
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
