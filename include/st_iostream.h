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

#include "st_string.h"
#include "st_formatter_util.h"

#include <ostream>
#include <istream>

namespace _ST_PRIVATE
{
    template <class char_T, class traits_T>
    class ostream_format_writer : public ST::format_writer
    {
    public:
        ostream_format_writer(const char *format_str,
                              std::basic_ostream<char_T, traits_T> &stream)
            : ST::format_writer(format_str), m_stream(stream) { }

        template <class write_char_T>
        typename std::enable_if<std::is_same<write_char_T, char>::value, void>::type
        write_data(const char *data, size_t size)
        {
            m_stream.write(data, size);
        }

        template <class write_char_T>
        typename std::enable_if<std::is_same<write_char_T, wchar_t>::value, void>::type
        write_data(const char *data, size_t size)
        {
            // TODO: There may be a more efficient way to do this...
            ST::wchar_buffer wide = ST::string(data, size).to_wchar();
            m_stream.write(wide.data(), wide.size());
        }

        template <class write_char_T>
        typename std::enable_if<std::is_same<write_char_T, char16_t>::value, void>::type
        write_data(const char *data, size_t size)
        {
            ST::utf16_buffer utf16 = ST::string(data, size).to_utf16();
            m_stream.write(utf16.data(), utf16.size());
        }

        template <class write_char_T>
        typename std::enable_if<std::is_same<write_char_T, char32_t>::value, void>::type
        write_data(const char *data, size_t size)
        {
            ST::utf32_buffer utf32 = ST::string(data, size).to_utf32();
            m_stream.write(utf32.data(), utf32.size());
        }

        ostream_format_writer &append(const char *data, size_t size = ST_AUTO_SIZE) override
        {
            write_data<char_T>(data, size);
            return *this;
        }

        ostream_format_writer &append_char(char ch, size_t count = 1) override
        {
            while (count) {
                m_stream.put(char_T(ch));
                --count;
            }
            return *this;
        }

    private:
        std::basic_ostream<char_T, traits_T> &m_stream;
    };
}

namespace ST
{
    template <class char_T, class traits_T, typename... args_T>
    void writef(std::basic_ostream<char_T, traits_T> &stream,
                const char *fmt_str, args_T &&...args)
    {
        _ST_PRIVATE::ostream_format_writer<char_T, traits_T> data(fmt_str, stream);
        apply_format(data, std::forward<args_T>(args)...);
    }
}

template <class char_T, class traits_T>
std::basic_ostream<char_T, traits_T> &operator<<(
        std::basic_ostream<char_T, traits_T> &stream, const ST::string &str)
{
    std::basic_string<char_T, traits_T> stl_string;
    str.to_std_string(stl_string);
    return stream << stl_string;
}

template <class char_T, class traits_T>
std::basic_istream<char_T, traits_T> &operator>>(
        std::basic_istream<char_T, traits_T> &stream, ST::string &str)
{
    std::basic_string<char_T, traits_T> stl_string;
    stream >> stl_string;
    str = ST::string::from_std_string(stl_string);
    return stream;
}

#endif // _ST_IOSTREAM_H
