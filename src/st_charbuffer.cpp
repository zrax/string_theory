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

#include "st_charbuffer.h"

static_assert(ST_SHORT_STRING_LEN >= sizeof(void *),
              "ST_SHORT_STRING_LEN must be at least as large as a pointer");
static_assert(ST_STACK_STRING_LEN >= sizeof(void *),
              "ST_STACK_STRING_LEN must be at least as large as a pointer");

/* Ensure these get instantiated by the library */
namespace ST
{
    template ST_EXPORT class buffer<char>;
    template ST_EXPORT class buffer<wchar_t>;
    template ST_EXPORT class buffer<char16_t>;
    template ST_EXPORT class buffer<char32_t>;
}
