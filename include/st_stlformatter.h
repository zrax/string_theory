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

#ifndef _ST_STLFORMATTER_H
#define _ST_STLFORMATTER_H

#include "st_formatter.h"
#include <string>
#include <complex>

ST_EXPORT ST_DECL_FORMAT_TYPE(const std::string &);
ST_EXPORT ST_DECL_FORMAT_TYPE(const std::wstring &);

template <typename value_T>
ST_EXPORT ST_FORMAT_TYPE(const std::complex<value_T> &)
{
    ST_FORMAT_FORWARD(value.real());
    output.append_char('+');
    ST_FORMAT_FORWARD(value.imag());
    output.append_char('i');
}

#endif // _ST_STLFORMATTER_H
