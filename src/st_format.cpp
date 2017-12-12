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

#include "st_formatter.h"
#include "st_format_priv.h"

#include <cstdlib>
#include <type_traits>
#include "st_assert.h"

/* DO NOT REMOVE THESE LINES.  Classes declared only in these headers won't
* get properly dllexported unless the compiler sees their declarations in at
* least one source file. */
#include "st_format.h"
#include "st_stdio.h"

#define BADCHAR_SUBSTITUTE (0xFFFDul)

#if defined(_MSC_VER) && (_MSC_VER < 1900)
#   define snprintf _snprintf
#   pragma warning(disable: 4996)
#endif

static const char *_scan_next_format(const char *format_str)
{
    ST_ASSERT(format_str, "Passed a null format string!");
    if (!format_str)
        return "";

    const char *ptr = format_str;
    while (*ptr) {
        if (*ptr == '{')
            return ptr;
        ++ptr;
    }

    return ptr;
}

char ST::format_writer::fetch_prefix()
{
    for ( ;; ) {
        const char *next = _scan_next_format(m_format_str);
        if (*next && *(next + 1) == '{') {
            // Escaped '{'
            append(m_format_str, 1 + next - m_format_str);
            m_format_str = next + 2;
            continue;
        }

        if (next != m_format_str)
            append(m_format_str, next - m_format_str);
        m_format_str = next;
        break;
    };

    return *m_format_str;
}

ST::format_spec ST::format_writer::fetch_next_format()
{
    ST_ASSERT(fetch_prefix() == '{', "Too many actual parameters for format string");

    ST::format_spec spec;
    const char *ptr = m_format_str;
    for ( ;; ) {
        ++ptr;

        switch (*ptr) {
        case 0:
            ST_ASSERT(false, "Unterminated format specifier");
            abort();
        case '}':
            // Done with format spec
            m_format_str = ptr + 1;
            return spec;

        case '<':
            spec.alignment = ST::align_left;
            break;
        case '>':
            spec.alignment = ST::align_right;
            break;
        case '_':
            spec.pad = *(ptr + 1);
            spec.numeric_pad = false;
            ST_ASSERT(spec.pad, "Unterminated format specifier");
            ++ptr;
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
            char *end = ST_NULLPTR;
            spec.minimum_length = static_cast<int>(strtol(ptr, &end, 10));
            ptr = end - 1;
            break;
        }
        case '.':
        {
            ST_ASSERT(*(ptr + 1), "Unterminated format specifier");
            char *end = ST_NULLPTR;
            spec.precision = static_cast<int>(strtol(ptr + 1, &end, 10));
            ptr = end - 1;
            break;
        }
        default:
            ST_ASSERT(false, "Unexpected character in format string");
            break;
        }
    }
}

void ST::format_writer::finalize()
{
    ST_ASSERT(fetch_prefix() == 0,
              "Not enough actual parameters for format string");
}

void ST::format_string(const ST::format_spec &format, ST::format_writer &output,
                       const char *text, size_t size,
                       ST::alignment_t default_alignment)
{
    char pad = format.pad ? format.pad : ' ';

    if (format.precision >= 0 && size > static_cast<size_t>(format.precision))
        size = static_cast<size_t>(format.precision);

    if (format.minimum_length > static_cast<int>(size)) {
        ST::alignment_t align =
            (format.alignment == ST::align_default)
            ? default_alignment : format.alignment;

        if (align == ST::align_right) {
            output.append_char(pad, format.minimum_length - size);
            output.append(text, size);
        } else {
            output.append(text, size);
            output.append_char(pad, format.minimum_length - size);
        }
    } else {
        output.append(text, size);
    }
}

enum numeric_type
{
    numeric_positive,
    numeric_negative,
    numeric_zero
};

static size_t _pad_size(const ST::format_spec &format, size_t size,
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

static void _format_numeric_prefix(const ST::format_spec &format,
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

static void _format_numeric_string(const ST::format_spec &format,
                                   ST::format_writer &output,
                                   const char *text, size_t size,
                                   numeric_type ntype)
{
    char pad = format.pad ? format.pad : ' ';

    size_t pad_size = _pad_size(format, size, ntype);

    if (format.numeric_pad) {
        _format_numeric_prefix(format, output, ntype);

        // numeric padding is always right-aligned
        output.append_char(pad, pad_size);
        output.append(text, size);
    } else {
        ST::alignment_t align =
            (format.alignment == ST::align_default)
            ? ST::align_right : format.alignment;

        if (align == ST::align_right) {
            output.append_char(pad, pad_size);
            _format_numeric_prefix(format, output, ntype);
            output.append(text, size);
        } else {
            _format_numeric_prefix(format, output, ntype);
            output.append(text, size);
            output.append_char(pad, pad_size);
        }
    }
}

template <typename int_T>
static void _format_numeric_s(const ST::format_spec &format,
                              ST::format_writer &output, int_T value)
{
    ST_STATIC_ASSERT(std::is_signed<int_T>::value,
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
    int_T abs = (value < 0) ? -value : value;

    size_t format_size = 0;
    int_T temp = abs;
    while (temp) {
        ++format_size;
        temp /= radix;
    }

    numeric_type ntype = (value < 0) ? numeric_negative : numeric_positive;
    if (format_size == 0) {
        format_size = 1;
        ntype = numeric_zero;
    }

    char buffer[68];
    ST_ASSERT(format_size < sizeof(buffer), "Format length too long");
    _format_numeric_impl<uint_T>(buffer + format_size,
                                 static_cast<uint_T>(abs), radix, upper_case);

    _format_numeric_string(format, output, buffer, format_size, ntype);
}

template <typename uint_T>
static void _format_numeric_u(const ST::format_spec &format,
                              ST::format_writer &output, uint_T value)
{
    ST_STATIC_ASSERT(std::is_unsigned<uint_T>::value,
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

    size_t format_size = 0;
    uint_T temp = value;
    while (temp) {
        ++format_size;
        temp /= radix;
    }

    numeric_type ntype = numeric_positive;
    if (format_size == 0) {
        format_size = 1;
        ntype = numeric_zero;
    }

    char buffer[68];
    ST_ASSERT(format_size < sizeof(buffer), "Format length too long");
    _format_numeric_impl<uint_T>(buffer + format_size, value, radix, upper_case);

    _format_numeric_string(format, output, buffer, format_size, ntype);
}

static void _format_char(const ST::format_spec &format,
                         ST::format_writer &output, int ch)
{
    ST_ASSERT(format.minimum_length == 0 && format.pad == 0,
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
        if (format.digit_class == ST::digit_char) \
            _format_char(format, output, static_cast<int>(value)); \
        else \
            _format_numeric_s<int_T>(format, output, value); \
    } \
    \
    ST_FORMAT_TYPE(uint_T) \
    { \
        if (format.digit_class == ST::digit_char) \
            _format_char(format, output, static_cast<int>(value)); \
        else \
            _format_numeric_u<uint_T>(format, output, value); \
    }

_ST_FORMAT_INT_TYPE(signed char, unsigned char)
_ST_FORMAT_INT_TYPE(short, unsigned short)
_ST_FORMAT_INT_TYPE(int, unsigned)
_ST_FORMAT_INT_TYPE(long, unsigned long)

#if defined(ST_HAVE_INT64) && !defined(ST_INT64_IS_LONG)
_ST_FORMAT_INT_TYPE(int64_t, uint64_t)
#endif

ST_FORMAT_TYPE(double)
{
    char pad = format.pad ? format.pad : ' ';

    // Cheating a bit here -- just pass it along to cstdio
    char format_buffer[32];
    size_t end = 0;

    format_buffer[end++] = '%';

    if (format.always_signed)
        format_buffer[end++] = '+';

    if (format.precision >= 0) {
        int count = snprintf(format_buffer + end, sizeof(format_buffer) - end,
                             ".%d", format.precision);

        // Ensure one more space (excluding \0) is available for the format specifier
        ST_ASSERT(count > 0 && count + end + 2 < sizeof(format_buffer),
                  "Not enough space for format string");
        end += count;
    }

    format_buffer[end++] =
        (format.float_class == ST::float_exp) ? 'e' :
        (format.float_class == ST::float_exp_upper) ? 'E' :
        (format.float_class == ST::float_fixed) ? 'f' : 'g';
    format_buffer[end] = 0;

    int format_size = snprintf(ST_NULLPTR, 0, format_buffer, value);
    ST_ASSERT(format_size > 0, "Your libc doesn't support reporting format size");
    ST::char_buffer out_buffer;

    if (format.minimum_length > format_size) {
        out_buffer.allocate(format.minimum_length);
        memset(out_buffer.data(), pad, format.minimum_length);
        if (format.alignment == ST::align_left) {
            snprintf(out_buffer.data(), format_size + 1, format_buffer, value);
            out_buffer[format_size] = pad;  // snprintf overwrites this
        } else {
            snprintf(&out_buffer[format.minimum_length - format_size], format_size + 1,
                     format_buffer, value);
        }
    } else {
        out_buffer.allocate(format_size);
        snprintf(out_buffer.data(), format_size + 1, format_buffer, value);
    }

    output.append(out_buffer.data(), out_buffer.size());
}

ST_FORMAT_TYPE(char)
{
    if (format.digit_class == ST::digit_char || format.digit_class == ST::digit_default)
        _format_char(format, output, value);
    else
        _format_numeric_u<unsigned int>(format, output, static_cast<unsigned int>(value));
}

ST_FORMAT_TYPE(wchar_t)
{
    if (format.digit_class == ST::digit_char || format.digit_class == ST::digit_default)
        _format_char(format, output, value);
    else
        _format_numeric_u<unsigned int>(format, output, static_cast<unsigned int>(value));
}
