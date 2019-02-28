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

#include "st_string.h"

#include <locale>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "st_assert.h"
#include "st_stringstream.h"
#include "st_format_simple.h"

#if !defined(ST_WCHAR_BYTES) || ((ST_WCHAR_BYTES != 2) && (ST_WCHAR_BYTES != 4))
#   error ST_WCHAR_SIZE must either be 2 (16-bit) or 4 (32-bit)
#endif

#if !defined(ST_SIZET_BYTES) || ((ST_SIZET_BYTES != 4) && (ST_SIZET_BYTES != 8))
#   error Supported size_t sizes are 4 (32-bit) or 8 (64-bit) bytes
#endif

#if defined(_MSC_VER) && (_MSC_VER < 1800)
#   define strtoll  _strtoi64
#   define strtoull _strtoui64
#endif

#define BADCHAR_SUBSTITUTE          0xFFFDul
#define BADCHAR_SUBSTITUTE_UTF8     "\xEF\xBF\xBD"
#define BADCHAR_SUBSTITUTE_UTF8_LEN 3

// This is 256MiB worth of UTF-8 string data
#define HUGE_BUFFER_SIZE 0x10000000


void ST::string::_convert_from_utf8(const char *utf8, size_t size,
                                    utf_validation_t validation)
{
    ST_ASSERT(size < HUGE_BUFFER_SIZE, "String data buffer is too large");

    if (!utf8) {
        m_buffer = char_buffer();
        return;
    }

    set(char_buffer(utf8, size), validation);
}

#define _CHECK_NEXT_SEQ_BYTE() \
    do { \
        ++cp; \
        if ((*cp & 0xC0) != 0x80) \
            return false; \
    } while (false)

static bool _validate_utf8(const ST::char_buffer &buffer)
{
    const unsigned char *cp = reinterpret_cast<const unsigned char *>(buffer.data());
    const unsigned char *ep = cp + buffer.size();
    for (; cp < ep; ++cp) {
        if (*cp < 0x80)
            continue;

        if ((*cp & 0xE0) == 0xC0) {
            // Two bytes
            if (cp + 2 > ep)
                return false;
            _CHECK_NEXT_SEQ_BYTE();
        } else if ((*cp & 0xF0) == 0xE0) {
            // Three bytes
            if (cp + 3 > ep)
                return false;
            _CHECK_NEXT_SEQ_BYTE();
            _CHECK_NEXT_SEQ_BYTE();
        } else if ((*cp & 0xF8) == 0xF0) {
            // Four bytes
            if (cp + 4 > ep)
                return false;
            _CHECK_NEXT_SEQ_BYTE();
            _CHECK_NEXT_SEQ_BYTE();
            _CHECK_NEXT_SEQ_BYTE();
        } else {
            // Invalid sequence byte
            return false;
        }
    }

    return true;
}

static ST::char_buffer _cleanup_utf8_buffer(const ST::char_buffer &buffer)
{
    ST::string_stream ss_clean;

    const unsigned char *sp = reinterpret_cast<const unsigned char *>(buffer.data());
    const unsigned char *ep = sp + buffer.size();
    while (sp < ep) {
        if (*sp < 0x80) {
            ss_clean.append_char(*sp++);
        } else if ((*sp & 0xE0) == 0xC0) {
            // Two bytes
            if (sp + 2 > ep || (sp[1] & 0xC0) != 0x80)
                ss_clean.append(BADCHAR_SUBSTITUTE_UTF8, BADCHAR_SUBSTITUTE_UTF8_LEN);
            else
                ss_clean.append(reinterpret_cast<const char *>(sp), 2);
            sp += 2;
        } else if ((*sp & 0xF0) == 0xE0) {
            // Three bytes
            if (sp + 3 > ep || (sp[1] & 0xC0) != 0x80 || (sp[2] & 0xC0) != 0x80)
                ss_clean.append(BADCHAR_SUBSTITUTE_UTF8, BADCHAR_SUBSTITUTE_UTF8_LEN);
            else
                ss_clean.append(reinterpret_cast<const char *>(sp), 3);
            sp += 3;
        } else if ((*sp & 0xF8) == 0xF0) {
            // Four bytes
            if (sp + 4 > ep || (sp[1] & 0xC0) != 0x80 || (sp[2] & 0xC0) != 0x80
                            || (sp[3] & 0xC0) != 0x80)
                ss_clean.append(BADCHAR_SUBSTITUTE_UTF8, BADCHAR_SUBSTITUTE_UTF8_LEN);
            else
                ss_clean.append(reinterpret_cast<const char *>(sp), 4);
            sp += 4;
        } else {
            // Invalid sequence byte
            ss_clean.append(BADCHAR_SUBSTITUTE_UTF8, BADCHAR_SUBSTITUTE_UTF8_LEN);
        }
    }

    return ST::char_buffer(ss_clean.raw_buffer(), ss_clean.size());
}

void ST::string::set(const char_buffer &init, utf_validation_t validation)
{
    switch (validation) {
    case assert_validity:
        ST_ASSERT(_validate_utf8(init), "Invalid UTF-8 sequence");
        m_buffer = init;
        break;

    case check_validity:
        if (!_validate_utf8(init))
            throw ST::unicode_error("Invalid UTF-8 sequence");
        m_buffer = init;
        break;

    case substitute_invalid:
        m_buffer = _cleanup_utf8_buffer(init);
        break;

    case assume_valid:
        m_buffer = init;
        break;

    default:
        ST_ASSERT(false, "Invalid validation type");
    }
}

#ifdef ST_HAVE_RVALUE_MOVE
void ST::string::set(char_buffer &&init, utf_validation_t validation)
{
    switch (validation) {
    case assert_validity:
        ST_ASSERT(_validate_utf8(init), "Invalid UTF-8 sequence");
        m_buffer = std::move(init);
        break;

    case check_validity:
        if (!_validate_utf8(init))
            throw ST::unicode_error("Invalid UTF-8 sequence");
        m_buffer = std::move(init);
        break;

    case substitute_invalid:
        m_buffer = _cleanup_utf8_buffer(init);
        break;

    case assume_valid:
        m_buffer = std::move(init);
        break;

    default:
        ST_ASSERT(false, "Invalid validation type");
    }
}
#endif

ST::string &ST::string::operator+=(const char *cstr)
{
    set(*this + cstr);
    return *this;
}

ST::string &ST::string::operator+=(const wchar_t *wstr)
{
    set(*this + wstr);
    return *this;
}

#ifdef ST_HAVE_CHAR_TYPES
ST::string &ST::string::operator+=(const char16_t *cstr)
{
    set(*this + cstr);
    return *this;
}

ST::string &ST::string::operator+=(const char32_t *cstr)
{
    set(*this + cstr);
    return *this;
}
#endif

ST::string &ST::string::operator+=(const ST::string &other)
{
    set(*this + other);
    return *this;
}

ST::string &ST::string::operator+=(char ch)
{
    set(*this + ch);
    return *this;
}

#ifdef ST_HAVE_CHAR_TYPES
ST::string &ST::string::operator+=(char16_t ch)
{
    set(*this + ch);
    return *this;
}

ST::string &ST::string::operator+=(char32_t ch)
{
    set(*this + ch);
    return *this;
}
#endif

ST::string &ST::string::operator+=(wchar_t ch)
{
    set(*this + ch);
    return *this;
}

void ST::string::_convert_from_utf16(const char16_t *utf16, size_t size,
                                     utf_validation_t validation)
{
    ST_ASSERT(size < HUGE_BUFFER_SIZE, "String data buffer is too large");

    m_buffer = char_buffer();
    if (!utf16)
        return;

    string_stream converted;
    const char16_t *sp = utf16;
    const char16_t *ep = sp + size;
    for (; sp < ep; ++sp) {
        if (*sp < 0x80) {
            converted.append_char(*sp);
        } else if (*sp < 0x800) {
            converted.append_char(0xC0 | ((*sp >>  6) & 0x1F));
            converted.append_char(0x80 | ((*sp      ) & 0x3F));
        } else if (*sp >= 0xD800 && *sp <= 0xDFFF) {
            // Surrogate pair
            if (sp + 1 >= ep) {
                switch (validation) {
                case check_validity:
                    throw ST::unicode_error("Incomplete surrogate pair");
                case assert_validity:
                    ST_ASSERT(false, "Incomplete surrogate pair");
                    /* fall through */
                case substitute_invalid:
                    converted.append(BADCHAR_SUBSTITUTE_UTF8, BADCHAR_SUBSTITUTE_UTF8_LEN);
                    break;
                case assume_valid:
                    // Encode the bad surrogate char as a UTF-8 value
                    converted.append_char(0xE0 | ((*sp >> 12) & 0x0F));
                    converted.append_char(0x80 | ((*sp >>  6) & 0x3F));
                    converted.append_char(0x80 | ((*sp      ) & 0x3F));
                    break;
                default:
                    ST_ASSERT(false, "Invalid validation type");
                }
            } else if (*sp < 0xDC00) {
                if (sp[1] >= 0xDC00 && sp[1] <= 0xDFFF) {
                    char32_t bigch = 0x10000 + ((sp[0] & 0x3FFF) << 10) + (sp[1] & 0x3FF);
                    converted.append_char(0xF0 | ((bigch >> 18) & 0x07));
                    converted.append_char(0x80 | ((bigch >> 12) & 0x3F));
                    converted.append_char(0x80 | ((bigch >>  6) & 0x3F));
                    converted.append_char(0x80 | ((bigch      ) & 0x3F));
                    ++sp;
                } else {
                    switch (validation) {
                    case check_validity:
                        throw ST::unicode_error("Incomplete surrogate pair");
                    case assert_validity:
                        ST_ASSERT(false, "Incomplete surrogate pair");
                        /* fall through */
                    case substitute_invalid:
                        converted.append(BADCHAR_SUBSTITUTE_UTF8, BADCHAR_SUBSTITUTE_UTF8_LEN);
                        break;
                    case assume_valid:
                        // Encode the bad surrogate char as a UTF-8 value
                        converted.append_char(0xE0 | ((*sp >> 12) & 0x0F));
                        converted.append_char(0x80 | ((*sp >>  6) & 0x3F));
                        converted.append_char(0x80 | ((*sp      ) & 0x3F));
                        break;
                    default:
                        ST_ASSERT(false, "Invalid validation type");
                    }
                }
            } else {
                if (sp[1] >= 0xD800 && sp[1] <= 0xDBFF) {
                    char32_t bigch = 0x10000 + (sp[0] & 0x3FFF) + ((sp[1] & 0x3FF) << 10);
                    converted.append_char(0xF0 | ((bigch >> 18) & 0x07));
                    converted.append_char(0x80 | ((bigch >> 12) & 0x3F));
                    converted.append_char(0x80 | ((bigch >>  6) & 0x3F));
                    converted.append_char(0x80 | ((bigch      ) & 0x3F));
                    ++sp;
                } else {
                    switch (validation) {
                    case check_validity:
                        throw ST::unicode_error("Incomplete surrogate pair");
                    case assert_validity:
                        ST_ASSERT(false, "Incomplete surrogate pair");
                        /* fall through */
                    case substitute_invalid:
                        converted.append(BADCHAR_SUBSTITUTE_UTF8, BADCHAR_SUBSTITUTE_UTF8_LEN);
                        break;
                    case assume_valid:
                        // Encode the bad surrogate char as a UTF-8 value
                        converted.append_char(0xE0 | ((*sp >> 12) & 0x0F));
                        converted.append_char(0x80 | ((*sp >>  6) & 0x3F));
                        converted.append_char(0x80 | ((*sp      ) & 0x3F));
                        break;
                    default:
                        ST_ASSERT(false, "Invalid validation type");
                    }
                }
            }
        } else {
            converted.append_char(0xE0 | ((*sp >> 12) & 0x0F));
            converted.append_char(0x80 | ((*sp >>  6) & 0x3F));
            converted.append_char(0x80 | ((*sp      ) & 0x3F));
        }
    }

    m_buffer = char_buffer(converted.raw_buffer(), converted.size());
}

void ST::string::_convert_from_utf32(const char32_t *utf32, size_t size,
                                     utf_validation_t validation)
{
    ST_ASSERT(size < HUGE_BUFFER_SIZE, "String data buffer is too large");

    m_buffer = char_buffer();
    if (!utf32)
        return;

    // Calculate the UTF-8 size
    size_t convlen = 0;
    const char32_t *sp = utf32;
    const char32_t *ep = sp + size;
    for (; sp < ep; ++sp) {
        if (*sp < 0x80) {
            convlen += 1;
        } else if (*sp < 0x800) {
            convlen += 2;
        } else if (*sp < 0x10000) {
            convlen += 3;
        } else if (*sp <= 0x10FFFF) {
            convlen += 4;
        } else {
            // Out-of-range code point always gets replaced
            convlen += BADCHAR_SUBSTITUTE_UTF8_LEN;
        }
    }

    // Perform the actual conversion
    m_buffer.allocate(convlen);
    char *dp = m_buffer.data();
    sp = utf32;
    for (; sp < ep; ++sp) {
        if (*sp < 0x80) {
            *dp++ = static_cast<char>(*sp);
        } else if (*sp < 0x800) {
            *dp++ = 0xC0 | ((*sp >>  6) & 0x1F);
            *dp++ = 0x80 | ((*sp      ) & 0x3F);
        } else if (*sp < 0x10000) {
            *dp++ = 0xE0 | ((*sp >> 12) & 0x0F);
            *dp++ = 0x80 | ((*sp >>  6) & 0x3F);
            *dp++ = 0x80 | ((*sp      ) & 0x3F);
        } else if (*sp <= 0x10FFFF) {
            *dp++ = 0xF0 | ((*sp >> 18) & 0x07);
            *dp++ = 0x80 | ((*sp >> 12) & 0x3F);
            *dp++ = 0x80 | ((*sp >>  6) & 0x3F);
            *dp++ = 0x80 | ((*sp      ) & 0x3F);
        } else {
            if (validation == check_validity)
                throw ST::unicode_error("Unicode character out of range");
            else if (validation == assert_validity)
                ST_ASSERT(false, "Unicode character out of range");
            std::char_traits<char>::copy(dp, BADCHAR_SUBSTITUTE_UTF8, BADCHAR_SUBSTITUTE_UTF8_LEN);
            dp += BADCHAR_SUBSTITUTE_UTF8_LEN;
        }
    }
}

void ST::string::_convert_from_wchar(const wchar_t *wstr, size_t size,
                                     utf_validation_t validation)
{
#if ST_WCHAR_BYTES == 2
    _convert_from_utf16(reinterpret_cast<const char16_t *>(wstr), size, validation);
#else
    _convert_from_utf32(reinterpret_cast<const char32_t *>(wstr), size, validation);
#endif
}

void ST::string::_convert_from_latin_1(const char *astr, size_t size)
{
    ST_ASSERT(size < HUGE_BUFFER_SIZE, "String data buffer is too large");

    m_buffer = char_buffer();
    if (!astr)
        return;

    // Calculate the UTF-8 size
    size_t convlen = 0;
    const char *sp = astr;
    const char *ep = sp + size;
    for (; sp < ep; ++sp) {
        if (*sp & 0x80)
            convlen += 2;
        else
            convlen += 1;
    }

    // Perform the actual conversion
    m_buffer.allocate(convlen);
    char *utf8 = m_buffer.data();
    char *dp = utf8;
    sp = astr;
    for (; sp < ep; ++sp) {
        if (*sp & 0x80) {
            *dp++ = 0xC0 | ((static_cast<unsigned char>(*sp) >> 6) & 0x1F);
            *dp++ = 0x80 | ((static_cast<unsigned char>(*sp)     ) & 0x3F);
        } else {
            *dp++ = *sp;
        }
    }
}

ST::utf16_buffer ST::string::to_utf16() const
{
    utf16_buffer result;
    if (empty())
        return result;

    // Calculate the UTF-16 size
    size_t convlen = 0;
    const unsigned char *utf8 = reinterpret_cast<const unsigned char *>(m_buffer.data());
    const unsigned char *sp = utf8;
    const unsigned char *ep = sp + m_buffer.size();
    while (sp < ep) {
        if (*sp < 0x80) {
            sp += 1;
        } else if ((*sp & 0xE0) == 0xC0) {
            sp += 2;
        } else if ((*sp & 0xF0) == 0xE0) {
            sp += 3;
        } else {
            // Encode with surrogate pair
            ++convlen;
            sp += 4;
        }
        ++convlen;
    }

    // Perform the actual conversion
    result.allocate(convlen);
    char16_t *dp = result.data();
    sp = utf8;
    while (sp < ep) {
        char32_t bigch = 0;
        if (*sp < 0x80) {
            *dp++ = *sp++;
        } else if ((*sp & 0xE0) == 0xC0) {
            bigch  = (*sp++ & 0x1F) << 6;
            bigch |= (*sp++ & 0x3F);
            *dp++ = static_cast<char16_t>(bigch);
        } else if ((*sp & 0xF0) == 0xE0) {
            bigch  = (*sp++ & 0x0F) << 12;
            bigch |= (*sp++ & 0x3F) << 6;
            bigch |= (*sp++ & 0x3F);
            *dp++ = static_cast<char16_t>(bigch);
        } else {
            bigch  = (*sp++ & 0x07) << 18;
            bigch |= (*sp++ & 0x3F) << 12;
            bigch |= (*sp++ & 0x3F) << 6;
            bigch |= (*sp++ & 0x3F);
            bigch -= 0x10000;

            *dp++ = 0xD800 | ((bigch >> 10) & 0x3FF);
            *dp++ = 0xDC00 | ((bigch      ) & 0x3FF);
        }
    }

    return result;
}

ST::utf32_buffer ST::string::to_utf32() const
{
    utf32_buffer result;
    if (empty())
        return result;

    // Calculate the UTF-32 size
    size_t convlen = 0;
    const unsigned char *utf8 = reinterpret_cast<const unsigned char *>(m_buffer.data());
    const unsigned char *sp = utf8;
    const unsigned char *ep = sp + m_buffer.size();
    while (sp < ep) {
        if (*sp < 0x80)
            sp += 1;
        else if ((*sp & 0xE0) == 0xC0)
            sp += 2;
        else if ((*sp & 0xF0) == 0xE0)
            sp += 3;
        else
            sp += 4;
        ++convlen;
    }

    // Perform the actual conversion
    result.allocate(convlen);
    char32_t *dp = result.data();
    sp = utf8;
    while (sp < ep) {
        char32_t bigch = 0;
        if (*sp < 0x80) {
            bigch = *sp++;
        } else if ((*sp & 0xE0) == 0xC0) {
            bigch  = (*sp++ & 0x1F) << 6;
            bigch |= (*sp++ & 0x3F);
        } else if ((*sp & 0xF0) == 0xE0) {
            bigch  = (*sp++ & 0x0F) << 12;
            bigch |= (*sp++ & 0x3F) << 6;
            bigch |= (*sp++ & 0x3F);
        } else {
            bigch  = (*sp++ & 0x07) << 18;
            bigch |= (*sp++ & 0x3F) << 12;
            bigch |= (*sp++ & 0x3F) << 6;
            bigch |= (*sp++ & 0x3F);
        }
        *dp++ = bigch;
    }

    return result;
}

ST::wchar_buffer ST::string::to_wchar() const
{
#if ST_WCHAR_BYTES == 2
    utf16_buffer utf16 = to_utf16();
    return wchar_buffer(reinterpret_cast<const wchar_t *>(utf16.data()), utf16.size());
#else
    utf32_buffer utf32 = to_utf32();
    return wchar_buffer(reinterpret_cast<const wchar_t *>(utf32.data()), utf32.size());
#endif
}

ST::char_buffer ST::string::to_latin_1(utf_validation_t validation) const
{
    char_buffer result;
    if (empty())
        return result;

    // Calculate the ASCII size
    size_t convlen = 0;
    const unsigned char *utf8 = reinterpret_cast<const unsigned char *>(m_buffer.data());
    const unsigned char *sp = utf8;
    const unsigned char *ep = sp + m_buffer.size();
    while (sp < ep) {
        if (*sp < 0x80)
            sp += 1;
        else if ((*sp & 0xE0) == 0xC0)
            sp += 2;
        else if ((*sp & 0xF0) == 0xE0)
            sp += 3;
        else
            sp += 4;
        ++convlen;
    }

    // Perform the actual conversion
    result.allocate(convlen);
    char *dp = result.data();
    sp = utf8;
    while (sp < ep) {
        char32_t bigch = 0;
        if (*sp < 0x80) {
            bigch = *sp++;
        } else if ((*sp & 0xE0) == 0xC0) {
            bigch  = (*sp++ & 0x1F) << 6;
            bigch |= (*sp++ & 0x3F);
        } else if ((*sp & 0xF0) == 0xE0) {
            bigch  = (*sp++ & 0x0F) << 12;
            bigch |= (*sp++ & 0x3F) << 6;
            bigch |= (*sp++ & 0x3F);
        } else {
            bigch  = (*sp++ & 0x07) << 18;
            bigch |= (*sp++ & 0x3F) << 12;
            bigch |= (*sp++ & 0x3F) << 6;
            bigch |= (*sp++ & 0x3F);
        }

        if (bigch >= 0x100) {
            switch (validation) {
            case check_validity:
                throw ST::unicode_error("Latin-1 character out of range");
            case assert_validity:
                ST_ASSERT(false, "Latin-1 character out of range");
                /* fall through */
            default:
                bigch = '?';
            }
        }
        *dp++ = static_cast<char>(bigch);
    }

    return result;
}

template <typename int_T>
static ST::string _mini_format_numeric_s(int radix, bool upper_case, int_T value)
{
    typedef typename std::make_unsigned<int_T>::type uint_T;
    ST::uint_formatter<uint_T> formatter;
    formatter.format(ST::safe_abs(value), radix, upper_case);

    ST::char_buffer result;
    if (value < 0) {
        result.allocate(formatter.size() + 1);
        std::char_traits<char>::copy(result.data() + 1, formatter.text(), formatter.size());
        result[0] = '-';
    } else {
        result.allocate(formatter.size());
        std::char_traits<char>::copy(result.data(), formatter.text(), formatter.size());
    }

    return ST::string(result, ST::assume_valid);
}

template <typename uint_T>
static ST::string _mini_format_numeric_u(int radix, bool upper_case, uint_T value)
{
    ST::uint_formatter<uint_T> formatter;
    formatter.format(value, radix, upper_case);

    ST::char_buffer result;
    result.allocate(formatter.size());
    std::char_traits<char>::copy(result.data(), formatter.text(), formatter.size());

    return ST::string(result, ST::assume_valid);
}

ST::string ST::string::from_int(int value, int base, bool upper_case)
{
    return _mini_format_numeric_s<int>(base, upper_case, value);
}

ST::string ST::string::from_uint(unsigned int value, int base, bool upper_case)
{
    return _mini_format_numeric_u<unsigned int>(base, upper_case, value);
}

template <typename float_T>
static ST::string _mini_format_float(float_T value, char format)
{
    ST::float_formatter<float_T> formatter;
    formatter.format(value, format);

    ST::char_buffer result;
    result.allocate(formatter.size());
    std::char_traits<char>::copy(result.data(), formatter.text(), formatter.size());
    return ST::string(result, ST::assume_valid);
}

ST::string ST::string::from_float(float value, char format)
{
    return _mini_format_float<float>(value, format);
}

ST::string ST::string::from_double(double value, char format)
{
    return _mini_format_float<double>(value, format);
}

#ifdef ST_HAVE_INT64
ST::string ST::string::from_int64(int64_t value, int base, bool upper_case)
{
    return _mini_format_numeric_s<int64_t>(base, upper_case, value);
}

ST::string ST::string::from_uint64(uint64_t value, int base, bool upper_case)
{
    return _mini_format_numeric_u<uint64_t>(base, upper_case, value);
}
#endif

int ST::string::to_int(int base) const ST_NOEXCEPT
{
    return static_cast<int>(strtol(c_str(), ST_NULLPTR, base));
}

int ST::string::to_int(ST::conversion_result &result, int base) const ST_NOEXCEPT
{
    if (empty()) {
        result.m_flags = ST::conversion_result::result_full_match;
        return 0;
    }

    char *end;
    int value = static_cast<int>(strtol(c_str(), &end, base));
    result.m_flags = 0;
    if (end != c_str())
        result.m_flags |= ST::conversion_result::result_ok;
    if (end == c_str() + size())
        result.m_flags |= ST::conversion_result::result_full_match;
    return value;
}

unsigned int ST::string::to_uint(int base) const ST_NOEXCEPT
{
    return static_cast<unsigned int>(strtoul(c_str(), ST_NULLPTR, base));
}

unsigned int ST::string::to_uint(ST::conversion_result &result, int base)
    const ST_NOEXCEPT
{
    if (empty()) {
        result.m_flags = ST::conversion_result::result_full_match;
        return 0;
    }

    char *end;
    unsigned int value = static_cast<unsigned int>(strtoul(c_str(), &end, base));
    result.m_flags = 0;
    if (end != c_str())
        result.m_flags |= ST::conversion_result::result_ok;
    if (end == c_str() + size())
        result.m_flags |= ST::conversion_result::result_full_match;
    return value;
}

float ST::string::to_float() const ST_NOEXCEPT
{
    // Use strtod to avoid requiring C99
    return static_cast<float>(strtod(c_str(), ST_NULLPTR));
}

float ST::string::to_float(ST::conversion_result &result) const ST_NOEXCEPT
{
    // Use strtod to avoid requiring C99
    return static_cast<float>(to_double(result));
}

double ST::string::to_double() const ST_NOEXCEPT
{
    return strtod(c_str(), ST_NULLPTR);
}

double ST::string::to_double(ST::conversion_result &result) const ST_NOEXCEPT
{
    if (empty()) {
        result.m_flags = ST::conversion_result::result_full_match;
        return 0;
    }

    char *end;
    double value = strtod(c_str(), &end);
    result.m_flags = 0;
    if (end != c_str())
        result.m_flags |= ST::conversion_result::result_ok;
    if (end == c_str() + size())
        result.m_flags |= ST::conversion_result::result_full_match;
    return value;
}

#ifdef ST_HAVE_INT64
int64_t ST::string::to_int64(int base) const ST_NOEXCEPT
{
    return static_cast<int64_t>(strtoll(c_str(), ST_NULLPTR, base));
}

int64_t ST::string::to_int64(ST::conversion_result &result, int base)
    const ST_NOEXCEPT
{
    char *end;
    int64_t value = static_cast<int64_t>(strtoll(c_str(), &end, base));
    result.m_flags = 0;
    if (end != c_str())
        result.m_flags = ST::conversion_result::result_ok;
    if (end == c_str() + size())
        result.m_flags |= ST::conversion_result::result_full_match;
    return value;
}

uint64_t ST::string::to_uint64(int base) const ST_NOEXCEPT
{
    return static_cast<uint64_t>(strtoull(c_str(), ST_NULLPTR, base));
}

uint64_t ST::string::to_uint64(ST::conversion_result &result, int base)
    const ST_NOEXCEPT
{
    char *end;
    uint64_t value = static_cast<uint64_t>(strtoull(c_str(), &end, base));
    result.m_flags = 0;
    if (end != c_str())
        result.m_flags = ST::conversion_result::result_ok;
    if (end == c_str() + size())
        result.m_flags |= ST::conversion_result::result_full_match;
    return value;
}
#endif

bool ST::string::to_bool() const ST_NOEXCEPT
{
    if (compare_i("true") == 0)
        return true;
    else if (compare_i("false") == 0)
        return false;
    return to_int() != 0;
}

bool ST::string::to_bool(conversion_result &result) const ST_NOEXCEPT
{
    if (compare_i("true") == 0) {
        result.m_flags = ST::conversion_result::result_ok
                       | ST::conversion_result::result_full_match;
        return true;
    } else if (compare_i("false") == 0) {
        result.m_flags = ST::conversion_result::result_ok
                       | ST::conversion_result::result_full_match;
        return false;
    }
    return to_int(result) != 0;
}

static int _compare_cs(const char *left, size_t lsize,
                       const char *right, size_t rsize)
{
    const size_t cmplen = std::min(lsize, rsize);
    const int cmp = std::char_traits<char>::compare(left, right, cmplen);
    return cmp ? cmp : lsize - rsize;
}

static int _compare_cs(const char *left, size_t lsize,
                       const char *right, size_t rsize, size_t maxlen)
{
    lsize = std::min(lsize, maxlen);
    rsize = std::min(rsize, maxlen);
    return _compare_cs(left, lsize, right, rsize);
}

static int _compare_ci(const char *left, const char *right, size_t fsize)
{
    const std::locale &c_locale = std::locale::classic();
    while (fsize--) {
        const char cl = std::tolower(*left++, c_locale);
        const char cr = std::tolower(*right++, c_locale);
        if (cl != cr)
            return cl - cr;
    }
    return 0;
}

static int _compare_ci(const char *left, size_t lsize,
                       const char *right, size_t rsize)
{
    const size_t cmplen = std::min(lsize, rsize);
    const int cmp = _compare_ci(left, right, cmplen);
    return cmp ? cmp : lsize - rsize;
}

static int _compare_ci(const char *left, size_t lsize,
                       const char *right, size_t rsize, size_t maxlen)
{
    lsize = std::min(lsize, maxlen);
    rsize = std::min(rsize, maxlen);
    return _compare_ci(left, lsize, right, rsize);
}

int ST::string::compare(const string &str, case_sensitivity_t cs) const ST_NOEXCEPT
{
    return (cs == case_sensitive) ? _compare_cs(c_str(), size(), str.c_str(), str.size())
                                  : _compare_ci(c_str(), size(), str.c_str(), str.size());
}

int ST::string::compare(const char *str, case_sensitivity_t cs) const ST_NOEXCEPT
{
    const size_t rsize = str ? std::char_traits<char>::length(str) : 0;
    return (cs == case_sensitive) ? _compare_cs(c_str(), size(), str ? str : "", rsize)
                                  : _compare_ci(c_str(), size(), str ? str : "", rsize);
}

int ST::string::compare_n(const string &str, size_t count,
                          case_sensitivity_t cs) const ST_NOEXCEPT
{
    return (cs == case_sensitive) ? _compare_cs(c_str(), size(), str.c_str(), str.size(), count)
                                  : _compare_ci(c_str(), size(), str.c_str(), str.size(), count);
}

int ST::string::compare_n(const char *str, size_t count,
                          case_sensitivity_t cs) const ST_NOEXCEPT
{
    const size_t rsize = str ? std::char_traits<char>::length(str) : 0;
    return (cs == case_sensitive) ? _compare_cs(c_str(), size(), str ? str : "", rsize, count)
                                  : _compare_ci(c_str(), size(), str ? str : "", rsize, count);
}

static const char *_stristr(const char *haystack, const char *needle)
{
    // The "easy" way
    size_t sublen = std::char_traits<char>::length(needle);
    const char *cp = haystack;
    const char *ep = cp + std::char_traits<char>::length(haystack);
    while (cp + sublen <= ep) {
        if (_compare_ci(cp, needle, sublen) == 0)
            return cp;
        ++cp;
    }
    return ST_NULLPTR;
}

static const char *_strichr(const char *haystack, int ch)
{
    if (ch > 0xFF || ch < 0)
        return ST_NULLPTR;

    const char *cp = haystack;
    const std::locale &c_locale = std::locale::classic();
    const int lch = std::tolower(static_cast<char>(ch), c_locale);
    while (*cp) {
        if (std::tolower(*cp, c_locale) == lch)
            return cp;
        ++cp;
    }
    return ST_NULLPTR;
}

ST_ssize_t ST::string::find(size_t start, char ch, case_sensitivity_t cs)
    const ST_NOEXCEPT
{
    if (start >= size())
        return -1;

    const char *cp = (cs == case_sensitive) ? strchr(c_str() + start, ch)
                                            : _strichr(c_str() + start, ch);
    return cp ? (cp - c_str()) : -1;
}

ST_ssize_t ST::string::find(size_t start, const char *substr, case_sensitivity_t cs)
    const ST_NOEXCEPT
{
    if (!substr || !substr[0] || start >= size())
        return -1;

    const char *cp = (cs == case_sensitive) ? strstr(c_str() + start, substr)
                                            : _stristr(c_str() + start, substr);
    return cp ? (cp - c_str()) : -1;
}

ST_ssize_t ST::string::find_last(size_t max, char ch, case_sensitivity_t cs) const ST_NOEXCEPT
{
    if (empty())
        return -1;

    const char *endp = c_str() + (max > size() ? size() : max);

    const char *start = c_str();
    const char *found = ST_NULLPTR;
    for ( ;; ) {
        const char *cp = (cs == case_sensitive) ? strchr(start, ch)
                                                : _strichr(start, ch);
        if (!cp || cp >= endp)
            break;
        found = cp;
        start = cp + 1;
    }
    return found ? (found - c_str()) : -1;
}

ST_ssize_t ST::string::find_last(size_t max, const char *substr, case_sensitivity_t cs)
    const ST_NOEXCEPT
{
    if (!substr || !substr[0] || empty())
        return -1;

    const char *endp = c_str() + (max > size() ? size() : max);

    const char *start = c_str();
    const char *found = ST_NULLPTR;
    for ( ;; ) {
        const char *cp = (cs == case_sensitive) ? strstr(start, substr)
                                                : _stristr(start, substr);
        if (!cp || cp >= endp)
            break;
        found = cp;
        start = cp + 1;
    }
    return found ? (found - c_str()) : -1;
}

ST::string ST::string::trim_left(const char *charset) const
{
    if (empty())
        return null;

    const char *cp = c_str();
    while (*cp && strchr(charset, *cp))
        ++cp;

    return substr(cp - c_str());
}

ST::string ST::string::trim_right(const char *charset) const
{
    if (empty())
        return null;

    const char *cp = c_str() + size();
    while (--cp >= c_str() && strchr(charset, *cp))
        ;

    return substr(0, cp - c_str() + 1);
}

ST::string ST::string::trim(const char *charset) const
{
    if (empty())
        return null;

    const char *lp = c_str();
    const char *rp = lp + size();
    while (*lp && strchr(charset, *lp))
        ++lp;
    while (--rp >= lp && strchr(charset, *rp))
        ;

    return substr(lp - c_str(), rp - lp + 1);
}

ST::string ST::string::substr(ST_ssize_t start, size_t count) const
{
    size_t max = size();

    if (count == ST_AUTO_SIZE)
        count = max;

    if (start < 0) {
        // Handle negative indexes from the right side of the string
        start += max;
        if (start < 0)
            start = 0;
    } else if (static_cast<size_t>(start) > max) {
        return null;
    }
    if (start + count > max)
        count = max - start;

    if (start == 0 && count == max)
        return *this;

    string sub;
    sub.m_buffer.allocate(count);
    std::char_traits<char>::copy(sub.m_buffer.data(), c_str() + start, count);

    return sub;
}

bool ST::string::starts_with(const ST::string &prefix, case_sensitivity_t cs) const ST_NOEXCEPT
{
    if (prefix.size() > size())
        return false;
    return compare_n(prefix, prefix.size(), cs) == 0;
}

bool ST::string::starts_with(const char *prefix, case_sensitivity_t cs) const ST_NOEXCEPT
{
    size_t count = prefix ? std::char_traits<char>::length(prefix) : 0;
    if (count > size())
        return false;
    return compare_n(prefix, count, cs) == 0;
}

bool ST::string::ends_with(const ST::string &suffix, case_sensitivity_t cs) const ST_NOEXCEPT
{
    if (suffix.size() > size())
        return false;

    size_t start = size() - suffix.size();
    return (cs == case_sensitive)
            ? std::char_traits<char>::compare(c_str() + start, suffix.c_str(), suffix.size()) == 0
            : _compare_ci(c_str() + start, suffix.c_str(), suffix.size()) == 0;
}

bool ST::string::ends_with(const char *suffix, case_sensitivity_t cs) const ST_NOEXCEPT
{
    size_t count = suffix ? std::char_traits<char>::length(suffix) : 0;
    if (count > size())
        return false;

    size_t start = size() - count;
    return (cs == case_sensitive)
            ? std::char_traits<char>::compare(c_str() + start, suffix ? suffix : "", count) == 0
            : _compare_ci(c_str() + start, suffix ? suffix : "", count) == 0;
}

ST::string ST::string::before_first(char sep, case_sensitivity_t cs) const
{
    ST_ssize_t first = find(sep, cs);
    if (first >= 0)
        return left(first);
    else
        return *this;
}

ST::string ST::string::before_first(const char *sep, case_sensitivity_t cs) const
{
    ST_ssize_t first = find(sep, cs);
    if (first >= 0)
        return left(first);
    else
        return *this;
}

ST::string ST::string::before_first(const string &sep, case_sensitivity_t cs) const
{
    ST_ssize_t first = find(sep, cs);
    if (first >= 0)
        return left(first);
    else
        return *this;
}

ST::string ST::string::after_first(char sep, case_sensitivity_t cs) const
{
    ST_ssize_t first = find(sep, cs);
    if (first >= 0)
        return substr(first + 1);
    else
        return null;
}

ST::string ST::string::after_first(const char *sep, case_sensitivity_t cs) const
{
    ST_ssize_t first = find(sep, cs);
    if (first >= 0)
        return substr(first + std::char_traits<char>::length(sep));
    else
        return null;
}

ST::string ST::string::after_first(const string &sep, case_sensitivity_t cs) const
{
    ST_ssize_t first = find(sep, cs);
    if (first >= 0)
        return substr(first + 1);
    else
        return null;
}

ST::string ST::string::before_last(char sep, case_sensitivity_t cs) const
{
    ST_ssize_t last = find_last(sep, cs);
    if (last >= 0)
        return left(last);
    else
        return null;
}

ST::string ST::string::before_last(const char *sep, case_sensitivity_t cs) const
{
    ST_ssize_t last = find_last(sep, cs);
    if (last >= 0)
        return left(last);
    else
        return null;
}

ST::string ST::string::before_last(const string &sep, case_sensitivity_t cs) const
{
    ST_ssize_t last = find_last(sep, cs);
    if (last >= 0)
        return left(last);
    else
        return null;
}

ST::string ST::string::after_last(char sep, case_sensitivity_t cs) const
{
    ST_ssize_t last = find_last(sep, cs);
    if (last >= 0)
        return substr(last + 1);
    else
        return *this;
}

ST::string ST::string::after_last(const char *sep, case_sensitivity_t cs) const
{
    ST_ssize_t last = find_last(sep, cs);
    if (last >= 0)
        return substr(last + std::char_traits<char>::length(sep));
    else
        return *this;
}

ST::string ST::string::after_last(const string &sep, case_sensitivity_t cs) const
{
    ST_ssize_t last = find_last(sep, cs);
    if (last >= 0)
        return substr(last + 1);
    else
        return *this;
}

ST::string ST::string::replace(const string &from, const string &to,
                               case_sensitivity_t cs,
                               utf_validation_t validation) const
{
    if (empty() || from.empty())
        return *this;

    string_stream out;
    const char *pstart = c_str();
    const char *pnext;
    for ( ;; ) {
        pnext = (cs == case_sensitive) ? strstr(pstart, from.c_str())
                                       : _stristr(pstart, from.c_str());
        if (!pnext)
            break;

        out.append(pstart, pnext - pstart);
        out << to;
        pstart = pnext + from.size();
    }

    if (*pstart)
        out << pstart;

    return out.to_string(true, validation);
}

ST::string ST::string::to_upper() const
{
    const std::locale &c_locale = std::locale::classic();

    string result;
    result.m_buffer.allocate(size());
    char *dupe = result.m_buffer.data();
    const char *sp = c_str();
    const char *ep = sp + size();
    char *dp = dupe;
    while (sp < ep)
        *dp++ = std::toupper(*sp++, c_locale);

    return result;
}

ST::string ST::string::to_lower() const
{
    const std::locale &c_locale = std::locale::classic();

    string result;
    result.m_buffer.allocate(size());
    char *dupe = result.m_buffer.data();
    const char *sp = c_str();
    const char *ep = sp + size();
    char *dp = dupe;
    while (sp < ep)
        *dp++ = std::tolower(*sp++, c_locale);

    return result;
}

std::vector<ST::string> ST::string::split(char split_char, size_t max_splits,
                                          case_sensitivity_t cs) const
{
    ST_ASSERT(split_char && static_cast<unsigned int>(split_char) < 0x80,
              "Split character should be in range '\\x01'-'\\x7f'");

    std::vector<string> result;

    const char *next = c_str();
    const char *end = next + size();
    while (max_splits) {
        const char *sp = (cs == case_sensitive) ? strchr(next, split_char)
                                                : _strichr(next, split_char);
        if (!sp)
            break;

        result.push_back(string(next, sp - next, assume_valid));
        next = sp + 1;
        --max_splits;
    }

    result.push_back(string(next, end - next, assume_valid));
    return result;
}

std::vector<ST::string> ST::string::split(const char *splitter, size_t max_splits,
                                          case_sensitivity_t cs) const
{
    ST_ASSERT(splitter, "ST::string::split called with null splitter");

    std::vector<string> result;
    if (!splitter)
        return result;

    // Performance improvement when splitter is "safe"
    utf_validation_t validation = assume_valid;
    const char *cp = splitter;
    while (*cp) {
        if (*cp & 0x80) {
            validation = assert_validity;
            break;
        }
        ++cp;
    }

    const char *next = c_str();
    const char *end = next + size();
    size_t splitlen = std::char_traits<char>::length(splitter);
    while (max_splits) {
        const char *sp = (cs == case_sensitive) ? strstr(next, splitter)
                                                : _stristr(next, splitter);
        if (!sp)
            break;

        result.push_back(string(next, sp - next, validation));
        next = sp + splitlen;
        --max_splits;
    }

    result.push_back(string(next, end - next, validation));
    return result;
}

std::vector<ST::string> ST::string::split(const string &splitter, size_t max_splits,
                                          case_sensitivity_t cs) const
{
    std::vector<string> result;

    const char *next = c_str();
    const char *end = next + size();
    while (max_splits) {
        const char *sp = (cs == case_sensitive) ? strstr(next, splitter.c_str())
                                                : _stristr(next, splitter.c_str());
        if (!sp)
            break;

        result.push_back(string(next, sp - next, assume_valid));
        next = sp + splitter.size();
        --max_splits;
    }

    result.push_back(string(next, end - next, assume_valid));
    return result;
}

std::vector<ST::string> ST::string::tokenize(const char *delims) const
{
    std::vector<string> result;

    const char *next = c_str();
    const char *end = next + size();
    while (next != end) {
        const char *cur = next;
        while (cur != end && !strchr(delims, *cur))
            ++cur;

        // Found a delimiter
        if (cur != next)
            result.push_back(string(next, cur - next, assume_valid));

        next = cur;
        while (next != end && strchr(delims, *next))
            ++next;
    }

    return result;
}

ST::string ST::string::fill(size_t count, char c)
{
    char_buffer result;
    result.allocate(count, c);
    return result;
}

size_t ST::hash::operator()(const string &str) const ST_NOEXCEPT
{
    /* FNV-1a hash.  See http://isthe.com/chongo/tech/comp/fnv/ for details */
#if ST_SIZET_BYTES == 4
#   define FNV_OFFSET_BASIS 0x811c9dc5UL
#   define FNV_PRIME        0x01000193UL
#elif ST_SIZET_BYTES == 8
#   define FNV_OFFSET_BASIS 0xcbf29ce484222325ULL
#   define FNV_PRIME        0x00000100000001b3ULL
#endif

    size_t hash = FNV_OFFSET_BASIS;
    const char *cp = str.c_str();
    const char *ep = cp + str.size();
    while (cp < ep) {
        hash ^= static_cast<size_t>(*cp++);
        hash *= FNV_PRIME;
    }
    return hash;
}

size_t ST::hash_i::operator()(const string &str) const ST_NOEXCEPT
{
    /* FNV-1a hash.  See http://isthe.com/chongo/tech/comp/fnv/ for details */
#if ST_SIZET_BYTES == 4
#   define FNV_OFFSET_BASIS 0x811c9dc5UL
#   define FNV_PRIME        0x01000193UL
#elif ST_SIZET_BYTES == 8
#   define FNV_OFFSET_BASIS 0xcbf29ce484222325ULL
#   define FNV_PRIME        0x00000100000001b3ULL
#endif

    const std::locale &c_locale = std::locale::classic();

    size_t hash = FNV_OFFSET_BASIS;
    const char *cp = str.c_str();
    const char *ep = cp + str.size();
    while (cp < ep) {
        hash ^= static_cast<size_t>(std::tolower(*cp++, c_locale));
        hash *= FNV_PRIME;
    }
    return hash;
}

ST::string ST::operator+(const ST::string &left, const ST::string &right)
{
    ST::char_buffer cat;
    cat.allocate(left.size() + right.size());
    std::char_traits<char>::copy(&cat[0], left.c_str(), left.size());
    std::char_traits<char>::copy(&cat[left.size()], right.c_str(), right.size());

    return ST::string(cat, ST::assume_valid);
}

static ST::string _append(const ST::string &left, char32_t right)
{
    size_t newsize = left.size();
    if (right < 0x80) {
        newsize += 1;
    } else if (right < 0x800) {
        newsize += 2;
    } else if (right < 0x10000) {
        newsize += 3;
    } else if (right <= 0x10FFFF) {
        newsize += 4;
    } else {
        // Out-of-range code point always gets replaced
        newsize += BADCHAR_SUBSTITUTE_UTF8_LEN;
    }

    ST::char_buffer cat;
    cat.allocate(newsize);
    char *catp = cat.data();
    std::char_traits<char>::copy(catp, left.c_str(), left.size());
    catp += left.size();

    if (right < 0x80) {
        *catp++ = static_cast<char>(right);
    } else if (right < 0x800) {
        *catp++ = 0xC0 | ((right >>  6) & 0x1F);
        *catp++ = 0x80 | ((right      ) & 0x3F);
    } else if (right < 0x10000) {
        *catp++ = 0xE0 | ((right >> 12) & 0x0F);
        *catp++ = 0x80 | ((right >>  6) & 0x3F);
        *catp++ = 0x80 | ((right      ) & 0x3F);
    } else if (right <= 0x10FFFF) {
        *catp++ = 0xF0 | ((right >> 18) & 0x07);
        *catp++ = 0x80 | ((right >> 12) & 0x3F);
        *catp++ = 0x80 | ((right >>  6) & 0x3F);
        *catp++ = 0x80 | ((right      ) & 0x3F);
    } else {
        ST_ASSERT(false, "Unicode character out of range");
        std::char_traits<char>::copy(catp, BADCHAR_SUBSTITUTE_UTF8, BADCHAR_SUBSTITUTE_UTF8_LEN);
        catp += BADCHAR_SUBSTITUTE_UTF8_LEN;
    }

    return ST::string(cat, ST::assume_valid);
}

ST::string ST::operator+(const ST::string &left, char right)
{
    char32_t uchar = static_cast<unsigned char>(right);
    return _append(left, uchar);
}

#ifdef ST_HAVE_CHAR_TYPES
ST::string ST::operator+(const ST::string &left, char16_t right)
{
    char32_t uchar = right;
    return _append(left, uchar);
}

ST::string ST::operator+(const ST::string &left, char32_t right)
{
    return _append(left, right);
}
#endif

ST::string ST::operator+(const ST::string &left, wchar_t right)
{
#if ST_WCHAR_BYTES == 2
    char32_t uchar = static_cast<unsigned short>(right);
#else
    char32_t uchar = static_cast<unsigned int>(right);
#endif
    return _append(left, uchar);
}

static ST::string _prepend(char32_t left, const ST::string &right)
{
    size_t newsize = right.size();
    if (left < 0x80) {
        newsize += 1;
    } else if (left < 0x800) {
        newsize += 2;
    } else if (left < 0x10000) {
        newsize += 3;
    } else if (left <= 0x10FFFF) {
        newsize += 4;
    } else {
        // Out-of-range code point always gets replaced
        newsize += BADCHAR_SUBSTITUTE_UTF8_LEN;
    }

    ST::char_buffer cat;
    cat.allocate(newsize);
    char *catp = cat.data();

    if (left < 0x80) {
        *catp++ = static_cast<char>(left);
    } else if (left < 0x800) {
        *catp++ = 0xC0 | ((left >>  6) & 0x1F);
        *catp++ = 0x80 | ((left      ) & 0x3F);
    } else if (left < 0x10000) {
        *catp++ = 0xE0 | ((left >> 12) & 0x0F);
        *catp++ = 0x80 | ((left >>  6) & 0x3F);
        *catp++ = 0x80 | ((left      ) & 0x3F);
    } else if (left <= 0x10FFFF) {
        *catp++ = 0xF0 | ((left >> 18) & 0x07);
        *catp++ = 0x80 | ((left >> 12) & 0x3F);
        *catp++ = 0x80 | ((left >>  6) & 0x3F);
        *catp++ = 0x80 | ((left      ) & 0x3F);
    } else {
        ST_ASSERT(false, "Unicode character out of range");
        std::char_traits<char>::copy(catp, BADCHAR_SUBSTITUTE_UTF8, BADCHAR_SUBSTITUTE_UTF8_LEN);
        catp += BADCHAR_SUBSTITUTE_UTF8_LEN;
    }

    std::char_traits<char>::copy(catp, right.c_str(), right.size());
    return ST::string(cat, ST::assume_valid);
}

ST::string ST::operator+(char left, const ST::string &right)
{
    char32_t uchar = static_cast<unsigned char>(left);
    return _prepend(uchar, right);
}

#ifdef ST_HAVE_CHAR_TYPES
ST::string ST::operator+(char16_t left, const ST::string &right)
{
    char32_t uchar = left;
    return _prepend(uchar, right);
}

ST::string ST::operator+(char32_t left, const ST::string &right)
{
    return _prepend(left, right);
}
#endif

ST::string ST::operator+(wchar_t left, const ST::string &right)
{
#if ST_WCHAR_BYTES == 2
    char32_t uchar = static_cast<unsigned short>(left);
#else
    char32_t uchar = static_cast<unsigned int>(left);
#endif
    return _prepend(uchar, right);
}
