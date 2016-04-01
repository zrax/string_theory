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
    struct ST_EXPORT format_data_object
    {
        const char *m_format_str;
        ST::string_stream m_output;
        ST::utf_validation_t m_validation;
        bool m_is_utf8;
    };

    ST_EXPORT ST::format_spec fetch_next_format(format_data_object &data);

    ST_EXPORT ST::string format(format_data_object &data);

    template <typename type_T, typename... args_T>
    ST_EXPORT ST::string format(format_data_object &data, type_T value, args_T ...args)
    {
        ST::format_spec format = fetch_next_format(data);
        _ST_impl_format_data_handler(format, data.m_output, value);
        return _ST_PRIVATE::format(data, args...);
    }
}

namespace ST
{
    template <typename type_T, typename... args_T>
    ST_EXPORT string format(const char *fmt_str, type_T value, args_T ...args)
    {
        _ST_PRIVATE::format_data_object data;
        data.m_format_str = fmt_str;
        data.m_is_utf8 = true;
        data.m_validation = ST_DEFAULT_VALIDATION;
        ST::format_spec format = _ST_PRIVATE::fetch_next_format(data);
        _ST_impl_format_data_handler(format, data.m_output, value);
        return _ST_PRIVATE::format(data, args...);
    }

    template <typename type_T, typename... args_T>
    ST_EXPORT string format(utf_validation_t validation, const char *fmt_str,
                            type_T value, args_T ...args)
    {
        _ST_PRIVATE::format_data_object data;
        data.m_format_str = fmt_str;
        data.m_is_utf8 = true;
        data.m_validation = validation;
        ST::format_spec format = _ST_PRIVATE::fetch_next_format(data);
        _ST_impl_format_data_handler(format, data.m_output, value);
        return _ST_PRIVATE::format(data, args...);
    }

    template <typename type_T, typename... args_T>
    ST_EXPORT string format_latin_1(const char *fmt_str, type_T value, args_T ...args)
    {
        _ST_PRIVATE::format_data_object data;
        data.m_format_str = fmt_str;
        data.m_is_utf8 = false;
        data.m_validation = assume_valid;
        ST::format_spec format = _ST_PRIVATE::fetch_next_format(data);
        _ST_impl_format_data_handler(format, data.m_output, value);
        return _ST_PRIVATE::format(data, args...);
    }
}

#endif // _ST_FORMAT_H
