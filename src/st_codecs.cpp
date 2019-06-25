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

#include "st_codecs.h"

#include "st_assert.h"

static const char _hex_chars[] = "0123456789abcdef";
static_assert(sizeof(_hex_chars) - 1 == 16, "Missing hex characters");

static const int _hex_values[] = {
    /* 00 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* 10 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* 20 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* 30 */  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
    /* 40 */ -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* 50 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* 60 */ -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* 70 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};
static_assert(sizeof(_hex_values) / sizeof(int) == 0x80, "Missing hex values");

static const char _b64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static_assert(sizeof(_b64_chars) - 1 == 64, "Missing base64 characters");

static const int _b64_values[] = {
    /* 00 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* 10 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    /* 20 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
    /* 30 */ 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
    /* 40 */ -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    /* 50 */ 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
    /* 60 */ -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    /* 70 */ 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
};
static_assert(sizeof(_b64_values) / sizeof(int) == 0x80, "Missing base64 values");

ST::string ST::hex_encode(const void *data, size_t size)
{
    if (size == 0)
        return ST::null;

    ST_ASSERT(data, "null data pointer passed to hex_encode");
    if (!data)
        return ST::null;

    size_t encoded_size = size * 2;
    ST::char_buffer buffer;
    buffer.allocate(encoded_size);
    char *output = buffer.data();
    const unsigned char *sp = static_cast<const unsigned char *>(data);
    while (size) {
        unsigned char byte = *sp++;
        *output++ = _hex_chars[(byte >> 4) & 0x0F];
        *output++ = _hex_chars[(byte     ) & 0x0F];
        --size;
    }

    return ST::string::from_validated(std::move(buffer));
}

ST::char_buffer ST::hex_decode(const ST::string &hex)
{
    if ((hex.size() % 2) != 0)
        throw codec_error("Invalid hex input length");

    size_t decode_size = hex.size() / 2;
    ST::char_buffer result;
    result.allocate(decode_size);
    ST_ssize_t written = hex_decode(hex, result.data(), decode_size);
    if (written < 0)
        throw codec_error("Invalid character in hex input");

    ST_ASSERT(static_cast<size_t>(written) == decode_size,
              "Conversion didn't match expected length");
    return result;
}

ST_ssize_t ST::hex_decode(const ST::string &hex, void *output, size_t output_size)
    noexcept
{
    if ((hex.size() % 2) != 0)
        return -1;

    size_t decode_size = hex.size() / 2;
    if (!output)
        return decode_size;

    if (static_cast<size_t>(decode_size) > output_size)
        return -1;

    char *outp = reinterpret_cast<char *>(output);
    char *endp = outp + decode_size;
    const unsigned char *sp = reinterpret_cast<const unsigned char *>(hex.c_str());

    while (outp < endp) {
        int bits[2] = {
            (sp[0] < 0x80) ? _hex_values[sp[0]] : -1,
            (sp[1] < 0x80) ? _hex_values[sp[1]] : -1
        };
        if (bits[0] < 0 || bits[1] < 0)
            return -1;

        *outp++ = (bits[0] << 4 | bits[1]);
        sp += 2;
    }

    return outp - reinterpret_cast<char *>(output);
}

static size_t _base64_encode_size(size_t size)
{
    return ((size + 2) / 3) * 4;
}

ST::string ST::base64_encode(const void *data, size_t size)
{
    if (size == 0)
        return ST::null;

    ST_ASSERT(data, "null data pointer passed to base64_encode");
    if (!data)
        return ST::null;

    ST::char_buffer buffer;
    buffer.allocate(_base64_encode_size(size));
    char *output = buffer.data();

    const unsigned char *sp = static_cast<const unsigned char *>(data);
    while (size > 2) {
        *output++ = _b64_chars[sp[0] >> 2];
        *output++ = _b64_chars[((sp[0] & 0x03) << 4) | ((sp[1] & 0xF0) >> 4)];
        *output++ = _b64_chars[((sp[1] & 0x0F) << 2) | ((sp[2] & 0xC0) >> 6)];
        *output++ = _b64_chars[sp[2] & 0x3F];
        size -= 3;
        sp += 3;
    }

    // Final bytes treated specially
    switch (size) {
    case 2:
        *output++ = _b64_chars[sp[0] >> 2];
        *output++ = _b64_chars[((sp[0] & 0x03) << 4) | ((sp[1] & 0xF0) >> 4)];
        *output++ = _b64_chars[((sp[1] & 0x0F) << 2)];
        *output++ = '=';
        break;
    case 1:
        *output++ = _b64_chars[sp[0] >> 2];
        *output++ = _b64_chars[((sp[0] & 0x03) << 4)];
        *output++ = '=';
        *output++ = '=';
        break;
    case 0:
        break;
    default:
        ST_ASSERT(false, "Unexpected bytes left after encoding loop");
        break;
    }

    return ST::string::from_validated(std::move(buffer));
}

static ST_ssize_t _base64_decode_size(size_t size, const char *data)
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

ST::char_buffer ST::base64_decode(const ST::string &base64)
{
    ST_ssize_t decode_size = _base64_decode_size(base64.size(), base64.c_str());
    if (decode_size < 0)
        throw codec_error("Invalid base64 input length");

    ST::char_buffer result;
    result.allocate(decode_size);
    ST_ssize_t written = base64_decode(base64, result.data(), decode_size);
    if (written < 0)
        throw codec_error("Invalid character in base64 input");

    ST_ASSERT(written == decode_size, "Conversion didn't match expected length");
    return result;
}

ST_ssize_t ST::base64_decode(const ST::string &base64, void *output, size_t output_size)
    noexcept
{
    ST_ssize_t decode_size = _base64_decode_size(base64.size(), base64.c_str());
    if (!output)
        return decode_size;

    if (decode_size < 0 || static_cast<size_t>(decode_size) > output_size)
        return -1;

    if (decode_size == 0)
        return 0;

    char *outp = reinterpret_cast<char *>(output);
    char *endp = outp + decode_size;
    const unsigned char *sp = reinterpret_cast<const unsigned char *>(base64.c_str());

    while (outp + 3 < endp) {
        int bits[4] = {
            (sp[0] < 0x80) ? _b64_values[sp[0]] : -1,
            (sp[1] < 0x80) ? _b64_values[sp[1]] : -1,
            (sp[2] < 0x80) ? _b64_values[sp[2]] : -1,
            (sp[3] < 0x80) ? _b64_values[sp[3]] : -1
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
        (sp[0] < 0x80) ? _b64_values[sp[0]] : -1,
        (sp[1] < 0x80) ? _b64_values[sp[1]] : -1,
        (sp[2] < 0x80) ? _b64_values[sp[2]] : -1,
        (sp[3] < 0x80) ? _b64_values[sp[3]] : -1
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
