/*  Copyright (c) 2018 Michael Hansen

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

#include "st_iostream.h"

#include <gtest/gtest.h>
#include <sstream>

/* This file does not extensively test formatting, as that is already tested
   by test_format.cpp.  Instead, this just tests the interfaces provided
   in <string_theory/iostream>. */

#define BUFFER_LEN (256)

#ifdef ST_HAVE_FORMAT
TEST(stdio, writef)
{
    std::stringstream sstream;
    ST::writef(sstream, "xxxxx");
    ST::writef(sstream, "Testing {>12_#} output", "formatted");

    EXPECT_EQ(sstream.str(), "xxxxxTesting ###formatted output");
}

TEST(stdio, writef_wide)
{
    std::wstringstream sstream;
    ST::writef(sstream, "xxxxx");
    ST::writef(sstream, "Testing {>12_#} output", "formatted");

    EXPECT_EQ(sstream.str(), L"xxxxxTesting ###formatted output");
}
#endif // ST_HAVE_FORMAT

TEST(stdio, stream_ops)
{
    std::stringstream sstream;
    sstream << ST_LITERAL("xxxxx") << " " << ST_LITERAL("yyyyy");

    sstream.seekg(0);
    ST::string x, y;
    sstream >> x;
    sstream >> y;

    EXPECT_EQ(ST_LITERAL("xxxxx"), x);
    EXPECT_EQ(ST_LITERAL("yyyyy"), y);
}

TEST(stdio, stream_ops_wide)
{
    std::wstringstream sstream;
    sstream << ST_LITERAL("xxxxx") << L" " << ST_LITERAL("yyyyy");

    sstream.seekg(0);
    ST::string x, y;
    sstream >> x;
    sstream >> y;

    EXPECT_EQ(ST_LITERAL("xxxxx"), x);
    EXPECT_EQ(ST_LITERAL("yyyyy"), y);
}
