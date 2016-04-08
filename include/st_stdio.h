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

#ifndef _ST_STDIO_H
#define _ST_STDIO_H

#include "st_formatter.h"
#include <cstdio>

namespace _ST_PRIVATE
{
    class ST_EXPORT stdio_format_writer : public ST::format_writer
    {
    public:
        stdio_format_writer(const char *format_str, FILE *stream)
            : ST::format_writer(format_str), m_stream(stream) { }

        stdio_format_writer &append(const char *data, size_t size = ST_AUTO_SIZE) ST_OVERRIDE
        {
            (void)fwrite(data, sizeof(char), size, m_stream);
            return *this;
        }

        stdio_format_writer &append_char(char ch, size_t count = 1) ST_OVERRIDE
        {
            while (count) {
                fputc(ch, m_stream);
                --count;
            }
            return *this;
        }

    private:
        ST::string_stream m_output;
        FILE *m_stream;
    };

    ST_EXPORT void printf(stdio_format_writer &data)
    {
        data.finalize();
    }

    template <typename type_T, typename... args_T>
    ST_EXPORT void printf(stdio_format_writer &data, type_T value, args_T ...args)
    {
        ST::format_spec format = data.fetch_next_format();
        _ST_impl_format_data_handler(format, data, value);
        _ST_PRIVATE::printf(data, args...);
    }
}

namespace ST
{
    template <typename type_T, typename... args_T>
    ST_EXPORT void printf(const char *fmt_str, type_T value, args_T ...args)
    {
        _ST_PRIVATE::stdio_format_writer data(fmt_str, stdout);
        ST::format_spec format = data.fetch_next_format();
        _ST_impl_format_data_handler(format, data, value);
        _ST_PRIVATE::printf(data, args...);
    }

    template <typename type_T, typename... args_T>
    ST_EXPORT void printf(FILE *out_file, const char *fmt_str,
                          type_T value, args_T ...args)
    {
        _ST_PRIVATE::stdio_format_writer data(fmt_str, out_file);
        ST::format_spec format = data.fetch_next_format();
        _ST_impl_format_data_handler(format, data, value);
        _ST_PRIVATE::printf(data, args...);
    }
}

#endif // _ST_STDIO_H
