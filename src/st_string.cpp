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

#include <cstring>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include "st_assert.h"
#include "st_stringstream.h"
#include "st_format_priv.h"

#if !defined(ST_WCHAR_BYTES) || ((ST_WCHAR_BYTES != 2) && (ST_WCHAR_BYTES != 4))
#   error ST_WCHAR_SIZE must either be 2 (16-bit) or 4 (32-bit)
#endif

#if !defined(ST_SIZET_BYTES) || ((ST_SIZET_BYTES != 4) && (ST_SIZET_BYTES != 8))
#   error Supported size_t sizes are 4 (32-bit) or 8 (64-bit) bytes
#endif

#ifdef _MSC_VER
#   define stricmp  _stricmp
#   define strnicmp _strnicmp
#   if _MSC_VER < 1800
#       define strtoll  _strtoi64
#       define strtoull _strtoui64
#   endif
#   if _MSC_VER < 1900
#       define snprintf _snprintf
#       pragma warning(disable: 4996)
#   endif
#else
#   define stricmp  strcasecmp
#   define strnicmp strncasecmp
#endif

int _ST_PRIVATE::_lower_char(int ch) ST_NOEXCEPT
{
    return tolower(ch);
}

int _ST_PRIVATE::_upper_char(int ch) ST_NOEXCEPT
{
    return toupper(ch);
}

#define BADCHAR_SUBSTITUTE          0xFFFDul
#define BADCHAR_SUBSTITUTE_UTF8     "\xEF\xBF\xBD"

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

static void _assert_utf8_buffer(const ST::char_buffer &buffer)
{
    const char *sp = buffer.data();
    const char *ep = sp + buffer.size();
    while (sp < ep) {
        unsigned char uch = *sp++;
        if ((uch & 0xF8) == 0xF0) {
            // Four bytes
            ST_ASSERT(sp + 3 <= ep, "Incomplete UTF-8 sequence");
            ST_ASSERT((*sp++) & 0x80, "Invalid UTF-8 sequence byte (1)");
            ST_ASSERT((*sp++) & 0x80, "Invalid UTF-8 sequence byte (2)");
            ST_ASSERT((*sp++) & 0x80, "Invalid UTF-8 sequence byte (3)");
        } else if ((uch & 0xF0) == 0xE0) {
            // Three bytes
            ST_ASSERT(sp + 2 <= ep, "Incomplete UTF-8 sequence");
            ST_ASSERT((*sp++) & 0x80, "Invalid UTF-8 sequence byte (1)");
            ST_ASSERT((*sp++) & 0x80, "Invalid UTF-8 sequence byte (2)");
        } else if ((uch & 0xE0) == 0xC0) {
            // Two bytes
            ST_ASSERT(sp + 1 <= ep, "Incomplete UTF-8 sequence");
            ST_ASSERT((*sp++) & 0x80, "Invalid UTF-8 sequence byte (1)");
        } else if ((uch & 0xC0) == 0x80) {
            ST_ASSERT(false, "Invalid UTF-8 marker byte");
        } else if ((uch & 0x80) != 0) {
            ST_ASSERT(false, "UTF-8 character out of range");
        }
    }
}

#define _CHECK_SEQUENCE(condition, message) \
    do { \
        if (!(condition)) \
            throw ST::unicode_error(message); \
    } while (0)

static void _check_utf8_buffer(const ST::char_buffer &buffer)
{
    const char *sp = buffer.data();
    const char *ep = sp + buffer.size();
    while (sp < ep) {
        unsigned char uch = *sp++;
        if ((uch & 0xF8) == 0xF0) {
            // Four bytes
            _CHECK_SEQUENCE(sp + 3 <= ep, "Incomplete UTF-8 sequence");
            _CHECK_SEQUENCE((*sp++) & 0x80, "Invalid UTF-8 sequence byte (1)");
            _CHECK_SEQUENCE((*sp++) & 0x80, "Invalid UTF-8 sequence byte (2)");
            _CHECK_SEQUENCE((*sp++) & 0x80, "Invalid UTF-8 sequence byte (3)");
        } else if ((uch & 0xF0) == 0xE0) {
            // Three bytes
            _CHECK_SEQUENCE(sp + 2 <= ep, "Incomplete UTF-8 sequence");
            _CHECK_SEQUENCE((*sp++) & 0x80, "Invalid UTF-8 sequence byte (1)");
            _CHECK_SEQUENCE((*sp++) & 0x80, "Invalid UTF-8 sequence byte (2)");
        } else if ((uch & 0xE0) == 0xC0) {
            // Two bytes
            _CHECK_SEQUENCE(sp + 1 <= ep, "Incomplete UTF-8 sequence");
            _CHECK_SEQUENCE((*sp++) & 0x80, "Invalid UTF-8 sequence byte (1)");
        } else if ((uch & 0xC0) == 0x80) {
            _CHECK_SEQUENCE(false, "Invalid UTF-8 marker byte");
        } else if ((uch & 0x80) != 0) {
            _CHECK_SEQUENCE(false, "UTF-8 character out of range");
        }
    }
}

static ST::char_buffer _cleanup_utf8_buffer(const ST::char_buffer &buffer)
{
    bool valid = true;
    ST::string_stream ss_clean;

    const char *sp = buffer.data();
    const char *ep = sp + buffer.size();
    while (sp < ep) {
        unsigned char uch = *sp++;
        if ((uch & 0xF8) == 0xF0) {
            // Four bytes
            if (sp + 3 > ep) {
                valid = false;
                ss_clean << BADCHAR_SUBSTITUTE_UTF8;
                break;
            }
            if ((sp[0] & 0x80) == 0 || (sp[1] & 0x80) == 0 || (sp[2] & 0x80) == 0) {
                valid = false;
                ss_clean << BADCHAR_SUBSTITUTE_UTF8;
            } else {
                ss_clean.append_char(uch);
                ss_clean.append(sp, 3);
            }
            sp += 3;
        } else if ((uch & 0xF0) == 0xE0) {
            // Three bytes
            if (sp + 2 > ep) {
                valid = false;
                ss_clean << BADCHAR_SUBSTITUTE_UTF8;
                break;
            }
            if ((sp[0] & 0x80) == 0 || (sp[1] & 0x80) == 0) {
                valid = false;
                ss_clean << BADCHAR_SUBSTITUTE_UTF8;
            } else {
                ss_clean.append_char(uch);
                ss_clean.append(sp, 2);
            }
            sp += 2;
        } else if ((uch & 0xE0) == 0xC0) {
            // Two bytes
            if (sp + 1 > ep) {
                valid = false;
                ss_clean << BADCHAR_SUBSTITUTE_UTF8;
                break;
            }
            if ((sp[0] & 0x80) == 0) {
                valid = false;
                ss_clean << BADCHAR_SUBSTITUTE_UTF8;
            } else {
                ss_clean.append_char(uch);
                ss_clean.append_char(*sp);
            }
            sp += 1;
        } else if ((uch & 0xC0) == 0x80 || (uch & 0x80) != 0) {
            valid = false;
            ss_clean << BADCHAR_SUBSTITUTE_UTF8;
        } else {
            ss_clean.append_char(uch);
        }
    }

    if (valid)
        return buffer;

    return ST::char_buffer(ss_clean.raw_buffer(), ss_clean.size());
}

void ST::string::set(const char_buffer &init, utf_validation_t validation)
{
    switch (validation) {
    case assert_validity:
        _assert_utf8_buffer(init);
        m_buffer = init;
        break;

    case check_validity:
        _check_utf8_buffer(init);
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
        _assert_utf8_buffer(init);
        m_buffer = std::move(init);
        break;

    case check_validity:
        _check_utf8_buffer(init);
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

    // Calculate the UTF-8 size
    size_t convlen = 0;
    const char16_t *sp = utf16;
    const char16_t *ep = sp + size;
    while (sp < ep) {
        if (*sp >= 0xD800 && *sp <= 0xDFFF) {
            // Surrogate pair
            convlen += 4;
            ++sp;
        } else if (*sp > 0x7FF) {
            convlen += 3;
        } else if (*sp > 0x7F) {
            convlen += 2;
        } else {
            convlen += 1;
        }
        ++sp;
    }

    // Perform the actual conversion
    char *utf8 = m_buffer.create_writable_buffer(convlen);
    char *dp = utf8;
    sp = utf16;
    while (sp < ep) {
        if (*sp >= 0xD800 && *sp <= 0xDFFF) {
            // Surrogate pair
            char32_t bigch = 0x10000;

            if (sp + 1 >= ep) {
                switch (validation) {
                case check_validity:
                    throw ST::unicode_error("Incomplete surrogate pair");
                case assert_validity:
                    ST_ASSERT(false, "Incomplete surrogate pair");
                    /* fall through */
                case substitute_invalid:
                    bigch = BADCHAR_SUBSTITUTE;
                    break;
                case assume_valid:
                    // Will encode the bad surrogate char as a UTF-8 value
                    break;
                default:
                    ST_ASSERT(false, "Invalid validation type");
                }
            } else if (*sp < 0xDC00) {
                bigch += (*sp++ & 0x3FF) << 10;
                if (*sp < 0xDC00 || *sp > 0xDFFF) {
                    switch (validation) {
                    case check_validity:
                        throw ST::unicode_error("Invalid surrogate pair");
                    case assert_validity:
                        ST_ASSERT(false, "Invalid surrogate pair");
                        /* fall through */
                    case substitute_invalid:
                        bigch = BADCHAR_SUBSTITUTE;
                        break;
                    case assume_valid:
                        // Will encode the bad surrogate char as a UTF-8 value
                        break;
                    default:
                        ST_ASSERT(false, "Invalid validation type");
                    }
                } else {
                    bigch += (*sp & 0x3FF);
                }
            } else {
                bigch += (*sp++ & 0x3FF);
                if (*sp < 0xD800 || *sp >= 0xDC00) {
                    switch (validation) {
                    case check_validity:
                        throw ST::unicode_error("Invalid surrogate pair");
                    case assert_validity:
                        ST_ASSERT(false, "Invalid surrogate pair");
                        /* fall through */
                    case substitute_invalid:
                        bigch = BADCHAR_SUBSTITUTE;
                        break;
                    case assume_valid:
                        // Will encode the bad surrogate char as a UTF-8 value
                        break;
                    default:
                        ST_ASSERT(false, "Invalid validation type");
                    }
                } else {
                    bigch += (*sp & 0x3FF) << 10;
                }
            }
            *dp++ = 0xF0 | ((bigch >> 18) & 0x07);
            *dp++ = 0x80 | ((bigch >> 12) & 0x3F);
            *dp++ = 0x80 | ((bigch >>  6) & 0x3F);
            *dp++ = 0x80 | ((bigch      ) & 0x3F);
        } else if (*sp > 0x7FF) {
            *dp++ = 0xE0 | ((*sp >> 12) & 0x0F);
            *dp++ = 0x80 | ((*sp >>  6) & 0x3F);
            *dp++ = 0x80 | ((*sp      ) & 0x3F);
        } else if (*sp > 0x7F) {
            *dp++ = 0xC0 | ((*sp >>  6) & 0x1F);
            *dp++ = 0x80 | ((*sp      ) & 0x3F);
        } else {
            *dp++ = static_cast<char>(*sp);
        }
        ++sp;
    }
    utf8[convlen] = 0;
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
    while (sp < ep) {
        if (*sp > 0x10FFFF) {
            // Out-of-range code point always gets replaced
            convlen += 3;
        } else if (*sp > 0xFFFF) {
            convlen += 4;
        } else if (*sp > 0x7FF) {
            convlen += 3;
        } else if (*sp > 0x7F) {
            convlen += 2;
        } else {
            convlen += 1;
        }
        ++sp;
    }

    // Perform the actual conversion
    char *utf8 = m_buffer.create_writable_buffer(convlen);
    char *dp = utf8;
    sp = utf32;
    while (sp < ep) {
        if (*sp > 0x10FFFF) {
            if (validation == check_validity)
                throw ST::unicode_error("Unicode character out of range");
            else if (validation == assert_validity)
                ST_ASSERT(false, "Unicode character out of range");
            *dp++ = 0xE0 | ((BADCHAR_SUBSTITUTE >> 12) & 0x0F);
            *dp++ = 0x80 | ((BADCHAR_SUBSTITUTE >>  6) & 0x3F);
            *dp++ = 0x80 | ((BADCHAR_SUBSTITUTE      ) & 0x3F);
        } else if (*sp > 0xFFFF) {
            *dp++ = 0xF0 | ((*sp >> 18) & 0x07);
            *dp++ = 0x80 | ((*sp >> 12) & 0x3F);
            *dp++ = 0x80 | ((*sp >>  6) & 0x3F);
            *dp++ = 0x80 | ((*sp      ) & 0x3F);
        } else if (*sp > 0x7FF) {
            *dp++ = 0xE0 | ((*sp >> 12) & 0x0F);
            *dp++ = 0x80 | ((*sp >>  6) & 0x3F);
            *dp++ = 0x80 | ((*sp      ) & 0x3F);
        } else if (*sp > 0x7F) {
            *dp++ = 0xC0 | ((*sp >>  6) & 0x1F);
            *dp++ = 0x80 | ((*sp      ) & 0x3F);
        } else {
            *dp++ = static_cast<char>(*sp);
        }
        ++sp;
    }
    utf8[convlen] = 0;
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
    while (sp < ep) {
        if ((*sp++) & 0x80)
            convlen += 2;
        else
            convlen += 1;
    }

    // Perform the actual conversion
    char *utf8 = m_buffer.create_writable_buffer(convlen);
    char *dp = utf8;
    sp = astr;
    while (sp < ep) {
        if (*sp & 0x80) {
            *dp++ = 0xC0 | ((static_cast<unsigned char>(*sp) >> 6) & 0x1F);
            *dp++ = 0x80 | ((static_cast<unsigned char>(*sp)     ) & 0x3F);
        } else {
            *dp++ = *sp;
        }
        ++sp;
    }
    utf8[convlen] = 0;
}

ST::utf16_buffer ST::string::to_utf16() const
{
    utf16_buffer result;
    if (is_empty())
        return result;

    // Calculate the UTF-16 size
    size_t convlen = 0;
    const char *utf8 = m_buffer.data();
    const char *sp = utf8;
    const char *ep = sp + m_buffer.size();
    while (sp < ep) {
        if ((*sp & 0xF8) == 0xF0) {
            // Encode with surrogate pair
            ++convlen;
            sp += 4;
        } else if ((*sp & 0xF0) == 0xE0) {
            sp += 3;
        } else if ((*sp & 0xE0) == 0xC0) {
            sp += 2;
        } else {
            sp += 1;
        }
        ++convlen;
    }

    // Perform the actual conversion
    char16_t *utf16 = result.create_writable_buffer(convlen);
    char16_t *dp = utf16;
    sp = utf8;
    while (sp < ep) {
        char32_t bigch = 0;
        if ((*sp & 0xF8) == 0xF0) {
            bigch  = (*sp++ & 0x07) << 18;
            bigch |= (*sp++ & 0x3F) << 12;
            bigch |= (*sp++ & 0x3F) << 6;
            bigch |= (*sp++ & 0x3F);
            bigch -= 0x10000;

            *dp++ = 0xD800 | ((bigch >> 10) & 0x3FF);
            *dp++ = 0xDC00 | ((bigch      ) & 0x3FF);
        } else if ((*sp & 0xF0) == 0xE0) {
            bigch  = (*sp++ & 0x0F) << 12;
            bigch |= (*sp++ & 0x3F) << 6;
            bigch |= (*sp++ & 0x3F);
            *dp++ = static_cast<char16_t>(bigch);
        } else if ((*sp & 0xE0) == 0xC0) {
            bigch  = (*sp++ & 0x1F) << 6;
            bigch |= (*sp++ & 0x3F);
            *dp++ = static_cast<char16_t>(bigch);
        } else {
            *dp++ = *sp++;
        }
    }
    utf16[convlen] = 0;

    return result;
}

ST::utf32_buffer ST::string::to_utf32() const
{
    utf32_buffer result;
    if (is_empty())
        return result;

    // Calculate the UTF-32 size
    size_t convlen = 0;
    const char *utf8 = m_buffer.data();
    const char *sp = utf8;
    const char *ep = sp + m_buffer.size();
    while (sp < ep) {
        if ((*sp & 0xF8) == 0xF0)
            sp += 4;
        else if ((*sp & 0xF0) == 0xE0)
            sp += 3;
        else if ((*sp & 0xE0) == 0xC0)
            sp += 2;
        else
            sp += 1;
        ++convlen;
    }

    // Perform the actual conversion
    char32_t *utf32 = result.create_writable_buffer(convlen);
    char32_t *dp = utf32;
    sp = utf8;
    while (sp < ep) {
        char32_t bigch = 0;
        if ((*sp & 0xF8) == 0xF0) {
            bigch  = (*sp++ & 0x07) << 18;
            bigch |= (*sp++ & 0x3F) << 12;
            bigch |= (*sp++ & 0x3F) << 6;
            bigch |= (*sp++ & 0x3F);
        } else if ((*sp & 0xF0) == 0xE0) {
            bigch  = (*sp++ & 0x0F) << 12;
            bigch |= (*sp++ & 0x3F) << 6;
            bigch |= (*sp++ & 0x3F);
        } else if ((*sp & 0xE0) == 0xC0) {
            bigch  = (*sp++ & 0x1F) << 6;
            bigch |= (*sp++ & 0x3F);
        } else {
            bigch = *sp++;
        }
        *dp++ = bigch;
    }
    utf32[convlen] = 0;

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
    if (is_empty())
        return result;

    // Calculate the ASCII size
    size_t convlen = 0;
    const char *utf8 = m_buffer.data();
    const char *sp = utf8;
    const char *ep = sp + m_buffer.size();
    while (sp < ep) {
        if ((*sp & 0xF8) == 0xF0)
            sp += 4;
        else if ((*sp & 0xF0) == 0xE0)
            sp += 3;
        else if ((*sp & 0xE0) == 0xC0)
            sp += 2;
        else
            sp += 1;
        ++convlen;
    }

    // Perform the actual conversion
    char *astr = result.create_writable_buffer(convlen);
    char *dp = astr;
    sp = utf8;
    while (sp < ep) {
        char32_t bigch = 0;
        if ((*sp & 0xF8) == 0xF0) {
            bigch  = (*sp++ & 0x07) << 18;
            bigch |= (*sp++ & 0x3F) << 12;
            bigch |= (*sp++ & 0x3F) << 6;
            bigch |= (*sp++ & 0x3F);
        } else if ((*sp & 0xF0) == 0xE0) {
            bigch  = (*sp++ & 0x0F) << 12;
            bigch |= (*sp++ & 0x3F) << 6;
            bigch |= (*sp++ & 0x3F);
        } else if ((*sp & 0xE0) == 0xC0) {
            bigch  = (*sp++ & 0x1F) << 6;
            bigch |= (*sp++ & 0x3F);
        } else {
            bigch = *sp++;
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
    astr[convlen] = 0;

    return result;
}

template <typename int_T>
static ST::string _mini_format_numeric_s(int radix, bool upper_case, int_T value)
{
    typedef typename std::make_unsigned<int_T>::type uint_T;
    int_T abs = (value < 0) ? -value : value;

    size_t format_size = 0;
    int_T temp = abs;
    while (temp) {
        ++format_size;
        temp /= radix;
    }

    if (format_size == 0)
        format_size = 1;

    if (value < 0)
        ++format_size;

    ST::char_buffer result;
    char *buffer = result.create_writable_buffer(format_size);
    _format_numeric_impl<uint_T>(buffer + format_size,
                                 static_cast<uint_T>(abs), radix, upper_case);
    buffer[format_size] = 0;

    if (value < 0)
        buffer[0] = '-';

    return ST::string(result, ST::assume_valid);
}

template <typename uint_T>
static ST::string _mini_format_numeric_u(int radix, bool upper_case, uint_T value)
{
    size_t format_size = 0;
    uint_T temp = value;
    while (temp) {
        ++format_size;
        temp /= radix;
    }

    if (format_size == 0)
        format_size = 1;

    ST::char_buffer result;
    char *buffer = result.create_writable_buffer(format_size);
    _format_numeric_impl<uint_T>(buffer + format_size, value, radix, upper_case);
    buffer[format_size] = 0;

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

ST::string ST::string::from_float(float value, char format)
{
    return from_double(double(value), format);
}

ST::string ST::string::from_double(double value, char format)
{
    static const char valid_formats[] = "efgEFG";
    if (!strchr(valid_formats, format)) {
        ST_ASSERT(false, "Unsupported floating-point format specifier");
        format = 'g';
    }

    char format_spec[] = { '%', format, 0 };
    int format_size = snprintf(ST_NULLPTR, 0, format_spec, value);
    ST_ASSERT(format_size > 0, "Your libc doesn't support reporting format size");

    ST::char_buffer out_buffer;
    char *fmt_out = out_buffer.create_writable_buffer(format_size);
    snprintf(fmt_out, format_size + 1, format_spec, value);
    return ST::string(out_buffer, ST::assume_valid);
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
    if (is_empty()) {
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
    if (is_empty()) {
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
    if (is_empty()) {
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

int ST::string::compare(const string &str, case_sensitivity_t cs) const ST_NOEXCEPT
{
    return (cs == case_sensitive) ? strcmp(c_str(), str.c_str())
                                  : stricmp(c_str(), str.c_str());
}

int ST::string::compare(const char *str, case_sensitivity_t cs) const ST_NOEXCEPT
{
    return (cs == case_sensitive) ? strcmp(c_str(), str ? str : "")
                                  : stricmp(c_str(), str ? str : "");
}

int ST::string::compare_n(const string &str, size_t count,
                          case_sensitivity_t cs) const ST_NOEXCEPT
{
    return (cs == case_sensitive) ? strncmp(c_str(), str.c_str(), count)
                                  : strnicmp(c_str(), str.c_str(), count);
}

int ST::string::compare_n(const char *str, size_t count,
                          case_sensitivity_t cs) const ST_NOEXCEPT
{
    return (cs == case_sensitive) ? strncmp(c_str(), str ? str : "", count)
                                  : strnicmp(c_str(), str ? str : "", count);
}

static const char *_stristr(const char *haystack, const char *needle)
{
    // The "easy" way
    size_t sublen = strlen(needle);
    const char *cp = haystack;
    const char *ep = cp + strlen(haystack);
    while (cp + sublen <= ep) {
        if (strnicmp(cp, needle, sublen) == 0)
            return cp;
        ++cp;
    }
    return ST_NULLPTR;
}

static const char *_strichr(const char *haystack, int ch)
{
    const char *cp = haystack;
    while (*cp) {
        if (_ST_PRIVATE::_lower_char(ch) == _ST_PRIVATE::_lower_char(*cp))
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
    if (is_empty())
        return -1;

    const char *endp = c_str() + (max > size() ? size() : max);

    const char *start = c_str();
    const char *found = nullptr;
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
    if (!substr || !substr[0] || is_empty())
        return -1;

    const char *endp = c_str() + (max > size() ? size() : max);

    const char *start = c_str();
    const char *found = nullptr;
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
    if (is_empty())
        return null;

    const char *cp = c_str();
    while (*cp && strchr(charset, *cp))
        ++cp;

    return substr(cp - c_str());
}

ST::string ST::string::trim_right(const char *charset) const
{
    if (is_empty())
        return null;

    const char *cp = c_str() + size();
    while (--cp >= c_str() && strchr(charset, *cp))
        ;

    return substr(0, cp - c_str() + 1);
}

ST::string ST::string::trim(const char *charset) const
{
    if (is_empty())
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
    char *sub_data = sub.m_buffer.create_writable_buffer(count);
    _ST_PRIVATE::_copy_buffer(sub_data, c_str() + start, count);
    sub_data[count] = 0;

    return sub;
}

bool ST::string::starts_with(const ST::string &prefix, case_sensitivity_t cs) const
{
    if (prefix.size() > size())
        return false;
    return compare_n(prefix, prefix.size(), cs) == 0;
}

bool ST::string::starts_with(const char *prefix, case_sensitivity_t cs) const
{
    size_t count = strlen(prefix);
    if (count > size())
        return false;
    return compare_n(prefix, count, cs) == 0;
}

bool ST::string::ends_with(const ST::string &suffix, case_sensitivity_t cs) const
{
    if (suffix.size() > size())
        return false;

    size_t start = size() - suffix.size();
    return (cs == case_sensitive) ? strncmp(c_str() + start, suffix.c_str(), suffix.size()) == 0
                                  : strnicmp(c_str() + start, suffix.c_str(), suffix.size()) == 0;
}

bool ST::string::ends_with(const char *suffix, case_sensitivity_t cs) const
{
    size_t count = strlen(suffix);
    if (count > size())
        return false;

    size_t start = size() - count;
    return (cs == case_sensitive) ? strncmp(c_str() + start, suffix, count) == 0
                                  : strnicmp(c_str() + start, suffix, count) == 0;
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
        return substr(first + strlen(sep));
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
        return substr(last + strlen(sep));
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
    if (is_empty() || from.is_empty())
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
    // TODO: Make this unicode aware
    string result;
    char *dupe = result.m_buffer.create_writable_buffer(size());
    const char *sp = c_str();
    const char *ep = sp + size();
    char *dp = dupe;
    while (sp < ep)
        *dp++ = _ST_PRIVATE::_upper_char(*sp++);
    dupe[size()] = 0;

    return result;
}

ST::string ST::string::to_lower() const
{
    // TODO: Make this unicode aware
    string result;
    char *dupe = result.m_buffer.create_writable_buffer(size());
    const char *sp = c_str();
    const char *ep = sp + size();
    char *dp = dupe;
    while (sp < ep)
        *dp++ = _ST_PRIVATE::_lower_char(*sp++);
    dupe[size()] = 0;

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
    size_t splitlen = strlen(splitter);
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
    char *data = result.create_writable_buffer(count);
    _ST_PRIVATE::_fill_buffer(data, c, count);
    data[count] = 0;
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

ST::string ST::operator+(const ST::string &left, const ST::string &right)
{
    ST::char_buffer cat;
    char *catp = cat.create_writable_buffer(left.size() + right.size());
    _ST_PRIVATE::_copy_buffer(catp, left.c_str(), left.size());
    _ST_PRIVATE::_copy_buffer(catp + left.size(), right.c_str(), right.size());
    catp[cat.size()] = 0;

    return ST::string(cat, ST::assume_valid);
}

ST::string ST::operator+(const ST::string &left, const char *right)
{
    ST::char_buffer cat;
    size_t rsize = strlen(right);
    char *catp = cat.create_writable_buffer(left.size() + rsize);
    _ST_PRIVATE::_copy_buffer(catp, left.c_str(), left.size());
    _ST_PRIVATE::_copy_buffer(catp + left.size(), right, rsize);
    catp[cat.size()] = 0;

    return cat;
}

ST::string ST::operator+(const char *left, const ST::string &right)
{
    ST::char_buffer cat;
    size_t lsize = strlen(left);
    char *catp = cat.create_writable_buffer(lsize + right.size());
    _ST_PRIVATE::_copy_buffer(catp, left, lsize);
    _ST_PRIVATE::_copy_buffer(catp + lsize, right.c_str(), right.size());
    catp[cat.size()] = 0;

    return cat;
}

static ST::string _append(const ST::string &left, char32_t right)
{
    size_t newsize = left.size();
    if (right > 0x10FFFF) {
        // Out-of-range code point always gets replaced
        newsize += 3;
    } else if (right > 0xFFFF) {
        newsize += 4;
    } else if (right > 0x7FF) {
        newsize += 3;
    } else if (right > 0x7F) {
        newsize += 2;
    } else {
        newsize += 1;
    }

    ST::char_buffer cat;
    char *catp = cat.create_writable_buffer(newsize);
    _ST_PRIVATE::_copy_buffer(catp, left.c_str(), left.size());
    catp += left.size();

    if (right > 0x10FFFF) {
        ST_ASSERT(false, "Unicode character out of range");
        *catp++ = 0xE0 | ((BADCHAR_SUBSTITUTE >> 12) & 0x0F);
        *catp++ = 0x80 | ((BADCHAR_SUBSTITUTE >>  6) & 0x3F);
        *catp++ = 0x80 | ((BADCHAR_SUBSTITUTE      ) & 0x3F);
    } else if (right > 0xFFFF) {
        *catp++ = 0xF0 | ((right >> 18) & 0x07);
        *catp++ = 0x80 | ((right >> 12) & 0x3F);
        *catp++ = 0x80 | ((right >>  6) & 0x3F);
        *catp++ = 0x80 | ((right      ) & 0x3F);
    } else if (right > 0x7FF) {
        *catp++ = 0xE0 | ((right >> 12) & 0x0F);
        *catp++ = 0x80 | ((right >>  6) & 0x3F);
        *catp++ = 0x80 | ((right      ) & 0x3F);
    } else if (right > 0x7F) {
        *catp++ = 0xC0 | ((right >>  6) & 0x1F);
        *catp++ = 0x80 | ((right      ) & 0x3F);
    } else {
        *catp++ = static_cast<char>(right);
    }

    *catp = 0;
    return cat;
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
    if (left > 0x10FFFF) {
        // Out-of-range code point always gets replaced
        newsize += 3;
    } else if (left > 0xFFFF) {
        newsize += 4;
    } else if (left > 0x7FF) {
        newsize += 3;
    } else if (left > 0x7F) {
        newsize += 2;
    } else {
        newsize += 1;
    }

    ST::char_buffer cat;
    char *catp = cat.create_writable_buffer(newsize);

    if (left > 0x10FFFF) {
        ST_ASSERT(false, "Unicode character out of range");
        *catp++ = 0xE0 | ((BADCHAR_SUBSTITUTE >> 12) & 0x0F);
        *catp++ = 0x80 | ((BADCHAR_SUBSTITUTE >>  6) & 0x3F);
        *catp++ = 0x80 | ((BADCHAR_SUBSTITUTE      ) & 0x3F);
    } else if (left > 0xFFFF) {
        *catp++ = 0xF0 | ((left >> 18) & 0x07);
        *catp++ = 0x80 | ((left >> 12) & 0x3F);
        *catp++ = 0x80 | ((left >>  6) & 0x3F);
        *catp++ = 0x80 | ((left      ) & 0x3F);
    } else if (left > 0x7FF) {
        *catp++ = 0xE0 | ((left >> 12) & 0x0F);
        *catp++ = 0x80 | ((left >>  6) & 0x3F);
        *catp++ = 0x80 | ((left      ) & 0x3F);
    } else if (left > 0x7F) {
        *catp++ = 0xC0 | ((left >>  6) & 0x1F);
        *catp++ = 0x80 | ((left      ) & 0x3F);
    } else {
        *catp++ = static_cast<char>(left);
    }

    _ST_PRIVATE::_copy_buffer(catp, right.c_str(), right.size());
    catp[right.size()] = 0;

    return cat;
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
