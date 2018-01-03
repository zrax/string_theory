/*  Copyright (c) 2018 Michael Hansen

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

#ifndef _ST_IOSTREAM_H
#define _ST_IOSTREAM_H

#include "st_formatter_util.h"

#include <ostream>

namespace _ST_PRIVATE
{
    class ST_EXPORT ostream_format_writer : public ST::format_writer
    {
    public:
        ostream_format_writer(const char *format_str, std::ostream &stream) ST_NOEXCEPT
            : ST::format_writer(format_str), m_stream(stream) { }

        ostream_format_writer &append(const char *data, size_t size = ST_AUTO_SIZE) ST_OVERRIDE
        {
            m_stream.write(data, size);
            return *this;
        }

        ostream_format_writer &append_char(char ch, size_t count = 1) ST_OVERRIDE
        {
            while (count) {
                m_stream.put(ch);
                --count;
            }
            return *this;
        }

    private:
        std::ostream &m_stream;
    };

    class ST_EXPORT wostream_format_writer : public ST::format_writer
    {
    public:
        wostream_format_writer(const char *format_str, std::wostream &stream) ST_NOEXCEPT
            : ST::format_writer(format_str), m_stream(stream) { }

        wostream_format_writer &append(const char *data, size_t size = ST_AUTO_SIZE) ST_OVERRIDE
        {
            // TODO: This is probably not very efficient...
            ST::wchar_buffer wide = ST::string(data, size).to_wchar();
            m_stream.write(wide.data(), size);
            return *this;
        }

        wostream_format_writer &append_char(char ch, size_t count = 1) ST_OVERRIDE
        {
            while (count) {
                m_stream.put(wchar_t(ch));
                --count;
            }
            return *this;
        }

    private:
        std::wostream &m_stream;
    };
}

namespace ST
{
    template <typename... args_T>
    void writef(std::ostream &stream, const char *fmt_str,
                args_T ...args)
    {
        _ST_PRIVATE::ostream_format_writer data(fmt_str, stream);
        _ST_PRIVATE::apply_format(data, args...);
    }

    template <typename... args_T>
    void writef(std::wostream &stream, const char *fmt_str,
                args_T ...args)
    {
        _ST_PRIVATE::wostream_format_writer data(fmt_str, stream);
        _ST_PRIVATE::apply_format(data, args...);
    }
}

#endif // _ST_IOSTREAM_H
