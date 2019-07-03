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

#ifndef _ST_STRING_HELPERS_H
#define _ST_STRING_HELPERS_H

#include "st_charbuffer.h"
#include "st_format_numeric.h"

#include <cstdlib>

namespace ST
{
    enum class utf_validation_t;
    class string;
}

namespace _ST_PRIVATE
{
    ST_EXPORT bool validate_utf8(const char *buffer, size_t size);
    ST_EXPORT size_t cleanup_utf8(char *output, const char *buffer, size_t size);

    inline bool validate_utf8_buffer(const ST::char_buffer &buffer)
    {
        return validate_utf8(buffer.data(), buffer.size());
    }

    inline ST::char_buffer cleanup_utf8_buffer(const ST::char_buffer &buffer)
    {
        size_t clean_size = cleanup_utf8(nullptr, buffer.data(), buffer.size());
        ST::char_buffer cb_clean;
        cb_clean.allocate(clean_size);
        cleanup_utf8(cb_clean.data(), buffer.data(), buffer.size());
        return cb_clean;
    }

    ST_EXPORT size_t measure_from_utf16(const char16_t *utf16, size_t size);
    ST_EXPORT const char *convert_from_utf16(char *dest, const char16_t *utf16, size_t size,
                                             ST::utf_validation_t validation);

    ST_EXPORT size_t measure_from_utf32(const char32_t *utf32, size_t size);
    ST_EXPORT const char *convert_from_utf32(char *dest, const char32_t *utf32, size_t size,
                                             ST::utf_validation_t validation);

    ST_EXPORT size_t measure(char32_t ch);

    ST_EXPORT size_t measure_from_latin_1(const char *astr, size_t size);
    ST_EXPORT void convert_from_latin_1(char *dest, const char *astr, size_t size);

    ST_EXPORT size_t measure_to_utf16(const char *utf8, size_t size);
    ST_EXPORT void convert_to_utf16(char16_t *dest, const char *utf8, size_t size);

    ST_EXPORT size_t measure_to_utf32(const char *utf8, size_t size);
    ST_EXPORT void convert_to_utf32(char32_t *dest, const char *utf8, size_t size);

    ST_EXPORT size_t measure_to_latin_1(const char *utf8, size_t size);
    ST_EXPORT const char *convert_to_latin_1(char *dest, const char *utf8, size_t size,
                                             ST::utf_validation_t validation);

    inline char cl_fast_lower(char ch)
    {
        // In the C locale, the only characters that get converted are ['A'..'Z']
        if (ch >= 'A' && ch <= 'Z')
            return ch + 32;
        return ch;
    }

    inline char cl_fast_upper(char ch)
    {
        // In the C locale, the only characters that get converted are ['a'..'z']
        if (ch >= 'a' && ch <= 'z')
            return ch - 32;
        return ch;
    }

    ST_EXPORT const char *find_cs(const char *haystack, const char *needle);
    ST_EXPORT const char *find_ci(const char *haystack, const char *needle);

    inline const char *find_cs(const char *haystack, size_t size, char ch)
    {
        return std::char_traits<char>::find(haystack, size, ch);
    }

    ST_EXPORT const char *find_ci(const char *haystack, size_t size, char ch);

    template <typename int_T>
    ST::char_buffer mini_format_int_s(int radix, bool upper_case, int_T value)
    {
        typedef typename std::make_unsigned<int_T>::type uint_T;
        ST::uint_formatter<uint_T> formatter;
        formatter.format(std::abs(value), radix, upper_case);

        ST::char_buffer result;
        if (value < 0) {
            result.allocate(formatter.size() + 1);
            std::char_traits<char>::copy(result.data() + 1, formatter.text(), formatter.size());
            result[0] = '-';
        } else {
            result.allocate(formatter.size());
            std::char_traits<char>::copy(result.data(), formatter.text(), formatter.size());
        }

        return result;
    }

    template <typename uint_T>
    ST::char_buffer mini_format_int_u(int radix, bool upper_case, uint_T value)
    {
        ST::uint_formatter<uint_T> formatter;
        formatter.format(value, radix, upper_case);

        ST::char_buffer result;
        result.allocate(formatter.size());
        std::char_traits<char>::copy(result.data(), formatter.text(), formatter.size());

        return result;
    }


    template <typename float_T>
    static ST::char_buffer mini_format_float(float_T value, char format)
    {
        ST::float_formatter<float_T> formatter;
        formatter.format(value, format);

        ST::char_buffer result;
        result.allocate(formatter.size());
        std::char_traits<char>::copy(result.data(), formatter.text(), formatter.size());

        return result;
    }

    ST_EXPORT const char *append_utf8(char *dest, char32_t ch);
}

#endif // _ST_STRING_HELPERS_H
