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

#include "st_stdio.h"

#include <gtest/gtest.h>

/* This file does not extensively test formatting, as that is already tested
   by test_format.cpp.  Instead, this just tests the stdio interfaces provided
   in <string_theory/stdio>. */

#define BUFFER_LEN (256)

TEST(stdio, printf)
{
    FILE *test_f = fopen("st_test.out", "w");
    ST_ASSERT(test_f, "Could not open output file for test");

    ST::printf(test_f, "xxxxx");
    ST::printf(test_f, "Testing {>12_#} output", "formatted");
    fclose(test_f);

    test_f = fopen("st_test.out", "r");
    ST_ASSERT(test_f, "Could not open output file for test");
    char buffer[BUFFER_LEN];
    fgets(buffer, BUFFER_LEN, test_f);
    buffer[BUFFER_LEN - 1] = 0;
    fclose(test_f);

    EXPECT_STREQ(buffer, "xxxxxTesting ###formatted output");
}
