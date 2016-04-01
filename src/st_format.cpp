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

#include "st_format.h"

#include <cstdlib>
#include <cstring>
#include <type_traits>
#include "st_assert.h"

#define BADCHAR_SUBSTITUTE (0xFFFDul)

static const char *_scan_next_format(_ST_PRIVATE::format_data_object &data)
{
    ST_ASSERT(data.m_format_str, "Passed a null format string!");

    const char *ptr = data.m_format_str;
    while (*ptr) {
        if (*ptr == '{')
            return ptr;
        ++ptr;
    }

    return ptr;
}

static void _fetch_prefix(_ST_PRIVATE::format_data_object &data)
{
    for ( ;; ) {
        const char *next = _scan_next_format(data);
        if (*next && *(next + 1) == '{') {
            // Escaped '{'
            data.m_output.append(data.m_format_str, 1 + next - data.m_format_str);
            data.m_format_str = next + 2;
            continue;
        }

        if (next != data.m_format_str)
            data.m_output.append(data.m_format_str, next - data.m_format_str);
        data.m_format_str = next;
        break;
    };
}

ST::format_spec _ST_PRIVATE::fetch_next_format(_ST_PRIVATE::format_data_object &data)
{
    _fetch_prefix(data);
    ST_ASSERT(*data.m_format_str == '{', "Too many actual parameters for format string");

    ST::format_spec spec;
    const char *ptr = data.m_format_str;
    for ( ;; ) {
        ++ptr;

        switch (*ptr) {
        case 0:
            ST_ASSERT(false, "Unterminated format specifier");
            abort();
        case '}':
            // Done with format spec
            data.m_format_str = ptr + 1;
            return spec;
            break;

        case '<':
            spec.m_alignment = ST::align_left;
            break;
        case '>':
            spec.m_alignment = ST::align_right;
            break;
        case '_':
            spec.m_pad = *(ptr + 1);
            ST_ASSERT(spec.m_pad, "Unterminated format specifier");
            ++ptr;
            break;
        case 'x':
            spec.m_digit_class = ST::digit_hex;
            break;
        case 'X':
            spec.m_digit_class = ST::digit_hex_upper;
            break;
        case '+':
            spec.m_always_signed = true;
            break;
        case 'd':
            spec.m_digit_class = ST::digit_dec;
            break;
        case 'o':
            spec.m_digit_class = ST::digit_oct;
            break;
        case 'b':
            spec.m_digit_class = ST::digit_bin;
            break;
        case 'c':
            spec.m_digit_class = ST::digit_char;
            break;
        case 'f':
            spec.m_float_class = ST::float_fixed;
            break;
        case 'e':
            spec.m_float_class = ST::float_exp;
            break;
        case 'E':
            spec.m_float_class = ST::float_exp_upper;
            break;
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        {
            char *end = ST_NULLPTR;
            spec.m_minimum_length = static_cast<int>(strtol(ptr, &end, 10));
            ptr = end - 1;
            break;
        }
        case '.':
        {
            ST_ASSERT(*(ptr + 1), "Unterminated format specifier");
            char *end = ST_NULLPTR;
            spec.m_precision = static_cast<int>(strtol(ptr + 1, &end, 10));
            ptr = end - 1;
            break;
        }
        default:
            ST_ASSERT(false, "Unexpected character in format string");
            break;
        }
    }
}

ST::string _ST_PRIVATE::format(_ST_PRIVATE::format_data_object &data)
{
    _fetch_prefix(data);
    ST_ASSERT(*data.m_format_str == 0, "Not enough actual parameters for format string");
    return data.m_output.to_string(data.m_is_utf8, data.m_validation);
}

void ST::format_string(const ST::format_spec &format, ST::string_stream &output,
                       const char *text, size_t size,
                       ST::alignment default_alignment)
{
    char pad = format.m_pad ? format.m_pad : ' ';

    if (format.m_minimum_length > static_cast<int>(size)) {
        ST::alignment align =
                (format.m_alignment == ST::align_default)
                ? default_alignment : format.m_alignment;

        if (align == ST::align_right) {
            output.append_char(pad, format.m_minimum_length - size);
            output.append(text, size);
        } else {
            output.append(text, size);
            output.append_char(pad, format.m_minimum_length - size);
        }
    } else {
        output.append(text, size);
    }
}

template <typename int_T>
static void _format_numeric_impl(char *output_end, int_T value, int radix,
                                 bool upper_case = false)
{
    if (value == 0) {
        *(output_end - 1) = '0';
        return;
    }

    while (value) {
        int digit = (value % radix);
        value /= radix;
        --output_end;

        if (digit < 10)
            *output_end = '0' + digit;
        else if (upper_case)
            *output_end = 'A' + digit - 10;
        else
            *output_end = 'a' + digit - 10;
    }
}

template <typename int_T>
static void _format_numeric(const ST::format_spec &format,
                            ST::string_stream &output, int_T value,
                            int radix, bool upper_case = false)
{
    ST_STATIC_ASSERT(std::is_unsigned<int_T>::value,
                     "Signed numerics are currently only supported in Decimal formatting");

    size_t format_size = 0;
    int_T temp = value;
    while (temp) {
        ++format_size;
        temp /= radix;
    }

    if (format_size == 0)
        format_size = 1;

    ST_ASSERT(format_size < 65, "Format length too long");

    char buffer[65];
    _format_numeric_impl<int_T>(buffer + format_size, value, radix, upper_case);
    ST::format_string(format, output, buffer, format_size, ST::align_right);
}

// Currently, only decimal formatting supports rendering negative numbers
template <typename int_T>
static void _format_decimal(const ST::format_spec &format,
                            ST::string_stream &output, int_T value)
{
    typedef typename std::make_unsigned<int_T>::type uint_T;
    uint_T abs = (value < 0) ? -(uint_T)value : value;

    size_t format_size = 0;
    uint_T temp = abs;
    while (temp) {
        ++format_size;
        temp /= 10;
    }

    if (format_size == 0)
        format_size = 1;

    if (value < 0 || format.m_always_signed)
        ++format_size;

    ST_ASSERT(format_size < 24, "Format length too long");

    char buffer[24];
    _format_numeric_impl<uint_T>(buffer + format_size, abs, 10);

    if (value < 0)
        buffer[0] = '-';
    else if (format.m_always_signed)
        buffer[0] = '+';

    ST::format_string(format, output, buffer, format_size, ST::align_right);
}

static void _format_char(const ST::format_spec &format,
                         ST::string_stream &output, int ch)
{
    ST_ASSERT(format.m_minimum_length == 0 && format.m_pad == 0,
             "Char formatting does not currently support padding");

    // Don't need to nul-terminate this, since string_buffer's constructor fixes it
    char utf8[4];
    size_t format_size;

    // Roughly copied from ST::string
    if (ch > 0x10FFFF) {
        // Character out of range; Use U+FFFD instead
        format_size = 3;
        utf8[0] = 0xE0 | ((BADCHAR_SUBSTITUTE >> 12) & 0x0F);
        utf8[1] = 0x80 | ((BADCHAR_SUBSTITUTE >>  6) & 0x3F);
        utf8[2] = 0x80 | ((BADCHAR_SUBSTITUTE      ) & 0x3F);
    } else if (ch > 0xFFFF) {
        format_size = 4;
        utf8[0] = 0xF0 | ((ch >> 18) & 0x07);
        utf8[1] = 0x80 | ((ch >> 12) & 0x3F);
        utf8[2] = 0x80 | ((ch >>  6) & 0x3F);
        utf8[3] = 0x80 | ((ch      ) & 0x3F);
    } else if (ch > 0x7FF) {
        format_size = 3;
        utf8[0] = 0xE0 | ((ch >> 12) & 0x0F);
        utf8[1] = 0x80 | ((ch >>  6) & 0x3F);
        utf8[2] = 0x80 | ((ch      ) & 0x3F);
    } else if (ch > 0x7F) {
        format_size = 2;
        utf8[0] = 0xC0 | ((ch >>  6) & 0x1F);
        utf8[1] = 0x80 | ((ch      ) & 0x3F);
    } else {
        format_size = 1;
        utf8[0] = (char)ch;
    }

    output.append(utf8, format_size);
}

#define _ST_FORMAT_INT_TYPE(int_T, uint_T) \
    ST_FORMAT_TYPE(int_T) \
    { \
        /* Note:  The use of unsigned here is not a typo -- we only format decimal \
           values with a sign, so we can convert everything else to unsigned. */ \
        switch (format.m_digit_class) { \
        case ST::digit_bin: \
            _format_numeric<uint_T>(format, output, value, 2); \
            break; \
        case ST::digit_oct: \
            _format_numeric<uint_T>(format, output, value, 8); \
            break; \
        case ST::digit_hex: \
            _format_numeric<uint_T>(format, output, value, 16, false); \
            break; \
        case ST::digit_hex_upper: \
            _format_numeric<uint_T>(format, output, value, 16, true); \
            break; \
        case ST::digit_dec: \
        case ST::digit_default: \
            _format_decimal<int_T>(format, output, value); \
            break; \
        case ST::digit_char: \
            _format_char(format, output, value); \
            break; \
        default: \
            ST_ASSERT(false, "Unexpected digit class"); \
            break; \
        } \
    } \
    \
    ST_FORMAT_TYPE(uint_T) \
    { \
        switch (format.m_digit_class) { \
        case ST::digit_bin: \
            _format_numeric<uint_T>(format, output, value, 2); \
            break; \
        case ST::digit_oct: \
            _format_numeric<uint_T>(format, output, value, 8); \
            break; \
        case ST::digit_hex: \
            _format_numeric<uint_T>(format, output, value, 16, false); \
            break; \
        case ST::digit_hex_upper: \
            _format_numeric<uint_T>(format, output, value, 16, true); \
            break; \
        case ST::digit_dec: \
        case ST::digit_default: \
            _format_decimal<uint_T>(format, output, value); \
            break; \
        case ST::digit_char: \
            _format_char(format, output, value); \
            break; \
        default: \
            ST_ASSERT(false, "Unexpected digit class"); \
            break; \
        } \
    }

_ST_FORMAT_INT_TYPE(signed char, unsigned char)
_ST_FORMAT_INT_TYPE(short, unsigned short)
_ST_FORMAT_INT_TYPE(int, unsigned)
_ST_FORMAT_INT_TYPE(long, unsigned long)

#if defined(ST_HAVE_INT64) && !defined(ST_INT64_IS_LONG)
_ST_FORMAT_INT_TYPE(int64_t, uint64_t)
#endif

ST_FORMAT_TYPE(float)
{
    ST_FORMAT_FORWARD(double(value));
}

ST_FORMAT_TYPE(double)
{
    char pad = format.m_pad ? format.m_pad : ' ';

    // Cheating a bit here -- just pass it along to cstdio
    char format_buffer[32];
    size_t end = 0;

    format_buffer[end++] = '%';

    if (format.m_always_signed)
        format_buffer[end++] = '+';

    if (format.m_precision >= 0) {
        int count = snprintf(format_buffer + end, sizeof(format_buffer) - end,
                             ".%d", format.m_precision);

        // Ensure one more space (excluding \0) is available for the format specifier
        ST_ASSERT(count > 0 && count + end + 2 < sizeof(format_buffer),
                  "Not enough space for format string");
        end += count;
    }

    format_buffer[end++] =
        (format.m_float_class == ST::float_exp) ? 'e' :
        (format.m_float_class == ST::float_exp_upper) ? 'E' :
        (format.m_float_class == ST::float_fixed) ? 'f' : 'g';
    format_buffer[end] = 0;

    int format_size = snprintf(ST_NULLPTR, 0, format_buffer, value);
    ST_ASSERT(format_size > 0, "Your libc doesn't support reporting format size");
    ST::char_buffer out_buffer;
    char *fmt_out;

    if (format.m_minimum_length > format_size) {
        fmt_out = out_buffer.create_writable_buffer(format.m_minimum_length);
        memset(fmt_out, pad, format.m_minimum_length);
        if (format.m_alignment == ST::align_left) {
            snprintf(fmt_out, format_size + 1, format_buffer, value);
            fmt_out[format_size] = pad;  // snprintf overwrites this
        } else {
            snprintf(fmt_out + (format.m_minimum_length - format_size), format_size + 1,
                     format_buffer, value);
        }
    } else {
        fmt_out = out_buffer.create_writable_buffer(format_size);
        snprintf(fmt_out, format_size + 1, format_buffer, value);
    }

    output.append(out_buffer.data(), out_buffer.size());
}

ST_FORMAT_TYPE(char)
{
    /* Note:  The use of unsigned here is not a typo -- we only format decimal
       values with a sign, so we can convert everything else to unsigned. */
    switch (format.m_digit_class) {
    case ST::digit_bin:
        _format_numeric<unsigned char>(format, output, value, 2);
        break;
    case ST::digit_oct:
        _format_numeric<unsigned char>(format, output, value, 8);
        break;
    case ST::digit_hex:
        _format_numeric<unsigned char>(format, output, value, 16, false);
        break;
    case ST::digit_hex_upper:
        _format_numeric<unsigned char>(format, output, value, 16, true);
        break;
    case ST::digit_dec:
        _format_decimal<signed char>(format, output, value);
        break;
    case ST::digit_char:
    case ST::digit_default:
        _format_char(format, output, value);
        break;
    default:
        ST_ASSERT(false, "Unexpected digit class");
        break;
    }
}

ST_FORMAT_TYPE(wchar_t)
{
    switch (format.m_digit_class) {
    case ST::digit_bin:
        _format_numeric<uint32_t>(format, output, value, 2);
        break;
    case ST::digit_oct:
        _format_numeric<uint32_t>(format, output, value, 8);
        break;
    case ST::digit_hex:
        _format_numeric<uint32_t>(format, output, value, 16, false);
        break;
    case ST::digit_hex_upper:
        _format_numeric<uint32_t>(format, output, value, 16, true);
        break;
    case ST::digit_dec:
        _format_decimal<uint32_t>(format, output, value);
        break;
    case ST::digit_char:
    case ST::digit_default:
        _format_char(format, output, value);
        break;
    default:
        ST_ASSERT(false, "Unexpected digit class");
        break;
    }
}

ST_FORMAT_TYPE(const char *)
{
    ST::format_string(format, output, value, strlen(value), ST::align_left);
}

ST_FORMAT_TYPE(const wchar_t *)
{
    ST::char_buffer utf8 = ST::string::from_wchar(value).to_utf8();
    ST::format_string(format, output, utf8.data(), utf8.size(), ST::align_left);
}

ST_FORMAT_TYPE(const ST::string &)
{
    ST::format_string(format, output, value.c_str(), value.size(), ST::align_left);
}

ST_FORMAT_TYPE(bool)
{
    ST_FORMAT_FORWARD(value ? "true" : "false");
}
