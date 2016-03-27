/*  This file is part of string_theory.

    string_theory is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    string_theory is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with string_theory.  If not, see <http://www.gnu.org/licenses/>.  */

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
        data.m_validation = assert_validity;
        ST::format_spec format = _ST_PRIVATE::fetch_next_format(data);
        _ST_impl_format_data_handler(format, data.m_output, value);
        return _ST_PRIVATE::format(data, args...);
    }

    template <typename type_T, typename... args_T>
    ST_EXPORT string format(const char *fmt_str, type_T value, args_T ...args,
                            utf_validation_t validation)
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
