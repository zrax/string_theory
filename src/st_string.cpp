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

#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#if !defined(ST_SIZET_BYTES) || ((ST_SIZET_BYTES != 4) && (ST_SIZET_BYTES != 8))
#   error Supported size_t sizes are 4 (32-bit) or 8 (64-bit) bytes
#endif

#define BADCHAR_SUBSTITUTE          0xFFFDul
#define BADCHAR_SUBSTITUTE_UTF8     "\xEF\xBF\xBD"
#define BADCHAR_SUBSTITUTE_UTF8_LEN 3

#define _CHECK_NEXT_SEQ_BYTE() \
    do { \
        ++cp; \
        if ((*cp & 0xC0) != 0x80) \
            return false; \
    } while (false)

bool _ST_PRIVATE::validate_utf8(const char *buffer, size_t size)
{
    const unsigned char *cp = reinterpret_cast<const unsigned char *>(buffer);
    const unsigned char *ep = cp + size;
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

static size_t _append_chars(char *&output, const char *src, size_t count)
{
    if (output) {
        std::char_traits<char>::copy(output, src, count);
        output += count;
    }
    return count;
}

size_t _ST_PRIVATE::cleanup_utf8(char *output, const char *buffer, size_t size)
{
    size_t output_size = 0;

    const unsigned char *sp = reinterpret_cast<const unsigned char *>(buffer);
    const unsigned char *ep = sp + size;
    while (sp < ep) {
        if (*sp < 0x80) {
            ++output_size;
            if (output)
                *output++ = static_cast<char>(*sp++);
        } else if ((*sp & 0xE0) == 0xC0) {
            // Two bytes
            if (sp + 2 > ep || (sp[1] & 0xC0) != 0x80)
                output_size += _append_chars(output, BADCHAR_SUBSTITUTE_UTF8,
                                             BADCHAR_SUBSTITUTE_UTF8_LEN);
            else
                output_size += _append_chars(output, reinterpret_cast<const char *>(sp), 2);
            sp += 2;
        } else if ((*sp & 0xF0) == 0xE0) {
            // Three bytes
            if (sp + 3 > ep || (sp[1] & 0xC0) != 0x80 || (sp[2] & 0xC0) != 0x80)
                output_size += _append_chars(output, BADCHAR_SUBSTITUTE_UTF8,
                                             BADCHAR_SUBSTITUTE_UTF8_LEN);
            else
                output_size += _append_chars(output, reinterpret_cast<const char *>(sp), 3);
            sp += 3;
        } else if ((*sp & 0xF8) == 0xF0) {
            // Four bytes
            if (sp + 4 > ep || (sp[1] & 0xC0) != 0x80 || (sp[2] & 0xC0) != 0x80
                            || (sp[3] & 0xC0) != 0x80)
                output_size += _append_chars(output, BADCHAR_SUBSTITUTE_UTF8,
                                             BADCHAR_SUBSTITUTE_UTF8_LEN);
            else
                output_size += _append_chars(output, reinterpret_cast<const char *>(sp), 4);
            sp += 4;
        } else {
            // Invalid sequence byte
            output_size += _append_chars(output, BADCHAR_SUBSTITUTE_UTF8,
                                         BADCHAR_SUBSTITUTE_UTF8_LEN);
        }
    }

    return output_size;
}

size_t _ST_PRIVATE::measure_from_utf16(const char16_t *utf16, size_t size)
{
    if (!utf16)
        return 0;

    size_t u8len = 0;
    const char16_t *sp = utf16;
    const char16_t *ep = sp + size;
    for (; sp < ep; ++sp) {
        if (*sp < 0x80) {
            u8len += 1;
        } else if (*sp < 0x800) {
            u8len += 2;
        } else if (*sp >= 0xD800 && *sp <= 0xDFFF) {
            // Surrogate pair
            if (sp + 1 >= ep) {
                u8len += BADCHAR_SUBSTITUTE_UTF8_LEN;
            } else if (*sp < 0xDC00) {
                if (sp[1] >= 0xDC00 && sp[1] <= 0xDFFF) {
                    u8len += 4;
                    ++sp;
                } else {
                    u8len += BADCHAR_SUBSTITUTE_UTF8_LEN;
                }
            } else {
                if (sp[1] >= 0xD800 && sp[1] <= 0xDBFF) {
                    u8len += 4;
                    ++sp;
                } else {
                    u8len += BADCHAR_SUBSTITUTE_UTF8_LEN;
                }
            }
        } else {
            u8len += 3;
        }
    }
    return u8len;
}

_ST_PRIVATE::conversion_error_t
_ST_PRIVATE::convert_from_utf16(char *dp, const char16_t *utf16, size_t size,
                                ST::utf_validation_t validation)
{
    const char16_t *sp = utf16;
    const char16_t *ep = sp + size;
    for (; sp < ep; ++sp) {
        if (*sp < 0x80) {
            *dp++ = static_cast<char>(*sp);
        } else if (*sp < 0x800) {
            *dp++ = 0xC0 | ((*sp >>  6) & 0x1F);
            *dp++ = 0x80 | ((*sp      ) & 0x3F);
        } else if (*sp >= 0xD800 && *sp <= 0xDFFF) {
            // Surrogate pair
            if (sp + 1 >= ep) {
                switch (validation) {
                case ST::check_validity:
                    return conversion_error_t::incomplete_surrogate_pair;
                case ST::substitute_invalid:
                    std::char_traits<char>::copy(dp, BADCHAR_SUBSTITUTE_UTF8,
                                                 BADCHAR_SUBSTITUTE_UTF8_LEN);
                    dp += BADCHAR_SUBSTITUTE_UTF8_LEN;
                    break;
                case ST::assume_valid:
                    // Encode the bad surrogate char as a UTF-8 value
                    *dp++ = 0xE0 | ((*sp >> 12) & 0x0F);
                    *dp++ = 0x80 | ((*sp >>  6) & 0x3F);
                    *dp++ = 0x80 | ((*sp      ) & 0x3F);
                    break;
                default:
                    ST_ASSERT(false, "Invalid validation type");
                }
            } else if (*sp < 0xDC00) {
                if (sp[1] >= 0xDC00 && sp[1] <= 0xDFFF) {
                    char32_t bigch = 0x10000 + ((sp[0] & 0x3FF) << 10) + (sp[1] & 0x3FF);
                    *dp++ = 0xF0 | ((bigch >> 18) & 0x07);
                    *dp++ = 0x80 | ((bigch >> 12) & 0x3F);
                    *dp++ = 0x80 | ((bigch >>  6) & 0x3F);
                    *dp++ = 0x80 | ((bigch      ) & 0x3F);
                    ++sp;
                } else {
                    switch (validation) {
                    case ST::check_validity:
                        return conversion_error_t::incomplete_surrogate_pair;
                    case ST::substitute_invalid:
                        std::char_traits<char>::copy(dp, BADCHAR_SUBSTITUTE_UTF8,
                                                     BADCHAR_SUBSTITUTE_UTF8_LEN);
                        dp += BADCHAR_SUBSTITUTE_UTF8_LEN;
                        break;
                    case ST::assume_valid:
                        // Encode the bad surrogate char as a UTF-8 value
                        *dp++ = 0xE0 | ((*sp >> 12) & 0x0F);
                        *dp++ = 0x80 | ((*sp >>  6) & 0x3F);
                        *dp++ = 0x80 | ((*sp      ) & 0x3F);
                        break;
                    default:
                        ST_ASSERT(false, "Invalid validation type");
                    }
                }
            } else {
                if (sp[1] >= 0xD800 && sp[1] <= 0xDBFF) {
                    char32_t bigch = 0x10000 + (sp[0] & 0x3FF) + ((sp[1] & 0x3FF) << 10);
                    *dp++ = 0xF0 | ((bigch >> 18) & 0x07);
                    *dp++ = 0x80 | ((bigch >> 12) & 0x3F);
                    *dp++ = 0x80 | ((bigch >>  6) & 0x3F);
                    *dp++ = 0x80 | ((bigch      ) & 0x3F);
                    ++sp;
                } else {
                    switch (validation) {
                    case ST::check_validity:
                        return conversion_error_t::incomplete_surrogate_pair;
                    case ST::substitute_invalid:
                        std::char_traits<char>::copy(dp, BADCHAR_SUBSTITUTE_UTF8,
                                                     BADCHAR_SUBSTITUTE_UTF8_LEN);
                        dp += BADCHAR_SUBSTITUTE_UTF8_LEN;
                        break;
                    case ST::assume_valid:
                        // Encode the bad surrogate char as a UTF-8 value
                        *dp++ = 0xE0 | ((*sp >> 12) & 0x0F);
                        *dp++ = 0x80 | ((*sp >>  6) & 0x3F);
                        *dp++ = 0x80 | ((*sp      ) & 0x3F);
                        break;
                    default:
                        ST_ASSERT(false, "Invalid validation type");
                    }
                }
            }
        } else {
            *dp++ = 0xE0 | ((*sp >> 12) & 0x0F);
            *dp++ = 0x80 | ((*sp >>  6) & 0x3F);
            *dp++ = 0x80 | ((*sp      ) & 0x3F);
        }
    }

    return conversion_error_t::success;
}

size_t _ST_PRIVATE::measure(char32_t ch)
{
    if (ch < 0x80) {
        return 1;
    } else if (ch < 0x800) {
        return 2;
    } else if (ch < 0x10000) {
        return 3;
    } else if (ch <= 0x10FFFF) {
        return 4;
    } else {
        // Out-of-range code point always gets replaced
        return BADCHAR_SUBSTITUTE_UTF8_LEN;
    }
}

size_t _ST_PRIVATE::measure_from_utf32(const char32_t *utf32, size_t size)
{
    if (!utf32)
        return 0;

    size_t u8len = 0;
    const char32_t *sp = utf32;
    const char32_t *ep = sp + size;
    for (; sp < ep; ++sp)
        u8len += _ST_PRIVATE::measure(*sp);
    return u8len;
}

_ST_PRIVATE::conversion_error_t
_ST_PRIVATE::convert_from_utf32(char *dp, const char32_t *utf32, size_t size,
                                ST::utf_validation_t validation)
{
    const char32_t *sp = utf32;
    const char32_t *ep = sp + size;
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
            if (validation == ST::check_validity)
                return conversion_error_t::out_of_range;
            std::char_traits<char>::copy(dp, BADCHAR_SUBSTITUTE_UTF8,
                                         BADCHAR_SUBSTITUTE_UTF8_LEN);
            dp += BADCHAR_SUBSTITUTE_UTF8_LEN;
        }
    }

    return conversion_error_t::success;
}

size_t _ST_PRIVATE::measure_from_latin_1(const char *astr, size_t size)
{
    if (!astr)
        return 0;

    size_t u8len = 0;
    const char *sp = astr;
    const char *ep = sp + size;
    for (; sp < ep; ++sp) {
        if (*sp & 0x80)
            u8len += 2;
        else
            u8len += 1;
    }
    return u8len;
}

void _ST_PRIVATE::convert_from_latin_1(char *dp, const char *astr, size_t size)
{
    const char *sp = astr;
    const char *ep = sp + size;
    for (; sp < ep; ++sp) {
        if (*sp & 0x80) {
            *dp++ = 0xC0 | ((static_cast<unsigned char>(*sp) >> 6) & 0x1F);
            *dp++ = 0x80 | ((static_cast<unsigned char>(*sp)     ) & 0x3F);
        } else {
            *dp++ = *sp;
        }
    }
}

size_t _ST_PRIVATE::measure_to_utf16(const char *utf8, size_t size)
{
    if (size == 0)
        return 0;

    size_t u16len = 0;
    const unsigned char *sp = reinterpret_cast<const unsigned char *>(utf8);
    const unsigned char *ep = sp + size;
    while (sp < ep) {
        if (*sp < 0x80) {
            sp += 1;
        } else if ((*sp & 0xE0) == 0xC0) {
            sp += 2;
        } else if ((*sp & 0xF0) == 0xE0) {
            sp += 3;
        } else {
            // Encode with surrogate pair
            ++u16len;
            sp += 4;
        }
        ++u16len;
    }
    return u16len;
}

void _ST_PRIVATE::convert_to_utf16(char16_t *dp, const char *utf8, size_t size)
{
    const unsigned char *sp = reinterpret_cast<const unsigned char *>(utf8);
    const unsigned char *ep = sp + size;
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
}

size_t _ST_PRIVATE::measure_to_utf32(const char *utf8, size_t size)
{
    if (size == 0)
        return 0;

    size_t u32len = 0;
    const unsigned char *sp = reinterpret_cast<const unsigned char *>(utf8);
    const unsigned char *ep = sp + size;
    while (sp < ep) {
        if (*sp < 0x80)
            sp += 1;
        else if ((*sp & 0xE0) == 0xC0)
            sp += 2;
        else if ((*sp & 0xF0) == 0xE0)
            sp += 3;
        else
            sp += 4;
        ++u32len;
    }
    return u32len;
}

void _ST_PRIVATE::convert_to_utf32(char32_t *dp, const char *utf8, size_t size)
{
    const unsigned char *sp = reinterpret_cast<const unsigned char *>(utf8);
    const unsigned char *ep = sp + size;
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
}

size_t _ST_PRIVATE::measure_to_latin_1(const char *utf8, size_t size)
{
    if (size == 0)
        return 0;

    size_t alen = 0;
    const unsigned char *sp = reinterpret_cast<const unsigned char *>(utf8);
    const unsigned char *ep = sp + size;
    while (sp < ep) {
        if (*sp < 0x80)
            sp += 1;
        else if ((*sp & 0xE0) == 0xC0)
            sp += 2;
        else if ((*sp & 0xF0) == 0xE0)
            sp += 3;
        else
            sp += 4;
        ++alen;
    }
    return alen;
}

_ST_PRIVATE::conversion_error_t
_ST_PRIVATE::convert_to_latin_1(char *dp, const char *utf8, size_t size,
                                ST::utf_validation_t validation)
{
    const unsigned char *sp = reinterpret_cast<const unsigned char *>(utf8);
    const unsigned char *ep = sp + size;
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
            if (validation == ST::check_validity)
                return conversion_error_t::latin1_out_of_range;
            else
                bigch = '?';
        }
        *dp++ = static_cast<char>(bigch);
    }

    return conversion_error_t::success;
}

int ST::string::to_int(int base) const noexcept
{
    return static_cast<int>(strtol(c_str(), nullptr, base));
}

int ST::string::to_int(ST::conversion_result &result, int base) const noexcept
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

unsigned int ST::string::to_uint(int base) const noexcept
{
    return static_cast<unsigned int>(strtoul(c_str(), nullptr, base));
}

unsigned int ST::string::to_uint(ST::conversion_result &result, int base)
    const noexcept
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

float ST::string::to_float() const noexcept
{
    return static_cast<float>(strtof(c_str(), nullptr));
}

float ST::string::to_float(ST::conversion_result &result) const noexcept
{
    if (empty()) {
        result.m_flags = ST::conversion_result::result_full_match;
        return 0;
    }

    char *end;
    float value = strtof(c_str(), &end);
    result.m_flags = 0;
    if (end != c_str())
        result.m_flags |= ST::conversion_result::result_ok;
    if (end == c_str() + size())
        result.m_flags |= ST::conversion_result::result_full_match;
    return value;
}

double ST::string::to_double() const noexcept
{
    return strtod(c_str(), nullptr);
}

double ST::string::to_double(ST::conversion_result &result) const noexcept
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
int64_t ST::string::to_int64(int base) const noexcept
{
    return static_cast<int64_t>(strtoll(c_str(), nullptr, base));
}

int64_t ST::string::to_int64(ST::conversion_result &result, int base)
    const noexcept
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

uint64_t ST::string::to_uint64(int base) const noexcept
{
    return static_cast<uint64_t>(strtoull(c_str(), nullptr, base));
}

uint64_t ST::string::to_uint64(ST::conversion_result &result, int base)
    const noexcept
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

static int _compare_cs(const char *left, const char *right, size_t fsize)
{
    return std::char_traits<char>::compare(left, right, fsize);
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
    while (fsize--) {
        const char cl = _ST_PRIVATE::cl_fast_lower(*left++);
        const char cr = _ST_PRIVATE::cl_fast_lower(*right++);
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

int ST::string::compare(const string &str, case_sensitivity_t cs) const noexcept
{
    return (cs == case_sensitive) ? _compare_cs(c_str(), size(), str.c_str(), str.size())
                                  : _compare_ci(c_str(), size(), str.c_str(), str.size());
}

int ST::string::compare(const char *str, case_sensitivity_t cs) const noexcept
{
    const size_t rsize = str ? std::char_traits<char>::length(str) : 0;
    return (cs == case_sensitive) ? _compare_cs(c_str(), size(), str ? str : "", rsize)
                                  : _compare_ci(c_str(), size(), str ? str : "", rsize);
}

int ST::string::compare_n(const string &str, size_t count,
                          case_sensitivity_t cs) const noexcept
{
    return (cs == case_sensitive) ? _compare_cs(c_str(), size(), str.c_str(), str.size(), count)
                                  : _compare_ci(c_str(), size(), str.c_str(), str.size(), count);
}

int ST::string::compare_n(const char *str, size_t count,
                          case_sensitivity_t cs) const noexcept
{
    const size_t rsize = str ? std::char_traits<char>::length(str) : 0;
    return (cs == case_sensitive) ? _compare_cs(c_str(), size(), str ? str : "", rsize, count)
                                  : _compare_ci(c_str(), size(), str ? str : "", rsize, count);
}

const char *_ST_PRIVATE::find_cs(const char *haystack, const char *needle)
{
    return strstr(haystack, needle);
}

const char *_ST_PRIVATE::find_ci(const char *haystack, const char *needle)
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
    return nullptr;
}

const char *_ST_PRIVATE::find_ci(const char *haystack, size_t size, char ch)
{
    const char *cp = haystack;
    const char *ep = haystack + size;
    const int lch = _ST_PRIVATE::cl_fast_lower(static_cast<char>(ch));
    while (cp < ep) {
        if (_ST_PRIVATE::cl_fast_lower(*cp) == lch)
            return cp;
        ++cp;
    }
    return nullptr;
}

ST_ssize_t ST::string::find(size_t start, char ch, case_sensitivity_t cs)
    const noexcept
{
    if (start >= size())
        return -1;

    const char *cp = (cs == case_sensitive)
            ? _ST_PRIVATE::find_cs(c_str() + start, size() - start, ch)
            : _ST_PRIVATE::find_ci(c_str() + start, size() - start, ch);
    return cp ? (cp - c_str()) : -1;
}

ST_ssize_t ST::string::find(size_t start, const char *substr, case_sensitivity_t cs)
    const noexcept
{
    if (!substr || !substr[0] || start >= size())
        return -1;

    const char *cp = (cs == case_sensitive)
            ? _ST_PRIVATE::find_cs(c_str() + start, substr)
            : _ST_PRIVATE::find_ci(c_str() + start, substr);
    return cp ? (cp - c_str()) : -1;
}

ST_ssize_t ST::string::find_last(size_t max, char ch, case_sensitivity_t cs) const noexcept
{
    if (empty())
        return -1;

    const char *endp = c_str() + (max > size() ? size() : max);

    const char *start = c_str();
    const char *found = nullptr;
    for ( ;; ) {
        const char *cp = (cs == case_sensitive)
                ? _ST_PRIVATE::find_cs(start, endp - start, ch)
                : _ST_PRIVATE::find_ci(start, endp - start, ch);
        if (!cp || cp >= endp)
            break;
        found = cp;
        start = cp + 1;
    }
    return found ? (found - c_str()) : -1;
}

ST_ssize_t ST::string::find_last(size_t max, const char *substr, case_sensitivity_t cs)
    const noexcept
{
    if (!substr || !substr[0] || empty())
        return -1;

    const char *endp = c_str() + (max > size() ? size() : max);

    const char *start = c_str();
    const char *found = nullptr;
    for ( ;; ) {
        const char *cp = (cs == case_sensitive)
                ? _ST_PRIVATE::find_cs(start, substr)
                : _ST_PRIVATE::find_ci(start, substr);
        if (!cp || cp >= endp)
            break;
        found = cp;
        start = cp + 1;
    }
    return found ? (found - c_str()) : -1;
}

bool ST::string::starts_with(const ST::string &prefix, case_sensitivity_t cs) const noexcept
{
    if (prefix.size() > size())
        return false;
    return compare_n(prefix, prefix.size(), cs) == 0;
}

bool ST::string::starts_with(const char *prefix, case_sensitivity_t cs) const noexcept
{
    size_t count = prefix ? std::char_traits<char>::length(prefix) : 0;
    if (count > size())
        return false;
    return compare_n(prefix, count, cs) == 0;
}

bool ST::string::ends_with(const ST::string &suffix, case_sensitivity_t cs) const noexcept
{
    if (suffix.size() > size())
        return false;

    size_t start = size() - suffix.size();
    return (cs == case_sensitive)
            ? _compare_cs(c_str() + start, suffix.c_str(), suffix.size()) == 0
            : _compare_ci(c_str() + start, suffix.c_str(), suffix.size()) == 0;
}

bool ST::string::ends_with(const char *suffix, case_sensitivity_t cs) const noexcept
{
    size_t count = suffix ? std::char_traits<char>::length(suffix) : 0;
    if (count > size())
        return false;

    size_t start = size() - count;
    return (cs == case_sensitive)
            ? _compare_cs(c_str() + start, suffix ? suffix : "", count) == 0
            : _compare_ci(c_str() + start, suffix ? suffix : "", count) == 0;
}

size_t ST::hash::operator()(const string &str) const noexcept
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

size_t ST::hash_i::operator()(const string &str) const noexcept
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
        hash ^= static_cast<size_t>(_ST_PRIVATE::cl_fast_lower(*cp++));
        hash *= FNV_PRIME;
    }
    return hash;
}

_ST_PRIVATE::conversion_error_t
_ST_PRIVATE::append_utf8(char *dp, char32_t ch)
{
    if (ch < 0x80) {
        *dp++ = static_cast<char>(ch);
    } else if (ch < 0x800) {
        *dp++ = 0xC0 | ((ch >>  6) & 0x1F);
        *dp++ = 0x80 | ((ch      ) & 0x3F);
    } else if (ch < 0x10000) {
        *dp++ = 0xE0 | ((ch >> 12) & 0x0F);
        *dp++ = 0x80 | ((ch >>  6) & 0x3F);
        *dp++ = 0x80 | ((ch      ) & 0x3F);
    } else if (ch <= 0x10FFFF) {
        *dp++ = 0xF0 | ((ch >> 18) & 0x07);
        *dp++ = 0x80 | ((ch >> 12) & 0x3F);
        *dp++ = 0x80 | ((ch >>  6) & 0x3F);
        *dp++ = 0x80 | ((ch      ) & 0x3F);
    } else {
        return conversion_error_t::out_of_range;
    }

    return conversion_error_t::success;
}
