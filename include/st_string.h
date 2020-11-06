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

#ifdef ST_HAVE_INT64
#   include <cstdint>
#endif

#include "st_string_priv.h"
#include "st_utf_conv.h"

#if defined(ST_ENABLE_STL_FILESYSTEM) && defined(ST_HAVE_CXX17_FILESYSTEM)
#   include <filesystem>
#endif

#define ST_WHITESPACE   " \t\r\n"

namespace ST
{
    enum class case_sensitivity_t
    {
        case_sensitive,
        case_insensitive
    };
    ST_ENUM_CONSTANT(case_sensitivity_t, case_sensitive);
    ST_ENUM_CONSTANT(case_sensitivity_t, case_insensitive);

    class conversion_result
    {
        enum
        {
            result_ok = (1 << 0),
            result_full_match = (1 << 1)
        };

    public:
        explicit conversion_result() noexcept : m_flags() { }

        ST_NODISCARD
        bool ok() const noexcept { return (m_flags & result_ok) != 0; }

        ST_NODISCARD
        bool full_match() const noexcept { return (m_flags & result_full_match) != 0; }

    private:
        int m_flags;
        friend class string;
    };

    static_assert(std::is_standard_layout<ST::conversion_result>::value,
                  "ST::conversion_result must be standard-layout to pass across the DLL boundary");

    class string
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

        void _set_utf8(const char *utf8, size_t size, utf_validation_t validation)
        {
            ST_ASSERT(size < ST_HUGE_BUFFER_SIZE, "String data buffer is too large");

            if (!utf8) {
                m_buffer = char_buffer();
                return;
            }

            set(char_buffer(utf8, size), validation);
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        void _set_utf8(const char8_t *utf8, size_t size, utf_validation_t validation)
        {
            _set_utf8(reinterpret_cast<const char *>(utf8), size, validation);
        }
#endif

        struct from_literal_t {};
        string(const from_literal_t &, const char *data, size_t size)
            : m_buffer(data, size) { }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        string(const from_literal_t &, const char8_t *data, size_t size)
            : m_buffer(reinterpret_cast<const char *>(data), size) { }
#endif

        struct from_validated_t {};
        string(const from_validated_t &, const char *data, size_t size)
            : m_buffer(data, size) { }
        string(const from_validated_t &, const char_buffer &buffer)
            : m_buffer(buffer) { }
        string(const from_validated_t &, char_buffer &&buffer)
            : m_buffer(std::move(buffer)) { }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        string(const from_validated_t &, const char8_t *data, size_t size)
            : m_buffer(reinterpret_cast<const char *>(data), size) { }
#endif

    public:
        string() noexcept { }

        ST_DEPRECATED_IN_3_4("Use empty initializer {} instead")
        string(const null_t &) noexcept { }

        string(const char *cstr, size_t size = ST_AUTO_SIZE,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = cstr ? std::char_traits<char>::length(cstr) : 0;
            _set_utf8(cstr, size, validation);
        }

        string(const wchar_t *wstr, size_t size = ST_AUTO_SIZE,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = wstr ? std::char_traits<wchar_t>::length(wstr) : 0;
            m_buffer = wchar_to_utf8(wstr, size, validation);
        }

        string(const char16_t *cstr, size_t size = ST_AUTO_SIZE,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = cstr ? std::char_traits<char16_t>::length(cstr) : 0;
            m_buffer = utf16_to_utf8(cstr, size, validation);
        }

        string(const char32_t *cstr, size_t size = ST_AUTO_SIZE,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = cstr ? std::char_traits<char32_t>::length(cstr) : 0;
            m_buffer = utf32_to_utf8(cstr, size, validation);
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        string(const char8_t *cstr, size_t size = ST_AUTO_SIZE,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = cstr ? std::char_traits<char8_t>::length(cstr) : 0;
            _set_utf8(cstr, size, validation);
        }
#endif

        string(const string &copy)
            : m_buffer(copy.m_buffer) { }

        string(string &&move) noexcept
            : m_buffer(std::move(move.m_buffer)) { }

        string(const char_buffer &init,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            set(init, validation);
        }

        string(char_buffer &&init,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            set(std::move(init), validation);
        }

        string(const utf16_buffer &init,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
            : m_buffer(utf16_to_utf8(init, validation))
        {
        }

        string(const utf32_buffer &init,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
            : m_buffer(utf32_to_utf8(init, validation))
        {
        }

        string(const wchar_buffer &init,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
            : m_buffer(wchar_to_utf8(init, validation))
        {
        }

#if defined(ST_ENABLE_STL_STRINGS)
        string(const std::string &init,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _set_utf8(init.c_str(), init.size(), validation);
        }

        string(const std::wstring &init,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
            : m_buffer(wchar_to_utf8(init.c_str(), init.size(), validation))
        {
        }

        string(const std::u16string &init,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
            : m_buffer(utf16_to_utf8(init.c_str(), init.size(), validation))
        {
        }

        string(const std::u32string &init,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
            : m_buffer(utf32_to_utf8(init.c_str(), init.size(), validation))
        {
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        string(const std::u8string &init,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _set_utf8(init.c_str(), init.size(), validation);
        }
#endif

#ifdef ST_HAVE_CXX17_STRING_VIEW
        string(const std::string_view &view,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _set_utf8(view.data(), view.size(), validation);
        }

        string(const std::wstring_view &view,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            m_buffer = wchar_to_utf8(view.data(), view.size(), validation);
        }

        string(const std::u16string_view &view,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            m_buffer = utf16_to_utf8(view.data(), view.size(), validation);
        }

        string(const std::u32string_view &view,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            m_buffer = utf32_to_utf8(view.data(), view.size(), validation);
        }
#endif

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        string(const std::u8string_view &view,
               utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _set_utf8(view.data(), view.size(), validation);
        }
#endif

#endif // defined(ST_ENABLE_STL_STRINGS)

#if defined(ST_ENABLE_STL_FILESYSTEM) && defined(ST_HAVE_CXX17_FILESYSTEM)
        string(const std::filesystem::path &path)
        {
            set(path);
        }
#endif

        ST_DEPRECATED_IN_3_4("Use clear() instead")
        void set(const null_t &) noexcept { m_buffer.clear(); }

        void set(const char *cstr, size_t size = ST_AUTO_SIZE,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = cstr ? std::char_traits<char>::length(cstr) : 0;
            _set_utf8(cstr, size, validation);
        }

        void set(const wchar_t *wstr, size_t size = ST_AUTO_SIZE,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = wstr ? std::char_traits<wchar_t>::length(wstr) : 0;
            m_buffer = wchar_to_utf8(wstr, size, validation);
        }

        void set(const char16_t *cstr, size_t size = ST_AUTO_SIZE,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = cstr ? std::char_traits<char16_t>::length(cstr) : 0;
            m_buffer = utf16_to_utf8(cstr, size, validation);
        }

        void set(const char32_t *cstr, size_t size = ST_AUTO_SIZE,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = cstr ? std::char_traits<char32_t>::length(cstr) : 0;
            m_buffer = utf32_to_utf8(cstr, size, validation);
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        void set(const char8_t *cstr, size_t size = ST_AUTO_SIZE,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = cstr ? std::char_traits<char8_t>::length(cstr) : 0;
            _set_utf8(cstr, size, validation);
        }
#endif

        void set(const string &copy)
        {
            m_buffer = copy.m_buffer;
        }

        void set(string &&move) noexcept
        {
            m_buffer = std::move(move.m_buffer);
        }

        void set(const char_buffer &init,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            switch (validation) {
            case check_validity:
                _ST_PRIVATE::raise_conversion_error(
                    _ST_PRIVATE::validate_utf8(init.data(), init.size()));
                m_buffer = init;
                break;

            case substitute_invalid:
                m_buffer = _ST_PRIVATE::cleanup_utf8_buffer(init);
                break;

            case assume_valid:
                m_buffer = init;
                break;

            default:
                ST_ASSERT(false, "Invalid validation type");
            }
        }

        void set(char_buffer &&init,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            switch (validation) {
            case check_validity:
                _ST_PRIVATE::raise_conversion_error(
                    _ST_PRIVATE::validate_utf8(init.data(), init.size()));
                m_buffer = std::move(init);
                break;

            case substitute_invalid:
                m_buffer = _ST_PRIVATE::cleanup_utf8_buffer(init);
                break;

            case assume_valid:
                m_buffer = std::move(init);
                break;

            default:
                ST_ASSERT(false, "Invalid validation type");
            }
        }

        void set(const utf16_buffer &init,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            m_buffer = utf16_to_utf8(init, validation);
        }

        void set(const utf32_buffer &init,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            m_buffer = utf32_to_utf8(init, validation);
        }

        void set(const wchar_buffer &init,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            m_buffer = wchar_to_utf8(init, validation);
        }

#if defined(ST_ENABLE_STL_STRINGS)
        void set(const std::string &init,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _set_utf8(init.c_str(), init.size(), validation);
        }

        void set(const std::wstring &init,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            m_buffer = wchar_to_utf8(init.c_str(), init.size(), validation);
        }

        void set(const std::u16string &init,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            m_buffer = utf16_to_utf8(init.c_str(), init.size(), validation);
        }

        void set(const std::u32string &init,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            m_buffer = utf32_to_utf8(init.c_str(), init.size(), validation);
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        void set(const std::u8string &init,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _set_utf8(init.c_str(), init.size(), validation);
        }
#endif

#ifdef ST_HAVE_CXX17_STRING_VIEW
        void set(const std::string_view &view,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _set_utf8(view.data(), view.size(), validation);
        }

        void set(const std::wstring_view &view,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            m_buffer = wchar_to_utf8(view.data(), view.size(), validation);
        }

        void set(const std::u16string_view &view,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            m_buffer = utf16_to_utf8(view.data(), view.size(), validation);
        }

        void set(const std::u32string_view &view,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            m_buffer = utf32_to_utf8(view.data(), view.size(), validation);
        }
#endif

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        void set(const std::u8string_view &view,
                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            _set_utf8(view.data(), view.size(), validation);
        }
#endif

#endif // defined(ST_ENABLE_STL_STRINGS)

#if defined(ST_ENABLE_STL_FILESYSTEM) && defined(ST_HAVE_CXX17_FILESYSTEM)
        void set(const std::filesystem::path &path)
        {
            auto path_utf8 = path.u8string();
            set_validated(path_utf8.c_str(), path_utf8.size());
        }
#endif

        void set_validated(const char *text, size_t size)
        {
            m_buffer = ST::char_buffer(text, size);
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        void set_validated(const char8_t *text, size_t size)
        {
            m_buffer = ST::char_buffer(reinterpret_cast<const char *>(text), size);
        }
#endif

        void set_validated(const char_buffer &buffer)
        {
            m_buffer = buffer;
        }

        void set_validated(char_buffer &&buffer)
        {
            m_buffer = std::move(buffer);
        }

        void clear() noexcept
        {
            m_buffer.clear();
        }

        ST_DEPRECATED_IN_3_4("Use clear() instead")
        string &operator=(const null_t &) noexcept
        {
            m_buffer.clear();
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

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        string &operator=(const char8_t *cstr)
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

        string &operator=(string &&move) noexcept
        {
            m_buffer = std::move(move.m_buffer);
            return *this;
        }

        string &operator=(const char_buffer &init)
        {
            set(init);
            return *this;
        }

        string &operator=(char_buffer &&init)
        {
            set(std::move(init));
            return *this;
        }

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

#if defined(ST_ENABLE_STL_STRINGS)
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

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        string &operator=(const std::u8string &init)
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

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        string &operator=(const std::u8string_view &view)
        {
            set(view);
            return *this;
        }
#endif

#endif // defined(ST_ENABLE_STL_STRINGS)

#if defined(ST_ENABLE_STL_FILESYSTEM) && defined(ST_HAVE_CXX17_FILESYSTEM)
        string &operator=(const std::filesystem::path &path)
        {
            set(path);
            return *this;
        }
#endif

        inline string &operator+=(const char *cstr);
        inline string &operator+=(const wchar_t *wstr);

        inline string &operator+=(const char16_t *cstr);
        inline string &operator+=(const char32_t *cstr);

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        inline string &operator+=(const char8_t *cstr);
#endif

        inline string &operator+=(const string &other);

        inline string &operator+=(char ch);
        inline string &operator+=(wchar_t ch);
        inline string &operator+=(char16_t ch);
        inline string &operator+=(char32_t ch);

        ST_NODISCARD
        static string from_literal(const char *literal, size_t size)
        {
            from_literal_t lit_marker;
            return string(lit_marker, literal, size);
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        ST_NODISCARD
        static string from_literal(const char8_t *literal, size_t size)
        {
            from_literal_t lit_marker;
            return string(lit_marker, literal, size);
        }
#endif

        ST_NODISCARD
        static string from_validated(const char *text, size_t size)
        {
            from_validated_t valid_tag;
            return string(valid_tag, text, size);
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        ST_NODISCARD
        static string from_validated(const char8_t *text, size_t size)
        {
            from_validated_t valid_tag;
            return string(valid_tag, text, size);
        }
#endif

        ST_NODISCARD
        static string from_validated(const char_buffer &buffer)
        {
            from_validated_t valid_tag;
            return string(valid_tag, buffer);
        }

        ST_NODISCARD
        static string from_validated(char_buffer &&buffer)
        {
            from_validated_t valid_tag;
            return string(valid_tag, std::move(buffer));
        }

        ST_NODISCARD
        static string from_utf8(const char *utf8, size_t size = ST_AUTO_SIZE,
                                utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = utf8 ? std::char_traits<char>::length(utf8) : 0;

            string str;
            str._set_utf8(utf8, size, validation);
            return str;
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        ST_NODISCARD
        static string from_utf8(const char8_t *utf8, size_t size = ST_AUTO_SIZE,
                                utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = utf8 ? std::char_traits<char8_t>::length(utf8) : 0;

            string str;
            str._set_utf8(utf8, size, validation);
            return str;
        }
#endif

        ST_NODISCARD
        static string from_utf16(const char16_t *utf16, size_t size = ST_AUTO_SIZE,
                                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = utf16 ? std::char_traits<char16_t>::length(utf16) : 0;

            return from_validated(utf16_to_utf8(utf16, size, validation));
        }

        ST_NODISCARD
        static string from_utf32(const char32_t *utf32, size_t size = ST_AUTO_SIZE,
                                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = utf32 ? std::char_traits<char32_t>::length(utf32) : 0;

            return from_validated(utf32_to_utf8(utf32, size, validation));
        }

        ST_NODISCARD
        static string from_wchar(const wchar_t *wstr, size_t size = ST_AUTO_SIZE,
                                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            if (size == ST_AUTO_SIZE)
                size = wstr ? std::char_traits<wchar_t>::length(wstr) : 0;

            return from_validated(wchar_to_utf8(wstr, size, validation));
        }

        ST_NODISCARD
        static string from_latin_1(const char *astr, size_t size = ST_AUTO_SIZE)
        {
            if (size == ST_AUTO_SIZE)
                size = astr ? std::char_traits<char>::length(astr) : 0;

            return from_validated(latin_1_to_utf8(astr, size));
        }

        ST_NODISCARD
        static string from_utf8(const char_buffer &utf8,
                                utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            string str;
            str._set_utf8(utf8.data(), utf8.size(), validation);
            return str;
        }

        ST_NODISCARD
        static string from_utf16(const utf16_buffer &utf16,
                                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            return from_validated(utf16_to_utf8(utf16, validation));
        }

        ST_NODISCARD
        static string from_utf32(const utf32_buffer &utf32,
                                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            return from_validated(utf32_to_utf8(utf32, validation));
        }

        ST_NODISCARD
        static string from_wchar(const wchar_buffer &wstr,
                                 utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            return from_validated(wchar_to_utf8(wstr, validation));
        }

        ST_NODISCARD
        static string from_latin_1(const char_buffer &astr)
        {
            return from_validated(latin_1_to_utf8(astr));
        }

#if defined(ST_ENABLE_STL_STRINGS)
        ST_NODISCARD
        static string from_std_string(const std::string &sstr,
                                      utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            string str;
            str._set_utf8(sstr.c_str(), sstr.size(), validation);
            return str;
        }

        ST_NODISCARD
        static string from_std_string(const std::wstring &wstr,
                                      utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            return from_validated(wchar_to_utf8(wstr.c_str(), wstr.size(), validation));
        }

        ST_NODISCARD
        static string from_std_wstring(const std::wstring &wstr,
                                       utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            return from_std_string(wstr, validation);
        }

        ST_NODISCARD
        static string from_std_string(const std::u16string &ustr,
                                      utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            return from_validated(utf16_to_utf8(ustr.c_str(), ustr.size(), validation));
        }

        ST_NODISCARD
        static string from_std_string(const std::u32string &ustr,
                                      utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            return from_validated(utf32_to_utf8(ustr.c_str(), ustr.size(), validation));
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        ST_NODISCARD
        static string from_std_string(const std::u8string &ustr,
                                      utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            string str;
            str._set_utf8(ustr.c_str(), ustr.size(), validation);
            return str;
        }
#endif

#ifdef ST_HAVE_CXX17_STRING_VIEW
        ST_NODISCARD
        static string from_std_string(const std::string_view &view,
                                      utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            string str;
            str._set_utf8(view.data(), view.size(), validation);
            return str;
        }

        ST_NODISCARD
        static string from_std_string(const std::wstring_view &view,
                                      utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            return from_validated(wchar_to_utf8(view.data(), view.size(), validation));
        }

        ST_NODISCARD
        static string from_std_wstring(const std::wstring_view &view,
                                       utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            return from_std_string(view, validation);
        }

        ST_NODISCARD
        static string from_std_string(const std::u16string_view &view,
                                      utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            return from_validated(utf16_to_utf8(view.data(), view.size(), validation));
        }

        ST_NODISCARD
        static string from_std_string(const std::u32string_view &view,
                                      utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            return from_validated(utf32_to_utf8(view.data(), view.size(), validation));
        }
#endif

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        ST_NODISCARD
        static string from_std_string(const std::u8string_view &view,
                                      utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            string str;
            str._set_utf8(view.data(), view.size(), validation);
            return str;
        }
#endif

#endif // defined(ST_ENABLE_STL_STRINGS)

#if defined(ST_ENABLE_STL_FILESYSTEM) && defined(ST_HAVE_CXX17_FILESYSTEM)
        ST_NODISCARD
        static string from_path(const std::filesystem::path &path)
        {
            string str;
            str.set(path);
            return str;
        }
#endif

        ST_NODISCARD
        const char *c_str() const noexcept
        {
            return m_buffer.c_str();
        }

        ST_NODISCARD
        const char *c_str(const char *substitute) const noexcept
        {
            return m_buffer.c_str(substitute);
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        ST_NODISCARD
        const char8_t *u8_str() const noexcept
        {
            return reinterpret_cast<const char8_t *>(m_buffer.data());
        }

        ST_NODISCARD
        const char8_t *u8_str(const char8_t *substitute) const noexcept
        {
            return empty() ? substitute : u8_str();
        }
#endif

        ST_NODISCARD
        const char &at(size_t position) const
        {
            return m_buffer.at(position);
        }

        ST_NODISCARD
        const char &operator[](size_t position) const noexcept
        {
            return m_buffer.operator[](position);
        }

        ST_NODISCARD
        const char &front() const noexcept
        {
            return m_buffer.front();
        }

        ST_NODISCARD
        const char &back() const noexcept
        {
            return m_buffer.back();
        }

        ST_NODISCARD
        const_iterator begin() const noexcept { return m_buffer.begin(); }

        ST_NODISCARD
        const_iterator cbegin() const noexcept { return m_buffer.cbegin(); }

        ST_NODISCARD
        const_iterator end() const noexcept { return m_buffer.end(); }

        ST_NODISCARD
        const_iterator cend() const noexcept { return m_buffer.cend(); }

        ST_NODISCARD
        const_reverse_iterator rbegin() const noexcept { return m_buffer.rbegin(); }

        ST_NODISCARD
        const_reverse_iterator crbegin() const noexcept { return m_buffer.crbegin(); }

        ST_NODISCARD
        const_reverse_iterator rend() const noexcept { return m_buffer.rend(); }

        ST_NODISCARD
        const_reverse_iterator crend() const noexcept { return m_buffer.crend(); }

        ST_NODISCARD
        char_buffer to_utf8() const noexcept { return m_buffer; }

        ST_NODISCARD
        utf16_buffer to_utf16() const
        {
            return utf8_to_utf16(m_buffer, assume_valid);
        }

        ST_NODISCARD
        utf32_buffer to_utf32() const
        {
            return utf8_to_utf32(m_buffer, assume_valid);
        }

        ST_NODISCARD
        wchar_buffer to_wchar() const
        {
            return utf8_to_wchar(m_buffer, assume_valid);
        }

        ST_NODISCARD
        char_buffer to_latin_1(bool substitute_out_of_range = true) const
        {
            return utf8_to_latin_1(m_buffer, assume_valid, substitute_out_of_range);
        }

        ST_NODISCARD
        ST_DEPRECATED_IN_3_0("use to_latin_1(bool) instead")
        char_buffer to_latin_1(utf_validation_t validation) const
        {
            return to_latin_1(validation == substitute_invalid);
        }

        void to_buffer(char_buffer &result, bool utf8 = true,
                       bool substitute_out_of_range = true) const
        {
            if (utf8)
                result = to_utf8();

            result = to_latin_1(substitute_out_of_range);
        }

        ST_DEPRECATED_IN_3_0("use to_buffer(char_buffer &, bool, bool) instead")
        void to_buffer(char_buffer &result, bool utf8,
                       utf_validation_t validation) const
        {
            if (utf8)
                result = to_utf8();

            result = to_latin_1(validation == substitute_invalid);
        }

        void to_buffer(utf16_buffer &result) const
        {
            result = to_utf16();
        }

        void to_buffer(utf32_buffer &result) const
        {
            result = to_utf32();
        }

        void to_buffer(wchar_buffer &result) const
        {
            result = to_wchar();
        }

#if defined(ST_ENABLE_STL_STRINGS)
        ST_NODISCARD
        std::string to_std_string(bool utf8 = true,
                                  bool substitute_out_of_range = true) const
        {
            if (utf8)
                return std::string(c_str(), size());

            return to_latin_1(substitute_out_of_range).to_std_string();
        }

        ST_NODISCARD
        ST_DEPRECATED_IN_3_0("use to_std_string(bool, bool) instead")
        std::string to_std_string(bool utf8, utf_validation_t validation) const
        {
            return to_std_string(utf8, validation == substitute_invalid);
        }

        void to_std_string(std::string &result, bool utf8 = true,
                           bool substitute_out_of_range = true) const
        {
            result = to_std_string(utf8, substitute_out_of_range);
        }

        ST_DEPRECATED_IN_3_0("use to_std_string(std::string &, bool, bool) instead")
        void to_std_string(std::string &result, bool utf8,
                           utf_validation_t validation) const
        {
            return to_std_string(result, utf8, validation == substitute_invalid);
        }

        ST_NODISCARD
        std::wstring to_std_wstring() const
        {
            return to_wchar().to_std_string();
        }

        void to_std_string(std::wstring &result) const
        {
            result = to_std_wstring();
        }

        ST_NODISCARD
        std::u16string to_std_u16string() const
        {
            return to_utf16().to_std_string();
        }

        void to_std_string(std::u16string &result) const
        {
            result = to_std_u16string();
        }

        ST_NODISCARD
        std::u32string to_std_u32string() const
        {
            return to_utf32().to_std_string();
        }

        void to_std_string(std::u32string &result) const
        {
            result = to_std_u32string();
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        ST_NODISCARD
        std::u8string to_std_u8string() const
        {
            return std::u8string(u8_str(), size());
        }

        void to_std_string(std::u8string &result) const
        {
            result = to_std_u8string();
        }
#endif

#if defined(ST_HAVE_CXX17_STRING_VIEW)
        ST_NODISCARD
        std::string_view view(size_t start = 0, size_t length = ST_AUTO_SIZE) const
        {
            return m_buffer.view(start, length);
        }
#endif

#endif // defined(ST_ENABLE_STL_STRINGS)

#if defined(ST_ENABLE_STL_FILESYSTEM) && defined(ST_HAVE_CXX17_FILESYSTEM)
        ST_NODISCARD
        std::filesystem::path to_path() const
        {
#if defined(ST_HAVE_CXX20_U8_FSPATH)
            return std::filesystem::path(u8_str(), u8_str() + size());
#else
            return std::filesystem::u8path(c_str(), c_str() + size());
#endif
        }
#endif // defined(ST_ENABLE_STL_FILESYSTEM) && defined(ST_HAVE_CXX17_FILESYSTEM)

        ST_NODISCARD
        size_t size() const noexcept { return m_buffer.size(); }

        ST_NODISCARD
        bool empty() const noexcept { return m_buffer.empty(); }

        ST_NODISCARD
        static string from_int(short value, int base = 10, bool upper_case = false)
        {
            return from_validated(_ST_PRIVATE::mini_format_int_s<short>(base, upper_case, value));
        }

        ST_NODISCARD
        static string from_int(int value, int base = 10, bool upper_case = false)
        {
            return from_validated(_ST_PRIVATE::mini_format_int_s<int>(base, upper_case, value));
        }

        ST_NODISCARD
        static string from_int(long value, int base = 10, bool upper_case = false)
        {
            return from_validated(_ST_PRIVATE::mini_format_int_s<long>(base, upper_case, value));
        }

        ST_NODISCARD
        static string from_int(long long value, int base = 10, bool upper_case = false)
        {
            return from_validated(_ST_PRIVATE::mini_format_int_s<long long>(base, upper_case, value));
        }

        ST_NODISCARD
        static string from_uint(unsigned short value, int base = 10, bool upper_case = false)
        {
            return from_validated(_ST_PRIVATE::mini_format_int_u<unsigned short>(base, upper_case, value));
        }

        ST_NODISCARD
        static string from_uint(unsigned int value, int base = 10, bool upper_case = false)
        {
            return from_validated(_ST_PRIVATE::mini_format_int_u<unsigned int>(base, upper_case, value));
        }

        ST_NODISCARD
        static string from_uint(unsigned long value, int base = 10, bool upper_case = false)
        {
            return from_validated(_ST_PRIVATE::mini_format_int_u<unsigned long>(base, upper_case, value));
        }

        ST_NODISCARD
        static string from_uint(unsigned long long value, int base = 10, bool upper_case = false)
        {
            return from_validated(_ST_PRIVATE::mini_format_int_u<unsigned long long>(base, upper_case, value));
        }

        ST_NODISCARD
        static string from_float(float value, char format='g')
        {
            return from_validated(_ST_PRIVATE::mini_format_float<float>(value, format));
        }

        ST_NODISCARD
        static string from_double(double value, char format='g')
        {
            return from_validated(_ST_PRIVATE::mini_format_float<double>(value, format));
        }

        ST_NODISCARD
        static string from_float(double value, char format='g')
        {
            return from_double(value, format);
        }

#ifdef ST_HAVE_INT64
        ST_NODISCARD
        ST_DEPRECATED_IN_4_0("use from_int(long / long long) instead")
        static string from_int64(int64_t value, int base = 10, bool upper_case = false)
        {
            return from_validated(_ST_PRIVATE::mini_format_int_s<int64_t>(base, upper_case, value));
        }

        ST_NODISCARD
        ST_DEPRECATED_IN_4_0("use from_uint(unsigned long / unsigned long long) instead")
        static string from_uint64(uint64_t value, int base = 10, bool upper_case = false)
        {
            return from_validated(_ST_PRIVATE::mini_format_int_u<uint64_t>(base, upper_case, value));
        }
#endif

        ST_NODISCARD
        static string from_bool(bool value)
        {
            return value ? from_literal("true", 4)
                         : from_literal("false", 5);
        }

        ST_NODISCARD
        long to_long(int base = 0) const noexcept
        {
            return strtol(c_str(), nullptr, base);
        }

        ST_NODISCARD
        long to_long(conversion_result &result, int base = 0) const noexcept
        {
            if (empty()) {
                result.m_flags = ST::conversion_result::result_full_match;
                return 0;
            }

            char *endp;
            long value = strtol(c_str(), &endp, base);
            result.m_flags = 0;
            if (endp != c_str())
                result.m_flags |= ST::conversion_result::result_ok;
            if (endp == c_str() + size())
                result.m_flags |= ST::conversion_result::result_full_match;
            return value;
        }

        ST_NODISCARD
        long long to_long_long(int base = 0) const noexcept
        {
            return strtoll(c_str(), nullptr, base);
        }

        ST_NODISCARD
        long long to_long_long(conversion_result &result, int base = 0) const noexcept
        {
            if (empty()) {
                result.m_flags = ST::conversion_result::result_full_match;
                return 0;
            }

            char *endp;
            long long value = strtoll(c_str(), &endp, base);
            result.m_flags = 0;
            if (endp != c_str())
                result.m_flags |= ST::conversion_result::result_ok;
            if (endp == c_str() + size())
                result.m_flags |= ST::conversion_result::result_full_match;
            return value;
        }

        ST_NODISCARD
        short to_short(int base = 0) const noexcept
        {
            return static_cast<short>(to_long(base));
        }

        ST_NODISCARD
        short to_short(conversion_result &result, int base = 0) const noexcept
        {
            return static_cast<short>(to_long(result, base));
        }

        ST_NODISCARD
        int to_int(int base = 0) const noexcept
        {
            return static_cast<int>(to_long(base));
        }

        ST_NODISCARD
        int to_int(conversion_result &result, int base = 0) const noexcept
        {
            return static_cast<int>(to_long(result, base));
        }

        ST_NODISCARD
        unsigned long to_ulong(int base = 0) const noexcept
        {
            return strtoul(c_str(), nullptr, base);
        }

        ST_NODISCARD
        unsigned long to_ulong(conversion_result &result, int base = 0) const noexcept
        {
            if (empty()) {
                result.m_flags = ST::conversion_result::result_full_match;
                return 0;
            }

            char *endp;
            unsigned long value = strtoul(c_str(), &endp, base);
            result.m_flags = 0;
            if (endp != c_str())
                result.m_flags |= ST::conversion_result::result_ok;
            if (endp == c_str() + size())
                result.m_flags |= ST::conversion_result::result_full_match;
            return value;
        }

        ST_NODISCARD
        unsigned long long to_ulong_long(int base = 0) const noexcept
        {
            return strtoull(c_str(), nullptr, base);
        }

        ST_NODISCARD
        unsigned long long to_ulong_long(conversion_result &result, int base = 0) const noexcept
        {
            if (empty()) {
                result.m_flags = ST::conversion_result::result_full_match;
                return 0;
            }

            char *endp;
            unsigned long long value = strtoull(c_str(), &endp, base);
            result.m_flags = 0;
            if (endp != c_str())
                result.m_flags |= ST::conversion_result::result_ok;
            if (endp == c_str() + size())
                result.m_flags |= ST::conversion_result::result_full_match;
            return value;
        }

        ST_NODISCARD
        unsigned short to_ushort(int base = 0) const noexcept
        {
            return static_cast<unsigned short>(to_ulong(base));
        }

        ST_NODISCARD
        unsigned short to_ushort(conversion_result &result, int base = 0) const noexcept
        {
            return static_cast<unsigned short>(to_ulong(result, base));
        }

        ST_NODISCARD
        unsigned int to_uint(int base = 0) const noexcept
        {
            return static_cast<unsigned int>(to_ulong(base));
        }

        ST_NODISCARD
        unsigned int to_uint(conversion_result &result, int base = 0) const noexcept
        {
            return static_cast<unsigned int>(to_ulong(result, base));
        }

        ST_NODISCARD
        float to_float() const noexcept
        {
            return static_cast<float>(strtof(c_str(), nullptr));
        }

        ST_NODISCARD
        float to_float(conversion_result &result) const noexcept
        {
            if (empty()) {
                result.m_flags = ST::conversion_result::result_full_match;
                return 0;
            }

            char *endp;
            float value = strtof(c_str(), &endp);
            result.m_flags = 0;
            if (endp != c_str())
                result.m_flags |= ST::conversion_result::result_ok;
            if (endp == c_str() + size())
                result.m_flags |= ST::conversion_result::result_full_match;
            return value;
        }

        ST_NODISCARD
        double to_double() const noexcept
        {
            return strtod(c_str(), nullptr);
        }

        ST_NODISCARD
        double to_double(conversion_result &result) const noexcept
        {
            if (empty()) {
                result.m_flags = ST::conversion_result::result_full_match;
                return 0;
            }

            char *endp;
            double value = strtod(c_str(), &endp);
            result.m_flags = 0;
            if (endp != c_str())
                result.m_flags |= ST::conversion_result::result_ok;
            if (endp == c_str() + size())
                result.m_flags |= ST::conversion_result::result_full_match;
            return value;
        }

#ifdef ST_HAVE_INT64
        ST_NODISCARD
        ST_DEPRECATED_IN_4_0("Use to_long() or to_long_long() instead")
        int64_t to_int64(int base = 0) const noexcept
        {
            return static_cast<int64_t>(to_long_long(base));
        }

        ST_NODISCARD
        ST_DEPRECATED_IN_4_0("Use to_long() or to_long_long() instead")
        int64_t to_int64(conversion_result &result, int base = 0) const noexcept
        {
            return static_cast<int64_t>(to_long_long(result, base));
        }

        ST_NODISCARD
        ST_DEPRECATED_IN_4_0("Use to_ulong() or to_ulong_long() instead")
        uint64_t to_uint64(int base = 0) const noexcept
        {
            return static_cast<uint64_t>(to_ulong_long(base));
        }

        ST_NODISCARD
        ST_DEPRECATED_IN_4_0("Use to_ulong() or to_ulong_long() instead")
        uint64_t to_uint64(conversion_result &result, int base = 0) const noexcept
        {
            return static_cast<uint64_t>(to_ulong_long(result, base));
        }
#endif

        ST_NODISCARD
        bool to_bool() const noexcept
        {
            if (compare_i("true") == 0)
                return true;
            else if (compare_i("false") == 0)
                return false;
            return to_int() != 0;
        }

        ST_NODISCARD
        bool to_bool(conversion_result &result) const noexcept
        {
            if (compare_i("true") == 0) {
                result.m_flags = ST::conversion_result::result_ok
                               | ST::conversion_result::result_full_match;
                return true;
            } else if (compare_i("false") == 0) {
                result.m_flags = ST::conversion_result::result_ok
                               | ST::conversion_result::result_full_match;
                return false;
            }
            return to_int(result) != 0;
        }

        ST_NODISCARD
        int compare(const string &str, case_sensitivity_t cs = case_sensitive)
            const noexcept
        {
            return (cs == case_sensitive)
                    ? _ST_PRIVATE::compare_cs(c_str(), size(), str.c_str(), str.size())
                    : _ST_PRIVATE::compare_ci(c_str(), size(), str.c_str(), str.size());
        }

        ST_NODISCARD
        int compare(const char *str, case_sensitivity_t cs = case_sensitive)
            const noexcept
        {
            const size_t rsize = str ? std::char_traits<char>::length(str) : 0;
            return (cs == case_sensitive)
                    ? _ST_PRIVATE::compare_cs(c_str(), size(), str ? str : "", rsize)
                    : _ST_PRIVATE::compare_ci(c_str(), size(), str ? str : "", rsize);
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        ST_NODISCARD
        int compare(const char8_t *str, case_sensitivity_t cs = case_sensitive)
            const noexcept
        {
            return compare(reinterpret_cast<const char *>(str), cs);
        }
#endif

        ST_NODISCARD
        int compare_n(const string &str, size_t count,
                      case_sensitivity_t cs = case_sensitive) const noexcept
        {
            return (cs == case_sensitive)
                    ? _ST_PRIVATE::compare_cs(c_str(), size(), str.c_str(), str.size(), count)
                    : _ST_PRIVATE::compare_ci(c_str(), size(), str.c_str(), str.size(), count);
        }

        ST_NODISCARD
        int compare_n(const char *str, size_t count,
                      case_sensitivity_t cs = case_sensitive) const noexcept
        {
            const size_t rsize = str ? std::char_traits<char>::length(str) : 0;
            return (cs == case_sensitive)
                    ? _ST_PRIVATE::compare_cs(c_str(), size(), str ? str : "", rsize, count)
                    : _ST_PRIVATE::compare_ci(c_str(), size(), str ? str : "", rsize, count);
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        ST_NODISCARD
        int compare_n(const char8_t *str, size_t count,
                      case_sensitivity_t cs = case_sensitive) const noexcept
        {
            return compare_n(reinterpret_cast<const char *>(str), count, cs);
        }
#endif

        ST_NODISCARD
        int compare_i(const string &str) const noexcept
        {
            return compare(str, case_insensitive);
        }

        ST_NODISCARD
        int compare_i(const char *str) const noexcept
        {
            return compare(str, case_insensitive);
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        ST_NODISCARD
        int compare_i(const char8_t *str) const noexcept
        {
            return compare(str, case_insensitive);
        }
#endif

        ST_NODISCARD
        int compare_ni(const string &str, size_t count) const noexcept
        {
            return compare_n(str, count, case_insensitive);
        }

        ST_NODISCARD
        int compare_ni(const char *str, size_t count) const noexcept
        {
            return compare_n(str, count, case_insensitive);
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        ST_NODISCARD
        int compare_ni(const char8_t *str, size_t count) const noexcept
        {
            return compare_n(str, count ,case_insensitive);
        }
#endif

        ST_NODISCARD
        bool operator<(const string &other) const noexcept
        {
            return compare(other) < 0;
        }

        ST_NODISCARD
        ST_DEPRECATED_IN_3_4("Use empty() instead")
        bool operator==(const null_t &) const noexcept
        {
            return empty();
        }

        ST_NODISCARD
        bool operator==(const string &other) const noexcept
        {
            return compare(other) == 0;
        }

        ST_NODISCARD
        bool operator==(const char *other) const noexcept
        {
            return compare(other) == 0;
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        ST_NODISCARD
        bool operator==(const char8_t *other) const noexcept
        {
            return compare(other) == 0;
        }
#endif

        ST_NODISCARD
        ST_DEPRECATED_IN_3_4("Use !empty() instead")
        bool operator!=(const null_t &) const noexcept
        {
            return !empty();
        }

        ST_NODISCARD
        bool operator!=(const string &other) const noexcept
        {
            return compare(other) != 0;
        }

        ST_NODISCARD
        bool operator!=(const char *other) const noexcept
        {
            return compare(other) != 0;
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        ST_NODISCARD
        bool operator!=(const char8_t *other) const noexcept
        {
            return compare(other) != 0;
        }
#endif

        ST_NODISCARD
        ST_ssize_t find(char ch, case_sensitivity_t cs = case_sensitive)
            const noexcept
        {
            return find(0, ch, cs);
        }

        ST_NODISCARD
        ST_ssize_t find(const char *substr, case_sensitivity_t cs = case_sensitive)
            const noexcept
        {
            return find(0, substr, cs);
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        ST_NODISCARD
        ST_ssize_t find(const char8_t *substr, case_sensitivity_t cs = case_sensitive)
            const noexcept
        {
            return find(0, substr, cs);
        }
#endif

        ST_NODISCARD
        ST_ssize_t find(const string &substr, case_sensitivity_t cs = case_sensitive)
            const noexcept
        {
            return find(0, substr.c_str(), cs);
        }

        ST_NODISCARD
        ST_ssize_t find(size_t start, char ch, case_sensitivity_t cs = case_sensitive)
            const noexcept
        {
            if (start >= size())
                return -1;

            const char *cp = (cs == case_sensitive)
                    ? _ST_PRIVATE::find_cs(c_str() + start, size() - start, ch)
                    : _ST_PRIVATE::find_ci(c_str() + start, size() - start, ch);
            return cp ? (cp - c_str()) : -1;
        }

        ST_NODISCARD
        ST_ssize_t find(size_t start, const char *substr, case_sensitivity_t cs = case_sensitive)
            const noexcept
        {
            if (!substr || !substr[0] || start >= size())
                return -1;

            const char *cp = (cs == case_sensitive)
                    ? _ST_PRIVATE::find_cs(c_str() + start, substr)
                    : _ST_PRIVATE::find_ci(c_str() + start, substr);
            return cp ? (cp - c_str()) : -1;
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        ST_NODISCARD
        ST_ssize_t find(size_t start, const char8_t *substr, case_sensitivity_t cs = case_sensitive)
            const noexcept
        {
            return find(start, reinterpret_cast<const char *>(substr), cs);
        }
#endif

        ST_NODISCARD
        ST_ssize_t find(size_t start, const string &substr, case_sensitivity_t cs = case_sensitive)
            const noexcept
        {
            return find(start, substr.c_str(), cs);
        }

        ST_NODISCARD
        ST_ssize_t find_last(char ch, case_sensitivity_t cs = case_sensitive)
            const noexcept
        {
            return find_last(ST_AUTO_SIZE, ch, cs);
        }

        ST_NODISCARD
        ST_ssize_t find_last(const char *substr, case_sensitivity_t cs = case_sensitive)
            const noexcept
        {
            return find_last(ST_AUTO_SIZE, substr, cs);
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        ST_NODISCARD
        ST_ssize_t find_last(const char8_t *substr, case_sensitivity_t cs = case_sensitive)
            const noexcept
        {
            return find_last(ST_AUTO_SIZE, substr, cs);
        }
#endif

        ST_NODISCARD
        ST_ssize_t find_last(const string &substr, case_sensitivity_t cs = case_sensitive)
            const noexcept
        {
            return find_last(ST_AUTO_SIZE, substr.c_str(), cs);
        }

        ST_NODISCARD
        ST_ssize_t find_last(size_t max, char ch, case_sensitivity_t cs = case_sensitive)
            const noexcept
        {
            if (empty())
                return -1;

            const char *endp = c_str() + (max > size() ? size() : max);

            const char *start = c_str();
            const char *found = nullptr;
            for ( ;; ) {
                const char *cp = (cs == case_sensitive)
                        ? _ST_PRIVATE::find_cs(start, endp - start, ch)
                        : _ST_PRIVATE::find_ci(start, endp - start, ch);
                if (!cp || cp >= endp)
                    break;
                found = cp;
                start = cp + 1;
            }
            return found ? (found - c_str()) : -1;
        }

        ST_NODISCARD
        ST_ssize_t find_last(size_t max, const char *substr, case_sensitivity_t cs = case_sensitive)
            const noexcept
        {
            if (!substr || !substr[0] || empty())
                return -1;

            const char *endp = c_str() + (max > size() ? size() : max);

            const char *start = c_str();
            const char *found = nullptr;
            for ( ;; ) {
                const char *cp = (cs == case_sensitive)
                        ? _ST_PRIVATE::find_cs(start, substr)
                        : _ST_PRIVATE::find_ci(start, substr);
                if (!cp || cp >= endp)
                    break;
                found = cp;
                start = cp + 1;
            }
            return found ? (found - c_str()) : -1;
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        ST_NODISCARD
        ST_ssize_t find_last(size_t max, const char8_t *substr, case_sensitivity_t cs = case_sensitive)
            const noexcept
        {
            return find_last(max, reinterpret_cast<const char *>(substr), cs);
        }
#endif

        ST_NODISCARD
        ST_ssize_t find_last(size_t max, const string &substr, case_sensitivity_t cs = case_sensitive)
            const noexcept
        {
            return find_last(max, substr.c_str(), cs);
        }

        ST_NODISCARD
        bool contains(char ch, case_sensitivity_t cs = case_sensitive)
            const noexcept
        {
            return find(ch, cs) >= 0;
        }

        ST_NODISCARD
        bool contains(const char *substr, case_sensitivity_t cs = case_sensitive)
            const noexcept
        {
            return find(substr, cs) >= 0;
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        ST_NODISCARD
        bool contains(const char8_t *substr, case_sensitivity_t cs = case_sensitive)
            const noexcept
        {
            return find(substr, cs) >= 0;
        }
#endif

        ST_NODISCARD
        bool contains(const string &substr, case_sensitivity_t cs = case_sensitive)
            const noexcept
        {
            return find(substr, cs) >= 0;
        }

        ST_NODISCARD
        string trim_left(const char *charset = ST_WHITESPACE) const
        {
            if (empty())
                return string();

            const char *cp = c_str();
            size_t cssize = std::char_traits<char>::length(charset);
            while (*cp && _ST_PRIVATE::find_cs(charset, cssize, *cp))
                ++cp;

            return substr(cp - c_str());
        }

        ST_NODISCARD
        string trim_right(const char *charset = ST_WHITESPACE) const
        {
            if (empty())
                return string();

            const char *cp = c_str() + size();
            size_t cssize = std::char_traits<char>::length(charset);
            while (--cp >= c_str() && _ST_PRIVATE::find_cs(charset, cssize, *cp))
                ;

            return substr(0, cp - c_str() + 1);
        }

        ST_NODISCARD
        string trim(const char *charset = ST_WHITESPACE) const
        {
            if (empty())
                return string();

            const char *lp = c_str();
            const char *rp = lp + size();
            size_t cssize = std::char_traits<char>::length(charset);
            while (*lp && _ST_PRIVATE::find_cs(charset, cssize, *lp))
                ++lp;
            while (--rp >= lp && _ST_PRIVATE::find_cs(charset, cssize, *rp))
                ;

            return substr(lp - c_str(), rp - lp + 1);
        }

        ST_NODISCARD
        string substr(ST_ssize_t start, size_t count = ST_AUTO_SIZE) const
        {
            size_t max = size();

            if (count == ST_AUTO_SIZE)
                count = max;

            if (start < 0) {
                // Handle negative indexes from the right side of the string
                start += max;
                if (start < 0)
                    start = 0;
            } else if (static_cast<size_t>(start) > max) {
                return string();
            }
            if (start + count > max)
                count = max - start;

            if (start == 0 && count == max)
                return *this;

            string sub;
            sub.m_buffer.allocate(count);
            std::char_traits<char>::copy(sub.m_buffer.data(), c_str() + start, count);

            return sub;
        }

        ST_NODISCARD
        string left(size_t size) const
        {
            return substr(0, size);
        }

        ST_NODISCARD
        string right(size_t size) const
        {
            return substr(this->size() - size, size);
        }

        ST_NODISCARD
        bool starts_with(const string &prefix, case_sensitivity_t cs = case_sensitive) const noexcept
        {
            if (prefix.size() > size())
                return false;
            return compare_n(prefix, prefix.size(), cs) == 0;
        }

        ST_NODISCARD
        bool starts_with(const char *prefix, case_sensitivity_t cs = case_sensitive) const noexcept
        {
            size_t count = prefix ? std::char_traits<char>::length(prefix) : 0;
            if (count > size())
                return false;
            return compare_n(prefix, count, cs) == 0;
        }

        ST_NODISCARD
        bool ends_with(const string &suffix, case_sensitivity_t cs = case_sensitive) const noexcept
        {
            if (suffix.size() > size())
                return false;

            size_t start = size() - suffix.size();
            return (cs == case_sensitive)
                    ? _ST_PRIVATE::compare_cs(c_str() + start, suffix.c_str(), suffix.size()) == 0
                    : _ST_PRIVATE::compare_ci(c_str() + start, suffix.c_str(), suffix.size()) == 0;
        }

        ST_NODISCARD
        bool ends_with(const char *suffix, case_sensitivity_t cs = case_sensitive) const noexcept
        {
            size_t count = suffix ? std::char_traits<char>::length(suffix) : 0;
            if (count > size())
                return false;

            size_t start = size() - count;
            return (cs == case_sensitive)
                    ? _ST_PRIVATE::compare_cs(c_str() + start, suffix ? suffix : "", count) == 0
                    : _ST_PRIVATE::compare_ci(c_str() + start, suffix ? suffix : "", count) == 0;
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        ST_NODISCARD
        bool starts_with(const char8_t *prefix, case_sensitivity_t cs = case_sensitive)
            const noexcept
        {
            return starts_with(reinterpret_cast<const char *>(prefix), cs);
        }

        ST_NODISCARD
        bool ends_with(const char8_t *suffix, case_sensitivity_t cs = case_sensitive)
            const noexcept
        {
            return ends_with(reinterpret_cast<const char *>(suffix), cs);
        }
#endif

        ST_NODISCARD
        string before_first(char sep, case_sensitivity_t cs = case_sensitive) const
        {
            ST_ssize_t first = find(sep, cs);
            if (first >= 0)
                return left(first);
            else
                return *this;
        }

        ST_NODISCARD
        string before_first(const char *sep, case_sensitivity_t cs = case_sensitive) const
        {
            ST_ssize_t first = find(sep, cs);
            if (first >= 0)
                return left(first);
            else
                return *this;
        }

        ST_NODISCARD
        string before_first(const string &sep, case_sensitivity_t cs = case_sensitive) const
        {
            ST_ssize_t first = find(sep, cs);
            if (first >= 0)
                return left(first);
            else
                return *this;
        }

        ST_NODISCARD
        string after_first(char sep, case_sensitivity_t cs = case_sensitive) const
        {
            ST_ssize_t first = find(sep, cs);
            if (first >= 0)
                return substr(first + 1);
            else
                return string();
        }

        ST_NODISCARD
        string after_first(const char *sep, case_sensitivity_t cs = case_sensitive) const
        {
            ST_ssize_t first = find(sep, cs);
            if (first >= 0)
                return substr(first + std::char_traits<char>::length(sep));
            else
                return string();
        }

        ST_NODISCARD
        string after_first(const string &sep, case_sensitivity_t cs = case_sensitive) const
        {
            ST_ssize_t first = find(sep, cs);
            if (first >= 0)
                return substr(first + 1);
            else
                return string();
        }

        ST_NODISCARD
        string before_last(char sep, case_sensitivity_t cs = case_sensitive) const
        {
            ST_ssize_t last = find_last(sep, cs);
            if (last >= 0)
                return left(last);
            else
                return string();
        }

        ST_NODISCARD
        string before_last(const char *sep, case_sensitivity_t cs = case_sensitive) const
        {
            ST_ssize_t last = find_last(sep, cs);
            if (last >= 0)
                return left(last);
            else
                return string();
        }

        ST_NODISCARD
        string before_last(const string &sep, case_sensitivity_t cs = case_sensitive) const
        {
            ST_ssize_t last = find_last(sep, cs);
            if (last >= 0)
                return left(last);
            else
                return string();
        }

        ST_NODISCARD
        string after_last(char sep, case_sensitivity_t cs = case_sensitive) const
        {
            ST_ssize_t last = find_last(sep, cs);
            if (last >= 0)
                return substr(last + 1);
            else
                return *this;
        }

        ST_NODISCARD
        string after_last(const char *sep, case_sensitivity_t cs = case_sensitive) const
        {
            ST_ssize_t last = find_last(sep, cs);
            if (last >= 0)
                return substr(last + std::char_traits<char>::length(sep));
            else
                return *this;
        }

        ST_NODISCARD
        string after_last(const string &sep, case_sensitivity_t cs = case_sensitive) const
        {
            ST_ssize_t last = find_last(sep, cs);
            if (last >= 0)
                return substr(last + 1);
            else
                return *this;
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        ST_NODISCARD
        string before_first(const char8_t *sep, case_sensitivity_t cs = case_sensitive) const
        {
            return before_first(reinterpret_cast<const char *>(sep), cs);
        }

        ST_NODISCARD
        string after_first(const char8_t *sep, case_sensitivity_t cs = case_sensitive) const
        {
            return after_first(reinterpret_cast<const char *>(sep), cs);
        }

        ST_NODISCARD
        string before_last(const char8_t *sep, case_sensitivity_t cs = case_sensitive) const
        {
            return before_last(reinterpret_cast<const char *>(sep), cs);
        }

        ST_NODISCARD
        string after_last(const char8_t *sep, case_sensitivity_t cs = case_sensitive) const
        {
            return after_last(reinterpret_cast<const char *>(sep), cs);
        }
#endif

        ST_NODISCARD
        string replace(const char *from, const char *to,
                       case_sensitivity_t cs = case_sensitive,
                       utf_validation_t validation = ST_DEFAULT_VALIDATION) const
        {
            return replace(from ? string(from, ST_AUTO_SIZE, validation) : string(),
                           to ? string(to, ST_AUTO_SIZE, validation) : string(), cs);
        }

        ST_NODISCARD
        string replace(const string &from, const char *to,
                       case_sensitivity_t cs = case_sensitive,
                       utf_validation_t validation = ST_DEFAULT_VALIDATION) const
        {
            return replace(from, to ? string(to, ST_AUTO_SIZE, validation) : string(), cs);
        }

        ST_NODISCARD
        string replace(const char *from, const string &to,
                       case_sensitivity_t cs = case_sensitive,
                       utf_validation_t validation = ST_DEFAULT_VALIDATION) const
        {
            return replace(from ? string(from, ST_AUTO_SIZE, validation) : string(), to, cs);
        }

        ST_NODISCARD
        string replace(const string &from, const string &to,
                       case_sensitivity_t cs = case_sensitive) const
        {
            if (empty() || from.empty())
                return *this;

            size_t outsize = size();
            const char *pstart = c_str();
            const char *pnext;
            const char *pend = pstart + size();
            if (from.size() != to.size()) {
                for ( ;; ) {
                    pnext = (cs == case_sensitive)
                            ? _ST_PRIVATE::find_cs(pstart, from.c_str())
                            : _ST_PRIVATE::find_ci(pstart, from.c_str());
                    if (!pnext)
                        break;

                    outsize += to.size() - from.size();
                    pstart = pnext + from.size();
                }
            }

            ST::char_buffer result;
            result.allocate(outsize);
            char *out = result.data();
            pstart = c_str();
            for ( ;; ) {
                pnext = (cs == case_sensitive)
                        ? _ST_PRIVATE::find_cs(pstart, from.c_str())
                        : _ST_PRIVATE::find_ci(pstart, from.c_str());
                if (!pnext)
                    break;

                std::char_traits<char>::copy(out, pstart, pnext - pstart);
                out += pnext - pstart;
                std::char_traits<char>::copy(out, to.c_str(), to.size());
                out += to.size();
                pstart = pnext + from.size();
            }

            if (pstart < pend)
                std::char_traits<char>::copy(out, pstart, pend - pstart);

            return result;
        }

        ST_NODISCARD
        ST_DEPRECATED_IN_3_0("validation parameter is not used in replace(ST::string, ST::string) override")
        string replace(const string &from, const string &to,
                       case_sensitivity_t cs, utf_validation_t validation) const
        {
            (void)validation;
            return replace(from, to, cs);
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        ST_NODISCARD
        string replace(const char8_t *from, const char8_t *to,
                       case_sensitivity_t cs = case_sensitive,
                       utf_validation_t validation = ST_DEFAULT_VALIDATION) const
        {
            return replace(from ? string(from, ST_AUTO_SIZE, validation) : string(),
                           to ? string(to, ST_AUTO_SIZE, validation) : string(), cs);
        }

        ST_NODISCARD
        string replace(const string &from, const char8_t *to,
                       case_sensitivity_t cs = case_sensitive,
                       utf_validation_t validation = ST_DEFAULT_VALIDATION) const
        {
            return replace(from, to ? string(to, ST_AUTO_SIZE, validation) : string(), cs);
        }

        ST_NODISCARD
        string replace(const char8_t *from, const string &to,
                       case_sensitivity_t cs = case_sensitive,
                       utf_validation_t validation = ST_DEFAULT_VALIDATION)
        {
            return replace(from ? string(from, ST_AUTO_SIZE, validation) : string(), to, cs);
        }
#endif

        ST_NODISCARD
        string to_upper() const
        {
            string result;
            result.m_buffer.allocate(size());
            char *dupe = result.m_buffer.data();
            const char *sp = c_str();
            const char *ep = sp + size();
            char *dp = dupe;
            while (sp < ep)
                *dp++ = _ST_PRIVATE::cl_fast_upper(*sp++);

            return result;
        }

        ST_NODISCARD
        string to_lower() const
        {
            string result;
            result.m_buffer.allocate(size());
            char *dupe = result.m_buffer.data();
            const char *sp = c_str();
            const char *ep = sp + size();
            char *dp = dupe;
            while (sp < ep)
                *dp++ = _ST_PRIVATE::cl_fast_lower(*sp++);

            return result;
        }

        ST_NODISCARD
        std::vector<string> split(char split_char, size_t max_splits = ST_AUTO_SIZE,
                                  case_sensitivity_t cs = case_sensitive) const
        {
            ST_ASSERT(split_char && static_cast<unsigned int>(split_char) < 0x80,
                      "Split character should be in range '\\x01'-'\\x7f'");

            std::vector<string> result;

            const char *next = c_str();
            const char *endp = next + size();
            while (max_splits) {
                const char *sp = (cs == case_sensitive)
                        ? _ST_PRIVATE::find_cs(next, endp - next, split_char)
                        : _ST_PRIVATE::find_ci(next, endp - next, split_char);
                if (!sp)
                    break;

                result.emplace_back(string::from_validated(next, sp - next));
                next = sp + 1;
                --max_splits;
            }

            result.emplace_back(string::from_validated(next, endp - next));
            return result;
        }

        ST_NODISCARD
        std::vector<string> split(const char *splitter,
                                  size_t max_splits = ST_AUTO_SIZE,
                                  case_sensitivity_t cs = case_sensitive) const
        {
            ST_ASSERT(splitter, "ST::string::split called with null splitter");

            std::vector<string> result;
            if (!splitter)
                return result;

            // Performance improvement when splitter is "safe"
            utf_validation_t validation = assume_valid;
            const char *cp = splitter;
            while (*cp) {
                if (*cp & 0x80) {
                    validation = check_validity;
                    break;
                }
                ++cp;
            }

            const char *next = c_str();
            const char *endp = next + size();
            size_t splitlen = std::char_traits<char>::length(splitter);
            while (max_splits) {
                const char *sp = (cs == case_sensitive)
                        ? _ST_PRIVATE::find_cs(next, splitter)
                        : _ST_PRIVATE::find_ci(next, splitter);
                if (!sp)
                    break;

                result.emplace_back(next, sp - next, validation);
                next = sp + splitlen;
                --max_splits;
            }

            result.emplace_back(next, endp - next, validation);
            return result;
        }

        ST_NODISCARD
        std::vector<string> split(const string &splitter,
                                  size_t max_splits = ST_AUTO_SIZE,
                                  case_sensitivity_t cs = case_sensitive) const
        {
            std::vector<string> result;

            const char *next = c_str();
            const char *endp = next + size();
            while (max_splits) {
                const char *sp = (cs == case_sensitive)
                        ? _ST_PRIVATE::find_cs(next, splitter.c_str())
                        : _ST_PRIVATE::find_ci(next, splitter.c_str());
                if (!sp)
                    break;

                result.push_back(string::from_validated(next, sp - next));
                next = sp + splitter.size();
                --max_splits;
            }

            result.push_back(string::from_validated(next, endp - next));
            return result;
        }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
        ST_NODISCARD
        std::vector<string> split(const char8_t *splitter,
                                  size_t max_splits = ST_AUTO_SIZE,
                                  case_sensitivity_t cs = case_sensitive) const
        {
            return split(reinterpret_cast<const char *>(splitter), max_splits, cs);
        }
#endif

        ST_NODISCARD
        std::vector<string> tokenize(const char *delims = ST_WHITESPACE) const
        {
            std::vector<string> result;

            const char *next = c_str();
            const char *endp = next + size();
            size_t dsize = std::char_traits<char>::length(delims);
            while (next != endp) {
                const char *cur = next;
                while (cur != endp && !_ST_PRIVATE::find_cs(delims, dsize, *cur))
                    ++cur;

                // Found a delimiter
                if (cur != next)
                    result.emplace_back(string::from_validated(next, cur - next));

                next = cur;
                while (next != endp && _ST_PRIVATE::find_cs(delims, dsize, *next))
                    ++next;
            }

            return result;
        }

        ST_NODISCARD
        static string fill(size_t count, char c)
        {
            char_buffer result(count, c);
            return result;
        }
    };

    static_assert(std::is_standard_layout<ST::string>::value,
                  "ST::string must be standard-layout to pass across the DLL boundary");

    struct hash
    {
        ST_NODISCARD
        size_t operator()(const string &str) const noexcept
        {
            /* FNV-1a hash.  See http://isthe.com/chongo/tech/comp/fnv/ for details */
            size_t hash = _ST_PRIVATE::fnv_constants<size_t>::offset_basis;
            const char *cp = str.c_str();
            const char *ep = cp + str.size();
            while (cp < ep) {
                hash ^= static_cast<size_t>(*cp++);
                hash *= _ST_PRIVATE::fnv_constants<size_t>::prime;
            }
            return hash;
        }
    };

    struct hash_i
    {
        ST_NODISCARD
        size_t operator()(const string &str) const noexcept
        {
            /* FNV-1a hash.  See http://isthe.com/chongo/tech/comp/fnv/ for details */
            size_t hash = _ST_PRIVATE::fnv_constants<size_t>::offset_basis;
            const char *cp = str.c_str();
            const char *ep = cp + str.size();
            while (cp < ep) {
                hash ^= static_cast<size_t>(_ST_PRIVATE::cl_fast_lower(*cp++));
                hash *= _ST_PRIVATE::fnv_constants<size_t>::prime;
            }
            return hash;
        }
    };

    struct less_i
    {
        ST_NODISCARD
        bool operator()(const string &left, const string &right)
            const noexcept
        {
            return left.compare_i(right) < 0;
        }
    };

    struct equal_i
    {
        ST_NODISCARD
        bool operator()(const string &left, const string &right)
            const noexcept
        {
            return left.compare_i(right) == 0;
        }
    };

    ST_NODISCARD
    inline string operator+(const string &left, const string &right)
    {
        ST::char_buffer cat;
        cat.allocate(left.size() + right.size());
        std::char_traits<char>::copy(&cat[0], left.c_str(), left.size());
        std::char_traits<char>::copy(&cat[left.size()], right.c_str(), right.size());

        return ST::string::from_validated(std::move(cat));
    }

    ST_NODISCARD
    inline string operator+(const string &left, const char *right)
    {
        return operator+(left, string::from_utf8(right));
    }

    ST_NODISCARD
    inline string operator+(const char *left, const string &right)
    {
        return operator+(string::from_utf8(left), right);
    }

    ST_NODISCARD
    inline string operator+(const string &left, const wchar_t *right)
    {
        return operator+(left, string::from_wchar(right));
    }

    ST_NODISCARD
    inline ST::string operator+(const wchar_t *left, const string &right)
    {
        return operator+(string::from_wchar(left), right);
    }

    ST_NODISCARD
    inline string operator+(const string &left, const char16_t *right)
    {
        return operator+(left, string::from_utf16(right));
    }

    ST_NODISCARD
    inline string operator+(const char16_t *left, const string &right)
    {
        return operator+(string::from_utf16(left), right);
    }

    ST_NODISCARD
    inline string operator+(const string &left, const char32_t *right)
    {
        return operator+(left, string::from_utf32(right));
    }

    ST_NODISCARD
    inline string operator+(const char32_t *left, const string &right)
    {
        return operator+(string::from_utf32(left), right);
    }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
    ST_NODISCARD
    inline string operator+(const string &left, const char8_t *right)
    {
        return operator+(left, string::from_utf8(right));
    }

    ST_NODISCARD
    inline string operator+(const char8_t *left, const string &right)
    {
        return operator+(string::from_utf8(left), right);
    }
#endif

    ST_NODISCARD
    inline string operator+(const string &left, char32_t right)
    {
        size_t addsize = _ST_PRIVATE::utf8_measure(right);

        ST::char_buffer cat;
        cat.allocate(left.size() + addsize);
        char *catp = cat.data();
        std::char_traits<char>::copy(catp, left.c_str(), left.size());
        catp += left.size();

        auto error = _ST_PRIVATE::write_utf8(catp, right);
        _ST_PRIVATE::raise_conversion_error(error);

        return ST::string::from_validated(std::move(cat));
    }

    ST_NODISCARD
    inline string operator+(const string &left, char16_t right)
    {
        const char32_t uchar = right;
        return operator+(left, uchar);
    }

    ST_NODISCARD
    inline string operator+(const string &left, char right)
    {
        const char32_t uchar = static_cast<unsigned char>(right);
        return operator+(left, uchar);
    }

    ST_NODISCARD
    inline string operator+(const string &left, wchar_t right)
    {
        const char32_t uchar = static_cast<unsigned int>(right);
        return operator+(left, uchar);
    }

    ST_NODISCARD
    inline string operator+(char32_t left, const string &right)
    {
        size_t addsize = _ST_PRIVATE::utf8_measure(left);

        ST::char_buffer cat;
        cat.allocate(right.size() + addsize);
        char *catp = cat.data();

        auto error = _ST_PRIVATE::write_utf8(catp, left);
        _ST_PRIVATE::raise_conversion_error(error);

        std::char_traits<char>::copy(catp, right.c_str(), right.size());

        return ST::string::from_validated(std::move(cat));
    }

    ST_NODISCARD
    inline string operator+(char16_t left, const string &right)
    {
        const char32_t uchar = left;
        return operator+(uchar, right);
    }

    ST_NODISCARD
    inline string operator+(char left, const string &right)
    {
        const char32_t uchar = static_cast<unsigned char>(left);
        return operator+(uchar, right);
    }

    ST_NODISCARD
    inline string operator+(wchar_t left, const string &right)
    {
        const char32_t uchar = static_cast<unsigned int>(left);
        return operator+(uchar, right);
    }

    ST_NODISCARD
    ST_DEPRECATED_IN_3_4("Use string::empty() instead")
    inline bool operator==(const null_t &, const string &right) noexcept
    {
        return right.empty();
    }

    ST_NODISCARD
    ST_DEPRECATED_IN_3_4("Use !string::empty() instead")
    inline bool operator!=(const null_t &, const string &right) noexcept
    {
        return !right.empty();
    }
}

ST::string &ST::string::operator+=(const char *cstr)
{
    set(*this + cstr);
    return *this;
}

ST::string &ST::string::operator+=(const wchar_t *wstr)
{
    set(*this + wstr);
    return *this;
}

ST::string &ST::string::operator+=(const char16_t *cstr)
{
    set(*this + cstr);
    return *this;
}

ST::string &ST::string::operator+=(const char32_t *cstr)
{
    set(*this + cstr);
    return *this;
}

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
ST::string &ST::string::operator+=(const char8_t *cstr)
{
    set(*this + cstr);
    return *this;
}
#endif

ST::string &ST::string::operator+=(const ST::string &other)
{
    set(*this + other);
    return *this;
}

ST::string &ST::string::operator+=(char ch)
{
    set(*this + ch);
    return *this;
}

ST::string &ST::string::operator+=(char16_t ch)
{
    set(*this + ch);
    return *this;
}

ST::string &ST::string::operator+=(char32_t ch)
{
    set(*this + ch);
    return *this;
}

ST::string &ST::string::operator+=(wchar_t ch)
{
    set(*this + ch);
    return *this;
}

namespace std
{
    template <>
    struct hash<ST::string>
    {
        ST_NODISCARD
        inline size_t operator()(const ST::string &str) const noexcept
        {
            return ST::hash()(str);
        }
    };
}

#define ST_LITERAL(str) \
    ST::string::from_literal("" str "", sizeof(str) - 1)

namespace ST { namespace literals
{
    ST_NODISCARD
    inline ST::string operator"" _st(const char *str, size_t size)
    {
        return ST::string::from_literal(str, size);
    }

    ST_NODISCARD
    inline ST::string operator"" _st(const wchar_t *str, size_t size)
    {
        return ST::string::from_wchar(str, size, ST::assume_valid);
    }

    ST_NODISCARD
    inline ST::string operator"" _st(const char16_t *str, size_t size)
    {
        return ST::string::from_utf16(str, size, ST::assume_valid);
    }

    ST_NODISCARD
    inline ST::string operator"" _st(const char32_t *str, size_t size)
    {
        return ST::string::from_utf32(str, size, ST::assume_valid);
    }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
    ST_NODISCARD
    inline ST::string operator"" _st(const char8_t *str, size_t size)
    {
        return ST::string::from_literal(str, size);
    }
#endif
}}

#endif // _ST_STRING_H
