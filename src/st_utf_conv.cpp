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

#include "st_utf_conv.h"

#define BADCHAR_SUBSTITUTE          0xFFFDul
#define BADCHAR_SUBSTITUTE_UTF8     "\xEF\xBF\xBD"
#define BADCHAR_SUBSTITUTE_UTF8_LEN 3

#define _CHECK_NEXT_SEQ_BYTE() \
    do { \
        ++cp; \
        if ((*cp & 0xC0) != 0x80) \
            return conversion_error_t::invalid_utf8_seq; \
    } while (false)

_ST_PRIVATE::conversion_error_t
_ST_PRIVATE::validate_utf8(const char *buffer, size_t size)
{
    const unsigned char *cp = reinterpret_cast<const unsigned char *>(buffer);
    const unsigned char *ep = cp + size;
    for (; cp < ep; ++cp) {
        if (*cp < 0x80)
            continue;

        if ((*cp & 0xE0) == 0xC0) {
            // Two bytes
            if (cp + 2 > ep)
                return conversion_error_t::incomplete_utf8_seq;
            _CHECK_NEXT_SEQ_BYTE();
        } else if ((*cp & 0xF0) == 0xE0) {
            // Three bytes
            if (cp + 3 > ep)
                return conversion_error_t::incomplete_utf8_seq;
            _CHECK_NEXT_SEQ_BYTE();
            _CHECK_NEXT_SEQ_BYTE();
        } else if ((*cp & 0xF8) == 0xF0) {
            // Four bytes
            if (cp + 4 > ep)
                return conversion_error_t::incomplete_utf8_seq;
            _CHECK_NEXT_SEQ_BYTE();
            _CHECK_NEXT_SEQ_BYTE();
            _CHECK_NEXT_SEQ_BYTE();
        } else {
            // Invalid sequence byte
            return conversion_error_t::invalid_utf8_seq;
        }
    }

    return conversion_error_t::success;
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
                *output++ = static_cast<char>(*sp);
            sp += 1;
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
            sp += 1;
        }
    }

    return output_size;
}

size_t _ST_PRIVATE::utf8_measure_from_utf16(const char16_t *utf16, size_t size)
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
_ST_PRIVATE::utf8_convert_from_utf16(char *dp, const char16_t *utf16, size_t size,
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

size_t _ST_PRIVATE::utf8_measure(char32_t ch)
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

size_t _ST_PRIVATE::utf8_measure_from_utf32(const char32_t *utf32, size_t size)
{
    if (!utf32)
        return 0;

    size_t u8len = 0;
    const char32_t *sp = utf32;
    const char32_t *ep = sp + size;
    for (; sp < ep; ++sp)
        u8len += _ST_PRIVATE::utf8_measure(*sp);
    return u8len;
}

_ST_PRIVATE::conversion_error_t
_ST_PRIVATE::utf8_convert_from_utf32(char *dp, const char32_t *utf32, size_t size,
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

size_t _ST_PRIVATE::utf8_measure_from_latin_1(const char *astr, size_t size)
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

void _ST_PRIVATE::utf8_convert_from_latin_1(char *dp, const char *astr, size_t size)
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

size_t _ST_PRIVATE::utf16_measure_from_utf8(const char *utf8, size_t size)
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
        } else if ((*sp & 0xF8) == 0xF0) {
            if (sp + 4 > ep || (sp[1] & 0xC0) != 0x80 || (sp[2] & 0xC0) != 0x80
                            || (sp[3] & 0xC0) != 0x80) {
                // Invalid or incomplete sequence
            } else {
                // Encode with surrogate pair
                ++u16len;
            }
            sp += 4;
        } else {
            // Invalid UTF-8 sequence byte
            sp += 1;
        }
        ++u16len;
    }
    return u16len;
}

_ST_PRIVATE::conversion_error_t
_ST_PRIVATE::utf16_convert_from_utf8(char16_t *dp, const char *utf8, size_t size,
                                     ST::utf_validation_t validation)
{
    const unsigned char *sp = reinterpret_cast<const unsigned char *>(utf8);
    const unsigned char *ep = sp + size;
    while (sp < ep) {
        char32_t bigch = 0;
        if (*sp < 0x80) {
            *dp++ = *sp++;
        } else if ((*sp & 0xE0) == 0xC0) {
            if (sp + 2 > ep || (sp[1] & 0xC0) != 0x80) {
                if (validation == ST::check_validity)
                    return conversion_error_t::incomplete_utf8_seq;
                *dp++ = BADCHAR_SUBSTITUTE;
                sp += 2;
            } else {
                bigch = (*sp++ & 0x1F) << 6;
                bigch |= (*sp++ & 0x3F);
                *dp++ = static_cast<char16_t>(bigch);
            }
        } else if ((*sp & 0xF0) == 0xE0) {
            if (sp + 3 > ep || (sp[1] & 0xC0) != 0x80 || (sp[2] & 0xC0) != 0x80) {
                if (validation == ST::check_validity)
                    return conversion_error_t::incomplete_utf8_seq;
                *dp++ = BADCHAR_SUBSTITUTE;
                sp += 3;
            } else {
                bigch  = (*sp++ & 0x0F) << 12;
                bigch |= (*sp++ & 0x3F) << 6;
                bigch |= (*sp++ & 0x3F);
                *dp++ = static_cast<char16_t>(bigch);
            }
        } else if ((*sp & 0xF8) == 0xF0) {
            if (sp + 4 > ep || (sp[1] & 0xC0) != 0x80 || (sp[2] & 0xC0) != 0x80
                            || (sp[3] & 0xC0) != 0x80) {
                if (validation == ST::check_validity)
                    return conversion_error_t::incomplete_utf8_seq;
                *dp++ = BADCHAR_SUBSTITUTE;
                sp += 4;
            } else {
                bigch  = (*sp++ & 0x07) << 18;
                bigch |= (*sp++ & 0x3F) << 12;
                bigch |= (*sp++ & 0x3F) << 6;
                bigch |= (*sp++ & 0x3F);
                bigch -= 0x10000;

                *dp++ = 0xD800 | ((bigch >> 10) & 0x3FF);
                *dp++ = 0xDC00 | ((bigch      ) & 0x3FF);
            }
        } else {
            if (validation == ST::check_validity)
                return conversion_error_t::invalid_utf8_seq;
            *dp++ = BADCHAR_SUBSTITUTE;
            sp += 1;
        }
    }

    return conversion_error_t::success;
}

size_t _ST_PRIVATE::utf32_measure_from_utf8(const char *utf8, size_t size)
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
        else if ((*sp & 0xF8) == 0xF0)
            sp += 4;
        else
            sp += 1;    // Invalid UTF-8 sequence byte
        ++u32len;
    }
    return u32len;
}

_ST_PRIVATE::conversion_error_t
_ST_PRIVATE::utf32_convert_from_utf8(char32_t *dp, const char *utf8, size_t size,
                                     ST::utf_validation_t validation)
{
    const unsigned char *sp = reinterpret_cast<const unsigned char *>(utf8);
    const unsigned char *ep = sp + size;
    while (sp < ep) {
        char32_t bigch = 0;
        if (*sp < 0x80) {
            bigch = *sp++;
        } else if ((*sp & 0xE0) == 0xC0) {
            if (sp + 2 > ep || (sp[1] & 0xC0) != 0x80) {
                if (validation == ST::check_validity)
                    return conversion_error_t::incomplete_utf8_seq;
                bigch = BADCHAR_SUBSTITUTE;
                sp += 2;
            } else {
                bigch  = (*sp++ & 0x1F) << 6;
                bigch |= (*sp++ & 0x3F);
            }
        } else if ((*sp & 0xF0) == 0xE0) {
            if (sp + 3 > ep || (sp[1] & 0xC0) != 0x80 || (sp[2] & 0xC0) != 0x80) {
                if (validation == ST::check_validity)
                    return conversion_error_t::incomplete_utf8_seq;
                bigch = BADCHAR_SUBSTITUTE;
                sp += 3;
            } else {
                bigch  = (*sp++ & 0x0F) << 12;
                bigch |= (*sp++ & 0x3F) << 6;
                bigch |= (*sp++ & 0x3F);
            }
        } else if ((*sp & 0xF8) == 0xF0) {
            if (sp + 4 > ep || (sp[1] & 0xC0) != 0x80 || (sp[2] & 0xC0) != 0x80
                            || (sp[3] & 0xC0) != 0x80) {
                if (validation == ST::check_validity)
                    return conversion_error_t::incomplete_utf8_seq;
                bigch = BADCHAR_SUBSTITUTE;
                sp += 4;
            } else {
                bigch  = (*sp++ & 0x07) << 18;
                bigch |= (*sp++ & 0x3F) << 12;
                bigch |= (*sp++ & 0x3F) << 6;
                bigch |= (*sp++ & 0x3F);
            }
        } else {
            if (validation == ST::check_validity)
                return conversion_error_t::invalid_utf8_seq;
            bigch = BADCHAR_SUBSTITUTE;
            sp += 1;
        }
        *dp++ = bigch;
    }

    return conversion_error_t::success;
}

size_t _ST_PRIVATE::latin_1_measure_from_utf8(const char *utf8, size_t size)
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
        else if ((*sp & 0xF8) == 0xF0)
            sp += 4;
        else
            sp += 1;    // Invalid UTF-8 sequence byte
        ++alen;
    }
    return alen;
}

_ST_PRIVATE::conversion_error_t
_ST_PRIVATE::latin_1_convert_from_utf8(char *dp, const char *utf8, size_t size,
                                       ST::utf_validation_t validation,
                                       bool substitute_out_of_range)
{
    const unsigned char *sp = reinterpret_cast<const unsigned char *>(utf8);
    const unsigned char *ep = sp + size;
    while (sp < ep) {
        char32_t bigch = 0;
        if (*sp < 0x80) {
            bigch = *sp++;
        } else if ((*sp & 0xE0) == 0xC0) {
            if (sp + 2 > ep || (sp[1] & 0xC0) != 0x80) {
                if (validation == ST::check_validity)
                    return conversion_error_t::incomplete_utf8_seq;
                bigch = '?';
                sp += 2;
            } else {
                bigch  = (*sp++ & 0x1F) << 6;
                bigch |= (*sp++ & 0x3F);
            }
        } else if ((*sp & 0xF0) == 0xE0) {
            if (sp + 3 > ep || (sp[1] & 0xC0) != 0x80 || (sp[2] & 0xC0) != 0x80) {
                if (validation == ST::check_validity)
                    return conversion_error_t::incomplete_utf8_seq;
                bigch = '?';
                sp += 3;
            } else {
                bigch  = (*sp++ & 0x0F) << 12;
                bigch |= (*sp++ & 0x3F) << 6;
                bigch |= (*sp++ & 0x3F);
            }
        } else if ((*sp & 0xF8) == 0xF0) {
            if (sp + 4 > ep || (sp[1] & 0xC0) != 0x80 || (sp[2] & 0xC0) != 0x80
                            || (sp[3] & 0xC0) != 0x80) {
                if (validation == ST::check_validity)
                    return conversion_error_t::incomplete_utf8_seq;
                bigch = '?';
                sp += 4;
            } else {
                bigch  = (*sp++ & 0x07) << 18;
                bigch |= (*sp++ & 0x3F) << 12;
                bigch |= (*sp++ & 0x3F) << 6;
                bigch |= (*sp++ & 0x3F);
            }
        } else {
            if (validation == ST::check_validity)
                return conversion_error_t::invalid_utf8_seq;
            bigch = '?';
            sp += 1;
        }

        if (bigch >= 0x100) {
            if (substitute_out_of_range)
                bigch = '?';
            else
                return conversion_error_t::latin1_out_of_range;
        }
        *dp++ = static_cast<char>(bigch);
    }

    return conversion_error_t::success;
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
