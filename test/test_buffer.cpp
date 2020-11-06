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

#include "st_string.h"

#include <gtest/gtest.h>

namespace ST
{
    // Teach GTest how to print an ST::buffer
    static void PrintTo(const ST::char_buffer &str, std::ostream *os)
    {
        *os << "ST::char_buffer{\"" << str.data() << "\"}";
    }

    static void PrintTo(const ST::wchar_buffer &str, std::ostream *os)
    {
        ST::string u8str = str;
        *os << "ST::wchar_buffer{\"" << u8str.c_str() << "\"}";
    }

    static void PrintTo(const ST::utf16_buffer &str, std::ostream *os)
    {
        ST::string u8str = str;
        *os << "ST::utf16_buffer{\"" << u8str.c_str() << "\"}";
    }

    static void PrintTo(const ST::utf32_buffer &str, std::ostream *os)
    {
        ST::string u8str = str;
        *os << "ST::utf32_buffer{\"" << u8str.c_str() << "\"}";
    }
}

/* Utility for comparing char16_t/char32_t buffers */
template <typename char_T>
static int T_strcmp(const char_T *left, const char_T *right)
{
    for ( ;; ) {
        if (*left != *right)
            return *left - *right;
        if (*left == 0)
            return 0;

        ++left;
        ++right;
    }
}

TEST(char_buffer, helpers)
{
    /* Ensure the utilities for testing the module function properly */
    EXPECT_EQ(0, T_strcmp("abc", "abc"));
    EXPECT_LT(0, T_strcmp("abc", "aba"));
    EXPECT_GT(0, T_strcmp("abc", "abe"));
    EXPECT_LT(0, T_strcmp("abc", "ab"));
    EXPECT_GT(0, T_strcmp("abc", "abcd"));
    EXPECT_EQ(0, T_strcmp("", ""));
    EXPECT_GT(0, T_strcmp("", "a"));
    EXPECT_LT(0, T_strcmp("a", ""));
}

TEST(char_buffer, utility)
{
    // Literal constructors
    EXPECT_EQ(ST::char_buffer(), ST_CHAR_LITERAL(""));
    EXPECT_EQ(ST::wchar_buffer(), ST_WCHAR_LITERAL(""));
    EXPECT_EQ(ST::utf16_buffer(), ST_UTF16_LITERAL(""));
    EXPECT_EQ(ST::utf32_buffer(), ST_UTF32_LITERAL(""));
    EXPECT_EQ(ST::char_buffer("abc", 3), ST_CHAR_LITERAL("abc"));
    EXPECT_EQ(ST::wchar_buffer(L"abc", 3), ST_WCHAR_LITERAL("abc"));
    EXPECT_EQ(ST::utf16_buffer(u"abc", 3), ST_UTF16_LITERAL("abc"));
    EXPECT_EQ(ST::utf32_buffer(U"abc", 3), ST_UTF32_LITERAL("abc"));

    EXPECT_EQ(0U, ST::char_buffer().size());
    EXPECT_TRUE(ST::char_buffer().empty());
    EXPECT_EQ(0U, ST::wchar_buffer().size());
    EXPECT_TRUE(ST::wchar_buffer().empty());
    EXPECT_EQ(0U, ST::utf16_buffer().size());
    EXPECT_TRUE(ST::utf16_buffer().empty());
    EXPECT_EQ(0U, ST::utf32_buffer().size());
    EXPECT_TRUE(ST::utf32_buffer().empty());
}

TEST(char_buffer, stack_construction)
{
    // If these change, this test may need to be updated to match
    ASSERT_EQ(16, ST_MAX_SSO_LENGTH);
    ASSERT_EQ(48, ST_MAX_SSO_SIZE);

    char empty[] = {0};
    wchar_t emptyw[] = {0};
    char16_t empty16[] = {0};
    char32_t empty32[] = {0};

    // Stack allocated
    char shortstr[] = "Short";
    wchar_t shortw[] = L"Short";
    char16_t short16[] = {'S', 'h', 'o', 'r', 't', 0};
    char32_t short32[] = {'S', 'h', 'o', 'r', 't', 0};

    // Heap allocated
    char longstr[] = "0123456789abcdefghij";
    wchar_t longw[] = L"0123456789abcdefghij";
    char16_t long16[] = {'0','1','2','3','4','5','6','7','8','9',
                         'a','b','c','d','e','f','g','h','i','j',0};
    char32_t long32[] = {'0','1','2','3','4','5','6','7','8','9',
                         'a','b','c','d','e','f','g','h','i','j',0};

    ST::char_buffer cb_empty(empty, 0);
    EXPECT_EQ(0, T_strcmp(cb_empty.data(), empty));
    EXPECT_EQ(0U, cb_empty.size());
    EXPECT_TRUE(cb_empty.empty());
    ST::wchar_buffer wcb_empty(emptyw, 0);
    EXPECT_EQ(0, T_strcmp(wcb_empty.data(), emptyw));
    EXPECT_EQ(0U, wcb_empty.size());
    EXPECT_TRUE(wcb_empty.empty());
    ST::utf16_buffer cb16_empty(empty16, 0);
    EXPECT_EQ(0, T_strcmp(cb16_empty.data(), empty16));
    EXPECT_EQ(0U, cb16_empty.size());
    EXPECT_TRUE(cb16_empty.empty());
    ST::utf32_buffer cb32_empty(empty32, 0);
    EXPECT_EQ(0, T_strcmp(cb32_empty.data(), empty32));
    EXPECT_EQ(0U, cb32_empty.size());
    EXPECT_TRUE(cb32_empty.empty());

    ST::char_buffer cb_short(shortstr, 5);
    EXPECT_EQ(0, T_strcmp(cb_short.data(), shortstr));
    EXPECT_EQ(5U, cb_short.size());
    EXPECT_FALSE(cb_short.empty());
    ST::wchar_buffer wcb_short(shortw, 5);
    EXPECT_EQ(0, T_strcmp(wcb_short.data(), shortw));
    EXPECT_EQ(5U, wcb_short.size());
    EXPECT_FALSE(wcb_short.empty());
    ST::utf16_buffer cb16_short(short16, 5);
    EXPECT_EQ(0, T_strcmp(cb16_short.data(), short16));
    EXPECT_EQ(5U, cb16_short.size());
    EXPECT_FALSE(cb_short.empty());
    ST::utf32_buffer cb32_short(short32, 5);
    EXPECT_EQ(0, T_strcmp(cb32_short.data(), short32));
    EXPECT_EQ(5U, cb32_short.size());
    EXPECT_FALSE(cb32_short.empty());

    ST::char_buffer cb_long(longstr, 20);
    EXPECT_EQ(0, T_strcmp(cb_long.data(), longstr));
    EXPECT_EQ(20U, cb_long.size());
    EXPECT_FALSE(cb_long.empty());
    ST::wchar_buffer wcb_long(longw, 20);
    EXPECT_EQ(0, T_strcmp(wcb_long.data(), longw));
    EXPECT_EQ(20U, wcb_long.size());
    EXPECT_FALSE(wcb_long.empty());
    ST::utf16_buffer cb16_long(long16, 20);
    EXPECT_EQ(0, T_strcmp(cb16_long.data(), long16));
    EXPECT_EQ(20U, cb16_long.size());
    EXPECT_FALSE(cb_long.empty());
    ST::utf32_buffer cb32_long(long32, 20);
    EXPECT_EQ(0, T_strcmp(cb32_long.data(), long32));
    EXPECT_EQ(20U, cb32_long.size());
    EXPECT_FALSE(cb32_long.empty());
}

// Only testing char and wchar_t for simplicity on compilers that
// don't support utf-16 and utf-32 literals yet
TEST(char_buffer, copy)
{
    // If these change, this test may need to be updated to match
    ASSERT_EQ(16, ST_MAX_SSO_LENGTH);
    ASSERT_EQ(48, ST_MAX_SSO_SIZE);

    ST::char_buffer cb1("Test", 4);
    ST::wchar_buffer wcb1(L"Test", 4);

    ST::char_buffer dest(cb1);
    EXPECT_EQ(0, T_strcmp(dest.data(), "Test"));
    EXPECT_EQ(4U, dest.size());
    ST::wchar_buffer wdest(wcb1);
    EXPECT_EQ(0, T_strcmp(wdest.data(), L"Test"));
    EXPECT_EQ(4U, wdest.size());

    ST::char_buffer cb2("operator=", 9);
    ST::wchar_buffer wcb2(L"operator=", 9);

    dest = cb2;
    EXPECT_EQ(0, T_strcmp(dest.data(), "operator="));
    EXPECT_EQ(9U, dest.size());
    wdest = wcb2;
    EXPECT_EQ(0, T_strcmp(wdest.data(), L"operator="));
    EXPECT_EQ(9U, wdest.size());

    ST::char_buffer cb3("0123456789abcdefghij", 20);
    ST::wchar_buffer wcb3(L"0123456789abcdefghij", 20);

    ST::char_buffer dest2(cb3);
    EXPECT_EQ(0, T_strcmp(dest2.data(), "0123456789abcdefghij"));
    EXPECT_EQ(20U, dest2.size());
    ST::wchar_buffer wdest2(wcb3);
    EXPECT_EQ(0, T_strcmp(wdest2.data(), L"0123456789abcdefghij"));
    EXPECT_EQ(20U, wdest2.size());

    ST::char_buffer cb4("9876543210zyxwvutsrqponm", 24);
    ST::wchar_buffer wcb4(L"9876543210zyxwvutsrqponm", 24);

    dest2 = cb4;
    EXPECT_EQ(0, T_strcmp(dest2.data(), "9876543210zyxwvutsrqponm"));
    EXPECT_EQ(24U, dest2.size());
    wdest2 = wcb4;
    EXPECT_EQ(0, T_strcmp(wdest2.data(), L"9876543210zyxwvutsrqponm"));
    EXPECT_EQ(24U, wdest2.size());
}

TEST(char_buffer, move)
{
    // If these change, this test may need to be updated to match
    ASSERT_EQ(16, ST_MAX_SSO_LENGTH);
    ASSERT_EQ(48, ST_MAX_SSO_SIZE);

    ST::char_buffer cb1("Test", 4);
    ST::wchar_buffer wcb1(L"Test", 4);

    ST::char_buffer dest(std::move(cb1));
    EXPECT_EQ(0, T_strcmp(dest.data(), "Test"));
    EXPECT_EQ(4U, dest.size());
    ST::wchar_buffer wdest(std::move(wcb1));
    EXPECT_EQ(0, T_strcmp(wdest.data(), L"Test"));
    EXPECT_EQ(4U, wdest.size());

    ST::char_buffer cb2("operator=", 9);
    ST::wchar_buffer wcb2(L"operator=", 9);

    dest = std::move(cb2);
    EXPECT_EQ(0, T_strcmp(dest.data(), "operator="));
    EXPECT_EQ(9U, dest.size());
    wdest = std::move(wcb2);
    EXPECT_EQ(0, T_strcmp(wdest.data(), L"operator="));
    EXPECT_EQ(9U, wdest.size());

    ST::char_buffer cb3("0123456789abcdefghij", 20);
    ST::wchar_buffer wcb3(L"0123456789abcdefghij", 20);

    ST::char_buffer dest2(std::move(cb3));
    EXPECT_EQ(0, T_strcmp(dest2.data(), "0123456789abcdefghij"));
    EXPECT_EQ(20U, dest2.size());
    ST::wchar_buffer wdest2(std::move(wcb3));
    EXPECT_EQ(0, T_strcmp(wdest2.data(), L"0123456789abcdefghij"));
    EXPECT_EQ(20U, wdest2.size());

    ST::char_buffer cb4("9876543210zyxwvutsrqponm", 24);
    ST::wchar_buffer wcb4(L"9876543210zyxwvutsrqponm", 24);

    dest2 = std::move(cb4);
    EXPECT_EQ(0, T_strcmp(dest2.data(), "9876543210zyxwvutsrqponm"));
    EXPECT_EQ(24U, dest2.size());
    wdest2 = std::move(wcb4);
    EXPECT_EQ(0, T_strcmp(wdest2.data(), L"9876543210zyxwvutsrqponm"));
    EXPECT_EQ(24U, wdest2.size());
}

#if defined(__clang__) && ((__clang_major__ > 3) || (__clang_major__ == 3 && __clang_minor__ > 5))
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wself-move"
#   if defined(__clang__) && (__clang_major__ > 6)
#       pragma GCC diagnostic ignored "-Wself-assign-overloaded"
#   endif
#endif

TEST(char_buffer, self_assign)
{
    // If this changes, this test may need to be updated to match
    ASSERT_EQ(16, ST_MAX_SSO_LENGTH);

    ST::char_buffer sbuf;
    sbuf = sbuf;
    EXPECT_EQ(0, T_strcmp(sbuf.data(), ""));

    ST::char_buffer shortbuf("0123456789", 10);
    sbuf = shortbuf;
    EXPECT_EQ(0, T_strcmp(sbuf.data(), "0123456789"));
    sbuf = sbuf;
    EXPECT_EQ(0, T_strcmp(sbuf.data(), "0123456789"));
    sbuf = std::move(sbuf);
    // Content not guaranteed after self-move

    ST::char_buffer longbuf("0123456789abcdefghij", 20);
    sbuf = longbuf;
    EXPECT_EQ(0, T_strcmp(sbuf.data(), "0123456789abcdefghij"));
    sbuf = sbuf;
    EXPECT_EQ(0, T_strcmp(sbuf.data(), "0123456789abcdefghij"));
    sbuf = std::move(sbuf);
    // Content not guaranteed after self-move
}

#if defined(__clang__) && ((__clang_major__ > 3) || (__clang_major__ == 3 && __clang_minor__ > 5))
#   pragma GCC diagnostic pop
#endif

TEST(char_buffer, compare)
{
    // Same length, chars
    EXPECT_EQ(ST::char_buffer("abc", 3), ST::char_buffer("abc", 3));
    EXPECT_NE(ST::char_buffer("abc", 3), ST::char_buffer("abd", 3));
    EXPECT_NE(ST::char_buffer("abc", 3), ST::char_buffer("abb", 3));
    EXPECT_NE(ST::char_buffer("abC", 3), ST::char_buffer("abc", 3));
    EXPECT_NE(ST::char_buffer("Abc", 3), ST::char_buffer("abc", 3));
    EXPECT_EQ(ST::char_buffer("", 0), ST::char_buffer());

    // Same length, wchars
    EXPECT_EQ(ST::wchar_buffer(L"abc", 3), ST::wchar_buffer(L"abc", 3));
    EXPECT_NE(ST::wchar_buffer(L"abc", 3), ST::wchar_buffer(L"abd", 3));
    EXPECT_NE(ST::wchar_buffer(L"abc", 3), ST::wchar_buffer(L"abb", 3));
    EXPECT_NE(ST::wchar_buffer(L"abC", 3), ST::wchar_buffer(L"abc", 3));
    EXPECT_NE(ST::wchar_buffer(L"Abc", 3), ST::wchar_buffer(L"abc", 3));
    EXPECT_EQ(ST::wchar_buffer(L"", 0), ST::wchar_buffer());

    // Mismatched length, chars
    EXPECT_NE(ST::char_buffer("abc", 3), ST::char_buffer("ab", 2));
    EXPECT_NE(ST::char_buffer("abc", 3), ST::char_buffer("abcd", 4));
    EXPECT_NE(ST::char_buffer("abc", 3), ST::char_buffer("", 0));
    EXPECT_NE(ST::char_buffer(), ST::char_buffer("abc", 3));

    // Mismatched length, wchars
    EXPECT_NE(ST::wchar_buffer(L"abc", 3), ST::wchar_buffer(L"ab", 2));
    EXPECT_NE(ST::wchar_buffer(L"abc", 3), ST::wchar_buffer(L"abcd", 4));
    EXPECT_NE(ST::wchar_buffer(L"abc", 3), ST::wchar_buffer(L"", 0));
    EXPECT_NE(ST::wchar_buffer(), ST::wchar_buffer(L"abc", 3));
}
