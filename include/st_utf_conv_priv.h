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

#ifndef _ST_UTF_CONV_PRIV_H
#define _ST_UTF_CONV_PRIV_H

namespace _ST_PRIVATE
{
    constexpr unsigned int badchar_substitute = 0xFFFDu;
    constexpr const char badchar_substitute_utf8[] = "\xEF\xBF\xBD";
    constexpr size_t badchar_substitute_utf8_len = sizeof(badchar_substitute_utf8) - 1;

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

#   define _ST_CHECK_NEXT_SEQ_BYTE() \
    do { \
        ++cp; \
        if ((*cp & 0xC0) != 0x80) \
            return conversion_error_t::invalid_utf8_seq; \
    } while (false)

    inline conversion_error_t validate_utf8(const char *buffer, size_t size)
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
                _ST_CHECK_NEXT_SEQ_BYTE();
            } else if ((*cp & 0xF0) == 0xE0) {
                // Three bytes
                if (cp + 3 > ep)
                    return conversion_error_t::incomplete_utf8_seq;
                _ST_CHECK_NEXT_SEQ_BYTE();
                _ST_CHECK_NEXT_SEQ_BYTE();
            } else if ((*cp & 0xF8) == 0xF0) {
                // Four bytes
                if (cp + 4 > ep)
                    return conversion_error_t::incomplete_utf8_seq;
                _ST_CHECK_NEXT_SEQ_BYTE();
                _ST_CHECK_NEXT_SEQ_BYTE();
                _ST_CHECK_NEXT_SEQ_BYTE();
            } else {
                // Invalid sequence byte
                return conversion_error_t::invalid_utf8_seq;
            }
        }

        return conversion_error_t::success;
    }

#undef _ST_CHECK_NEXT_SEQ_BYTE

    inline size_t append_chars(char *&output, const char *src, size_t count)
    {
        if (output) {
            std::char_traits<char>::copy(output, src, count);
            output += count;
        }
        return count;
    }

    inline size_t cleanup_utf8(char *output, const char *buffer, size_t size)
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
                if (sp + 2 > ep || (sp[1] & 0xC0) != 0x80) {
                    output_size += append_chars(output, badchar_substitute_utf8,
                                                badchar_substitute_utf8_len);
                    sp += 1;
                } else {
                    output_size += append_chars(output, reinterpret_cast<const char *>(sp), 2);
                    sp += 2;
                }
            } else if ((*sp & 0xF0) == 0xE0) {
                // Three bytes
                if (sp + 3 > ep || (sp[1] & 0xC0) != 0x80 || (sp[2] & 0xC0) != 0x80) {
                    output_size += append_chars(output, badchar_substitute_utf8,
                                                badchar_substitute_utf8_len);
                    sp += 1;
                } else {
                    output_size += append_chars(output, reinterpret_cast<const char *>(sp), 3);
                    sp += 3;
                }
            } else if ((*sp & 0xF8) == 0xF0) {
                // Four bytes
                if (sp + 4 > ep || (sp[1] & 0xC0) != 0x80 || (sp[2] & 0xC0) != 0x80
                                || (sp[3] & 0xC0) != 0x80) {
                    output_size += append_chars(output, badchar_substitute_utf8,
                                                badchar_substitute_utf8_len);
                    sp += 1;
                } else {
                    output_size += append_chars(output, reinterpret_cast<const char *>(sp), 4);
                    sp += 4;
                }
            } else {
                // Invalid sequence byte
                output_size += append_chars(output, badchar_substitute_utf8,
                                            badchar_substitute_utf8_len);
                sp += 1;
            }
        }

        return output_size;
    }

    inline ST::char_buffer cleanup_utf8_buffer(const ST::char_buffer &buffer)
    {
        size_t clean_size = cleanup_utf8(nullptr, buffer.data(), buffer.size());
        ST::char_buffer cb_clean;
        cb_clean.allocate(clean_size);
        cleanup_utf8(cb_clean.data(), buffer.data(), buffer.size());
        return cb_clean;
    }

    inline char32_t error_char(conversion_error_t value)
    {
        return static_cast<char32_t>(value) | 0x400000u;
    }

    inline conversion_error_t char_error(char32_t ch)
    {
        return (ch & 0x400000u) != 0
               ? static_cast<conversion_error_t>(ch & ~0x400000u)
               : conversion_error_t::success;
    }

    inline char32_t extract_utf8(const unsigned char *&utf8, const unsigned char *end)
    {
        char32_t bigch;
        if (*utf8 < 0x80) {
            return *utf8++;
        } else if ((*utf8 & 0xE0) == 0xC0) {
            if (utf8 + 2 > end || (utf8[1] & 0xC0) != 0x80) {
                utf8 += 1;
                return error_char(conversion_error_t::incomplete_utf8_seq);
            }
            bigch  = (*utf8++ & 0x1F) << 6;
            bigch |= (*utf8++ & 0x3F);
            return bigch;
        } else if ((*utf8 & 0xF0) == 0xE0) {
            if (utf8 + 3 > end || (utf8[1] & 0xC0) != 0x80 || (utf8[2] & 0xC0) != 0x80) {
                utf8 += 1;
                return error_char(conversion_error_t::incomplete_utf8_seq);
            }
            bigch  = (*utf8++ & 0x0F) << 12;
            bigch |= (*utf8++ & 0x3F) << 6;
            bigch |= (*utf8++ & 0x3F);
            return bigch;
        } else if ((*utf8 & 0xF8) == 0xF0) {
            if (utf8 + 4 > end || (utf8[1] & 0xC0) != 0x80 || (utf8[2] & 0xC0) != 0x80
                               || (utf8[3] & 0xC0) != 0x80) {
                utf8 += 1;
                return error_char(conversion_error_t::incomplete_utf8_seq);
            }
            bigch  = (*utf8++ & 0x07) << 18;
            bigch |= (*utf8++ & 0x3F) << 12;
            bigch |= (*utf8++ & 0x3F) << 6;
            bigch |= (*utf8++ & 0x3F);
            return bigch;
        }

        utf8 += 1;
        return error_char(conversion_error_t::invalid_utf8_seq);
    }

    inline size_t utf8_measure(char32_t ch)
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
            return badchar_substitute_utf8_len;
        }
    }

    inline conversion_error_t write_utf8(char *&dest, char32_t ch)
    {
        if (ch < 0x80) {
            *dest++ = static_cast<char>(ch);
        } else if (ch < 0x800) {
            *dest++ = 0xC0 | ((ch >>  6) & 0x1F);
            *dest++ = 0x80 | ((ch      ) & 0x3F);
        } else if (ch < 0x10000) {
            *dest++ = 0xE0 | ((ch >> 12) & 0x0F);
            *dest++ = 0x80 | ((ch >>  6) & 0x3F);
            *dest++ = 0x80 | ((ch      ) & 0x3F);
        } else if (ch <= 0x10FFFF) {
            *dest++ = 0xF0 | ((ch >> 18) & 0x07);
            *dest++ = 0x80 | ((ch >> 12) & 0x3F);
            *dest++ = 0x80 | ((ch >>  6) & 0x3F);
            *dest++ = 0x80 | ((ch      ) & 0x3F);
        } else {
            return conversion_error_t::out_of_range;
        }

        return conversion_error_t::success;

    }

    inline char32_t extract_utf16(const char16_t *&utf16, const char16_t *end)
    {
        char32_t bigch;
        if (*utf16 >= 0xD800 && *utf16 <= 0xDFFF) {
            // Surrogate pair
            if (utf16 + 1 >= end) {
                utf16 += 1;
                return error_char(conversion_error_t::incomplete_surrogate_pair);
            } else if (*utf16 < 0xDC00) {
                if (utf16[1] >= 0xDC00 && utf16[1] <= 0xDFFF) {
                    bigch = 0x10000 + ((utf16[0] & 0x3FF) << 10) + (utf16[1] & 0x3FF);
                    utf16 += 2;
                    return bigch;
                }
                utf16 += 1;
                return error_char(conversion_error_t::incomplete_surrogate_pair);
            } else {
                if (utf16[1] >= 0xD800 && utf16[1] <= 0xDBFF) {
                    bigch = 0x10000 + (utf16[0] & 0x3FF) + ((utf16[1] & 0x3FF) << 10);
                    utf16 += 2;
                    return bigch;
                }
                utf16 += 1;
                return error_char(conversion_error_t::incomplete_surrogate_pair);
            }
        }

        return static_cast<char32_t>(*utf16++);
    }

    inline size_t utf16_measure(char32_t ch)
    {
        // Out-of-range code point always gets replaced
        if (ch < 0x10000 || ch > 0x10FFFF)
            return 1;

        // Surrogate pair
        return 2;
    }

    inline conversion_error_t write_utf16(char16_t *&dest, char32_t ch)
    {
        if (ch < 0x10000) {
            *dest++ = static_cast<char16_t>(ch);
        } else if (ch <= 0x10FFFF) {
            ch -= 0x10000;
            *dest++ = 0xD800 | ((ch >> 10) & 0x3FF);
            *dest++ = 0xDC00 | ((ch      ) & 0x3FF);
        } else {
            return conversion_error_t::out_of_range;
        }

        return conversion_error_t::success;
    }

    inline size_t utf8_measure_from_utf16(const char16_t *utf16, size_t size)
    {
        if (!utf16)
            return 0;

        size_t u8len = 0;
        const char16_t *sp = utf16;
        const char16_t *ep = sp + size;
        while (sp < ep)
            u8len += utf8_measure(extract_utf16(sp, ep));
        return u8len;
    }

    inline conversion_error_t utf8_convert_from_utf16(char *dest,
                    const char16_t *utf16, size_t size,
                    ST::utf_validation_t validation)
    {
        const char16_t *sp = utf16;
        const char16_t *ep = sp + size;
        while (sp < ep) {
            char32_t bigch = extract_utf16(sp, ep);

            conversion_error_t error = char_error(bigch);
            if (error != conversion_error_t::success) {
                if (validation == ST::check_validity)
                    return error;
                std::char_traits<char>::copy(dest, badchar_substitute_utf8,
                                             badchar_substitute_utf8_len);
                dest += badchar_substitute_utf8_len;
            } else {
                error = write_utf8(dest, bigch);
                ST_ASSERT(error == conversion_error_t::success, "Input character out of range");
            }
        }

        return conversion_error_t::success;
    }

    inline size_t utf8_measure_from_utf32(const char32_t *utf32, size_t size)
    {
        if (!utf32)
            return 0;

        size_t u8len = 0;
        const char32_t *sp = utf32;
        const char32_t *ep = sp + size;
        while (sp < ep)
            u8len += utf8_measure(*sp++);
        return u8len;
    }

    inline conversion_error_t utf8_convert_from_utf32(char *dest,
                    const char32_t *utf32, size_t size,
                    ST::utf_validation_t validation)
    {
        const char32_t *sp = utf32;
        const char32_t *ep = sp + size;
        while (sp < ep) {
            const conversion_error_t error = write_utf8(dest, *sp++);
            if (error != conversion_error_t::success) {
                if (validation == ST::check_validity)
                    return error;
                std::char_traits<char>::copy(dest, badchar_substitute_utf8,
                                             badchar_substitute_utf8_len);
                dest += badchar_substitute_utf8_len;
            }
        }

        return conversion_error_t::success;
    }

    inline size_t utf8_measure_from_latin_1(const char *astr, size_t size)
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

    inline void utf8_convert_from_latin_1(char *dest, const char *astr, size_t size)
    {
        const char *sp = astr;
        const char *ep = sp + size;
        for (; sp < ep; ++sp) {
            if (*sp & 0x80) {
                *dest++ = 0xC0 | ((static_cast<unsigned char>(*sp) >> 6) & 0x1F);
                *dest++ = 0x80 | ((static_cast<unsigned char>(*sp)     ) & 0x3F);
            } else {
                *dest++ = *sp;
            }
        }
    }

    inline size_t utf16_measure_from_utf8(const char *utf8, size_t size)
    {
        if (!utf8)
            return 0;

        size_t u16len = 0;
        const unsigned char *sp = reinterpret_cast<const unsigned char *>(utf8);
        const unsigned char *ep = sp + size;
        while (sp < ep)
            u16len += utf16_measure(extract_utf8(sp, ep));
        return u16len;
    }

    inline conversion_error_t utf16_convert_from_utf8(char16_t *dest,
                    const char *utf8, size_t size,
                    ST::utf_validation_t validation)
    {
        const unsigned char *sp = reinterpret_cast<const unsigned char *>(utf8);
        const unsigned char *ep = sp + size;
        while (sp < ep) {
            char32_t bigch = extract_utf8(sp, ep);

            conversion_error_t error = char_error(bigch);
            if (error != conversion_error_t::success) {
                if (validation == ST::check_validity)
                    return error;
                *dest++ = badchar_substitute;
            } else {
                error = write_utf16(dest, bigch);
                ST_ASSERT(error == conversion_error_t::success, "Input character out of range");
            }
        }

        return conversion_error_t::success;
    }

    inline size_t utf16_measure_from_utf32(const char32_t *utf32, size_t size)
    {
        if (!utf32)
            return 0;

        size_t u16len = 0;
        const char32_t *sp = utf32;
        const char32_t *ep = sp + size;
        while (sp < ep)
            u16len += utf16_measure(*sp++);
        return u16len;
    }

    inline conversion_error_t utf16_convert_from_utf32(char16_t *dest,
                    const char32_t *utf32, size_t size,
                    ST::utf_validation_t validation)
    {
        const char32_t *sp = utf32;
        const char32_t *ep = sp + size;
        while (sp < ep) {
            const conversion_error_t error = write_utf16(dest, *sp++);
            if (error != conversion_error_t::success) {
                if (validation == ST::check_validity)
                    return error;
                *dest++ = badchar_substitute;
            }
        }

        return conversion_error_t::success;
    }

    inline size_t utf32_measure_from_utf8(const char *utf8, size_t size)
    {
        if (!utf8)
            return 0;

        size_t u32len = 0;
        const unsigned char *sp = reinterpret_cast<const unsigned char *>(utf8);
        const unsigned char *ep = sp + size;
        while (sp < ep) {
            (void)extract_utf8(sp, ep);
            ++u32len;
        }
        return u32len;
    }

    inline conversion_error_t utf32_convert_from_utf8(char32_t *dest,
                    const char *utf8, size_t size,
                    ST::utf_validation_t validation)
    {
        const unsigned char *sp = reinterpret_cast<const unsigned char *>(utf8);
        const unsigned char *ep = sp + size;
        while (sp < ep) {
            char32_t bigch = extract_utf8(sp, ep);

            const conversion_error_t error = char_error(bigch);
            if (error != conversion_error_t::success) {
                if (validation == ST::check_validity)
                    return error;
                bigch = badchar_substitute;
            }

            *dest++ = bigch;
        }

        return conversion_error_t::success;
    }

    inline size_t utf32_measure_from_utf16(const char16_t *utf16, size_t size)
    {
        if (!utf16)
            return 0;

        size_t u32len = 0;
        const char16_t *sp = utf16;
        const char16_t *ep = sp + size;
        while (sp < ep) {
            (void)extract_utf16(sp, ep);
            ++u32len;
        }
        return u32len;
    }

    inline conversion_error_t utf32_convert_from_utf16(char32_t *dest,
                    const char16_t *utf16, size_t size,
                    ST::utf_validation_t validation)
    {
        const char16_t *sp = utf16;
        const char16_t *ep = sp + size;
        while (sp < ep) {
            char32_t bigch = extract_utf16(sp, ep);

            const conversion_error_t error = char_error(bigch);
            if (error != conversion_error_t::success) {
                if (validation == ST::check_validity)
                    return error;
                bigch = badchar_substitute;
            }

            *dest++ = bigch;
        }

        return conversion_error_t::success;
    }

    inline void utf16_convert_from_latin_1(char16_t *dest, const char *astr, size_t size)
    {
        const char *sp = astr;
        const char *ep = sp + size;
        while (sp < ep)
            *dest++ = static_cast<unsigned char>(*sp++);
    }

    inline void utf32_convert_from_latin_1(char32_t *dest, const char *astr, size_t size)
    {
        const char *sp = astr;
        const char *ep = sp + size;
        while (sp < ep)
            *dest++ = static_cast<unsigned char>(*sp++);
    }

    inline size_t latin_1_measure_from_utf8(const char *utf8, size_t size)
    {
        // This always returns the same answer as UTF-32
        return utf32_measure_from_utf8(utf8, size);
    }

    inline conversion_error_t latin_1_convert_from_utf8(char *dest,
                    const char *utf8, size_t size,
                    ST::utf_validation_t validation,
                    bool substitute_out_of_range)
    {
        const unsigned char *sp = reinterpret_cast<const unsigned char *>(utf8);
        const unsigned char *ep = sp + size;
        while (sp < ep) {
            char32_t bigch = extract_utf8(sp, ep);

            const conversion_error_t error = char_error(bigch);
            if (error != conversion_error_t::success) {
                if (validation == ST::check_validity)
                    return error;
                bigch = '?';
            }

            if (bigch >= 0x100) {
                if (substitute_out_of_range)
                    bigch = '?';
                else
                    return conversion_error_t::latin1_out_of_range;
            }
            *dest++ = static_cast<char>(bigch);
        }

        return conversion_error_t::success;
    }

    inline size_t latin_1_measure_from_utf16(const char16_t *utf16, size_t size)
    {
        // This always returns the same answer as UTF-32
        return utf32_measure_from_utf16(utf16, size);
    }

    inline conversion_error_t latin_1_convert_from_utf16(char *dest,
                    const char16_t *utf16, size_t size,
                    ST::utf_validation_t validation,
                    bool substitute_out_of_range)
    {
        const char16_t *sp = utf16;
        const char16_t *ep = sp + size;
        while (sp < ep) {
            char32_t bigch = extract_utf16(sp, ep);

            const conversion_error_t error = char_error(bigch);
            if (error != conversion_error_t::success) {
                if (validation == ST::check_validity)
                    return error;
                bigch = '?';
            }

            if (bigch >= 0x100) {
                if (substitute_out_of_range)
                    bigch = '?';
                else
                    return conversion_error_t::latin1_out_of_range;
            }
            *dest++ = static_cast<char>(bigch);
        }

        return conversion_error_t::success;
    }

    inline conversion_error_t latin_1_convert_from_utf32(char *dest,
                    const char32_t *utf32, size_t size,
                    ST::utf_validation_t validation,
                    bool substitute_out_of_range)
    {
        const char32_t *sp = utf32;
        const char32_t *ep = sp + size;
        while (sp < ep) {
            char32_t bigch = *sp++;

            if (bigch > 0x10FFFF && validation == ST::check_validity)
                return conversion_error_t::out_of_range;

            if (bigch >= 0x100) {
                if (substitute_out_of_range)
                    bigch = '?';
                else
                    return conversion_error_t::latin1_out_of_range;
            }
            *dest++ = static_cast<char>(bigch);
        }

        return conversion_error_t::success;
    }
}

#endif // _ST_UTF_CONV_PRIV_H
