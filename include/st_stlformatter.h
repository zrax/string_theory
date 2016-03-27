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

#ifndef _ST_STLFORMATTER_H
#define _ST_STLFORMATTER_H

#include "st_formatter.h"
#include <string>
#include <complex>

ST_DECL_FORMAT_TYPE_EXPORT(const std::string &);
ST_DECL_FORMAT_TYPE_EXPORT(const std::wstring &);

template <typename value_T>
ST_EXPORT ST_FORMAT_TYPE(const std::complex<value_T> &)
{
    ST_FORMAT_FORWARD(value.real());
    output.append_char('+');
    ST_FORMAT_FORWARD(value.imag());
    output.append_char('i');
}

#endif // _ST_STLFORMATTER_H
