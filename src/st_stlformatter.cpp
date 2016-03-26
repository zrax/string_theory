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

#include "st_stlformatter.h"

ST_FORMAT_TYPE(const std::string &)
{
    ST::format_string(format, output, value.c_str(), value.size(), ST::align_left);
}

ST_FORMAT_TYPE(const std::wstring &)
{
    ST::char_buffer utf8 = ST::string::from_wchar(value.c_str(), value.size()).to_utf8();
    ST::format_string(format, output, utf8.data(), utf8.size(), ST::align_left);
}
