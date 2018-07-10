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

#include "st_stringstream.h"

#include <gtest/gtest.h>

namespace ST
{
    // Teach GTest how to print an ST::string
    static void PrintTo(const ST::string &str, std::ostream *os)
    {
        *os << "ST::string{\"" << str.c_str() << "\"}";
    }
}

TEST(string_stream, empty)
{
    ST::string_stream ss;
    EXPECT_EQ(0U, ss.size());
    EXPECT_EQ(ST_LITERAL(""), ss.to_string());

    ss << "";
    EXPECT_EQ(0U, ss.size());
    EXPECT_EQ(ST_LITERAL(""), ss.to_string());

    ss.append("", 0);
    EXPECT_EQ(0U, ss.size());
    EXPECT_EQ(ST_LITERAL(""), ss.to_string());

    ss.append(ST_NULLPTR, 0);
    EXPECT_EQ(0U, ss.size());
    EXPECT_EQ(ST_LITERAL(""), ss.to_string());
}

TEST(string_stream, append)
{
    // Explicit length
    ST::string_stream ss;
    ss.append("aaaaaaaaaa", 5);
    EXPECT_EQ(5U, ss.size());
    EXPECT_EQ(ST_LITERAL("aaaaa"), ss.to_string());

    ss.append("", 0);
    EXPECT_EQ(5U, ss.size());
    EXPECT_EQ(ST_LITERAL("aaaaa"), ss.to_string());

    ss.append("bbbbbbbbbb", 10);
    EXPECT_EQ(15U, ss.size());
    EXPECT_EQ(ST_LITERAL("aaaaabbbbbbbbbb"), ss.to_string());

    // Automatic length
    ST::string_stream ss2;
    ss2.append("aaaaa");
    EXPECT_EQ(5U, ss2.size());
    EXPECT_EQ(ST_LITERAL("aaaaa"), ss2.to_string());

    ss2.append("");
    EXPECT_EQ(5U, ss2.size());
    EXPECT_EQ(ST_LITERAL("aaaaa"), ss2.to_string());

    ss2.append("bbbbbbbbbb");
    EXPECT_EQ(15U, ss2.size());
    EXPECT_EQ(ST_LITERAL("aaaaabbbbbbbbbb"), ss2.to_string());

    // Cause a heap allocation
    ST::string s1 = ST::string::fill(ST_STACK_STRING_LEN / 2, 'x');
    ST::string s2 = ST::string::fill(ST_STACK_STRING_LEN / 2, 'y');
    ST::string s3 = ST_LITERAL("z");
    ST::string_stream ss3;
    ss3.append(s1.c_str(), s1.size());
    ss3.append(s2.c_str(), s2.size());
    ss3.append(s3.c_str(), s3.size());
    EXPECT_EQ(s1.size() + s2.size() + s3.size(), ss3.size());
    EXPECT_EQ(s1 + s2 + s3, ss3.to_string());

    ST::string s4 = ST::string::fill(ST_STACK_STRING_LEN * 4, 'x');
    ST::string_stream ss4;
    ss4.append(s3.c_str(), s3.size());
    ss4.append(s4.c_str(), s4.size());
    EXPECT_EQ(s3.size() + s4.size(), ss4.size());
    EXPECT_EQ(s3 + s4, ss4.to_string());
}

TEST(string_stream, append_char)
{
    ST::string_stream ss;
    ss.append_char('x');
    EXPECT_EQ(1U, ss.size());
    EXPECT_EQ(ST_LITERAL("x"), ss.to_string());

    ss.append_char('y', 10);
    EXPECT_EQ(11U, ss.size());
    EXPECT_EQ(ST_LITERAL("xyyyyyyyyyy"), ss.to_string());

    // Cause a heap allocation
    ST::string_stream ss2;
    ss2.append_char('x', ST_STACK_STRING_LEN / 2);
    ss2.append_char('y', ST_STACK_STRING_LEN / 2);
    ss2.append_char('z');
    ST::string s1 = ST::string::fill(ST_STACK_STRING_LEN / 2, 'x');
    ST::string s2 = ST::string::fill(ST_STACK_STRING_LEN / 2, 'y');
    ST::string s3 = ST_LITERAL("z");
    EXPECT_EQ(s1.size() + s2.size() + s3.size(), ss2.size());
    EXPECT_EQ(s1 + s2 + s3, ss2.to_string());

    ST::string_stream ss3;
    ss3.append_char('z');
    ss3.append_char('x', ST_STACK_STRING_LEN * 4);
    ST::string s4 = ST::string::fill(ST_STACK_STRING_LEN * 4, 'x');
    EXPECT_EQ(s3.size() + s4.size(), ss3.size());
    EXPECT_EQ(s3 + s4, ss3.to_string());
}

TEST(string_stream, to_string)
{
    const char latin1[] = "\x20\x7e\xa0\xff";
    static const char utf8_test_data[] =
        "\x20"              "\x7f"
        "\xc3\xbf"          "\xc4\x80"
        "\xe7\xbf\xbf"
        "\xef\xbf\xbf"      "\xf0\x90\x80\x80"
        "\xf0\x90\x80\xa0"  "\xf1\x80\x80\x80"
        "\xf4\x8f\xbf\xbf";

    ST::string_stream ss;
    ss.append(latin1);
    EXPECT_EQ(ST::string::from_latin_1(latin1), ss.to_string(false));

    ST::string_stream ss2;
    ss2.append(utf8_test_data);
    EXPECT_EQ(ST::string::from_utf8(utf8_test_data), ss2.to_string(true));
}

TEST(string_stream, cat_strings)
{
    // C strings
    ST::string_stream ss;
    ss << "aaaaa";
    EXPECT_EQ(5U, ss.size());
    EXPECT_EQ(ST_LITERAL("aaaaa"), ss.to_string());

    ss << "";
    EXPECT_EQ(5U, ss.size());
    EXPECT_EQ(ST_LITERAL("aaaaa"), ss.to_string());

    ss << "bbbbbbbbbb";
    EXPECT_EQ(15U, ss.size());
    EXPECT_EQ(ST_LITERAL("aaaaabbbbbbbbbb"), ss.to_string());

    // ST::string
    ST::string_stream ss2;
    ss2 << ST_LITERAL("aaaaa");
    EXPECT_EQ(5U, ss2.size());
    EXPECT_EQ(ST_LITERAL("aaaaa"), ss2.to_string());

    ss2 << ST::string();
    EXPECT_EQ(5U, ss2.size());
    EXPECT_EQ(ST_LITERAL("aaaaa"), ss2.to_string());

    ss2 << ST_LITERAL("bbbbbbbbbb");
    EXPECT_EQ(15U, ss2.size());
    EXPECT_EQ(ST_LITERAL("aaaaabbbbbbbbbb"), ss2.to_string());

    // Cause a heap allocation
    ST::string s1 = ST::string::fill(ST_STACK_STRING_LEN / 2, 'x');
    ST::string s2 = ST::string::fill(ST_STACK_STRING_LEN / 2, 'y');
    ST::string s3 = ST_LITERAL("z");
    ST::string_stream ss3;
    ss3 << s1 << s2 << s3;
    EXPECT_EQ(s1.size() + s2.size() + s3.size(), ss3.size());
    EXPECT_EQ(s1 + s2 + s3, ss3.to_string());

    ST::string s4 = ST::string::fill(ST_STACK_STRING_LEN * 4, 'x');
    ST::string_stream ss4;
    ss4 << s3 << s4;
    EXPECT_EQ(s3.size() + s4.size(), ss4.size());
    EXPECT_EQ(s3 + s4, ss4.to_string());

    // Append char
    ST::string_stream ss5;
    ss5 << s1 << s2 << 'z';
    EXPECT_EQ(s1.size() + s2.size() + 1, ss5.size());
    EXPECT_EQ(s1 + s2 + 'z', ss5.to_string());

    ST::string_stream ss6;
    ss6 << 'z' << s4;
    EXPECT_EQ(1 + s4.size(), ss6.size());
    EXPECT_EQ('z' + s4, ss6.to_string());
}

TEST(string_stream, cat_ints)
{
    EXPECT_EQ(ST_LITERAL("0"), (ST::string_stream() << 0).to_string());
    EXPECT_EQ(ST_LITERAL("-80000"),(ST::string_stream() << -80000).to_string());
    EXPECT_EQ(ST_LITERAL("80000"),(ST::string_stream() << 80000).to_string());

    EXPECT_EQ(ST_LITERAL("0"), (ST::string_stream() << 0L).to_string());
    EXPECT_EQ(ST_LITERAL("-1000000"), (ST::string_stream() << -1000000L).to_string());
    EXPECT_EQ(ST_LITERAL("1000000"), (ST::string_stream() << 1000000L).to_string());

    EXPECT_EQ(ST_LITERAL("0"), (ST::string_stream() << 0LL).to_string());
    EXPECT_EQ(ST_LITERAL("-1000000000000"), (ST::string_stream() << -1000000000000LL).to_string());
    EXPECT_EQ(ST_LITERAL("1000000000000"), (ST::string_stream() << 1000000000000LL).to_string());

    static const int int32_min = std::numeric_limits<int>::min();
    static const int int32_max = std::numeric_limits<int>::max();
    EXPECT_EQ(ST_LITERAL("-2147483648"), (ST::string_stream() << int32_min).to_string());
    EXPECT_EQ(ST_LITERAL("2147483647"), (ST::string_stream() << int32_max).to_string());

    static const long long int64_min = std::numeric_limits<long long>::min();
    static const long long int64_max = std::numeric_limits<long long>::max();
    EXPECT_EQ(ST_LITERAL("-9223372036854775808"), (ST::string_stream() << int64_min).to_string());
    EXPECT_EQ(ST_LITERAL("9223372036854775807"), (ST::string_stream() << int64_max).to_string());
}

TEST(string_stream, cat_uints)
{
    EXPECT_EQ(ST_LITERAL("0"), (ST::string_stream() << 0U).to_string());
    EXPECT_EQ(ST_LITERAL("80000"),(ST::string_stream() << 80000U).to_string());

    EXPECT_EQ(ST_LITERAL("0"), (ST::string_stream() << 0UL).to_string());
    EXPECT_EQ(ST_LITERAL("1000000"), (ST::string_stream() << 1000000UL).to_string());

    EXPECT_EQ(ST_LITERAL("0"), (ST::string_stream() << 0ULL).to_string());
    EXPECT_EQ(ST_LITERAL("1000000000000"), (ST::string_stream() << 1000000000000ULL).to_string());

    static const unsigned int uint32_max = std::numeric_limits<unsigned int>::max();
    EXPECT_EQ(ST_LITERAL("4294967295"), (ST::string_stream() << uint32_max).to_string());

    static const unsigned long long uint64_max = std::numeric_limits<unsigned long long>::max();
    EXPECT_EQ(ST_LITERAL("18446744073709551615"), (ST::string_stream() << uint64_max).to_string());
}

TEST(string_stream, cat_floats)
{
    EXPECT_EQ(ST_LITERAL("0"), (ST::string_stream() << 0.0f).to_string());
    EXPECT_EQ(ST_LITERAL("0"), (ST::string_stream() << 0.0).to_string());

    EXPECT_EQ(ST_LITERAL("-16"), (ST::string_stream() << -16.0f).to_string());
    EXPECT_EQ(ST_LITERAL("16"), (ST::string_stream() << 16.0f).to_string());
    EXPECT_EQ(ST_LITERAL("1.6"), (ST::string_stream() << 1.6f).to_string());
    EXPECT_EQ(ST_LITERAL("16384.5"), (ST::string_stream() << 16384.5f).to_string());
    EXPECT_EQ(ST_LITERAL("0.0078"), (ST::string_stream() << 0.0078f).to_string());

    EXPECT_EQ(ST_LITERAL("-16"), (ST::string_stream() << -16.0).to_string());
    EXPECT_EQ(ST_LITERAL("16"), (ST::string_stream() << 16.0).to_string());
    EXPECT_EQ(ST_LITERAL("1.6"), (ST::string_stream() << 1.6).to_string());
    EXPECT_EQ(ST_LITERAL("16384.5"), (ST::string_stream() << 16384.5).to_string());
    EXPECT_EQ(ST_LITERAL("0.0078"), (ST::string_stream() << 0.0078).to_string());

    // Special values (Different CRTs have very different ways of representing
    // infinity and NaN textually :( )
    EXPECT_TRUE((ST::string_stream() << std::numeric_limits<float>::infinity())
                    .to_string().find("inf", ST::case_insensitive) >= 0);
    EXPECT_TRUE((ST::string_stream() << std::numeric_limits<double>::infinity())
                    .to_string().find("inf", ST::case_insensitive) >= 0);
    EXPECT_TRUE((ST::string_stream() << std::numeric_limits<float>::quiet_NaN())
                    .to_string().find("nan", ST::case_insensitive) >= 0);
    EXPECT_TRUE((ST::string_stream() << std::numeric_limits<float>::quiet_NaN())
                    .to_string().find("nan", ST::case_insensitive) >= 0);
}

TEST(string_stream, truncate)
{
    ST::string_stream ss;
    ss << ST_LITERAL("aaaaa");
    ss << ST_LITERAL("bbbbbbbbbb");
    ss.truncate(10);
    EXPECT_EQ(10U, ss.size());
    EXPECT_EQ(ST_LITERAL("aaaaabbbbb"), ss.to_string());

    ss.truncate();
    EXPECT_EQ(0U, ss.size());
    EXPECT_EQ(ST::string(), ss.to_string());

    ss << ST_LITERAL("ccccc");
    ss << ST_LITERAL("dddddddddd");
    EXPECT_EQ(15U, ss.size());
    EXPECT_EQ(ST_LITERAL("cccccdddddddddd"), ss.to_string());

    ss.truncate(30);
    EXPECT_EQ(15U, ss.size());
    EXPECT_EQ(ST_LITERAL("cccccdddddddddd"), ss.to_string());
}

TEST(string_stream, erase)
{
    ST::string_stream ss;
    ss << ST_LITERAL("aaaaa");
    ss << ST_LITERAL("bbbbbbbbbb");
    ss.erase(5);
    EXPECT_EQ(10U, ss.size());
    EXPECT_EQ(ST_LITERAL("aaaaabbbbb"), ss.to_string());

    ss.erase(10);
    EXPECT_EQ(0U, ss.size());
    EXPECT_EQ(ST::string(), ss.to_string());

    ss << ST_LITERAL("ccccc");
    ss << ST_LITERAL("dddddddddd");
    EXPECT_EQ(15U, ss.size());
    EXPECT_EQ(ST_LITERAL("cccccdddddddddd"), ss.to_string());

    ss.erase(30);
    EXPECT_EQ(0U, ss.size());
    EXPECT_EQ(ST::string(), ss.to_string());
}
