/*  Copyright (c) 2019 Michael Hansen

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

#ifndef _ST_FORMAT_PRIV_H
#define _ST_FORMAT_PRIV_H

#include "st_utf_conv_priv.h"

namespace _ST_PRIVATE
{
    enum numeric_type
    {
        numeric_positive,
        numeric_negative,
        numeric_zero
    };

    inline size_t pad_size(const ST::format_spec &format, size_t size,
                           numeric_type ntype)
    {
        ST_ssize_t pad_size = format.minimum_length - size;

        if (ntype == numeric_negative || format.always_signed)
            --pad_size;

        if (ntype != numeric_zero && format.class_prefix) {
            switch (format.digit_class) {
            case ST::digit_hex:
            case ST::digit_hex_upper:
            case ST::digit_bin:
                pad_size -= 2;
                break;
            case ST::digit_oct:
                pad_size -= 1;
                break;
            default:
                break;
            }
        }

        return (pad_size > 0) ? static_cast<size_t>(pad_size) : 0;
    }

    inline void format_numeric_prefix(const ST::format_spec &format,
                                      ST::format_writer &output,
                                      numeric_type ntype)
    {
        if (ntype == numeric_negative)
            output.append_char('-');
        else if (format.always_signed)
            output.append_char('+');

        if (ntype != numeric_zero && format.class_prefix) {
            switch (format.digit_class) {
            case ST::digit_hex:
                output.append("0x", 2);
                break;
            case ST::digit_hex_upper:
                output.append("0X", 2);
                break;
            case ST::digit_bin:
                output.append("0b", 2);
                break;
            case ST::digit_oct:
                output.append_char('0');
                break;
            default:
                break;
            }
        }
    }

    inline void format_numeric_string(const ST::format_spec &format,
                                      ST::format_writer &output,
                                      const char *text, size_t size,
                                      numeric_type ntype)
    {
        const char pad = format.pad ? format.pad : ' ';
        const size_t psize = pad_size(format, size, ntype);

        if (format.numeric_pad) {
            format_numeric_prefix(format, output, ntype);

            // numeric padding is always right-aligned
            output.append_char(pad, psize);
            output.append(text, size);
        } else {
            ST::alignment_t align =
                (format.alignment == ST::align_default)
                ? ST::align_right : format.alignment;

            if (align == ST::align_right) {
                output.append_char(pad, psize);
                format_numeric_prefix(format, output, ntype);
                output.append(text, size);
            } else {
                format_numeric_prefix(format, output, ntype);
                output.append(text, size);
                output.append_char(pad, psize);
            }
        }
    }

    template <typename int_T>
    void format_numeric_s(const ST::format_spec &format,
                          ST::format_writer &output, int_T value)
    {
        static_assert(std::is_signed<int_T>::value,
                      "Use _format_numeric_u for unsigned numerics");

        int radix = 10;
        bool upper_case = false;
        switch (format.digit_class) {
        case ST::digit_hex_upper:
            upper_case = true;
            /* fall through */
        case ST::digit_hex:
            radix = 16;
            break;
        case ST::digit_oct:
            radix = 8;
            break;
        case ST::digit_bin:
            radix = 2;
            break;
        case ST::digit_dec:
        case ST::digit_default:
            break;
        default:
            ST_ASSERT(false, "Invalid digit class for _format_numeric_s");
        }

        typedef typename std::make_unsigned<int_T>::type uint_T;
        ST::uint_formatter<uint_T> formatter;
        formatter.format(static_cast<uint_T>(std::abs(value)), radix, upper_case);

        const numeric_type ntype = (value == 0) ? numeric_zero
                                 : (value < 0) ? numeric_negative
                                 : numeric_positive;

        format_numeric_string(format, output, formatter.text(), formatter.size(), ntype);
    }

    template <typename uint_T>
    void format_numeric_u(const ST::format_spec &format,
                          ST::format_writer &output, uint_T value)
    {
        static_assert(std::is_unsigned<uint_T>::value,
                      "Use _format_numeric_s for signed numerics");

        int radix = 10;
        bool upper_case = false;
        switch (format.digit_class) {
        case ST::digit_hex_upper:
            upper_case = true;
            /* fall through */
        case ST::digit_hex:
            radix = 16;
            break;
        case ST::digit_oct:
            radix = 8;
            break;
        case ST::digit_bin:
            radix = 2;
            break;
        case ST::digit_dec:
        case ST::digit_default:
            break;
        default:
            ST_ASSERT(false, "Invalid digit class for _format_numeric_u");
        }

        ST::uint_formatter<uint_T> formatter;
        formatter.format(value, radix, upper_case);

        const numeric_type ntype = (value == 0) ? numeric_zero : numeric_positive;

        format_numeric_string(format, output, formatter.text(), formatter.size(), ntype);
    }

    inline void format_char(const ST::format_spec &format,
                            ST::format_writer &output, int ch)
    {
        if (format.minimum_length != 0 || format.pad != 0)
            ST_ASSERT(false, "Char formatting does not currently support padding");

        // Don't need to nul-terminate this, since we just write a fixed length
        char utf8[4];
        char *dest = utf8;
        conversion_error_t error = write_utf8(dest, ch);
        if (error != conversion_error_t::success)
            append_chars(dest, badchar_substitute_utf8, badchar_substitute_utf8_len);

        ST_ASSERT(size_t(dest - utf8) <= sizeof(utf8), "Destination buffer too small");

        output.append(utf8, dest - utf8);
    }
}

#endif // _ST_FORMAT_PRIV_H
