/*  This file is part of string_theory.

    string_theory is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    string_theory is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with string_theory.  If not, see <http://www.gnu.org/licenses/>.  */

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
              m_digit_class(), m_float_class(), m_pad(), m_always_signed() { }

        int m_minimum_length;
        int m_precision;
        alignment m_alignment;
        digit_class m_digit_class;
        float_class m_float_class;
        char m_pad;
        bool m_always_signed;
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
