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

#include "st_string_darwin.h"

#include <gtest/gtest.h>

TEST(string, CFString_compatibility)
{
    CFStringRef cfStr = CFSTR("Hello World");
    ST::string stStr(cfStr);
    EXPECT_EQ(stStr, ST_LITERAL("Hello World"));

    ST::string s = ST::string::from_CFString(CFSTR("Testing"));
    ST::string s2 = ST_LITERAL("Testing");
    CFStringRef s_cf = s2.to_CFString();
    //EXPECT_EQ(1, CFGetRetainCount(s_cf));
    EXPECT_EQ(kCFCompareEqualTo, CFStringCompare(s_cf, CFSTR("Testing"), 0));
    CFRelease(s_cf);

    s = cfStr;
    EXPECT_EQ(s, stStr);
}
