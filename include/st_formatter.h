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

#ifndef _ST_FORMATTER_H
#define _ST_FORMATTER_H

#include "st_string.h"

#include <functional>

#if !defined(ST_NO_STL_STRINGS)
#   include <string>
#   include <complex>
#endif

namespace ST
{
    enum class alignment_t
    {
        align_default,
        align_left,
        align_right
    };
    ST_ENUM_CONSTANT(alignment_t, align_default);
    ST_ENUM_CONSTANT(alignment_t, align_left);
    ST_ENUM_CONSTANT(alignment_t, align_right);

    enum class digit_class_t
    {
        digit_default,
        digit_dec,
        digit_hex,
        digit_hex_upper,
        digit_oct,
        digit_bin,
        digit_char
    };
    ST_ENUM_CONSTANT(digit_class_t, digit_default);
    ST_ENUM_CONSTANT(digit_class_t, digit_dec);
    ST_ENUM_CONSTANT(digit_class_t, digit_hex);
    ST_ENUM_CONSTANT(digit_class_t, digit_hex_upper);
    ST_ENUM_CONSTANT(digit_class_t, digit_oct);
    ST_ENUM_CONSTANT(digit_class_t, digit_bin);
    ST_ENUM_CONSTANT(digit_class_t, digit_char);

    enum class float_class_t
    {
        float_default,
        float_fixed,
        float_exp,
        float_exp_upper
    };
    ST_ENUM_CONSTANT(float_class_t, float_default);
    ST_ENUM_CONSTANT(float_class_t, float_fixed);
    ST_ENUM_CONSTANT(float_class_t, float_exp);
    ST_ENUM_CONSTANT(float_class_t, float_exp_upper);

    struct format_spec
    {
        format_spec() noexcept
            : minimum_length(), precision(-1), arg_index(-1), alignment(),
              digit_class(), float_class(), pad(), always_signed(),
              class_prefix(), numeric_pad() { }

        int minimum_length;
        int precision;
        int arg_index;
        alignment_t alignment;
        digit_class_t digit_class;
        float_class_t float_class;
        char pad;
        bool always_signed;
        bool class_prefix;
        bool numeric_pad;
    };

    static_assert(std::is_standard_layout<ST::format_spec>::value,
                  "ST::format_spec must be standard-layout to pass across the DLL boundary");

    class format_writer
    {
        ST_DISABLE_COPY(format_writer);

    public:
        format_writer(const char *format) : m_format_str(format)
        {
            if (!m_format_str)
                throw std::invalid_argument("Passed a null format string!");
        }

        virtual ~format_writer() noexcept { }

        virtual format_writer &append(const char *data, size_t size = ST_AUTO_SIZE) = 0;
        virtual format_writer &append_char(char ch, size_t count = 1) = 0;

        bool next_format()
        {
            switch (fetch_prefix()) {
            case 0:
                return false;
            case '{':
                return true;
            default:
                throw ST::bad_format("Error parsing format string");
            }
        }

        ST::format_spec parse_format()
        {
            ST_ASSERT(*m_format_str == '{', "parse_format() called with no format");

            ST::format_spec spec;
            for ( ;; ) {
                switch (*++m_format_str) {
                case 0:
                    throw ST::bad_format("Unterminated format specifier");
                case '}':
                    // Done with format spec
                    ++m_format_str;
                    return spec;

                case '<':
                    spec.alignment = ST::align_left;
                    break;
                case '>':
                    spec.alignment = ST::align_right;
                    break;
                case '_':
                    spec.pad = *(m_format_str + 1);
                    spec.numeric_pad = false;
                    if (!spec.pad)
                        throw ST::bad_format("Unterminated format specifier");
                    ++m_format_str;
                    break;
                case '0':
                    // For easier porting from %08X-style printf strings
                    spec.pad = '0';
                    spec.numeric_pad = true;
                    break;
                case '#':
                    spec.class_prefix = true;
                    break;
                case 'x':
                    spec.digit_class = ST::digit_hex;
                    break;
                case 'X':
                    spec.digit_class = ST::digit_hex_upper;
                    break;
                case '+':
                    spec.always_signed = true;
                    break;
                case 'd':
                    spec.digit_class = ST::digit_dec;
                    break;
                case 'o':
                    spec.digit_class = ST::digit_oct;
                    break;
                case 'b':
                    spec.digit_class = ST::digit_bin;
                    break;
                case 'c':
                    spec.digit_class = ST::digit_char;
                    break;
                case 'f':
                    spec.float_class = ST::float_fixed;
                    break;
                case 'e':
                    spec.float_class = ST::float_exp;
                    break;
                case 'E':
                    spec.float_class = ST::float_exp_upper;
                    break;
                case '1': case '2': case '3': case '4': case '5':
                case '6': case '7': case '8': case '9':
                {
                    char *end = nullptr;
                    spec.minimum_length = static_cast<int>(strtol(m_format_str, &end, 10));
                    m_format_str = end - 1;
                    break;
                }
                case '.':
                {
                    if (*++m_format_str == 0)
                        throw ST::bad_format("Unterminated format specifier");
                    char *end = nullptr;
                    spec.precision = static_cast<int>(strtol(m_format_str, &end, 10));
                    m_format_str = end - 1;
                    break;
                }
                case '&':
                {
                    if (*++m_format_str == 0)
                        throw ST::bad_format("Unterminated format specifier");
                    char *end = nullptr;
                    spec.arg_index = static_cast<int>(strtol(m_format_str, &end, 10));
                    m_format_str = end - 1;
                    break;
                }
                default:
                    throw ST::bad_format("Unexpected character in format string");
                }
            }
        }

    private:
        const char *m_format_str;

        ST_EXPORT char fetch_prefix();
    };

    ST_EXPORT void format_string(const format_spec &format, format_writer &output,
                                 const char *text, size_t size,
                                 alignment_t default_alignment = align_left);

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
    inline void format_string(const format_spec &format, format_writer &output,
                              const char8_t *text, size_t size,
                              alignment_t default_alignment = align_left)
    {
        format_string(format, output, reinterpret_cast<const char *>(text),
                      size, default_alignment);
    }
#endif

    typedef std::function<void(const ST::format_spec &, ST::format_writer &)>
        formatter_ref_t;

    template <typename type_T>
    formatter_ref_t make_formatter_ref(type_T value)
    {
        return [value](const ST::format_spec &format, ST::format_writer &output) {
            format_type(format, output, value);
        };
    }

    template <typename arg0_T, typename... args_T>
    void apply_format(ST::format_writer &data, arg0_T &&arg0, args_T &&...args)
    {
        enum { num_formatters = 1 + sizeof...(args) };
        formatter_ref_t formatters[num_formatters] = {
            make_formatter_ref(std::forward<arg0_T>(arg0)),
            make_formatter_ref(std::forward<args_T>(args))...
        };
        size_t index = 0;
        while (data.next_format()) {
            ST::format_spec format = data.parse_format();
            size_t formatter_id = (format.arg_index >= 0)
                                  ? format.arg_index - 1
                                  : index++;
            if (formatter_id >= num_formatters)
                throw std::out_of_range("Parameter index out of range");
            formatters[formatter_id](format, data);
        }
    }

    inline void apply_format(ST::format_writer &data)
    {
        if (data.next_format())
            throw std::out_of_range("Parameter index out of range");
    }
}

namespace ST
{
    ST_EXPORT void format_type(const ST::format_spec &, ST::format_writer &, char);
    ST_EXPORT void format_type(const ST::format_spec &, ST::format_writer &, char16_t);
    ST_EXPORT void format_type(const ST::format_spec &, ST::format_writer &, char32_t);
    ST_EXPORT void format_type(const ST::format_spec &, ST::format_writer &, signed char);
    ST_EXPORT void format_type(const ST::format_spec &, ST::format_writer &, unsigned char);
    ST_EXPORT void format_type(const ST::format_spec &, ST::format_writer &, short);
    ST_EXPORT void format_type(const ST::format_spec &, ST::format_writer &, unsigned short);
    ST_EXPORT void format_type(const ST::format_spec &, ST::format_writer &, int);
    ST_EXPORT void format_type(const ST::format_spec &, ST::format_writer &, unsigned int);
    ST_EXPORT void format_type(const ST::format_spec &, ST::format_writer &, long);
    ST_EXPORT void format_type(const ST::format_spec &, ST::format_writer &, unsigned long);
    ST_EXPORT void format_type(const ST::format_spec &, ST::format_writer &, long long);
    ST_EXPORT void format_type(const ST::format_spec &, ST::format_writer &, unsigned long long);
    ST_EXPORT void format_type(const ST::format_spec &, ST::format_writer &, double);

    inline void format_type(const ST::format_spec &format, ST::format_writer &output,
                            wchar_t value)
    {
        format_type(format, output, static_cast<char32_t>(value));
    }

    inline void format_type(const ST::format_spec &format, ST::format_writer &output,
                            float value)
    {
        format_type(format, output, double(value));
    }

    inline void format_type(const ST::format_spec &format, ST::format_writer &output,
                            const char *text)
    {
        if (text)
            ST::format_string(format, output, text, std::char_traits<char>::length(text));
    }

    inline void format_type(const ST::format_spec &format, ST::format_writer &output,
                            const wchar_t *wtext)
    {
        if (wtext) {
            ST::char_buffer utf8 = ST::string::from_wchar(wtext).to_utf8();
            ST::format_string(format, output, utf8.data(), utf8.size());
        }
    }

    inline void format_type(const ST::format_spec &format, ST::format_writer &output,
                            const ST::string &str)
    {
        ST::format_string(format, output, str.c_str(), str.size());
    }

#if !defined(ST_NO_STL_STRINGS)

    inline void format_type(const ST::format_spec &format, ST::format_writer &output,
                            const std::string &str)
    {
        ST::format_string(format, output, str.c_str(), str.size());
    }

    inline void format_type(const ST::format_spec &format, ST::format_writer &output,
                            const std::wstring &str)
    {
        ST::char_buffer utf8 = ST::string::from_wchar(str.c_str(), str.size()).to_utf8();
        ST::format_string(format, output, utf8.data(), utf8.size());
    }

    inline void format_type(const ST::format_spec &format, ST::format_writer &output,
                            const std::u16string &str)
    {
        ST::char_buffer utf8 = ST::string::from_utf16(str.c_str(), str.size()).to_utf8();
        ST::format_string(format, output, utf8.data(), utf8.size());
    }

    inline void format_type(const ST::format_spec &format, ST::format_writer &output,
                            const std::u32string &str)
    {
        ST::char_buffer utf8 = ST::string::from_utf32(str.c_str(), str.size()).to_utf8();
        ST::format_string(format, output, utf8.data(), utf8.size());
    }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES

    inline void format_type(const ST::format_spec &format, ST::format_writer &output,
                            const std::u8string &str)
    {
        ST::format_string(format, output, reinterpret_cast<const char*>(str.c_str()), str.size());
    }

#endif

    template<typename value_T>
    void format_type(const ST::format_spec &format, ST::format_writer &output,
                     const std::complex<value_T> &value)
    {
        format_type(format, output, value.real());
        output.append_char('+');
        format_type(format, output, value.imag());
        output.append_char('i');
    }

#ifdef ST_HAVE_CXX17_FILESYSTEM

    inline void format_type(const ST::format_spec &format, ST::format_writer &output,
                            const std::filesystem::path &path)
    {
        auto u8path = path.u8string();
        ST::format_string(format, output, u8path.c_str(), u8path.size());
    }

#endif

#ifdef ST_HAVE_EXPERIMENTAL_FILESYSTEM

    inline void format_type(const ST::format_spec &format, ST::format_writer &output,
                            const std::experimental::filesystem::path &path)
    {
        auto u8path = path.u8string();
        ST::format_string(format, output, u8path.c_str(), u8path.size());
    }

#endif

#ifdef ST_HAVE_CXX17_STRING_VIEW

    inline void format_type(const ST::format_spec &format, ST::format_writer &output,
                            const std::string_view &view)
    {
        ST::format_string(format, output, view.data(), view.size());
    }

    inline void format_type(const ST::format_spec &format, ST::format_writer &output,
                            const std::wstring_view &view)
    {
        ST::char_buffer utf8 = ST::string::from_wchar(view.data(), view.size()).to_utf8();
        ST::format_string(format, output, utf8.data(), utf8.size());
    }

    inline void format_type(const ST::format_spec &format, ST::format_writer &output,
                            const std::u16string_view &view)
    {
        ST::char_buffer utf8 = ST::string::from_utf16(view.data(), view.size()).to_utf8();
        ST::format_string(format, output, utf8.data(), utf8.size());
    }

    inline void format_type(const ST::format_spec &format, ST::format_writer &output,
                            const std::u32string_view &view)
    {
        ST::char_buffer utf8 = ST::string::from_utf32(view.data(), view.size()).to_utf8();
        ST::format_string(format, output, utf8.data(), utf8.size());
    }

#endif

#ifdef ST_HAVE_CXX20_CHAR8_TYPES

    inline void format_type(const ST::format_spec &format, ST::format_writer &output,
                            const std::u8string_view &view)
    {
        ST::format_string(format, output, reinterpret_cast<const char*>(view.data()), view.size());
    }

#endif

#ifdef ST_HAVE_EXPERIMENTAL_STRING_VIEW

    inline void format_type(const ST::format_spec &format, ST::format_writer &output,
                            const std::experimental::string_view &view)
    {
        ST::format_string(format, output, view.data(), view.size());
    }

    inline void format_type(const ST::format_spec &format, ST::format_writer &output,
                            const std::experimental::wstring_view &view)
    {
        ST::char_buffer utf8 = ST::string::from_wchar(view.data(), view.size()).to_utf8();
        ST::format_string(format, output, utf8.data(), utf8.size());
    }

    inline void format_type(const ST::format_spec &format, ST::format_writer &output,
                            const std::experimental::u16string_view &view)
    {
        ST::char_buffer utf8 = ST::string::from_utf16(view.data(), view.size()).to_utf8();
        ST::format_string(format, output, utf8.data(), utf8.size());
    }

    inline void format_type(const ST::format_spec &format, ST::format_writer &output,
                            const std::experimental::u32string_view &view)
    {
        ST::char_buffer utf8 = ST::string::from_utf32(view.data(), view.size()).to_utf8();
        ST::format_string(format, output, utf8.data(), utf8.size());
    }

#endif

#endif // !defined(ST_NO_STL_STRINGS)

    inline void format_type(const ST::format_spec &format, ST::format_writer &output,
                            bool value)
    {
        if (value)
            ST::format_string(format, output, "true", 4);
        else
            ST::format_string(format, output, "false", 5);
    }
}

/* These macros are maintained for backwards compatibility.  They should be
   considered deprecated, and may be removed from a future version of
   string_theory (4.0 or later) */
#define ST_DECL_FORMAT_TYPE(type_T) \
    void format_type(const ST::format_spec &, ST::format_writer &, type_T)

#define ST_FORMAT_TYPE(type_T) \
    void format_type(const ST::format_spec &format, ST::format_writer &output, type_T value)

#define ST_FORMAT_FORWARD(fwd_value) \
    format_type(format, output, (fwd_value))

#define ST_INVOKE_FORMATTER format_type

#endif // _ST_FORMATTER_H
