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

#include "st_string.h"
#include "st_assert.h"

#include <gtest/gtest.h>
#include <cstring>
#include <iostream>

#ifdef ST_HAVE_RVALUE_MOVE
TEST(regress, github_5)
{
    static const char latin1_data[] = "F\xfcr Elise";
    static const char utf8_data[] = "F\xc3\xbcr Elise";
    ST::string s;
    ST::char_buffer init(latin1_data, strlen(latin1_data));
    try {
        // This should throw
        s = std::move(init);
        ASSERT_FALSE(true);
    } catch (ST::unicode_error &) {
        s = ST::string::from_latin_1(init);
    }

    EXPECT_STREQ(utf8_data, s.c_str());
}
#endif
