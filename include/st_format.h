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

#include "st_formatter.h"

namespace _ST_PRIVATE
{
    class ST_EXPORT string_format_writer : public ST::format_writer
    {
    public:
        string_format_writer(const char *format_str, bool is_utf8,
                             ST::utf_validation_t validation)
            : ST::format_writer(format_str), m_validation(validation),
              m_is_utf8(is_utf8) { }

        string_format_writer &append(const char *data, size_t size = ST_AUTO_SIZE) ST_OVERRIDE
        {
            m_output.append(data, size);
            return *this;
        }

        string_format_writer &append_char(char ch, size_t count = 1) ST_OVERRIDE
        {
            m_output.append_char(ch, count);
            return *this;
        }

        ST::string to_string()
        {
            return m_output.to_string(m_is_utf8, m_validation);
        }

    private:
        ST::string_stream m_output;
        ST::utf_validation_t m_validation;
        bool m_is_utf8;
    };

    inline ST::string format(string_format_writer &data)
    {
        data.finalize();
        return data.to_string();
    }

    template <typename type_T, typename... args_T>
    ST::string format(string_format_writer &data, type_T value, args_T ...args)
    {
        ST::format_spec format = data.fetch_next_format();
        _ST_impl_format_data_handler(format, data, value);
        return _ST_PRIVATE::format(data, args...);
    }
}

namespace ST
{
    template <typename type_T, typename... args_T>
    string format(const char *fmt_str, type_T value, args_T ...args)
    {
        _ST_PRIVATE::string_format_writer data(fmt_str, true, ST_DEFAULT_VALIDATION);
        ST::format_spec format = data.fetch_next_format();
        _ST_impl_format_data_handler(format, data, value);
        return _ST_PRIVATE::format(data, args...);
    }

    template <typename type_T, typename... args_T>
    string format(utf_validation_t validation, const char *fmt_str,
                  type_T value, args_T ...args)
    {
        _ST_PRIVATE::string_format_writer data(fmt_str, true, validation);
        ST::format_spec format = data.fetch_next_format();
        _ST_impl_format_data_handler(format, data, value);
        return _ST_PRIVATE::format(data, args...);
    }

    template <typename type_T, typename... args_T>
    string format_latin_1(const char *fmt_str, type_T value, args_T ...args)
    {
        _ST_PRIVATE::string_format_writer data(fmt_str, false, assume_valid);
        ST::format_spec format = data.fetch_next_format();
        _ST_impl_format_data_handler(format, data, value);
        return _ST_PRIVATE::format(data, args...);
    }
}

#endif // _ST_FORMAT_H
