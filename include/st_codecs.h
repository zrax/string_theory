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

#ifndef _ST_CODECS_H
#define _ST_CODECS_H

#include "st_string.h"

#include "st_codecs_priv.h"

namespace ST
{
    inline string hex_encode(const void *data, size_t size)
    {
        if (size == 0)
            return ST::string();

        if (!data)
            throw std::invalid_argument("null data pointer passed to hex_encode");

        ST::char_buffer buffer;
        buffer.allocate(size * 2);
        _ST_PRIVATE::hex_encode(buffer.data(), data, size);
        return ST::string::from_validated(std::move(buffer));
    }

    inline string hex_encode(const char_buffer &data)
    {
        return hex_encode(data.data(), data.size());
    }

    inline ST_ssize_t hex_decode(const string &hex, void *output,
                                 size_t output_size) noexcept
    {
        return _ST_PRIVATE::hex_decode(hex, output, output_size);
    }

    inline char_buffer hex_decode(const string &hex)
    {
        if ((hex.size() % 2) != 0)
            throw codec_error("Invalid hex input length");

        size_t decode_size = hex.size() / 2;
        ST::char_buffer result;
        result.allocate(decode_size);
        ST_ssize_t written = _ST_PRIVATE::hex_decode(hex, result.data(), decode_size);
        if (written < 0)
            throw codec_error("Invalid character in hex input");

        ST_ASSERT(static_cast<size_t>(written) == decode_size,
                  "Conversion didn't match expected length");
        return result;
    }

    inline string base64_encode(const void *data, size_t size)
    {
        if (size == 0)
            return ST::string();

        if (!data)
            throw std::invalid_argument("null data pointer passed to base64_encode");

        ST::char_buffer buffer;
        buffer.allocate(_ST_PRIVATE::b64_encode_size(size));
        _ST_PRIVATE::b64_encode(buffer.data(), data, size);
        return ST::string::from_validated(std::move(buffer));
    }

    inline string base64_encode(const char_buffer &data)
    {
        return base64_encode(data.data(), data.size());
    }

    inline ST_ssize_t base64_decode(const string &base64, void *output,
                                    size_t output_size) noexcept
    {
        return _ST_PRIVATE::b64_decode(base64, output, output_size);
    }

    inline char_buffer base64_decode(const string &base64)
    {
        ST_ssize_t decode_size = _ST_PRIVATE::b64_decode_size(base64.size(), base64.c_str());
        if (decode_size < 0)
            throw codec_error("Invalid base64 input length");

        ST::char_buffer result;
        result.allocate(decode_size);
        ST_ssize_t written = _ST_PRIVATE::b64_decode(base64, result.data(), decode_size);
        if (written < 0)
            throw codec_error("Invalid character in base64 input");

        ST_ASSERT(written == decode_size, "Conversion didn't match expected length");
        return result;
    }
}

#endif // _ST_CODECS_H
