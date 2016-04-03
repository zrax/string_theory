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

#include "st_stringstream.h"

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
              m_class_prefix() { }

        int m_minimum_length;
        int m_precision;
        alignment m_alignment;
        digit_class m_digit_class;
        float_class m_float_class;
        char m_pad;
        bool m_always_signed;
        bool m_class_prefix;
    };

    ST_EXPORT void format_string(const format_spec &format, string_stream &output,
                                const char *text, size_t size,
                                alignment default_alignment);
}

#define ST_DECL_FORMAT_TYPE(type_T) \
    void _ST_impl_format_data_handler(const ST::format_spec &, \
                                      ST::string_stream &, type_T)

#define ST_FORMAT_TYPE(type_T) \
    void _ST_impl_format_data_handler(const ST::format_spec &format, \
                                      ST::string_stream &output, type_T value)

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

ST_EXPORT ST_DECL_FORMAT_TYPE(float);
ST_EXPORT ST_DECL_FORMAT_TYPE(double);

ST_EXPORT ST_DECL_FORMAT_TYPE(const char *);
ST_EXPORT ST_DECL_FORMAT_TYPE(const wchar_t *);
ST_EXPORT ST_DECL_FORMAT_TYPE(const ST::string &);

ST_EXPORT ST_DECL_FORMAT_TYPE(bool);

#endif // _ST_FORMATTER_H
