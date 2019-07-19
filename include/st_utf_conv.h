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

#ifndef _ST_UTF_CONV_H
#define _ST_UTF_CONV_H

#include "st_charbuffer.h"

// This is 256MiB worth of UTF-8 string data
#define ST_HUGE_BUFFER_SIZE 0x10000000

namespace ST
{
    enum class utf_validation_t
    {
        assume_valid,       //! Don't do any checking (when possible)
        substitute_invalid, //! Replace invalid sequences with a substitute
        check_validity,     //! Throw a ST::unicode_error for invalid sequences
    };
    ST_ENUM_CONSTANT(utf_validation_t, assume_valid);
    ST_ENUM_CONSTANT(utf_validation_t, substitute_invalid);
    ST_ENUM_CONSTANT(utf_validation_t, check_validity);
}

namespace _ST_PRIVATE
{
    enum class conversion_error_t
    {
        success,
        incomplete_utf8_seq,
        incomplete_surrogate_pair,
        invalid_utf8_seq,
        out_of_range,
        latin1_out_of_range,
    };

    inline void raise_conversion_error(conversion_error_t err)
    {
        switch (err) {
        case conversion_error_t::success:
            return;
        case conversion_error_t::incomplete_utf8_seq:
            throw ST::unicode_error("Incomplete UTF-8 sequence");
        case conversion_error_t::incomplete_surrogate_pair:
            throw ST::unicode_error("Incomplete surrogate pair");
        case conversion_error_t::invalid_utf8_seq:
            throw ST::unicode_error("Invalid UTF-8 sequence byte");
        case conversion_error_t::out_of_range:
            throw ST::unicode_error("Unicode character out of range");
        case conversion_error_t::latin1_out_of_range:
            throw ST::unicode_error("Latin-1 character out of range");
        default:
            ST_ASSERT(false, "Invalid conversion_error_t value");
        }
    }

    ST_EXPORT conversion_error_t validate_utf8(const char *buffer, size_t size);
    ST_EXPORT size_t cleanup_utf8(char *output, const char *buffer, size_t size);

    inline ST::char_buffer cleanup_utf8_buffer(const ST::char_buffer &buffer)
    {
        size_t clean_size = cleanup_utf8(nullptr, buffer.data(), buffer.size());
        ST::char_buffer cb_clean;
        cb_clean.allocate(clean_size);
        cleanup_utf8(cb_clean.data(), buffer.data(), buffer.size());
        return cb_clean;
    }

    ST_EXPORT size_t utf8_measure(char32_t ch);

    ST_EXPORT size_t utf8_measure_from_utf16(const char16_t *utf16, size_t size);
    ST_EXPORT conversion_error_t utf8_convert_from_utf16(char *dest,
                    const char16_t *utf16, size_t size,
                    ST::utf_validation_t validation);

    ST_EXPORT size_t utf8_measure_from_utf32(const char32_t *utf32, size_t size);
    ST_EXPORT conversion_error_t utf8_convert_from_utf32(char *dest,
                    const char32_t *utf32, size_t size,
                    ST::utf_validation_t validation);

    ST_EXPORT size_t utf8_measure_from_latin_1(const char *astr, size_t size);
    ST_EXPORT void utf8_convert_from_latin_1(char *dest, const char *astr, size_t size);

    ST_EXPORT size_t utf16_measure_from_utf8(const char *utf8, size_t size);
    ST_EXPORT conversion_error_t utf16_convert_from_utf8(char16_t *dest,
                    const char *utf8, size_t size,
                    ST::utf_validation_t validation);

    ST_EXPORT size_t utf16_measure_from_utf32(const char32_t *utf32, size_t size);
    ST_EXPORT conversion_error_t utf16_convert_from_utf32(char16_t *dest,
                    const char32_t *utf32, size_t size,
                    ST::utf_validation_t validation);

    ST_EXPORT size_t utf32_measure_from_utf8(const char *utf8, size_t size);
    ST_EXPORT conversion_error_t utf32_convert_from_utf8(char32_t *dest,
                    const char *utf8, size_t size,
                    ST::utf_validation_t validation);

    ST_EXPORT size_t utf32_measure_from_utf16(const char16_t *utf16, size_t size);
    ST_EXPORT conversion_error_t utf32_convert_from_utf16(char32_t *dest,
                    const char16_t *utf16, size_t size,
                    ST::utf_validation_t validation);

    ST_EXPORT void utf16_convert_from_latin_1(char16_t *dest, const char *astr, size_t size);
    ST_EXPORT void utf32_convert_from_latin_1(char32_t *dest, const char *astr, size_t size);

    ST_EXPORT size_t latin_1_measure_from_utf8(const char *utf8, size_t size);
    ST_EXPORT conversion_error_t latin_1_convert_from_utf8(char *dest,
                    const char *utf8, size_t size,
                    ST::utf_validation_t validation,
                    bool substitute_out_of_range);

    ST_EXPORT size_t latin_1_measure_from_utf16(const char16_t *utf16, size_t size);
    ST_EXPORT conversion_error_t latin_1_convert_from_utf16(char *dest,
                    const char16_t *utf16, size_t size,
                    ST::utf_validation_t validation,
                    bool substitute_out_of_range);

    ST_EXPORT conversion_error_t latin_1_convert_from_utf32(char *dest,
                    const char32_t *utf32, size_t size,
                    ST::utf_validation_t validation,
                    bool substitute_out_of_range);

    ST_EXPORT conversion_error_t append_utf8(char *dest, char32_t ch);
}

namespace ST
{
    inline char_buffer utf16_to_utf8(const char16_t *utf16, size_t size,
                                     utf_validation_t validation)
    {
        ST_ASSERT(size < ST_HUGE_BUFFER_SIZE, "String data buffer is too large");

        char_buffer result;
        size_t u8size = _ST_PRIVATE::utf8_measure_from_utf16(utf16, size);
        if (u8size == 0)
            return result;

        result.allocate(u8size);
        auto error = _ST_PRIVATE::utf8_convert_from_utf16(result.data(), utf16,
                                                          size, validation);
        _ST_PRIVATE::raise_conversion_error(error);

        return result;
    }

    inline char_buffer utf16_to_utf8(const utf16_buffer &utf16, utf_validation_t validation)
    {
        return utf16_to_utf8(utf16.data(), utf16.size(), validation);
    }

    inline char_buffer utf32_to_utf8(const char32_t *utf32, size_t size,
                                     utf_validation_t validation)
    {
        ST_ASSERT(size < ST_HUGE_BUFFER_SIZE, "String data buffer is too large");

        char_buffer result;
        size_t u8size = _ST_PRIVATE::utf8_measure_from_utf32(utf32, size);
        if (u8size == 0)
            return result;

        result.allocate(u8size);
        auto error = _ST_PRIVATE::utf8_convert_from_utf32(result.data(), utf32,
                                                          size, validation);
        _ST_PRIVATE::raise_conversion_error(error);

        return result;
    }

    inline char_buffer utf32_to_utf8(const utf32_buffer &utf32, utf_validation_t validation)
    {
        return utf32_to_utf8(utf32.data(), utf32.size(), validation);
    }

    template <typename WcType = wchar_t>
    typename std::enable_if<sizeof(WcType) == sizeof(char16_t), char_buffer>::type
    wchar_to_utf8(const wchar_t *wstr, size_t size, utf_validation_t validation)
    {
        return utf16_to_utf8(reinterpret_cast<const char16_t *>(wstr), size, validation);
    }

    template <typename WcType = wchar_t>
    typename std::enable_if<sizeof(WcType) == sizeof(char32_t), char_buffer>::type
    wchar_to_utf8(const wchar_t *wstr, size_t size, utf_validation_t validation)
    {
        return utf32_to_utf8(reinterpret_cast<const char32_t *>(wstr), size, validation);
    }

    inline char_buffer wchar_to_utf8(const wchar_buffer &wstr, utf_validation_t validation)
    {
        return wchar_to_utf8(wstr.data(), wstr.size(), validation);
    }

    inline char_buffer latin_1_to_utf8(const char *astr, size_t size)
    {
        ST_ASSERT(size < ST_HUGE_BUFFER_SIZE, "String data buffer is too large");

        char_buffer result;
        size_t u8size = _ST_PRIVATE::utf8_measure_from_latin_1(astr, size);
        if (u8size == 0)
            return result;

        result.allocate(u8size);
        _ST_PRIVATE::utf8_convert_from_latin_1(result.data(), astr, size);

        return result;
    }

    inline char_buffer latin_1_to_utf8(const char_buffer &astr)
    {
        return latin_1_to_utf8(astr.data(), astr.size());
    }

    inline utf16_buffer utf8_to_utf16(const char *utf8, size_t size,
                                      utf_validation_t validation)
    {
        ST_ASSERT(size < ST_HUGE_BUFFER_SIZE, "String data buffer is too large");

        size_t u16size = _ST_PRIVATE::utf16_measure_from_utf8(utf8, size);
        if (u16size == 0)
            return null;

        utf16_buffer result;
        result.allocate(u16size);
        auto error = _ST_PRIVATE::utf16_convert_from_utf8(result.data(), utf8,
                                                          size, validation);
        _ST_PRIVATE::raise_conversion_error(error);

        return result;
    }

    inline utf16_buffer utf8_to_utf16(const char_buffer &utf8, utf_validation_t validation)
    {
        return utf8_to_utf16(utf8.data(), utf8.size(), validation);
    }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
    inline utf16_buffer utf8_to_utf16(const char8_t *utf8, size_t size,
                                      utf_validation_t validation)
    {
        return utf8_to_utf16(reinterpret_cast<const char *>(utf8), size, validation);
    }
#endif

    inline utf16_buffer utf32_to_utf16(const char32_t *utf32, size_t size,
                                       utf_validation_t validation)
    {
        ST_ASSERT(size < ST_HUGE_BUFFER_SIZE, "String data buffer is too large");

        size_t u16size = _ST_PRIVATE::utf16_measure_from_utf32(utf32, size);
        if (u16size == 0)
            return null;

        utf16_buffer result;
        result.allocate(u16size);
        auto error = _ST_PRIVATE::utf16_convert_from_utf32(result.data(), utf32,
                                                           size, validation);
        _ST_PRIVATE::raise_conversion_error(error);

        return result;
    }

    inline utf16_buffer utf32_to_utf16(const utf32_buffer &utf32, utf_validation_t validation)
    {
        return utf32_to_utf16(utf32.data(), utf32.size(), validation);
    }

    template <typename WcType = wchar_t>
    typename std::enable_if<sizeof(WcType) == sizeof(char16_t), utf16_buffer>::type
    wchar_to_utf16(const wchar_t *wstr, size_t size, utf_validation_t validation)
    {
        (void)validation;
        return utf16_buffer(reinterpret_cast<const char16_t *>(wstr), size);
    }

    template <typename WcType = wchar_t>
    typename std::enable_if<sizeof(WcType) == sizeof(char32_t), utf16_buffer>::type
    wchar_to_utf16(const wchar_t *wstr, size_t size, utf_validation_t validation)
    {
        return utf32_to_utf16(reinterpret_cast<const char32_t *>(wstr), size, validation);
    }

    inline utf16_buffer wchar_to_utf16(const wchar_buffer &wstr, utf_validation_t validation)
    {
        return wchar_to_utf16(wstr.data(), wstr.size(), validation);
    }

    inline utf16_buffer latin_1_to_utf16(const char *astr, size_t size)
    {
        ST_ASSERT(size < ST_HUGE_BUFFER_SIZE, "String data buffer is too large");

        utf16_buffer result;
        if (!astr || size == 0)
            return result;

        result.allocate(size);
        _ST_PRIVATE::utf16_convert_from_latin_1(result.data(), astr, size);

        return result;
    }

    inline utf16_buffer latin_1_to_utf16(const char_buffer &astr)
    {
        return latin_1_to_utf16(astr.data(), astr.size());
    }

    inline utf32_buffer utf8_to_utf32(const char *utf8, size_t size,
                                      utf_validation_t validation)
    {
        ST_ASSERT(size < ST_HUGE_BUFFER_SIZE, "String data buffer is too large");

        size_t u32size = _ST_PRIVATE::utf32_measure_from_utf8(utf8, size);
        if (u32size == 0)
            return null;

        utf32_buffer result;
        result.allocate(u32size);
        auto error = _ST_PRIVATE::utf32_convert_from_utf8(result.data(), utf8,
                                                          size, validation);
        _ST_PRIVATE::raise_conversion_error(error);

        return result;
    }

    inline utf32_buffer utf8_to_utf32(const char_buffer &utf8, utf_validation_t validation)
    {
        return utf8_to_utf32(utf8.data(), utf8.size(), validation);
    }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
    inline utf32_buffer utf8_to_utf32(const char8_t *utf8, size_t size,
                                      utf_validation_t validation)
    {
        return utf8_to_utf32(reinterpret_cast<const char *>(utf8), size, validation);
    }
#endif

    inline utf32_buffer utf16_to_utf32(const char16_t *utf16, size_t size,
                                       utf_validation_t validation)
    {
        ST_ASSERT(size < ST_HUGE_BUFFER_SIZE, "String data buffer is too large");

        size_t u32size = _ST_PRIVATE::utf32_measure_from_utf16(utf16, size);
        if (u32size == 0)
            return null;

        utf32_buffer result;
        result.allocate(u32size);
        auto error = _ST_PRIVATE::utf32_convert_from_utf16(result.data(), utf16,
                                                           size, validation);
        _ST_PRIVATE::raise_conversion_error(error);

        return result;
    }

    inline utf32_buffer utf16_to_utf32(const utf16_buffer &utf16, utf_validation_t validation)
    {
        return utf16_to_utf32(utf16.data(), utf16.size(), validation);
    }

    template <typename WcType = wchar_t>
    typename std::enable_if<sizeof(WcType) == sizeof(char16_t), utf32_buffer>::type
    wchar_to_utf32(const wchar_t *wstr, size_t size, utf_validation_t validation)
    {
        return utf16_to_utf32(reinterpret_cast<const char16_t *>(wstr), size, validation);
    }

    template <typename WcType = wchar_t>
    typename std::enable_if<sizeof(WcType) == sizeof(char32_t), utf32_buffer>::type
    wchar_to_utf32(const wchar_t *wstr, size_t size, utf_validation_t validation)
    {
        (void)validation;
        return utf32_buffer(reinterpret_cast<const char32_t *>(wstr), size);
    }

    inline utf32_buffer wchar_to_utf32(const wchar_buffer &wstr, utf_validation_t validation)
    {
        return wchar_to_utf32(wstr.data(), wstr.size(), validation);
    }

    inline utf32_buffer latin_1_to_utf32(const char *astr, size_t size)
    {
        ST_ASSERT(size < ST_HUGE_BUFFER_SIZE, "String data buffer is too large");

        utf32_buffer result;
        if (!astr || size == 0)
            return result;

        result.allocate(size);
        _ST_PRIVATE::utf32_convert_from_latin_1(result.data(), astr, size);

        return result;
    }

    inline utf32_buffer latin_1_to_utf32(const char_buffer &astr)
    {
        return latin_1_to_utf32(astr.data(), astr.size());
    }

    template <typename WcType = wchar_t>
    typename std::enable_if<sizeof(WcType) == sizeof(char16_t), wchar_buffer>::type
    utf8_to_wchar(const char *utf8, size_t size, utf_validation_t validation)
    {
        ST_ASSERT(size < ST_HUGE_BUFFER_SIZE, "String data buffer is too large");

        size_t u16size = _ST_PRIVATE::utf16_measure_from_utf8(utf8, size);
        if (u16size == 0)
            return null;

        wchar_buffer result;
        result.allocate(u16size);
        auto error = _ST_PRIVATE::utf16_convert_from_utf8(reinterpret_cast<char16_t *>(result.data()),
                                                          utf8, size, validation);
        _ST_PRIVATE::raise_conversion_error(error);

        return result;
    }

    template <typename WcType = wchar_t>
    typename std::enable_if<sizeof(WcType) == sizeof(char32_t), wchar_buffer>::type
    utf8_to_wchar(const char *utf8, size_t size, utf_validation_t validation)
    {
        ST_ASSERT(size < ST_HUGE_BUFFER_SIZE, "String data buffer is too large");

        size_t u32size = _ST_PRIVATE::utf32_measure_from_utf8(utf8, size);
        if (u32size == 0)
            return null;

        wchar_buffer result;
        result.allocate(u32size);
        auto error = _ST_PRIVATE::utf32_convert_from_utf8(reinterpret_cast<char32_t *>(result.data()),
                                                          utf8, size, validation);
        _ST_PRIVATE::raise_conversion_error(error);

        return result;
    }

    inline wchar_buffer utf8_to_wchar(const char_buffer &utf8, utf_validation_t validation)
    {
        return utf8_to_wchar(utf8.data(), utf8.size(), validation);
    }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
    inline wchar_buffer utf8_to_wchar(const char8_t *utf8, size_t size,
                                      utf_validation_t validation)
    {
        return utf8_to_wchar(reinterpret_cast<const char *>(utf8), size, validation);
    }
#endif

    template <typename WcType = wchar_t>
    typename std::enable_if<sizeof(WcType) == sizeof(char16_t), wchar_buffer>::type
    utf16_to_wchar(const char16_t *utf16, size_t size, utf_validation_t validation)
    {
        (void)validation;
        return wchar_buffer(reinterpret_cast<const wchar_t *>(utf16), size);
    }

    template <typename WcType = wchar_t>
    typename std::enable_if<sizeof(WcType) == sizeof(char32_t), wchar_buffer>::type
    utf16_to_wchar(const char16_t *utf16, size_t size, utf_validation_t validation)
    {
        ST_ASSERT(size < ST_HUGE_BUFFER_SIZE, "String data buffer is too large");

        size_t u32size = _ST_PRIVATE::utf32_measure_from_utf16(utf16, size);
        if (u32size == 0)
            return null;

        wchar_buffer result;
        result.allocate(u32size);
        auto error = _ST_PRIVATE::utf32_convert_from_utf16(reinterpret_cast<char32_t *>(result.data()),
                                                           utf16, size, validation);
        _ST_PRIVATE::raise_conversion_error(error);

        return result;
    }

    inline wchar_buffer utf16_to_wchar(const utf16_buffer &utf16, utf_validation_t validation)
    {
        return utf16_to_wchar(utf16.data(), utf16.size(), validation);
    }

    template <typename WcType = wchar_t>
    typename std::enable_if<sizeof(WcType) == sizeof(char16_t), wchar_buffer>::type
    utf32_to_wchar(const char32_t *utf32, size_t size, utf_validation_t validation)
    {
        ST_ASSERT(size < ST_HUGE_BUFFER_SIZE, "String data buffer is too large");

        size_t u16size = _ST_PRIVATE::utf16_measure_from_utf32(utf32, size);
        if (u16size == 0)
            return null;

        wchar_buffer result;
        result.allocate(u16size);
        auto error = _ST_PRIVATE::utf16_convert_from_utf32(reinterpret_cast<char16_t *>(result.data()),
                                                           utf32, size, validation);
        _ST_PRIVATE::raise_conversion_error(error);

        return result;
    }

    template <typename WcType = wchar_t>
    typename std::enable_if<sizeof(WcType) == sizeof(char32_t), wchar_buffer>::type
    utf32_to_wchar(const char32_t *utf32, size_t size, utf_validation_t validation)
    {
        (void)validation;
        return wchar_buffer(reinterpret_cast<const wchar_t *>(utf32), size);
    }

    inline wchar_buffer utf32_to_wchar(const utf32_buffer &utf32, utf_validation_t validation)
    {
        return utf32_to_wchar(utf32.data(), utf32.size(), validation);
    }

    template <typename WcType = wchar_t>
    typename std::enable_if<sizeof(WcType) == sizeof(char16_t), wchar_buffer>::type
    latin_1_to_wchar(const char *astr, size_t size)
    {
        ST_ASSERT(size < ST_HUGE_BUFFER_SIZE, "String data buffer is too large");

        wchar_buffer result;
        if (!astr || size == 0)
            return result;

        result.allocate(size);
        _ST_PRIVATE::utf16_convert_from_latin_1(reinterpret_cast<char16_t *>(result.data()),
                                                astr, size);

        return result;
    }

    template <typename WcType = wchar_t>
    typename std::enable_if<sizeof(WcType) == sizeof(char32_t), wchar_buffer>::type
    latin_1_to_wchar(const char *astr, size_t size)
    {
        ST_ASSERT(size < ST_HUGE_BUFFER_SIZE, "String data buffer is too large");

        wchar_buffer result;
        if (!astr || size == 0)
            return result;

        result.allocate(size);
        _ST_PRIVATE::utf32_convert_from_latin_1(reinterpret_cast<char32_t *>(result.data()),
                                                astr, size);

        return result;
    }

    inline wchar_buffer latin_1_to_wchar(const char_buffer &astr)
    {
        return latin_1_to_wchar(astr.data(), astr.size());
    }

    inline char_buffer utf8_to_latin_1(const char *utf8, size_t size,
                                       utf_validation_t validation,
                                       bool substitute_out_of_range = true)
    {
        ST_ASSERT(size < ST_HUGE_BUFFER_SIZE, "String data buffer is too large");

        size_t asize = _ST_PRIVATE::latin_1_measure_from_utf8(utf8, size);
        if (asize == 0)
            return null;

        char_buffer result;
        result.allocate(asize);
        auto error = _ST_PRIVATE::latin_1_convert_from_utf8(result.data(), utf8,
                                                            size, validation,
                                                            substitute_out_of_range);
        _ST_PRIVATE::raise_conversion_error(error);

        return result;
    }

    inline char_buffer utf8_to_latin_1(const char_buffer &utf8,
                                       utf_validation_t validation,
                                       bool substitute_out_of_range = true)
    {
        return utf8_to_latin_1(utf8.data(), utf8.size(), validation,
                               substitute_out_of_range);
    }

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
    inline char_buffer utf8_to_latin_1(const char8_t *utf8, size_t size,
                                       utf_validation_t validation,
                                       bool substitute_out_of_range = true)
    {
        return utf8_to_latin_1(reinterpret_cast<const char *>(utf8), size,
                               validation, substitute_out_of_range);
    }
#endif

    inline char_buffer utf16_to_latin_1(const char16_t *utf16, size_t size,
                                        utf_validation_t validation,
                                        bool substitute_out_of_range = true)
    {
        ST_ASSERT(size < ST_HUGE_BUFFER_SIZE, "String data buffer is too large");

        size_t asize = _ST_PRIVATE::latin_1_measure_from_utf16(utf16, size);
        if (asize == 0)
            return null;

        char_buffer result;
        result.allocate(asize);
        auto error = _ST_PRIVATE::latin_1_convert_from_utf16(result.data(), utf16,
                                                             size, validation,
                                                             substitute_out_of_range);
        _ST_PRIVATE::raise_conversion_error(error);

        return result;
    }

    inline char_buffer utf16_to_latin_1(const utf16_buffer &utf16,
                                        utf_validation_t validation,
                                        bool substitute_out_of_range = true)
    {
        return utf16_to_latin_1(utf16.data(), utf16.size(), validation,
                                substitute_out_of_range);
    }

    inline char_buffer utf32_to_latin_1(const char32_t *utf32, size_t size,
                                        utf_validation_t validation,
                                        bool substitute_out_of_range = true)
    {
        ST_ASSERT(size < ST_HUGE_BUFFER_SIZE, "String data buffer is too large");

        char_buffer result;
        result.allocate(size);
        auto error = _ST_PRIVATE::latin_1_convert_from_utf32(result.data(), utf32,
                                                             size, validation,
                                                             substitute_out_of_range);
        _ST_PRIVATE::raise_conversion_error(error);

        return result;
    }

    inline char_buffer utf32_to_latin_1(const utf32_buffer &utf32,
                                        utf_validation_t validation,
                                        bool substitute_out_of_range = true)
    {
        return utf32_to_latin_1(utf32.data(), utf32.size(), validation,
                                substitute_out_of_range);
    }

    template <typename WcType = wchar_t>
    typename std::enable_if<sizeof(WcType) == sizeof(char16_t), char_buffer>::type
    wchar_to_latin_1(const wchar_t *wstr, size_t size, utf_validation_t validation,
                     bool substitute_out_of_range = true)
    {
        return utf16_to_latin_1(reinterpret_cast<const char16_t *>(wstr), size,
                                validation, substitute_out_of_range);
    }

    template <typename WcType = wchar_t>
    typename std::enable_if<sizeof(WcType) == sizeof(char32_t), char_buffer>::type
    wchar_to_latin_1(const wchar_t *wstr, size_t size, utf_validation_t validation,
                     bool substitute_out_of_range = true)
    {
        return utf32_to_latin_1(reinterpret_cast<const char32_t *>(wstr), size,
                                validation, substitute_out_of_range);
    }

    inline char_buffer wchar_to_latin_1(const wchar_buffer &wstr,
                                        utf_validation_t validation,
                                        bool substitute_out_of_range = true)
    {
        return wchar_to_latin_1(wstr.data(), wstr.size(), validation,
                                substitute_out_of_range);
    }
}

#endif // _ST_UTF_CONV_H
