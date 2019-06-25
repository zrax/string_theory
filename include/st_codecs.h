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

#include <cstddef>

namespace ST
{
    ST_EXPORT string hex_encode(const void *data, size_t size);

    inline string hex_encode(const char_buffer &data)
    {
        return hex_encode(data.data(), data.size());
    }

    ST_EXPORT char_buffer hex_decode(const string &hex);
    ST_EXPORT ST_ssize_t hex_decode(const string &hex, void *output,
                                    size_t output_size) noexcept;

    ST_EXPORT string base64_encode(const void *data, size_t size);

    inline string base64_encode(const char_buffer &data)
    {
        return base64_encode(data.data(), data.size());
    }

    ST_EXPORT char_buffer base64_decode(const string &base64);
    ST_EXPORT ST_ssize_t base64_decode(const string &base64, void *output,
                                       size_t output_size) noexcept;
}

#endif // _ST_CODECS_H
