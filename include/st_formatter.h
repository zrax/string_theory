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

namespace ST
{
    enum alignment
    {
        align_default,
        align_left,
        align_right
    };

    enum digit_class
    {
        digit_default,
        digit_dec,
        digit_hex,
        digit_hex_upper,
        digit_oct,
        digit_bin,
        digit_char
    };

    enum float_class
    {
        float_default,
        float_fixed,
        float_exp,
        float_exp_upper
    };

    struct ST_EXPORT format_spec
    {
        format_spec()
            : m_minimum_length(), m_precision(-1), m_alignment(),
              m_digit_class(), m_float_class(), m_pad(), m_always_signed(),
              m_class_prefix(), m_numeric_pad() { }

        int m_minimum_length;
        int m_precision;
        alignment m_alignment;
        digit_class m_digit_class;
        float_class m_float_class;
        char m_pad;
        bool m_always_signed;
        bool m_class_prefix;
        bool m_numeric_pad;
    };

    class ST_EXPORT format_writer
    {
        ST_DISABLE_COPY(format_writer);

    public:
        format_writer(const char *format) : m_format_str(format) { }
        virtual ~format_writer() { }

        virtual format_writer &append(const char *data, size_t size = ST_AUTO_SIZE) = 0;
        virtual format_writer &append_char(char ch, size_t count = 1) = 0;

        ST::format_spec fetch_next_format();
        void finalize();

    private:
        const char *m_format_str;

        char fetch_prefix();
    };

    ST_EXPORT void format_string(const format_spec &format, format_writer &output,
                                 const char *text, size_t size,
                                 alignment default_alignment);
}

#define ST_DECL_FORMAT_TYPE(type_T) \
    void _ST_impl_format_data_handler(const ST::format_spec &, \
                                      ST::format_writer &, type_T)

#define ST_FORMAT_TYPE(type_T) \
    void _ST_impl_format_data_handler(const ST::format_spec &format, \
                                      ST::format_writer &output, type_T value)

#define ST_FORMAT_FORWARD(fwd_value) \
    _ST_impl_format_data_handler(format, output, (fwd_value))

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
    ST::format_string(format, output, value, ST::char_buffer::strlen(value), ST::align_left);
}

inline ST_FORMAT_TYPE(const wchar_t *)
{
    ST::char_buffer utf8 = ST::string::from_wchar(value).to_utf8();
    ST::format_string(format, output, utf8.data(), utf8.size(), ST::align_left);
}

inline ST_FORMAT_TYPE(const ST::string &)
{
    ST::format_string(format, output, value.c_str(), value.size(), ST::align_left);
}

inline ST_FORMAT_TYPE(bool)
{
    if (value)
        ST::format_string(format, output, "true", 4, ST::align_left);
    else
        ST::format_string(format, output, "false", 5, ST::align_left);
}

#endif // _ST_FORMATTER_H
