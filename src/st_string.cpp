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

#include "st_string.h"

#include <cstring>
#include <cctype>
#include "st_assert.h"
#include "st_stringstream.h"

const ST::string ST::string::null;

#if !defined(ST_WCHAR_BYTES) || ((ST_WCHAR_BYTES != 2) && (ST_WCHAR_BYTES != 4))
#   error ST_WCHAR_SIZE must either be 2 (16-bit) or 4 (32-bit)
#endif

#if !defined(ST_SIZET_BYTES) || ((ST_SIZET_BYTES != 4) && (ST_SIZET_BYTES != 8))
#   error Supported size_t sizes are 4 (32-bit) or 8 (64-bit) bytes
#endif

#ifdef _MSC_VER
#   define stricmp  _stricmp
#   define strnicmp _strnicmp
#else
#   define stricmp  strcasecmp
#   define strnicmp strncasecmp
#endif

int ST::_lower_char(int ch)
{
    return tolower(ch);
}

int ST::_upper_char(int ch)
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
    ST_ASSERT(size == ST_AUTO_SIZE || size < HUGE_BUFFER_SIZE,
              "String data buffer is too large");

    if (!utf8) {
        m_buffer = char_buffer();
        return;
    }

    if (size == ST_AUTO_SIZE)
        size = ::strlen(utf8);

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
        m_buffer = init;
        _assert_utf8_buffer(m_buffer);
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
    m_buffer = std::move(init);

    switch (validation) {
    case assert_validity:
        _assert_utf8_buffer(m_buffer);
        break;

    case substitute_invalid:
        m_buffer = _cleanup_utf8_buffer(m_buffer);
        break;

    case assume_valid:
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

ST::string &ST::string::operator+=(const ST::string &other)
{
    set(*this + other);
    return *this;
}

void ST::string::_convert_from_utf16(const char16_t *utf16, size_t size,
                                     utf_validation_t validation)
{
    ST_ASSERT(size == ST_AUTO_SIZE || size < HUGE_BUFFER_SIZE,
              "String data buffer is too large");

    m_buffer = char_buffer();
    if (!utf16)
        return;

    if (size == ST_AUTO_SIZE)
        size = utf16_buffer::strlen(utf16);

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
    ST_ASSERT(size == ST_AUTO_SIZE || size < HUGE_BUFFER_SIZE,
              "String data buffer is too large");

    m_buffer = char_buffer();
    if (!utf32)
        return;

    if (size == ST_AUTO_SIZE)
        size = utf32_buffer::strlen(utf32);

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
            if (validation == assert_validity)
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
    ST_ASSERT(size == ST_AUTO_SIZE || size < HUGE_BUFFER_SIZE,
              "String data buffer is too large");

    m_buffer = char_buffer();
    if (!astr)
        return;

    if (size == ST_AUTO_SIZE)
        size = ::strlen(astr);

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
            *dp++ = bigch;
        } else if ((*sp & 0xE0) == 0xC0) {
            bigch  = (*sp++ & 0x1F) << 6;
            bigch |= (*sp++ & 0x3F);
            *dp++ = bigch;
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
    return *reinterpret_cast<wchar_buffer *>(&utf16);
#else
    utf32_buffer utf32 = to_utf32();
    return *reinterpret_cast<wchar_buffer *>(&utf32);
#endif
}

ST::char_buffer ST::string::to_latin_1() const
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
        *dp++ = (bigch < 0x100) ? bigch : '?';
    }
    astr[convlen] = 0;

    return result;
}

int ST::string::to_int(int base) const ST_NOEXCEPT
{
    return static_cast<int>(strtol(c_str(), nullptr, base));
}

unsigned int ST::string::to_uint(int base) const ST_NOEXCEPT
{
    return static_cast<unsigned int>(strtoul(c_str(), nullptr, base));
}

float ST::string::to_float() const ST_NOEXCEPT
{
    // Use strtod to avoid requiring C99
    return static_cast<float>(strtod(c_str(), nullptr));
}

double ST::string::to_double() const ST_NOEXCEPT
{
    return strtod(c_str(), nullptr);
}

#ifdef ST_HAVE_INT64
int64_t ST::string::to_int64(int base) const ST_NOEXCEPT
{
    return static_cast<int64_t>(strtoll(c_str(), nullptr, base));
}

uint64_t ST::string::to_uint64(int base) const ST_NOEXCEPT
{
    return static_cast<uint64_t>(strtoull(c_str(), nullptr, base));
}
#endif

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

ST_ssize_t ST::string::find(char ch, case_sensitivity_t cs) const ST_NOEXCEPT
{
    if (is_empty())
        return -1;

    if (cs == case_sensitive) {
        const char *cp = strchr(c_str(), ch);
        return cp ? (cp - c_str()) : -1;
    } else {
        const char *cp = c_str();
        while (*cp) {
            if (ST::_lower_char(ch) == ST::_lower_char(*cp))
                return cp - c_str();
            ++cp;
        }
        return -1;
    }
}

ST_ssize_t ST::string::find(const char *substr, case_sensitivity_t cs)
    const ST_NOEXCEPT
{
    if (!substr || !substr[0])
        return -1;

    if (cs == case_sensitive) {
        const char *cp = strstr(c_str(), substr);
        return cp ? (cp - c_str()) : -1;
    } else {
        // The "easy" way
        size_t sublen = strlen(substr);
        const char *cp = c_str();
        const char *ep = cp + size();
        while (cp + sublen <= ep) {
            if (strnicmp(cp, substr, sublen) == 0)
                return cp - c_str();
            ++cp;
        }
        return -1;
    }
}

ST_ssize_t ST::string::find_last(char ch, case_sensitivity_t cs) const ST_NOEXCEPT
{
    if (is_empty())
        return -1;

    if (cs == case_sensitive) {
        const char *cp = strrchr(c_str(), ch);
        return cp ? (cp - c_str()) : -1;
    } else {
        const char *cp = c_str() + size();
        while (--cp >= c_str()) {
            if (ST::_lower_char(ch) == ST::_lower_char(*cp))
                return cp - c_str();
        }
        return -1;
    }
}

ST_ssize_t ST::string::find_last(const char *substr, case_sensitivity_t cs)
    const ST_NOEXCEPT
{
    if (!substr || !substr[0])
        return -1;

    size_t sublen = strlen(substr);
    const char *cp = c_str() + size() - sublen;
    if (cs == case_sensitive) {
        while (cp >= c_str()) {
            if (strncmp(cp, substr, sublen) == 0)
                return cp - c_str();
            --cp;
        }
    } else {
        while (cp >= c_str()) {
            if (strnicmp(cp, substr, sublen) == 0)
                return cp - c_str();
            --cp;
        }
    }
    return -1;
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

ST::string ST::string::replace(const char *from, const char *to,
                               utf_validation_t validation) const
{
    if (is_empty() || !from || !from[0])
        return *this;

    if (!to)
        to = "";

    string_stream out;
    const char *pstart = c_str();
    const char *pnext;
    size_t flen = strlen(from), tlen = strlen(to);
    while ( (pnext = strstr(pstart, from)) ) {
        out.append(pstart, pnext - pstart);
        out.append(to, tlen);
        pstart = pnext + flen;
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
        *dp++ = ST::_upper_char(*sp++);
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
        *dp++ = ST::_lower_char(*sp++);
    dupe[size()] = 0;

    return result;
}

std::vector<ST::string> ST::string::split(const char *splitter,
                                          size_t max_splits) const
{
    std::vector<string> result;

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
        const char *sp = strstr(next, splitter);
        if (!sp)
            break;

        result.push_back(string(next, sp - next, validation));
        next = sp + splitlen;
        --max_splits;
    }

    result.push_back(string(next, end - next, validation));
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

ST::string operator+(const ST::string &left, const ST::string &right)
{
    ST::char_buffer cat;
    char *catp = cat.create_writable_buffer(left.size() + right.size());
    _ST_PRIVATE::_copy_buffer(catp, left.c_str(), left.size());
    _ST_PRIVATE::_copy_buffer(catp + left.size(), right.c_str(), right.size());
    catp[cat.size()] = 0;

    return ST::string(cat, ST::assume_valid);
}

ST::string operator+(const ST::string &left, const char *right)
{
    ST::char_buffer cat;
    size_t rsize = strlen(right);
    char *catp = cat.create_writable_buffer(left.size() + rsize);
    _ST_PRIVATE::_copy_buffer(catp, left.c_str(), left.size());
    _ST_PRIVATE::_copy_buffer(catp + left.size(), right, rsize);
    catp[cat.size()] = 0;

    return cat;
}

ST::string operator+(const char *left, const ST::string &right)
{
    ST::char_buffer cat;
    size_t lsize = strlen(left);
    char *catp = cat.create_writable_buffer(lsize + right.size());
    _ST_PRIVATE::_copy_buffer(catp, left, lsize);
    _ST_PRIVATE::_copy_buffer(catp + lsize, right.c_str(), right.size());
    catp[cat.size()] = 0;

    return cat;
}
