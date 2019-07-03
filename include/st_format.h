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

#ifndef _ST_FORMAT_H
#define _ST_FORMAT_H

#include "st_formatter_util.h"
#include "st_stringstream.h"

namespace _ST_PRIVATE
{
    class string_format_writer : public ST::format_writer
    {
    public:
        string_format_writer(const char *format_str)
            : ST::format_writer(format_str) { }

        string_format_writer &append(const char *data, size_t size = ST_AUTO_SIZE) override
        {
            m_output.append(data, size);
            return *this;
        }

        string_format_writer &append_char(char ch, size_t count = 1) override
        {
            m_output.append_char(ch, count);
            return *this;
        }

        ST::string to_string(bool utf8_encoded, ST::utf_validation_t validation)
        {
            return m_output.to_string(utf8_encoded, validation);
        }

    private:
        ST::string_stream m_output;
    };
}

namespace ST
{
    template <typename... args_T>
    string format(const char *fmt_str, args_T &&...args)
    {
        _ST_PRIVATE::string_format_writer data(fmt_str);
        apply_format(data, std::forward<args_T>(args)...);
        return data.to_string(true, ST_DEFAULT_VALIDATION);
    }

    template <typename... args_T>
    string format(utf_validation_t validation, const char *fmt_str,
                  args_T &&...args)
    {
        _ST_PRIVATE::string_format_writer data(fmt_str);
        apply_format(data, std::forward<args_T>(args)...);
        return data.to_string(true, validation);
    }

    template <typename... args_T>
    string format_latin_1(const char *fmt_str, args_T &&...args)
    {
        _ST_PRIVATE::string_format_writer data(fmt_str);
        apply_format(data, std::forward<args_T>(args)...);
        return data.to_string(false, assume_valid);
    }
}

namespace _ST_PRIVATE
{
    class udl_formatter
    {
    public:
        explicit udl_formatter(const char *fmt_str)
            : m_format(fmt_str) { }

        template <typename... args_T>
        ST::string operator()(args_T &&...args)
        {
            return ST::format(m_format, std::forward<args_T>(args)...);
        }

    private:
        const char *m_format;
    };
}

namespace ST { namespace literals
{
    _ST_PRIVATE::udl_formatter operator"" _sfmt(const char *fmt_str, size_t)
    {
        return _ST_PRIVATE::udl_formatter(fmt_str);
    }
}}

#endif // _ST_FORMAT_H
