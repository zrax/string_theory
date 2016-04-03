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

#include "st_stlformatter.h"
#include "st_format.h"

#include <gtest/gtest.h>
#include <string>
#include <limits>

TEST(format, escapes)
{
    EXPECT_EQ(ST::string("{x"), ST::format("{{{}", "x"));
    EXPECT_EQ(ST::string("x{"), ST::format("{}{{", "x"));
    EXPECT_EQ(ST::string("{{{{"), ST::format("{{{}{{{{", "{"));
    EXPECT_EQ(ST::string("{xxx{{yyy{"), ST::format("{{{}{{{{{}{{", "xxx", "yyy"));
}

TEST(format, errors)
{
    EXPECT_DEATH(ST::format("{}", 1, 2), "Too many actual parameters for format string");
    EXPECT_DEATH(ST::format("{} {}", 1), "Not enough actual parameters for format string");
    EXPECT_DEATH(ST::format("{", 1), "Unterminated format specifier");
    EXPECT_DEATH(ST::format("{.", 1), "Unterminated format specifier");
    EXPECT_DEATH(ST::format("{_", 1), "Unterminated format specifier");
    EXPECT_DEATH(ST::format(ST_NULLPTR, 1), "Passed a null format string");
}

TEST(format, strings)
{
    EXPECT_EQ(ST::string("TEST"), ST::format("{}", "TEST"));
    EXPECT_EQ(ST::string("xxTESTxx"), ST::format("xx{}xx", "TEST"));
    EXPECT_EQ(ST::string("xxTESTxx"), ST::format("xx{2}xx", "TEST"));
    EXPECT_EQ(ST::string("xxTESTxx"), ST::format("xx{>2}xx", "TEST"));
    EXPECT_EQ(ST::string("xxTESTxx"), ST::format("xx{<2}xx", "TEST"));
    EXPECT_EQ(ST::string("xxTESTxx"), ST::format("xx{_-2}xx", "TEST"));
    EXPECT_EQ(ST::string("xxTEST  xx"), ST::format("xx{6}xx", "TEST"));
    EXPECT_EQ(ST::string("xxTEST  xx"), ST::format("xx{<6}xx", "TEST"));
    EXPECT_EQ(ST::string("xx  TESTxx"), ST::format("xx{>6}xx", "TEST"));
    EXPECT_EQ(ST::string("xxTEST--xx"), ST::format("xx{_-6}xx", "TEST"));
    EXPECT_EQ(ST::string("xxTEST--xx"), ST::format("xx{<_-6}xx", "TEST"));
    EXPECT_EQ(ST::string("xx--TESTxx"), ST::format("xx{>_-6}xx", "TEST"));
    EXPECT_EQ(ST::string("xxONE  TWO    THREExx"), ST::format("xx{5}{<5}{>7}xx", "ONE", "TWO", "THREE"));

    // Ensure braces are parsed properly within padding chars
    EXPECT_EQ(ST::string("xxTEST}}xx"), ST::format("xx{_}6}xx", "TEST"));
    EXPECT_EQ(ST::string("xxTEST}}xx"), ST::format("xx{6_}}xx", "TEST"));
    EXPECT_EQ(ST::string("xxTEST{{xx"), ST::format("xx{_{6}xx", "TEST"));
    EXPECT_EQ(ST::string("xxTEST{{xx"), ST::format("xx{6_{}xx", "TEST"));
}

TEST(format, string_classes)
{
    // These should be handled just like normal const char* string params
    // (see above), so just need to test that the wrappers are working
    EXPECT_EQ(ST::string("xxTESTxx"), ST::format("xx{}xx", L"TEST"));
    EXPECT_EQ(ST::string("xxTESTxx"), ST::format("xx{}xx", ST::string("TEST")));
    EXPECT_EQ(ST::string("xxTESTxx"), ST::format("xx{}xx", std::string("TEST")));
    EXPECT_EQ(ST::string("xxTESTxx"), ST::format("xx{}xx", std::wstring(L"TEST")));
}

TEST(format, chars)
{
    EXPECT_EQ(ST::string("xxAxx"), ST::format("xx{}xx", 'A'));
    EXPECT_EQ(ST::string("xxAxx"), ST::format("xx{c}xx", (signed char)'A'));
    EXPECT_EQ(ST::string("xxAxx"), ST::format("xx{c}xx", (unsigned char)'A'));

    // UTF-8 encoding of wide (16-bit) char
    EXPECT_EQ(ST::string("xx\xef\xbf\xbexx"), ST::format("xx{}xx", L'\ufffe'));
    EXPECT_EQ(ST::string("xx\xe7\xbf\xbexx"), ST::format("xx{c}xx", (short)0x7ffe));
    EXPECT_EQ(ST::string("xx\xef\xbf\xbexx"), ST::format("xx{c}xx", (unsigned short)0xfffe));

    // UTF-8 encoding of UCS4 (32-bit) char
    EXPECT_EQ(ST::string("xx\xf4\x8f\xbf\xbfxx"), ST::format("xx{c}xx", (int)0x10ffff));
    EXPECT_EQ(ST::string("xx\xf4\x8f\xbf\xbfxx"), ST::format("xx{c}xx", (unsigned int)0x10ffff));
    EXPECT_EQ(ST::string("xx\xf4\x8f\xbf\xbfxx"), ST::format("xx{c}xx", (long)0x10ffff));
    EXPECT_EQ(ST::string("xx\xf4\x8f\xbf\xbfxx"), ST::format("xx{c}xx", (unsigned long)0x10ffff));
#ifdef ST_HAVE_INT64
    EXPECT_EQ(ST::string("xx\xf4\x8f\xbf\xbfxx"), ST::format("xx{c}xx", (int64_t)0x10ffff));
    EXPECT_EQ(ST::string("xx\xf4\x8f\xbf\xbfxx"), ST::format("xx{c}xx", (uint64_t)0x10ffff));
#endif
}

TEST(format, decimal)
{
    EXPECT_EQ(ST::string("xx1234xx"), ST::format("xx{}xx", 1234));
    EXPECT_EQ(ST::string("xx1234xx"), ST::format("xx{d}xx", 1234));
    EXPECT_EQ(ST::string("xx1234xx"), ST::format("xx{2}xx", 1234));
    EXPECT_EQ(ST::string("xx1234xx"), ST::format("xx{>2}xx", 1234));
    EXPECT_EQ(ST::string("xx1234xx"), ST::format("xx{<2}xx", 1234));
    EXPECT_EQ(ST::string("xx  1234xx"), ST::format("xx{6}xx", 1234));
    EXPECT_EQ(ST::string("xx  1234xx"), ST::format("xx{>6}xx", 1234));
    EXPECT_EQ(ST::string("xx1234  xx"), ST::format("xx{<6}xx", 1234));

    // Character types
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{d}xx", '\0'));
    EXPECT_EQ(ST::string("xx65xx"), ST::format("xx{d}xx", 'A'));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{d}xx", L'\0'));
    EXPECT_EQ(ST::string("xx65xx"), ST::format("xx{d}xx", L'A'));
    EXPECT_EQ(ST::string("xx32767xx"), ST::format("xx{d}xx", L'\u7fff'));

    // Numeric char types
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{}xx", (signed char)0));
    EXPECT_EQ(ST::string("xx127xx"), ST::format("xx{}xx", std::numeric_limits<signed char>::max()));
    EXPECT_EQ(ST::string("xx+127xx"), ST::format("xx{+}xx", std::numeric_limits<signed char>::max()));
    EXPECT_EQ(ST::string("xx-128xx"), ST::format("xx{}xx", std::numeric_limits<signed char>::min()));
    EXPECT_EQ(ST::string("xx-128xx"), ST::format("xx{+}xx", std::numeric_limits<signed char>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{}xx", (unsigned char)0));
    EXPECT_EQ(ST::string("xx255xx"), ST::format("xx{}xx", std::numeric_limits<unsigned char>::max()));
    EXPECT_EQ(ST::string("xx+255xx"), ST::format("xx{+}xx", std::numeric_limits<unsigned char>::max()));

    // 16-bit ints
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{}xx", (short)0));
    EXPECT_EQ(ST::string("xx32767xx"), ST::format("xx{}xx", std::numeric_limits<short>::max()));
    EXPECT_EQ(ST::string("xx+32767xx"), ST::format("xx{+}xx", std::numeric_limits<short>::max()));
    EXPECT_EQ(ST::string("xx-32768xx"), ST::format("xx{}xx", std::numeric_limits<short>::min()));
    EXPECT_EQ(ST::string("xx-32768xx"), ST::format("xx{+}xx", std::numeric_limits<short>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{}xx", (unsigned short)0));
    EXPECT_EQ(ST::string("xx65535xx"), ST::format("xx{}xx", std::numeric_limits<unsigned short>::max()));
    EXPECT_EQ(ST::string("xx+65535xx"), ST::format("xx{+}xx", std::numeric_limits<unsigned short>::max()));

    // 32-bit ints
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{}xx", (int)0));
    EXPECT_EQ(ST::string("xx2147483647xx"), ST::format("xx{}xx", std::numeric_limits<int>::max()));
    EXPECT_EQ(ST::string("xx+2147483647xx"), ST::format("xx{+}xx", std::numeric_limits<int>::max()));
    EXPECT_EQ(ST::string("xx-2147483648xx"), ST::format("xx{}xx", std::numeric_limits<int>::min()));
    EXPECT_EQ(ST::string("xx-2147483648xx"), ST::format("xx{+}xx", std::numeric_limits<int>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{}xx", (unsigned int)0));
    EXPECT_EQ(ST::string("xx4294967295xx"), ST::format("xx{}xx", std::numeric_limits<unsigned int>::max()));
    EXPECT_EQ(ST::string("xx+4294967295xx"), ST::format("xx{+}xx", std::numeric_limits<unsigned int>::max()));

#ifdef ST_HAVE_INT64
    // 64-bit ints
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{}xx", (int64_t)0));
    EXPECT_EQ(ST::string("xx9223372036854775807xx"), ST::format("xx{}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST::string("xx+9223372036854775807xx"), ST::format("xx{+}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST::string("xx-9223372036854775808xx"), ST::format("xx{}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST::string("xx-9223372036854775808xx"), ST::format("xx{+}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{}xx", (uint64_t)0));
    EXPECT_EQ(ST::string("xx18446744073709551615xx"), ST::format("xx{}xx", std::numeric_limits<uint64_t>::max()));
    EXPECT_EQ(ST::string("xx+18446744073709551615xx"), ST::format("xx{+}xx", std::numeric_limits<uint64_t>::max()));
#endif
}

TEST(format, decimal_prefix)
{
    EXPECT_EQ(ST::string("xx1234xx"), ST::format("xx{#}xx", 1234));
    EXPECT_EQ(ST::string("xx1234xx"), ST::format("xx{#d}xx", 1234));
    EXPECT_EQ(ST::string("xx1234xx"), ST::format("xx{#2}xx", 1234));
    EXPECT_EQ(ST::string("xx1234xx"), ST::format("xx{>#2}xx", 1234));
    EXPECT_EQ(ST::string("xx1234xx"), ST::format("xx{<2#}xx", 1234));
    EXPECT_EQ(ST::string("xx  1234xx"), ST::format("xx{#6}xx", 1234));
    EXPECT_EQ(ST::string("xx  1234xx"), ST::format("xx{>#6}xx", 1234));
    EXPECT_EQ(ST::string("xx1234  xx"), ST::format("xx{<#6}xx", 1234));

    // Character types
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#d}xx", '\0'));
    EXPECT_EQ(ST::string("xx65xx"), ST::format("xx{#d}xx", 'A'));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#d}xx", L'\0'));
    EXPECT_EQ(ST::string("xx65xx"), ST::format("xx{#d}xx", L'A'));
    EXPECT_EQ(ST::string("xx32767xx"), ST::format("xx{#d}xx", L'\u7fff'));

    // 32-bit ints
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#}xx", (int)0));
    EXPECT_EQ(ST::string("xx2147483647xx"), ST::format("xx{#}xx", std::numeric_limits<int>::max()));
    EXPECT_EQ(ST::string("xx+2147483647xx"), ST::format("xx{+#}xx", std::numeric_limits<int>::max()));
    EXPECT_EQ(ST::string("xx-2147483648xx"), ST::format("xx{#}xx", std::numeric_limits<int>::min()));
    EXPECT_EQ(ST::string("xx-2147483648xx"), ST::format("xx{+#}xx", std::numeric_limits<int>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#}xx", (unsigned int)0));
    EXPECT_EQ(ST::string("xx4294967295xx"), ST::format("xx{#}xx", std::numeric_limits<unsigned int>::max()));
    EXPECT_EQ(ST::string("xx+4294967295xx"), ST::format("xx{+#}xx", std::numeric_limits<unsigned int>::max()));

#ifdef ST_HAVE_INT64
    // 64-bit ints
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#}xx", (int64_t)0));
    EXPECT_EQ(ST::string("xx9223372036854775807xx"), ST::format("xx{#}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST::string("xx+9223372036854775807xx"), ST::format("xx{+#}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST::string("xx-9223372036854775808xx"), ST::format("xx{#}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST::string("xx-9223372036854775808xx"), ST::format("xx{+#}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#}xx", (uint64_t)0));
    EXPECT_EQ(ST::string("xx18446744073709551615xx"), ST::format("xx{#}xx", std::numeric_limits<uint64_t>::max()));
    EXPECT_EQ(ST::string("xx+18446744073709551615xx"), ST::format("xx{+#}xx", std::numeric_limits<uint64_t>::max()));
#endif
}

TEST(format, hex)
{
    EXPECT_EQ(ST::string("xx4d2xx"), ST::format("xx{x}xx", 1234));
    EXPECT_EQ(ST::string("xx4d2xx"), ST::format("xx{x}xx", 1234));
    EXPECT_EQ(ST::string("xx4d2xx"), ST::format("xx{2x}xx", 1234));
    EXPECT_EQ(ST::string("xx4d2xx"), ST::format("xx{>2x}xx", 1234));
    EXPECT_EQ(ST::string("xx4d2xx"), ST::format("xx{<2x}xx", 1234));
    EXPECT_EQ(ST::string("xx   4d2xx"), ST::format("xx{6x}xx", 1234));
    EXPECT_EQ(ST::string("xx   4d2xx"), ST::format("xx{>6x}xx", 1234));
    EXPECT_EQ(ST::string("xx4d2   xx"), ST::format("xx{<6x}xx", 1234));

    // Character types
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{x}xx", '\0'));
    EXPECT_EQ(ST::string("xx41xx"), ST::format("xx{x}xx", 'A'));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{x}xx", L'\0'));
    EXPECT_EQ(ST::string("xx41xx"), ST::format("xx{x}xx", L'A'));
    EXPECT_EQ(ST::string("xx7fffxx"), ST::format("xx{x}xx", L'\u7fff'));

    // Numeric char types
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{x}xx", (signed char)0));
    EXPECT_EQ(ST::string("xx7fxx"), ST::format("xx{x}xx", std::numeric_limits<signed char>::max()));
    EXPECT_EQ(ST::string("xx+7fxx"), ST::format("xx{+x}xx", std::numeric_limits<signed char>::max()));
    EXPECT_EQ(ST::string("xx-80xx"), ST::format("xx{x}xx", std::numeric_limits<signed char>::min()));
    EXPECT_EQ(ST::string("xx-80xx"), ST::format("xx{+x}xx", std::numeric_limits<signed char>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{x}xx", (unsigned char)0));
    EXPECT_EQ(ST::string("xxffxx"), ST::format("xx{x}xx", std::numeric_limits<unsigned char>::max()));

    // 16-bit ints
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{x}xx", (short)0));
    EXPECT_EQ(ST::string("xx7fffxx"), ST::format("xx{x}xx", std::numeric_limits<short>::max()));
    EXPECT_EQ(ST::string("xx+7fffxx"), ST::format("xx{+x}xx", std::numeric_limits<short>::max()));
    EXPECT_EQ(ST::string("xx-8000xx"), ST::format("xx{x}xx", std::numeric_limits<short>::min()));
    EXPECT_EQ(ST::string("xx-8000xx"), ST::format("xx{+x}xx", std::numeric_limits<short>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{x}xx", (unsigned short)0));
    EXPECT_EQ(ST::string("xxffffxx"), ST::format("xx{x}xx", std::numeric_limits<unsigned short>::max()));

    // 32-bit ints
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{x}xx", (int)0));
    EXPECT_EQ(ST::string("xx7fffffffxx"), ST::format("xx{x}xx", std::numeric_limits<int>::max()));
    EXPECT_EQ(ST::string("xx+7fffffffxx"), ST::format("xx{+x}xx", std::numeric_limits<int>::max()));
    EXPECT_EQ(ST::string("xx-80000000xx"), ST::format("xx{x}xx", std::numeric_limits<int>::min()));
    EXPECT_EQ(ST::string("xx-80000000xx"), ST::format("xx{+x}xx", std::numeric_limits<int>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{x}xx", (unsigned int)0));
    EXPECT_EQ(ST::string("xxffffffffxx"), ST::format("xx{x}xx", std::numeric_limits<unsigned int>::max()));

#ifdef ST_HAVE_INT64
    // 64-bit ints
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{x}xx", (int64_t)0));
    EXPECT_EQ(ST::string("xx7fffffffffffffffxx"), ST::format("xx{x}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST::string("xx+7fffffffffffffffxx"), ST::format("xx{+x}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST::string("xx-8000000000000000xx"), ST::format("xx{x}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST::string("xx-8000000000000000xx"), ST::format("xx{+x}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{x}xx", (uint64_t)0));
    EXPECT_EQ(ST::string("xxffffffffffffffffxx"), ST::format("xx{x}xx", std::numeric_limits<uint64_t>::max()));
#endif
}

TEST(format, hex_prefix)
{
    EXPECT_EQ(ST::string("xx0x4d2xx"), ST::format("xx{#x}xx", 1234));
    EXPECT_EQ(ST::string("xx0x4d2xx"), ST::format("xx{#x}xx", 1234));
    EXPECT_EQ(ST::string("xx0x4d2xx"), ST::format("xx{#2x}xx", 1234));
    EXPECT_EQ(ST::string("xx0x4d2xx"), ST::format("xx{>#2x}xx", 1234));
    EXPECT_EQ(ST::string("xx0x4d2xx"), ST::format("xx{<#2x}xx", 1234));
    EXPECT_EQ(ST::string("xx   0x4d2xx"), ST::format("xx{#8x}xx", 1234));
    EXPECT_EQ(ST::string("xx   0x4d2xx"), ST::format("xx{>#8x}xx", 1234));
    EXPECT_EQ(ST::string("xx0x4d2   xx"), ST::format("xx{<#8x}xx", 1234));

    // Character types
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#x}xx", '\0'));
    EXPECT_EQ(ST::string("xx0x41xx"), ST::format("xx{#x}xx", 'A'));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#x}xx", L'\0'));
    EXPECT_EQ(ST::string("xx0x41xx"), ST::format("xx{#x}xx", L'A'));
    EXPECT_EQ(ST::string("xx0x7fffxx"), ST::format("xx{#x}xx", L'\u7fff'));

    // 32-bit ints
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#x}xx", (int)0));
    EXPECT_EQ(ST::string("xx0x7fffffffxx"), ST::format("xx{#x}xx", std::numeric_limits<int>::max()));
    EXPECT_EQ(ST::string("xx+0x7fffffffxx"), ST::format("xx{+#x}xx", std::numeric_limits<int>::max()));
    EXPECT_EQ(ST::string("xx-0x80000000xx"), ST::format("xx{#x}xx", std::numeric_limits<int>::min()));
    EXPECT_EQ(ST::string("xx-0x80000000xx"), ST::format("xx{+#x}xx", std::numeric_limits<int>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#x}xx", (unsigned int)0));
    EXPECT_EQ(ST::string("xx0xffffffffxx"), ST::format("xx{#x}xx", std::numeric_limits<unsigned int>::max()));

#ifdef ST_HAVE_INT64
    // 64-bit ints
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#x}xx", (int64_t)0));
    EXPECT_EQ(ST::string("xx0x7fffffffffffffffxx"), ST::format("xx{#x}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST::string("xx+0x7fffffffffffffffxx"), ST::format("xx{+#x}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST::string("xx-0x8000000000000000xx"), ST::format("xx{#x}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST::string("xx-0x8000000000000000xx"), ST::format("xx{+#x}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#x}xx", (uint64_t)0));
    EXPECT_EQ(ST::string("xx0xffffffffffffffffxx"), ST::format("xx{#x}xx", std::numeric_limits<uint64_t>::max()));
#endif
}

TEST(format, hex_upper)
{
    EXPECT_EQ(ST::string("xx4D2xx"), ST::format("xx{X}xx", 1234));
    EXPECT_EQ(ST::string("xx4D2xx"), ST::format("xx{X}xx", 1234));
    EXPECT_EQ(ST::string("xx4D2xx"), ST::format("xx{2X}xx", 1234));
    EXPECT_EQ(ST::string("xx4D2xx"), ST::format("xx{>2X}xx", 1234));
    EXPECT_EQ(ST::string("xx4D2xx"), ST::format("xx{<2X}xx", 1234));
    EXPECT_EQ(ST::string("xx   4D2xx"), ST::format("xx{6X}xx", 1234));
    EXPECT_EQ(ST::string("xx   4D2xx"), ST::format("xx{>6X}xx", 1234));
    EXPECT_EQ(ST::string("xx4D2   xx"), ST::format("xx{<6X}xx", 1234));

    // Character types
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{X}xx", '\0'));
    EXPECT_EQ(ST::string("xx41xx"), ST::format("xx{X}xx", 'A'));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{X}xx", L'\0'));
    EXPECT_EQ(ST::string("xx41xx"), ST::format("xx{X}xx", L'A'));
    EXPECT_EQ(ST::string("xx7FFFxx"), ST::format("xx{X}xx", L'\u7fff'));

    // Numeric char types
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{X}xx", (signed char)0));
    EXPECT_EQ(ST::string("xx7Fxx"), ST::format("xx{X}xx", std::numeric_limits<signed char>::max()));
    EXPECT_EQ(ST::string("xx+7Fxx"), ST::format("xx{+X}xx", std::numeric_limits<signed char>::max()));
    EXPECT_EQ(ST::string("xx-80xx"), ST::format("xx{X}xx", std::numeric_limits<signed char>::min()));
    EXPECT_EQ(ST::string("xx-80xx"), ST::format("xx{+X}xx", std::numeric_limits<signed char>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{X}xx", (unsigned char)0));
    EXPECT_EQ(ST::string("xxFFxx"), ST::format("xx{X}xx", std::numeric_limits<unsigned char>::max()));

    // 16-bit ints
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{X}xx", (short)0));
    EXPECT_EQ(ST::string("xx7FFFxx"), ST::format("xx{X}xx", std::numeric_limits<short>::max()));
    EXPECT_EQ(ST::string("xx+7FFFxx"), ST::format("xx{+X}xx", std::numeric_limits<short>::max()));
    EXPECT_EQ(ST::string("xx-8000xx"), ST::format("xx{X}xx", std::numeric_limits<short>::min()));
    EXPECT_EQ(ST::string("xx-8000xx"), ST::format("xx{+X}xx", std::numeric_limits<short>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{X}xx", (unsigned short)0));
    EXPECT_EQ(ST::string("xxFFFFxx"), ST::format("xx{X}xx", std::numeric_limits<unsigned short>::max()));

    // 32-bit ints
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{X}xx", (int)0));
    EXPECT_EQ(ST::string("xx7FFFFFFFxx"), ST::format("xx{X}xx", std::numeric_limits<int>::max()));
    EXPECT_EQ(ST::string("xx+7FFFFFFFxx"), ST::format("xx{+X}xx", std::numeric_limits<int>::max()));
    EXPECT_EQ(ST::string("xx-80000000xx"), ST::format("xx{X}xx", std::numeric_limits<int>::min()));
    EXPECT_EQ(ST::string("xx-80000000xx"), ST::format("xx{+X}xx", std::numeric_limits<int>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{X}xx", (unsigned int)0));
    EXPECT_EQ(ST::string("xxFFFFFFFFxx"), ST::format("xx{X}xx", std::numeric_limits<unsigned int>::max()));

#ifdef ST_HAVE_INT64
    // 64-bit ints
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{X}xx", (int64_t)0));
    EXPECT_EQ(ST::string("xx7FFFFFFFFFFFFFFFxx"), ST::format("xx{X}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST::string("xx+7FFFFFFFFFFFFFFFxx"), ST::format("xx{+X}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST::string("xx-8000000000000000xx"), ST::format("xx{X}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST::string("xx-8000000000000000xx"), ST::format("xx{+X}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{X}xx", (uint64_t)0));
    EXPECT_EQ(ST::string("xxFFFFFFFFFFFFFFFFxx"), ST::format("xx{X}xx", std::numeric_limits<uint64_t>::max()));
#endif
}

TEST(format, hex_upper_prefix)
{
    EXPECT_EQ(ST::string("xx0X4D2xx"), ST::format("xx{#X}xx", 1234));
    EXPECT_EQ(ST::string("xx0X4D2xx"), ST::format("xx{#X}xx", 1234));
    EXPECT_EQ(ST::string("xx0X4D2xx"), ST::format("xx{#2X}xx", 1234));
    EXPECT_EQ(ST::string("xx0X4D2xx"), ST::format("xx{>#2X}xx", 1234));
    EXPECT_EQ(ST::string("xx0X4D2xx"), ST::format("xx{<#2X}xx", 1234));
    EXPECT_EQ(ST::string("xx   0X4D2xx"), ST::format("xx{#8X}xx", 1234));
    EXPECT_EQ(ST::string("xx   0X4D2xx"), ST::format("xx{>#8X}xx", 1234));
    EXPECT_EQ(ST::string("xx0X4D2   xx"), ST::format("xx{<#8X}xx", 1234));

    // Character types
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#X}xx", '\0'));
    EXPECT_EQ(ST::string("xx0X41xx"), ST::format("xx{#X}xx", 'A'));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#X}xx", L'\0'));
    EXPECT_EQ(ST::string("xx0X41xx"), ST::format("xx{#X}xx", L'A'));
    EXPECT_EQ(ST::string("xx0X7FFFxx"), ST::format("xx{#X}xx", L'\u7fff'));

    // 32-bit ints
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#X}xx", (int)0));
    EXPECT_EQ(ST::string("xx0X7FFFFFFFxx"), ST::format("xx{#X}xx", std::numeric_limits<int>::max()));
    EXPECT_EQ(ST::string("xx+0X7FFFFFFFxx"), ST::format("xx{+#X}xx", std::numeric_limits<int>::max()));
    EXPECT_EQ(ST::string("xx-0X80000000xx"), ST::format("xx{#X}xx", std::numeric_limits<int>::min()));
    EXPECT_EQ(ST::string("xx-0X80000000xx"), ST::format("xx{+#X}xx", std::numeric_limits<int>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#X}xx", (unsigned int)0));
    EXPECT_EQ(ST::string("xx0XFFFFFFFFxx"), ST::format("xx{#X}xx", std::numeric_limits<unsigned int>::max()));

#ifdef ST_HAVE_INT64
    // 64-bit ints
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#X}xx", (int64_t)0));
    EXPECT_EQ(ST::string("xx0X7FFFFFFFFFFFFFFFxx"), ST::format("xx{#X}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST::string("xx+0X7FFFFFFFFFFFFFFFxx"), ST::format("xx{+#X}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST::string("xx-0X8000000000000000xx"), ST::format("xx{#X}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST::string("xx-0X8000000000000000xx"), ST::format("xx{+#X}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#X}xx", (uint64_t)0));
    EXPECT_EQ(ST::string("xx0XFFFFFFFFFFFFFFFFxx"), ST::format("xx{#X}xx", std::numeric_limits<uint64_t>::max()));
#endif
}

TEST(format, octal)
{
    EXPECT_EQ(ST::string("xx2322xx"), ST::format("xx{o}xx", 1234));
    EXPECT_EQ(ST::string("xx2322xx"), ST::format("xx{o}xx", 1234));
    EXPECT_EQ(ST::string("xx2322xx"), ST::format("xx{2o}xx", 1234));
    EXPECT_EQ(ST::string("xx2322xx"), ST::format("xx{>2o}xx", 1234));
    EXPECT_EQ(ST::string("xx2322xx"), ST::format("xx{<2o}xx", 1234));
    EXPECT_EQ(ST::string("xx  2322xx"), ST::format("xx{6o}xx", 1234));
    EXPECT_EQ(ST::string("xx  2322xx"), ST::format("xx{>6o}xx", 1234));
    EXPECT_EQ(ST::string("xx2322  xx"), ST::format("xx{<6o}xx", 1234));

    // Character types
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{o}xx", '\0'));
    EXPECT_EQ(ST::string("xx101xx"), ST::format("xx{o}xx", 'A'));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{o}xx", L'\0'));
    EXPECT_EQ(ST::string("xx101xx"), ST::format("xx{o}xx", L'A'));
    EXPECT_EQ(ST::string("xx77777xx"), ST::format("xx{o}xx", L'\u7fff'));

    // Numeric char types
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{o}xx", (signed char)0));
    EXPECT_EQ(ST::string("xx177xx"), ST::format("xx{o}xx", std::numeric_limits<signed char>::max()));
    EXPECT_EQ(ST::string("xx+177xx"), ST::format("xx{+o}xx", std::numeric_limits<signed char>::max()));
    EXPECT_EQ(ST::string("xx-200xx"), ST::format("xx{o}xx", std::numeric_limits<signed char>::min()));
    EXPECT_EQ(ST::string("xx-200xx"), ST::format("xx{+o}xx", std::numeric_limits<signed char>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{o}xx", (unsigned char)0));
    EXPECT_EQ(ST::string("xx377xx"), ST::format("xx{o}xx", std::numeric_limits<unsigned char>::max()));

    // 16-bit ints
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{o}xx", (short)0));
    EXPECT_EQ(ST::string("xx77777xx"), ST::format("xx{o}xx", std::numeric_limits<short>::max()));
    EXPECT_EQ(ST::string("xx+77777xx"), ST::format("xx{+o}xx", std::numeric_limits<short>::max()));
    EXPECT_EQ(ST::string("xx-100000xx"), ST::format("xx{o}xx", std::numeric_limits<short>::min()));
    EXPECT_EQ(ST::string("xx-100000xx"), ST::format("xx{+o}xx", std::numeric_limits<short>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{o}xx", (unsigned short)0));
    EXPECT_EQ(ST::string("xx177777xx"), ST::format("xx{o}xx", std::numeric_limits<unsigned short>::max()));

    // 32-bit ints
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{o}xx", (int)0));
    EXPECT_EQ(ST::string("xx17777777777xx"), ST::format("xx{o}xx", std::numeric_limits<int>::max()));
    EXPECT_EQ(ST::string("xx+17777777777xx"), ST::format("xx{+o}xx", std::numeric_limits<int>::max()));
    EXPECT_EQ(ST::string("xx-20000000000xx"), ST::format("xx{o}xx", std::numeric_limits<int>::min()));
    EXPECT_EQ(ST::string("xx-20000000000xx"), ST::format("xx{+o}xx", std::numeric_limits<int>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{o}xx", (unsigned int)0));
    EXPECT_EQ(ST::string("xx37777777777xx"), ST::format("xx{o}xx", std::numeric_limits<unsigned int>::max()));

#ifdef ST_HAVE_INT64
    // 64-bit ints
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{o}xx", (int64_t)0));
    EXPECT_EQ(ST::string("xx777777777777777777777xx"), ST::format("xx{o}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST::string("xx+777777777777777777777xx"), ST::format("xx{+o}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST::string("xx-1000000000000000000000xx"), ST::format("xx{o}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST::string("xx-1000000000000000000000xx"), ST::format("xx{+o}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{o}xx", (uint64_t)0));
    EXPECT_EQ(ST::string("xx1777777777777777777777xx"), ST::format("xx{o}xx", std::numeric_limits<uint64_t>::max()));
#endif
}

TEST(format, octal_prefix)
{
    EXPECT_EQ(ST::string("xx02322xx"), ST::format("xx{#o}xx", 1234));
    EXPECT_EQ(ST::string("xx02322xx"), ST::format("xx{#o}xx", 1234));
    EXPECT_EQ(ST::string("xx02322xx"), ST::format("xx{#2o}xx", 1234));
    EXPECT_EQ(ST::string("xx02322xx"), ST::format("xx{>#2o}xx", 1234));
    EXPECT_EQ(ST::string("xx02322xx"), ST::format("xx{<#2o}xx", 1234));
    EXPECT_EQ(ST::string("xx  02322xx"), ST::format("xx{#7o}xx", 1234));
    EXPECT_EQ(ST::string("xx  02322xx"), ST::format("xx{>#7o}xx", 1234));
    EXPECT_EQ(ST::string("xx02322  xx"), ST::format("xx{<#7o}xx", 1234));

    // Character types
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#o}xx", '\0'));
    EXPECT_EQ(ST::string("xx0101xx"), ST::format("xx{#o}xx", 'A'));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#o}xx", L'\0'));
    EXPECT_EQ(ST::string("xx0101xx"), ST::format("xx{#o}xx", L'A'));
    EXPECT_EQ(ST::string("xx077777xx"), ST::format("xx{#o}xx", L'\u7fff'));

    // 32-bit ints
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#o}xx", (int)0));
    EXPECT_EQ(ST::string("xx017777777777xx"), ST::format("xx{#o}xx", std::numeric_limits<int>::max()));
    EXPECT_EQ(ST::string("xx+017777777777xx"), ST::format("xx{+#o}xx", std::numeric_limits<int>::max()));
    EXPECT_EQ(ST::string("xx-020000000000xx"), ST::format("xx{#o}xx", std::numeric_limits<int>::min()));
    EXPECT_EQ(ST::string("xx-020000000000xx"), ST::format("xx{+#o}xx", std::numeric_limits<int>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#o}xx", (unsigned int)0));
    EXPECT_EQ(ST::string("xx037777777777xx"), ST::format("xx{#o}xx", std::numeric_limits<unsigned int>::max()));

#ifdef ST_HAVE_INT64
    // 64-bit ints
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#o}xx", (int64_t)0));
    EXPECT_EQ(ST::string("xx0777777777777777777777xx"), ST::format("xx{#o}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST::string("xx+0777777777777777777777xx"), ST::format("xx{+#o}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST::string("xx-01000000000000000000000xx"), ST::format("xx{#o}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST::string("xx-01000000000000000000000xx"), ST::format("xx{+#o}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#o}xx", (uint64_t)0));
    EXPECT_EQ(ST::string("xx01777777777777777777777xx"), ST::format("xx{#o}xx", std::numeric_limits<uint64_t>::max()));
#endif
}

TEST(format, binary)
{
    EXPECT_EQ(ST::string("xx10011010010xx"), ST::format("xx{b}xx", 1234));
    EXPECT_EQ(ST::string("xx10011010010xx"), ST::format("xx{b}xx", 1234));
    EXPECT_EQ(ST::string("xx10011010010xx"), ST::format("xx{2b}xx", 1234));
    EXPECT_EQ(ST::string("xx10011010010xx"), ST::format("xx{>2b}xx", 1234));
    EXPECT_EQ(ST::string("xx10011010010xx"), ST::format("xx{<2b}xx", 1234));
    EXPECT_EQ(ST::string("xx     10011010010xx"), ST::format("xx{16b}xx", 1234));
    EXPECT_EQ(ST::string("xx     10011010010xx"), ST::format("xx{>16b}xx", 1234));
    EXPECT_EQ(ST::string("xx10011010010     xx"), ST::format("xx{<16b}xx", 1234));

    // Character types
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{b}xx", '\0'));
    EXPECT_EQ(ST::string("xx1000001xx"), ST::format("xx{b}xx", 'A'));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{b}xx", L'\0'));
    EXPECT_EQ(ST::string("xx1000001xx"), ST::format("xx{b}xx", L'A'));
    EXPECT_EQ(ST::string("xx111111111111111xx"), ST::format("xx{b}xx", L'\u7fff'));

    // Numeric char types
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{b}xx", (signed char)0));
    EXPECT_EQ(ST::string("xx1111111xx"), ST::format("xx{b}xx", std::numeric_limits<signed char>::max()));
    EXPECT_EQ(ST::string("xx+1111111xx"), ST::format("xx{+b}xx", std::numeric_limits<signed char>::max()));
    EXPECT_EQ(ST::string("xx-10000000xx"), ST::format("xx{b}xx", std::numeric_limits<signed char>::min()));
    EXPECT_EQ(ST::string("xx-10000000xx"), ST::format("xx{+b}xx", std::numeric_limits<signed char>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{b}xx", (unsigned char)0));
    EXPECT_EQ(ST::string("xx11111111xx"), ST::format("xx{b}xx", std::numeric_limits<unsigned char>::max()));

    // 16-bit ints
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{b}xx", (short)0));
    EXPECT_EQ(ST::string("xx111111111111111xx"), ST::format("xx{b}xx", std::numeric_limits<short>::max()));
    EXPECT_EQ(ST::string("xx+111111111111111xx"), ST::format("xx{+b}xx", std::numeric_limits<short>::max()));
    EXPECT_EQ(ST::string("xx-1000000000000000xx"), ST::format("xx{b}xx", std::numeric_limits<short>::min()));
    EXPECT_EQ(ST::string("xx-1000000000000000xx"), ST::format("xx{+b}xx", std::numeric_limits<short>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{b}xx", (unsigned short)0));
    EXPECT_EQ(ST::string("xx1111111111111111xx"), ST::format("xx{b}xx", std::numeric_limits<unsigned short>::max()));

    // 32-bit ints
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{b}xx", (int)0));
    EXPECT_EQ(ST::string("xx1111111111111111111111111111111xx"),
              ST::format("xx{b}xx", std::numeric_limits<int>::max()));
    EXPECT_EQ(ST::string("xx+1111111111111111111111111111111xx"),
              ST::format("xx{+b}xx", std::numeric_limits<int>::max()));
    EXPECT_EQ(ST::string("xx-10000000000000000000000000000000xx"),
              ST::format("xx{b}xx", std::numeric_limits<int>::min()));
    EXPECT_EQ(ST::string("xx-10000000000000000000000000000000xx"),
              ST::format("xx{+b}xx", std::numeric_limits<int>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{b}xx", (unsigned int)0));
    EXPECT_EQ(ST::string("xx11111111111111111111111111111111xx"),
              ST::format("xx{b}xx", std::numeric_limits<unsigned int>::max()));

#ifdef ST_HAVE_INT64
    // 64-bit ints
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{b}xx", (int64_t)0));
    EXPECT_EQ(ST::string("xx111111111111111111111111111111111111111111111111111111111111111xx"),
              ST::format("xx{b}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST::string("xx+111111111111111111111111111111111111111111111111111111111111111xx"),
              ST::format("xx{+b}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST::string("xx-1000000000000000000000000000000000000000000000000000000000000000xx"),
              ST::format("xx{b}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST::string("xx-1000000000000000000000000000000000000000000000000000000000000000xx"),
              ST::format("xx{+b}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{b}xx", (uint64_t)0));
    EXPECT_EQ(ST::string("xx1111111111111111111111111111111111111111111111111111111111111111xx"),
              ST::format("xx{b}xx", std::numeric_limits<uint64_t>::max()));
#endif
}

TEST(format, binary_prefix)
{
    EXPECT_EQ(ST::string("xx0b10011010010xx"), ST::format("xx{#b}xx", 1234));
    EXPECT_EQ(ST::string("xx0b10011010010xx"), ST::format("xx{#b}xx", 1234));
    EXPECT_EQ(ST::string("xx0b10011010010xx"), ST::format("xx{#2b}xx", 1234));
    EXPECT_EQ(ST::string("xx0b10011010010xx"), ST::format("xx{>#2b}xx", 1234));
    EXPECT_EQ(ST::string("xx0b10011010010xx"), ST::format("xx{<#2b}xx", 1234));
    EXPECT_EQ(ST::string("xx   0b10011010010xx"), ST::format("xx{#16b}xx", 1234));
    EXPECT_EQ(ST::string("xx   0b10011010010xx"), ST::format("xx{>#16b}xx", 1234));
    EXPECT_EQ(ST::string("xx0b10011010010   xx"), ST::format("xx{<#16b}xx", 1234));

    // Character types
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#b}xx", '\0'));
    EXPECT_EQ(ST::string("xx0b1000001xx"), ST::format("xx{#b}xx", 'A'));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#b}xx", L'\0'));
    EXPECT_EQ(ST::string("xx0b1000001xx"), ST::format("xx{#b}xx", L'A'));
    EXPECT_EQ(ST::string("xx0b111111111111111xx"), ST::format("xx{#b}xx", L'\u7fff'));

    // 32-bit ints
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#b}xx", (int)0));
    EXPECT_EQ(ST::string("xx0b1111111111111111111111111111111xx"),
              ST::format("xx{#b}xx", std::numeric_limits<int>::max()));
    EXPECT_EQ(ST::string("xx+0b1111111111111111111111111111111xx"),
              ST::format("xx{+#b}xx", std::numeric_limits<int>::max()));
    EXPECT_EQ(ST::string("xx-0b10000000000000000000000000000000xx"),
              ST::format("xx{#b}xx", std::numeric_limits<int>::min()));
    EXPECT_EQ(ST::string("xx-0b10000000000000000000000000000000xx"),
              ST::format("xx{+#b}xx", std::numeric_limits<int>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#b}xx", (unsigned int)0));
    EXPECT_EQ(ST::string("xx0b11111111111111111111111111111111xx"),
              ST::format("xx{#b}xx", std::numeric_limits<unsigned int>::max()));

#ifdef ST_HAVE_INT64
    // 64-bit ints
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#b}xx", (int64_t)0));
    EXPECT_EQ(ST::string("xx0b111111111111111111111111111111111111111111111111111111111111111xx"),
              ST::format("xx{#b}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST::string("xx+0b111111111111111111111111111111111111111111111111111111111111111xx"),
              ST::format("xx{+#b}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST::string("xx-0b1000000000000000000000000000000000000000000000000000000000000000xx"),
              ST::format("xx{#b}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST::string("xx-0b1000000000000000000000000000000000000000000000000000000000000000xx"),
              ST::format("xx{+#b}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST::string("xx0xx"), ST::format("xx{#b}xx", (uint64_t)0));
    EXPECT_EQ(ST::string("xx0b1111111111111111111111111111111111111111111111111111111111111111xx"),
              ST::format("xx{#b}xx", std::numeric_limits<uint64_t>::max()));
#endif
}

TEST(format, floating_point)
{
    // The actual formatting is handled by libc, so we just need to test
    // that the flags get passed along properly.

    EXPECT_EQ(ST::string("xx1.5xx"), ST::format("xx{}xx", 1.5));
    EXPECT_EQ(ST::string("xx+1.5xx"), ST::format("xx{+}xx", 1.5));
    EXPECT_EQ(ST::string("xx-1.5xx"), ST::format("xx{}xx", -1.5));
    EXPECT_EQ(ST::string("xx-1.5xx"), ST::format("xx{+}xx", -1.5));

    // Padding
    EXPECT_EQ(ST::string("xx  1.50xx"), ST::format("xx{6.2f}xx", 1.5));
    EXPECT_EQ(ST::string("xx -1.50xx"), ST::format("xx{6.2f}xx", -1.5));
    EXPECT_EQ(ST::string("xx1.50  xx"), ST::format("xx{<6.2f}xx", 1.5));
    EXPECT_EQ(ST::string("xx-1.50 xx"), ST::format("xx{<6.2f}xx", -1.5));

    // Fixed notation
    EXPECT_EQ(ST::string("xx3.14xx"), ST::format("xx{.2f}xx", 3.14159));
    EXPECT_EQ(ST::string("xx3.141590xx"), ST::format("xx{.6f}xx", 3.14159));
    EXPECT_EQ(ST::string("xx16384.00xx"), ST::format("xx{.2f}xx", 16384.0));
    EXPECT_EQ(ST::string("xx0.01xx"), ST::format("xx{.2f}xx", 1.0 / 128));

    // MSVC uses 3 digits for the exponent, whereas GCC uses two :/
#ifdef _MSC_VER
#   define EXTRA_DIGIT "0"
#else
#   define EXTRA_DIGIT ""
#endif

    // Scientific notation (MSVC uses 3 digits for the exponent)
    EXPECT_EQ(ST::string("xx3.14e+" EXTRA_DIGIT "00xx"), ST::format("xx{.2e}xx", 3.14159));
    EXPECT_EQ(ST::string("xx3.141590e+" EXTRA_DIGIT "00xx"), ST::format("xx{.6e}xx", 3.14159));
    EXPECT_EQ(ST::string("xx1.64e+" EXTRA_DIGIT "04xx"), ST::format("xx{.2e}xx", 16384.0));
    EXPECT_EQ(ST::string("xx7.81e-" EXTRA_DIGIT "03xx"), ST::format("xx{.2e}xx", 1.0 / 128));

    // Scientific notation (upper-case E)
    EXPECT_EQ(ST::string("xx3.14E+" EXTRA_DIGIT "00xx"), ST::format("xx{.2E}xx", 3.14159));
    EXPECT_EQ(ST::string("xx3.141590E+" EXTRA_DIGIT "00xx"), ST::format("xx{.6E}xx", 3.14159));
    EXPECT_EQ(ST::string("xx1.64E+" EXTRA_DIGIT "04xx"), ST::format("xx{.2E}xx", 16384.0));
    EXPECT_EQ(ST::string("xx7.81E-" EXTRA_DIGIT "03xx"), ST::format("xx{.2E}xx", 1.0 / 128));

    // Automatic (based on input)
    EXPECT_EQ(ST::string("xx3.1xx"), ST::format("xx{.2}xx", 3.14159));
    EXPECT_EQ(ST::string("xx3.14159xx"), ST::format("xx{.6}xx", 3.14159));
    EXPECT_EQ(ST::string("xx1.6e+" EXTRA_DIGIT "04xx"), ST::format("xx{.2}xx", 16384.0));
    EXPECT_EQ(ST::string("xx0.0078xx"), ST::format("xx{.2}xx", 1.0 / 128));
}

TEST(format, booleans)
{
    // This basically just uses the string formatter with constant strings
    EXPECT_EQ(ST::string("xxtrue xx"), ST::format("xx{5}xx", true));
    EXPECT_EQ(ST::string("xxfalsexx"), ST::format("xx{5}xx", false));
}
