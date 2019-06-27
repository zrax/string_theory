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

    class ST_EXPORT format_writer
    {
        ST_DISABLE_COPY(format_writer);

    public:
        format_writer(const char *format) noexcept : m_format_str(format) { }
        virtual ~format_writer() noexcept { }

        virtual format_writer &append(const char *data, size_t size = ST_AUTO_SIZE) = 0;
        virtual format_writer &append_char(char ch, size_t count = 1) = 0;

        bool next_format();
        ST::format_spec parse_format();

    private:
        const char *m_format_str;

        char fetch_prefix();
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
}

#define ST_DECL_FORMAT_TYPE(type_T) \
    void _ST_impl_format_data_handler(const ST::format_spec &, \
                                      ST::format_writer &, type_T)

#define ST_FORMAT_TYPE(type_T) \
    void _ST_impl_format_data_handler(const ST::format_spec &format, \
                                      ST::format_writer &output, type_T value)

#define ST_FORMAT_FORWARD(fwd_value) \
    _ST_impl_format_data_handler(format, output, (fwd_value))

#define ST_INVOKE_FORMATTER _ST_impl_format_data_handler

ST_EXPORT ST_DECL_FORMAT_TYPE(char);
ST_EXPORT ST_DECL_FORMAT_TYPE(wchar_t);
ST_EXPORT ST_DECL_FORMAT_TYPE(signed char);
ST_EXPORT ST_DECL_FORMAT_TYPE(unsigned char);
ST_EXPORT ST_DECL_FORMAT_TYPE(short);
ST_EXPORT ST_DECL_FORMAT_TYPE(unsigned short);
ST_EXPORT ST_DECL_FORMAT_TYPE(int);
ST_EXPORT ST_DECL_FORMAT_TYPE(unsigned int);
ST_EXPORT ST_DECL_FORMAT_TYPE(long);
ST_EXPORT ST_DECL_FORMAT_TYPE(unsigned long);

#if defined(ST_HAVE_INT64) && !defined(ST_INT64_IS_LONG)
ST_EXPORT ST_DECL_FORMAT_TYPE(int64_t);
ST_EXPORT ST_DECL_FORMAT_TYPE(uint64_t);
#endif

ST_EXPORT ST_DECL_FORMAT_TYPE(double);

inline ST_FORMAT_TYPE(float)
{
    ST_FORMAT_FORWARD(double(value));
}

inline ST_FORMAT_TYPE(const char *)
{
    if (value)
        ST::format_string(format, output, value, std::char_traits<char>::length(value));
}

inline ST_FORMAT_TYPE(const wchar_t *)
{
    if (value) {
        ST::char_buffer utf8 = ST::string::from_wchar(value).to_utf8();
        ST::format_string(format, output, utf8.data(), utf8.size());
    }
}

inline ST_FORMAT_TYPE(const ST::string &)
{
    ST::format_string(format, output, value.c_str(), value.size());
}

#if !defined(ST_NO_STL_STRINGS)

inline ST_FORMAT_TYPE(const std::string &)
{
    ST::format_string(format, output, value.c_str(), value.size());
}

inline ST_FORMAT_TYPE(const std::wstring &)
{
    ST::char_buffer utf8 = ST::string::from_wchar(value.c_str(), value.size()).to_utf8();
    ST::format_string(format, output, utf8.data(), utf8.size());
}

template <typename value_T>
ST_FORMAT_TYPE(const std::complex<value_T> &)
{
    ST_FORMAT_FORWARD(value.real());
    output.append_char('+');
    ST_FORMAT_FORWARD(value.imag());
    output.append_char('i');
}

#ifdef ST_HAVE_CXX17_FILESYSTEM
inline ST_FORMAT_TYPE(const std::filesystem::path &)
{
    auto u8path = value.u8string();
    ST::format_string(format, output, u8path.c_str(), u8path.size());
}
#endif

#ifdef ST_HAVE_EXPERIMENTAL_FILESYSTEM
inline ST_FORMAT_TYPE(const std::experimental::filesystem::path &)
{
    auto u8path = value.u8string();
    ST::format_string(format, output, u8path.c_str(), u8path.size());
}
#endif

#ifdef ST_HAVE_CXX17_STRING_VIEW
inline ST_FORMAT_TYPE(const std::string_view &)
{
    ST::format_string(format, output, value.data(), value.size());
}

inline ST_FORMAT_TYPE(const std::wstring_view &)
{
    ST::char_buffer utf8 = ST::string::from_wchar(value.data(), value.size()).to_utf8();
    ST::format_string(format, output, utf8.data(), utf8.size());
}
#endif

#ifdef ST_HAVE_EXPERIMENTAL_STRING_VIEW
inline ST_FORMAT_TYPE(const std::experimental::string_view &)
{
    ST::format_string(format, output, value.data(), value.size());
}

inline ST_FORMAT_TYPE(const std::experimental::wstring_view &)
{
    ST::char_buffer utf8 = ST::string::from_wchar(value.data(), value.size()).to_utf8();
    ST::format_string(format, output, utf8.data(), utf8.size());
}
#endif

#endif // !defined(ST_NO_STL_STRINGS)

inline ST_FORMAT_TYPE(bool)
{
    if (value)
        ST::format_string(format, output, "true", 4);
    else
        ST::format_string(format, output, "false", 5);
}

#endif // _ST_FORMATTER_H
