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

#ifndef _ST_CODECS_PRIV_H
#define _ST_CODECS_PRIV_H

namespace _ST_PRIVATE
{
    inline void hex_encode(char *output, const void *data, size_t size) noexcept
    {
        static constexpr const char hex_chars[] = "0123456789abcdef";
        static_assert(sizeof(hex_chars) - 1 == 16, "Missing hex characters");

        auto sp = static_cast<const unsigned char *>(data);
        while (size) {
            unsigned char byte = *sp++;
            *output++ = hex_chars[(byte >> 4) & 0x0F];
            *output++ = hex_chars[(byte     ) & 0x0F];
            --size;
        }
    }

    inline ST_ssize_t hex_decode(const ST::string &hex, void *output,
                                 size_t output_size) noexcept
    {
        static constexpr const int hex_values[] = {
            /* 00 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            /* 10 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            /* 20 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            /* 30 */  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
            /* 40 */ -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            /* 50 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            /* 60 */ -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            /* 70 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            /* 80 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            /* 90 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            /* A0 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            /* B0 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            /* C0 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            /* D0 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            /* E0 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            /* F0 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        };
        static_assert(sizeof(hex_values) / sizeof(int) == 0x100, "Missing hex values");

        if ((hex.size() % 2) != 0)
            return -1;

        size_t decode_size = hex.size() / 2;
        if (!output)
            return decode_size;

        if (static_cast<size_t>(decode_size) > output_size)
            return -1;

        char *outp = reinterpret_cast<char *>(output);
        char *endp = outp + decode_size;
        auto sp = reinterpret_cast<const unsigned char *>(hex.c_str());

        while (outp < endp) {
            int bits[2] = { hex_values[sp[0]], hex_values[sp[1]] };
            if (bits[0] < 0 || bits[1] < 0)
                return -1;

            *outp++ = (bits[0] << 4 | bits[1]);
            sp += 2;
        }

        return outp - reinterpret_cast<char *>(output);
    }

    inline size_t b64_encode_size(size_t size)
    {
        return ((size + 2) / 3) * 4;
    }

    inline ST_ssize_t b64_decode_size(size_t size, const char *data)
    {
        if ((size % 4) != 0)
            return -1;

        size_t result = (size / 4) * 3;
        if (size > 0 && data[size - 1] == '=')
            result -= 1;
        if (size > 1 && data[size - 2] == '=')
            result -= 1;

        return static_cast<ST_ssize_t>(result);
    }

    inline void b64_encode(char *output, const void *data, size_t size) noexcept
    {
        static constexpr const char b64_chars[] =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        static_assert(sizeof(b64_chars) - 1 == 64, "Missing base64 characters");

        auto sp = static_cast<const unsigned char *>(data);
        while (size > 2) {
            *output++ = b64_chars[sp[0] >> 2];
            *output++ = b64_chars[((sp[0] & 0x03) << 4) | ((sp[1] & 0xF0) >> 4)];
            *output++ = b64_chars[((sp[1] & 0x0F) << 2) | ((sp[2] & 0xC0) >> 6)];
            *output++ = b64_chars[sp[2] & 0x3F];
            size -= 3;
            sp += 3;
        }

        // Final bytes treated specially
        switch (size) {
        case 2:
            *output++ = b64_chars[sp[0] >> 2];
            *output++ = b64_chars[((sp[0] & 0x03) << 4) | ((sp[1] & 0xF0) >> 4)];
            *output++ = b64_chars[((sp[1] & 0x0F) << 2)];
            *output++ = '=';
            break;
        case 1:
            *output++ = b64_chars[sp[0] >> 2];
            *output++ = b64_chars[((sp[0] & 0x03) << 4)];
            *output++ = '=';
            *output++ = '=';
            break;
        case 0:
            break;
        default:
            ST_ASSERT(false, "Unexpected bytes left after encoding loop");
            break;
        }
    }

    inline ST_ssize_t b64_decode(const ST::string &base64, void *output,
                                 size_t output_size) noexcept
    {
        static constexpr const int b64_values[] = {
            /* 00 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            /* 10 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            /* 20 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
            /* 30 */ 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
            /* 40 */ -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
            /* 50 */ 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
            /* 60 */ -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
            /* 70 */ 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
            /* 80 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            /* 90 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            /* A0 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            /* B0 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            /* C0 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            /* D0 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            /* E0 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            /* F0 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        };
        static_assert(sizeof(b64_values) / sizeof(int) == 0x100, "Missing base64 values");

        ST_ssize_t decode_size = b64_decode_size(base64.size(), base64.c_str());
        if (!output)
            return decode_size;

        if (decode_size < 0 || static_cast<size_t>(decode_size) > output_size)
            return -1;

        if (decode_size == 0)
            return 0;

        char *outp = reinterpret_cast<char *>(output);
        char *endp = outp + decode_size;
        auto sp = reinterpret_cast<const unsigned char *>(base64.c_str());

        while (outp + 3 < endp) {
            int bits[4] = {
                b64_values[sp[0]], b64_values[sp[1]],
                b64_values[sp[2]], b64_values[sp[3]]
            };
            if (bits[0] < 0 || bits[1] < 0 || bits[2] < 0 || bits[3] < 0)
                return -1;

            *outp++ = (bits[0] << 2) | ((bits[1] >> 4) & 0x03);
            *outp++ = ((bits[1] << 4) & 0xF0) | ((bits[2] >> 2) & 0x0F);
            *outp++ = ((bits[2] << 6) & 0xC0) | (bits[3] & 0x3F);

            sp += 4;
        }

        // Final chars treated specially
        int bits[4] = {
            b64_values[sp[0]], b64_values[sp[1]],
            b64_values[sp[2]], b64_values[sp[3]]
        };

        if (bits[0] < 0 || bits[1] < 0)
            return -1;

        *outp++ = (bits[0] << 2) | ((bits[1] >> 4) & 0x03);
        if (sp[2] != '=') {
            if (bits[2] < 0)
                return -1;
            *outp++ = ((bits[1] << 4) & 0xF0) | ((bits[2] >> 2) & 0x0F);
        }
        if (sp[3] != '=') {
            if (bits[2] < 0 || bits[3] < 0)
                return -1;
            *outp++ = ((bits[2] << 6) & 0xC0) | (bits[3] & 0x3F);
        }

        return outp - reinterpret_cast<char *>(output);
    }
}

#endif // _ST_CODECS_PRIV_H
