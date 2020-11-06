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
#include <wchar.h>
#include <cmath>
#include <limits>
#include <iostream>

#ifdef _MSC_VER
#   pragma warning(disable: 4996)
#endif

namespace ST
{
    // Teach GTest how to print an ST::string
    static void PrintTo(const ST::string &str, std::ostream *os)
    {
        *os << "ST::string{\"" << str.c_str() << "\"}";
    }
}

template <typename T, size_t Size>
constexpr size_t text_size(const T (&)[Size]) { return Size - 1; }

static const char32_t test_data[] = {
    0x20,       0x7f,       /* Normal ASCII chars */
    0xff,       0x100,      /* 8-bit boundary chars */
    0x7fff,                 /* UTF-8 2-byte boundary */
    0xffff,     0x10000,    /* 16-bit boundary chars */
    0x10020,    0x40000,    /* Non-edge UTF-16 surrogate pairs */
    0x10ffff,               /* Highest Unicode character */
    0                       /* Null terminator */
};

/* UTF-8 version of above test data */
static const char utf8_test_data[] =
    "\x20"              "\x7f"
    "\xc3\xbf"          "\xc4\x80"
    "\xe7\xbf\xbf"
    "\xef\xbf\xbf"      "\xf0\x90\x80\x80"
    "\xf0\x90\x80\xa0"  "\xf1\x80\x80\x80"
    "\xf4\x8f\xbf\xbf";

/* UTF-16 version of test data */
static const char16_t utf16_test_data[] = {
    0x20, 0x7f,
    0xff, 0x100,
    0x7fff,
    0xffff,
    /* surrogate pairs for chars >0xffff */
    0xd800, 0xdc00,
    0xd800, 0xdc20,
    0xd8c0, 0xdc00,
    0xdbff, 0xdfff,
    0
};

/* wide version of test data */
static const wchar_t wide_test_data[] =
    L"\x20"         L"\x7f"
    L"\xff"         L"\u0100"
    L"\u7fff"
    L"\uffff"       L"\U00010000"
    L"\U00010020"   L"\U00040000"
    L"\U0010FFFF";

/* Latin-1 test data */
static const char latin1_data[] = "\x20\x7e\xa0\xff";
static const char latin1_utf8[] = "\x20\x7e\xc2\xa0\xc3\xbf";
static const char16_t latin1_utf16[] = { 0x20, 0x7e, 0xa0, 0xff, 0 };
static const char32_t latin1_utf32[] = { 0x20, 0x7e, 0xa0, 0xff, 0 };
static const wchar_t latin1_wide[] = L"\x20\x7e\u00a0\u00ff";

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

TEST(string, helpers)
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

TEST(string, utility)
{

    EXPECT_EQ(0U, ST::string().size());
    EXPECT_TRUE(ST::string().empty());

    // Short and Long string length
    EXPECT_EQ(4U, ST_LITERAL("1234").size());
    EXPECT_EQ(15U, ST_LITERAL("123456789012345").size());
    EXPECT_EQ(16U, ST_LITERAL("1234567890123456").size());
    EXPECT_EQ(32U, ST_LITERAL("12345678901234567890123456789012").size());

    // ST::string stores data as UTF-8 internally
    EXPECT_EQ(text_size(utf8_test_data), ST::string(utf8_test_data).size());
}

TEST(string, stack_construction)
{
    char stack_buf[256];
    strcpy(stack_buf, "Test");
    ST::string test(stack_buf);

    EXPECT_EQ(ST_LITERAL("Test"), test);
    EXPECT_EQ(strlen("Test"), test.size());

    wchar_t wstack_buf[256];
    wcscpy(wstack_buf, L"Test");
    ST::string wtest(wstack_buf);

    EXPECT_EQ(ST::string(L"Test"), wtest);
    EXPECT_EQ(strlen("Test"), wtest.size());

    strcpy(stack_buf, "operator=");
    test = stack_buf;

    EXPECT_EQ(ST_LITERAL("operator="), test);
    EXPECT_EQ(strlen("operator="), test.size());

    wcscpy(wstack_buf, L"operator=");
    wtest = wstack_buf;

    EXPECT_EQ(ST::string(L"operator="), wtest);
    EXPECT_EQ(strlen("operator="), wtest.size());
}

TEST(string, copy)
{
    // If this changes, this test may need to be updated to match
    ASSERT_EQ(16, ST_MAX_SSO_LENGTH);

    ST::string s1("Test");
    ST::string dest(s1);
    EXPECT_EQ(ST_LITERAL("Test"), dest);
    EXPECT_EQ(4U, dest.size());

    ST::string s2("operator=");
    dest = s2;
    EXPECT_EQ(ST_LITERAL("operator="), dest);
    EXPECT_EQ(9U, dest.size());

    ST::string s3("0123456789abcdefghij");
    ST::string dest2(s3);
    EXPECT_EQ(ST_LITERAL("0123456789abcdefghij"), dest2);
    EXPECT_EQ(20U, dest2.size());

    ST::string s4("9876543210zyxwvutsrqponm");
    dest2 = s4;
    EXPECT_EQ(ST_LITERAL("9876543210zyxwvutsrqponm"), dest2);
    EXPECT_EQ(24U, dest2.size());
}

TEST(string, move)
{
    // If this changes, this test may need to be updated to match
    ASSERT_EQ(16, ST_MAX_SSO_LENGTH);

    ST::string s1("Test");
    ST::string dest(std::move(s1));
    EXPECT_EQ(ST_LITERAL("Test"), dest);
    EXPECT_EQ(4U, dest.size());

    ST::string s2("operator=");
    dest = std::move(s2);
    EXPECT_EQ(ST_LITERAL("operator="), dest);
    EXPECT_EQ(9U, dest.size());

    ST::string s3("0123456789abcdefghij");
    ST::string dest2(std::move(s3));
    EXPECT_EQ(ST_LITERAL("0123456789abcdefghij"), dest2);
    EXPECT_EQ(20U, dest2.size());

    ST::string s4("9876543210zyxwvutsrqponm");
    dest2 = std::move(s4);
    EXPECT_EQ(ST_LITERAL("9876543210zyxwvutsrqponm"), dest2);
    EXPECT_EQ(24U, dest2.size());
}

TEST(string, conv_utf8)
{
    // From UTF-16 to UTF-8
    ST::char_buffer from_utf16 = ST::utf16_to_utf8(utf16_test_data, text_size(utf16_test_data),
                                                   ST::check_validity);
    EXPECT_EQ(text_size(utf8_test_data), from_utf16.size());
    EXPECT_EQ(0, T_strcmp(utf8_test_data, from_utf16.data()));

    // From UTF-32 to UTF-8
    ST::char_buffer from_utf32 = ST::utf32_to_utf8(test_data, text_size(test_data),
                                                   ST::check_validity);
    EXPECT_EQ(text_size(utf8_test_data), from_utf32.size());
    EXPECT_EQ(0, T_strcmp(utf8_test_data, from_utf32.data()));

    // From Wide to UTF-8
    ST::char_buffer from_wide = ST::wchar_to_utf8(wide_test_data, text_size(wide_test_data),
                                                  ST::check_validity);
    EXPECT_EQ(text_size(utf8_test_data), from_wide.size());
    EXPECT_EQ(0, T_strcmp(utf8_test_data, from_wide.data()));

    // From Latin-1 to UTF-8
    ST::char_buffer from_latin_1 = ST::latin_1_to_utf8(latin1_data, text_size(latin1_data));
    EXPECT_EQ(text_size(latin1_utf8), from_latin_1.size());
    EXPECT_EQ(0, T_strcmp(latin1_utf8, from_latin_1.data()));
}

TEST(string, string_utf8)
{
    // From UTF-8 to ST::string
    ST::string from_utf8 = ST::string::from_utf8(utf8_test_data);
    EXPECT_STREQ(utf8_test_data, from_utf8.c_str());
    EXPECT_EQ(text_size(utf8_test_data), from_utf8.size());
    ST::utf32_buffer unicode = from_utf8.to_utf32();
    EXPECT_EQ(0, T_strcmp(test_data, unicode.data()));

    // From ST::string to UTF-8
    ST::string to_utf8 = ST::string::from_utf32(test_data);
    EXPECT_STREQ(utf8_test_data, to_utf8.c_str());

    // Empty strings
    ST::string empty = ST::string::from_utf8("");
    EXPECT_EQ(0U, empty.size());
    EXPECT_EQ(0, T_strcmp(empty.c_str(), ""));

    const char32_t empty_data[] = { 0 };
    empty = ST::string::from_utf32(empty_data);
    EXPECT_EQ(0U, empty.size());
    EXPECT_EQ(0, T_strcmp(empty.c_str(), ""));
}

TEST(string, conv_utf16)
{
    // From UTF-8 to UTF-16
    ST::utf16_buffer from_utf8 = ST::utf8_to_utf16(utf8_test_data, text_size(utf8_test_data),
                                                   ST::check_validity);
    EXPECT_EQ(text_size(utf16_test_data), from_utf8.size());
    EXPECT_EQ(0, T_strcmp(utf16_test_data, from_utf8.data()));

    // From UTF-32 to UTF-16
    ST::utf16_buffer from_utf32 = ST::utf32_to_utf16(test_data, text_size(test_data),
                                                     ST::check_validity);
    EXPECT_EQ(text_size(utf16_test_data), from_utf32.size());
    EXPECT_EQ(0, T_strcmp(utf16_test_data, from_utf32.data()));

    // From Wide to UTF-16
    ST::utf16_buffer from_wide = ST::wchar_to_utf16(wide_test_data, text_size(wide_test_data),
                                                    ST::check_validity);
    EXPECT_EQ(text_size(utf16_test_data), from_wide.size());
    EXPECT_EQ(0, T_strcmp(utf16_test_data, from_wide.data()));

    // From Latin-1 to UTF-16
    ST::utf16_buffer from_latin_1 = ST::latin_1_to_utf16(latin1_data, text_size(latin1_data));
    EXPECT_EQ(text_size(latin1_utf16), from_latin_1.size());
    EXPECT_EQ(0, T_strcmp(latin1_utf16, from_latin_1.data()));
}

TEST(string, string_utf16)
{
    // From UTF-16 to ST::string
    ST::string from_utf16 = ST::string::from_utf16(utf16_test_data);
    EXPECT_EQ(text_size(utf8_test_data), from_utf16.size());
    ST::utf32_buffer unicode = from_utf16.to_utf32();
    EXPECT_EQ(0, T_strcmp(test_data, unicode.data()));

    // From ST::string to UTF-16
    ST::utf16_buffer to_utf16 = ST::string::from_utf32(test_data).to_utf16();
    EXPECT_EQ(0, T_strcmp(utf16_test_data, to_utf16.data()));

    // Empty string
    const char16_t empty_data[] = { 0 };
    ST::string empty = ST::string::from_utf16(empty_data);
    EXPECT_EQ(0U, empty.size());
    EXPECT_EQ(0, T_strcmp(empty.c_str(), ""));
}

TEST(string, conv_utf32)
{
    // From UTF-8 to UTF-32
    ST::utf32_buffer from_utf8 = ST::utf8_to_utf32(utf8_test_data, text_size(utf8_test_data),
                                                   ST::check_validity);
    EXPECT_EQ(text_size(test_data), from_utf8.size());
    EXPECT_EQ(0, T_strcmp(test_data, from_utf8.data()));

    // From UTF-16 to UTF-32
    ST::utf32_buffer from_utf32 = ST::utf16_to_utf32(utf16_test_data, text_size(utf16_test_data),
                                                     ST::check_validity);
    EXPECT_EQ(text_size(test_data), from_utf32.size());
    EXPECT_EQ(0, T_strcmp(test_data, from_utf32.data()));

    // From Wide to UTF-32
    ST::utf32_buffer from_wide = ST::wchar_to_utf32(wide_test_data, text_size(wide_test_data),
                                                    ST::check_validity);
    EXPECT_EQ(text_size(test_data), from_wide.size());
    EXPECT_EQ(0, T_strcmp(test_data, from_wide.data()));

    // From Latin-1 to UTF-32
    ST::utf32_buffer from_latin_1 = ST::latin_1_to_utf32(latin1_data, text_size(latin1_data));
    EXPECT_EQ(text_size(latin1_utf32), from_latin_1.size());
    EXPECT_EQ(0, T_strcmp(latin1_utf32, from_latin_1.data()));
}

TEST(string, conv_latin_1)
{
    // From UTF-8 to Latin-1
    ST::char_buffer from_utf8 = ST::utf8_to_latin_1(latin1_utf8, text_size(latin1_utf8),
                                                    ST::check_validity);
    EXPECT_EQ(text_size(latin1_data), from_utf8.size());
    EXPECT_EQ(0, T_strcmp(latin1_data, from_utf8.data()));

    // From UTF-16 to Latin-1
    ST::char_buffer from_utf16 = ST::utf16_to_latin_1(latin1_utf16, text_size(latin1_utf16),
                                                      ST::check_validity);
    EXPECT_EQ(text_size(latin1_data), from_utf16.size());
    EXPECT_EQ(0, T_strcmp(latin1_data, from_utf16.data()));

    // From UTF-32 to Latin-1
    ST::char_buffer from_latin_1 = ST::utf32_to_latin_1(latin1_utf32, text_size(latin1_utf32),
                                                        ST::check_validity);
    EXPECT_EQ(text_size(latin1_data), from_latin_1.size());
    EXPECT_EQ(0, T_strcmp(latin1_data, from_latin_1.data()));

    // From Wide to Latin-1
    ST::char_buffer from_wide = ST::wchar_to_latin_1(latin1_wide, text_size(latin1_wide),
                                                     ST::check_validity);
    EXPECT_EQ(text_size(latin1_data), from_wide.size());
    EXPECT_EQ(0, T_strcmp(latin1_data, from_wide.data()));
}

TEST(string, string_latin_1)
{
    // From Latin-1 to ST::string
    const char latin1[] = "\x20\x7e\xa0\xff";
    const char32_t unicode_cp0[] = { 0x20, 0x7e, 0xa0, 0xff, 0 };
    static const size_t latin1_utf8_length = 6;
    ST::string from_latin1 = ST::string::from_latin_1(latin1);
    EXPECT_EQ(latin1_utf8_length, from_latin1.size());
    ST::utf32_buffer unicode = from_latin1.to_utf32();
    EXPECT_EQ(0, T_strcmp(unicode_cp0, unicode.data()));

    // From ST::string to Latin-1
    ST::char_buffer to_latin1 = ST::string::from_utf32(unicode_cp0).to_latin_1();
    EXPECT_STREQ(latin1, to_latin1.data());

    // Empty string
    ST::string empty = ST::string::from_latin_1("");
    EXPECT_EQ(0U, empty.size());
    EXPECT_EQ(0, T_strcmp(empty.c_str(), ""));
}

TEST(string, conv_wchar)
{
    // From UTF-8 to Wide
    ST::wchar_buffer from_utf8 = ST::utf8_to_wchar(utf8_test_data, text_size(utf8_test_data),
                                                   ST::check_validity);
    EXPECT_EQ(text_size(wide_test_data), from_utf8.size());
    EXPECT_EQ(0, T_strcmp(wide_test_data, from_utf8.data()));

    // From UTF-16 to Wide
    ST::wchar_buffer from_utf16 = ST::utf16_to_wchar(utf16_test_data, text_size(utf16_test_data),
                                                     ST::check_validity);
    EXPECT_EQ(text_size(wide_test_data), from_utf16.size());
    EXPECT_EQ(0, T_strcmp(wide_test_data, from_utf16.data()));

    // From UTF-32 to Wide
    ST::wchar_buffer from_utf32 = ST::utf32_to_wchar(test_data, text_size(test_data),
                                                     ST::check_validity);
    EXPECT_EQ(text_size(wide_test_data), from_utf32.size());
    EXPECT_EQ(0, T_strcmp(wide_test_data, from_utf32.data()));

    // From Latin-1 to Wide
    ST::wchar_buffer from_latin_1 = ST::latin_1_to_wchar(latin1_data, text_size(latin1_data));
    EXPECT_EQ(text_size(latin1_wide), from_latin_1.size());
    EXPECT_EQ(0, T_strcmp(latin1_wide, from_latin_1.data()));
}

TEST(string, string_wchar)
{
    // UTF-8 and UTF-16 are already tested, so just make sure we test
    // wchar_t and L"" conversions

    const wchar_t wtext[] = L"\x20\x7f\xff\u0100\ufffe";
    const char32_t unicode_text[] = { 0x20, 0x7f, 0xff, 0x100, 0xfffe, 0 };
    static const size_t wtext_utf8_length = 9;
    ST::string from_wchar = ST::string::from_wchar(wtext);
    EXPECT_EQ(wtext_utf8_length, from_wchar.size());
    ST::utf32_buffer unicode = from_wchar.to_utf32();
    EXPECT_EQ(0, T_strcmp(unicode_text, unicode.data()));

    // From ST::string to wchar_t
    ST::wchar_buffer to_wchar = ST::string::from_utf32(unicode_text).to_wchar();
    EXPECT_STREQ(wtext, to_wchar.data());

    // Empty string
    ST::string empty = ST::string::from_wchar(L"");
    EXPECT_EQ(0U, empty.size());
    EXPECT_EQ(0, T_strcmp(empty.c_str(), ""));
}

TEST(string, validation)
{
    // Truncated sequences
    EXPECT_THROW({ (void)ST::string::from_utf8("\xC0", ST_AUTO_SIZE, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::string::from_utf8("\xE0", ST_AUTO_SIZE, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::string::from_utf8("\xE0\x80", ST_AUTO_SIZE, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::string::from_utf8("\xF0", ST_AUTO_SIZE, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::string::from_utf8("\xF0\x80", ST_AUTO_SIZE, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::string::from_utf8("\xF0\x80\x80", ST_AUTO_SIZE, ST::check_validity); }, ST::unicode_error);

    const auto replacement = ST_LITERAL("\xef\xbf\xbdx");
    const auto replacement2 = ST_LITERAL("\xef\xbf\xbd\xef\xbf\xbdx");
    const auto replacement3 = ST_LITERAL("\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbdx");
    EXPECT_EQ(replacement, ST::string::from_utf8("\xC0x", ST_AUTO_SIZE, ST::substitute_invalid));
    EXPECT_EQ(replacement, ST::string::from_utf8("\xE0x", ST_AUTO_SIZE, ST::substitute_invalid));
    EXPECT_EQ(replacement2, ST::string::from_utf8("\xE0\x80x", ST_AUTO_SIZE, ST::substitute_invalid));
    EXPECT_EQ(replacement, ST::string::from_utf8("\xF0x", ST_AUTO_SIZE, ST::substitute_invalid));
    EXPECT_EQ(replacement2, ST::string::from_utf8("\xF0\x80x", ST_AUTO_SIZE, ST::substitute_invalid));
    EXPECT_EQ(replacement3, ST::string::from_utf8("\xF0\x80\x80x", ST_AUTO_SIZE, ST::substitute_invalid));

    // Invalid sequences
    EXPECT_THROW({ (void)ST::string::from_utf8("\x80", ST_AUTO_SIZE, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::string::from_utf8("\xC0x", ST_AUTO_SIZE, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::string::from_utf8("\xE0xx", ST_AUTO_SIZE, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::string::from_utf8("\xF0xxx", ST_AUTO_SIZE, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::string::from_utf8("\xF8\x80\x80\x80\x80", ST_AUTO_SIZE, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::string::from_utf8("\xFC\x80\x80\x80\x80\x80", ST_AUTO_SIZE, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::string::from_utf8("\xFE\x80\x80\x80\x80\x80\x80", ST_AUTO_SIZE, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::string::from_utf8("\xFF\x80\x80\x80\x80\x80\x80\x80", ST_AUTO_SIZE, ST::check_validity); }, ST::unicode_error);

    const auto replacement2x = ST_LITERAL("\xef\xbf\xbdxx");
    const auto replacement3x = ST_LITERAL("\xef\xbf\xbdxxx");
    const auto replacement4 = ST_LITERAL("\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbdx");
    const auto replacement4x = ST_LITERAL("\xef\xbf\xbdxxxx");
    EXPECT_EQ(replacement, ST::string::from_utf8("\x80x", ST_AUTO_SIZE, ST::substitute_invalid));
    EXPECT_EQ(replacement, ST::string::from_utf8("\xC0x", ST_AUTO_SIZE, ST::substitute_invalid));
    EXPECT_EQ(replacement2x, ST::string::from_utf8("\xE0xx", ST_AUTO_SIZE, ST::substitute_invalid));
    EXPECT_EQ(replacement3x, ST::string::from_utf8("\xF0xxx", ST_AUTO_SIZE, ST::substitute_invalid));
    EXPECT_EQ(replacement4, ST::string::from_utf8("\xF8\x80\x80\x80\x80x", ST_AUTO_SIZE, ST::substitute_invalid));
    EXPECT_EQ(replacement4x, ST::string::from_utf8("\xF8xxxx", ST_AUTO_SIZE, ST::substitute_invalid));

    // Pass through bad data from ST_LITERAL and ST::assume_valid
    const char junk[] = "\xFCxx\x80xx";
    EXPECT_EQ(0, T_strcmp(junk, ST_LITERAL("\xFCxx\x80xx").c_str()));
    EXPECT_EQ(0, T_strcmp(junk, ST::string::from_utf8(junk, ST_AUTO_SIZE, ST::assume_valid).c_str()));
}

TEST(string, conv_utf8_validation)
{
    const char16_t truncL[] = { 0xd800, 'x', 0 };
    const char16_t truncH[] = { 0xdc00, 'x', 0 };
    EXPECT_THROW({
        (void)ST::utf16_to_utf8(truncL, text_size(truncL),
                                ST::check_validity);
    }, ST::unicode_error);
    EXPECT_THROW({
        (void)ST::utf16_to_utf8(truncH, text_size(truncH),
                                ST::check_validity);
    }, ST::unicode_error);
    EXPECT_EQ(0, T_strcmp("\xef\xbf\xbdx", ST::utf16_to_utf8(truncL, text_size(truncL),
                                                             ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp("\xef\xbf\xbdx", ST::utf16_to_utf8(truncH, text_size(truncH),
                                                             ST::substitute_invalid).c_str()));

    const char16_t incompleteL[] = { 0xd800, 'x', 'x', 0 };
    const char16_t incompleteH[] = { 0xdc00, 'x', 'x', 0 };
    EXPECT_THROW({
        (void)ST::utf16_to_utf8(incompleteL, text_size(incompleteL),
                                ST::check_validity);
    }, ST::unicode_error);
    EXPECT_THROW({
        (void)ST::utf16_to_utf8(incompleteH, text_size(incompleteH),
                                ST::check_validity);
    }, ST::unicode_error);
    EXPECT_EQ(0, T_strcmp("\xef\xbf\xbdxx", ST::utf16_to_utf8(incompleteL, text_size(incompleteL),
                                                              ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp("\xef\xbf\xbdxx", ST::utf16_to_utf8(incompleteH, text_size(incompleteH),
                                                              ST::substitute_invalid).c_str()));

    const char16_t doubleL[] = { 0xd800, 0xdbff, 'x', 0 };
    const char16_t doubleH[] = { 0xdc00, 0xdfff, 'x', 0 };
    EXPECT_THROW({
        (void)ST::utf16_to_utf8(doubleL, text_size(doubleL),
                                ST::check_validity);
    }, ST::unicode_error);
    EXPECT_THROW({
        (void)ST::utf16_to_utf8(doubleH, text_size(doubleH),
                                ST::check_validity);
    }, ST::unicode_error);
    EXPECT_EQ(0, T_strcmp("\xef\xbf\xbd\xef\xbf\xbdx",
                          ST::utf16_to_utf8(doubleL, text_size(doubleL),
                                            ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp("\xef\xbf\xbd\xef\xbf\xbdx",
                          ST::utf16_to_utf8(doubleH, text_size(doubleH),
                                            ST::substitute_invalid).c_str()));

    // Out of range UTF-32
    const char32_t range32[] = { 0x110000, 'x', 0 };
    EXPECT_THROW({
        (void)ST::utf32_to_utf8(range32, text_size(range32),
                                ST::check_validity);
    }, ST::unicode_error);
    EXPECT_EQ(0, T_strcmp("\xef\xbf\xbdx", ST::utf32_to_utf8(range32, text_size(range32),
                                                             ST::substitute_invalid).c_str()));
}

TEST(string, conv_utf16_validation)
{
    // Truncated UTF-8 sequences
    EXPECT_THROW({ (void)ST::utf8_to_utf16("\xC0", 1, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_utf16("\xE0", 1, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_utf16("\xE0\x80", 2, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_utf16("\xF0", 1, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_utf16("\xF0\x80", 2, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_utf16("\xF0\x80\x80", 3, ST::check_validity); }, ST::unicode_error);

    EXPECT_EQ(0, T_strcmp(u"\ufffdx", ST::utf8_to_utf16("\xC0x", 2, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp(u"\ufffdx", ST::utf8_to_utf16("\xE0x", 2, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp(u"\ufffd\ufffdx", ST::utf8_to_utf16("\xE0\x80x", 3, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp(u"\ufffdx", ST::utf8_to_utf16("\xF0x", 2, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp(u"\ufffd\ufffdx", ST::utf8_to_utf16("\xF0\x80x", 3, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp(u"\ufffd\ufffd\ufffdx", ST::utf8_to_utf16("\xF0\x80\x80x", 4, ST::substitute_invalid).c_str()));

    // Invalid UTF-8 sequences
    EXPECT_THROW({ (void)ST::utf8_to_utf16("\x80", 1, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_utf16("\xC0x", 2, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_utf16("\xE0xx", 3, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_utf16("\xF0xxx", 4, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_utf16("\xF8\x80\x80\x80\x80", 5, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_utf16("\xFC\x80\x80\x80\x80\x80", 6, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_utf16("\xFE\x80\x80\x80\x80\x80\x80", 7, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_utf16("\xFF\x80\x80\x80\x80\x80\x80\x80", 8, ST::check_validity); }, ST::unicode_error);

    EXPECT_EQ(0, T_strcmp(u"\ufffdx", ST::utf8_to_utf16("\x80x", 2, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp(u"\ufffdx", ST::utf8_to_utf16("\xC0x", 2, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp(u"\ufffdxx", ST::utf8_to_utf16("\xE0xx", 3, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp(u"\ufffdxxx", ST::utf8_to_utf16("\xF0xxx", 4, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp(u"\ufffd\ufffd\ufffd\ufffd\ufffdx",
                          ST::utf8_to_utf16("\xF8\x80\x80\x80\x80x", 6, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp(u"\ufffdxxxx", ST::utf8_to_utf16("\xF8xxxx", 5, ST::substitute_invalid).c_str()));

    // Out of range UTF-32
    const char32_t range32[] = { 0x110000, 'x', 0 };
    EXPECT_THROW({
        (void)ST::utf32_to_utf16(range32, text_size(range32),
                                 ST::check_validity);
    }, ST::unicode_error);
    EXPECT_EQ(0, T_strcmp(u"\ufffdx", ST::utf32_to_utf16(range32, text_size(range32),
                                                          ST::substitute_invalid).c_str()));
}

TEST(string, conv_utf32_validation)
{
    // Truncated UTF-8 sequences
    EXPECT_THROW({ (void)ST::utf8_to_utf32("\xC0", 1, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_utf32("\xE0", 1, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_utf32("\xE0\x80", 2, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_utf32("\xF0", 1, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_utf32("\xF0\x80", 2, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_utf32("\xF0\x80\x80", 3, ST::check_validity); }, ST::unicode_error);

    EXPECT_EQ(0, T_strcmp(U"\ufffdx", ST::utf8_to_utf32("\xC0x", 2, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp(U"\ufffdx", ST::utf8_to_utf32("\xE0x", 2, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp(U"\ufffd\ufffdx", ST::utf8_to_utf32("\xE0\x80x", 3, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp(U"\ufffdx", ST::utf8_to_utf32("\xF0x", 2, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp(U"\ufffd\ufffdx", ST::utf8_to_utf32("\xF0\x80x", 3, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp(U"\ufffd\ufffd\ufffdx", ST::utf8_to_utf32("\xF0\x80\x80x", 4, ST::substitute_invalid).c_str()));

    // Invalid UTF-8 sequences
    EXPECT_THROW({ (void)ST::utf8_to_utf32("\x80", 1, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_utf32("\xC0x", 2, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_utf32("\xE0xx", 3, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_utf32("\xF0xxx", 4, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_utf32("\xF8\x80\x80\x80\x80", 5, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_utf32("\xFC\x80\x80\x80\x80\x80", 6, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_utf32("\xFE\x80\x80\x80\x80\x80\x80", 7, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_utf32("\xFF\x80\x80\x80\x80\x80\x80\x80", 8, ST::check_validity); }, ST::unicode_error);

    EXPECT_EQ(0, T_strcmp(U"\ufffdx", ST::utf8_to_utf32("\x80x", 2, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp(U"\ufffdx", ST::utf8_to_utf32("\xC0x", 2, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp(U"\ufffdxx", ST::utf8_to_utf32("\xE0xx", 3, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp(U"\ufffdxxx", ST::utf8_to_utf32("\xF0xxx", 4, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp(U"\ufffd\ufffd\ufffd\ufffd\ufffdx",
                          ST::utf8_to_utf32("\xF8\x80\x80\x80\x80x", 6, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp(U"\ufffdxxxx", ST::utf8_to_utf32("\xF8xxxx", 5, ST::substitute_invalid).c_str()));

    // Bad UTF-16 sequences
    const char16_t truncL[] = { 0xd800, 'x', 0 };
    const char16_t truncH[] = { 0xdc00, 'x', 0 };
    EXPECT_THROW({
        (void)ST::utf16_to_utf32(truncL, text_size(truncL),
                                 ST::check_validity);
    }, ST::unicode_error);
    EXPECT_THROW({
        (void)ST::utf16_to_utf32(truncH, text_size(truncH),
                                 ST::check_validity);
    }, ST::unicode_error);
    EXPECT_EQ(0, T_strcmp(U"\ufffdx", ST::utf16_to_utf32(truncL, text_size(truncL),
                                                         ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp(U"\ufffdx", ST::utf16_to_utf32(truncH, text_size(truncH),
                                                         ST::substitute_invalid).c_str()));

    const char16_t incompleteL[] = { 0xd800, 'x', 'x', 0 };
    const char16_t incompleteH[] = { 0xdc00, 'x', 'x', 0 };
    EXPECT_THROW({
        (void)ST::utf16_to_utf32(incompleteL, text_size(incompleteL),
                                 ST::check_validity);
    }, ST::unicode_error);
    EXPECT_THROW({
        (void)ST::utf16_to_utf32(incompleteH, text_size(incompleteH),
                                 ST::check_validity);
    }, ST::unicode_error);
    EXPECT_EQ(0, T_strcmp(U"\ufffdxx", ST::utf16_to_utf32(incompleteL, text_size(incompleteL),
                                                          ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp(U"\ufffdxx", ST::utf16_to_utf32(incompleteH, text_size(incompleteH),
                                                          ST::substitute_invalid).c_str()));

    const char16_t doubleL[] = { 0xd800, 0xdbff, 'x', 0 };
    const char16_t doubleH[] = { 0xdc00, 0xdfff, 'x', 0 };
    EXPECT_THROW({
        (void)ST::utf16_to_utf32(doubleL, text_size(doubleL),
                                 ST::check_validity);
    }, ST::unicode_error);
    EXPECT_THROW({
        (void)ST::utf16_to_utf32(doubleH, text_size(doubleH),
                                 ST::check_validity);
    }, ST::unicode_error);
    EXPECT_EQ(0, T_strcmp(U"\ufffd\ufffdx", ST::utf16_to_utf32(doubleL, text_size(doubleL),
                                                               ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp(U"\ufffd\ufffdx", ST::utf16_to_utf32(doubleH, text_size(doubleH),
                                                               ST::substitute_invalid).c_str()));
}

TEST(string, conv_latin_1_validation)
{
    // Truncated UTF-8 sequences
    EXPECT_THROW({ (void)ST::utf8_to_latin_1("\xC0", 1, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_latin_1("\xE0", 1, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_latin_1("\xE0\x80", 2, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_latin_1("\xF0", 1, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_latin_1("\xF0\x80", 2, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_latin_1("\xF0\x80\x80", 3, ST::check_validity); }, ST::unicode_error);

    EXPECT_EQ(0, T_strcmp("?x", ST::utf8_to_latin_1("\xC0x", 2, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp("?x", ST::utf8_to_latin_1("\xE0x", 2, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp("??x", ST::utf8_to_latin_1("\xE0\x80x", 3, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp("?x", ST::utf8_to_latin_1("\xF0x", 2, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp("??x", ST::utf8_to_latin_1("\xF0\x80x", 3, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp("???x", ST::utf8_to_latin_1("\xF0\x80\x80x", 4, ST::substitute_invalid).c_str()));

    // Invalid UTF-8 sequences
    EXPECT_THROW({ (void)ST::utf8_to_latin_1("\x80", 1, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_latin_1("\xC0x", 2, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_latin_1("\xE0xx", 3, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_latin_1("\xF0xxx", 4, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_latin_1("\xF8\x80\x80\x80\x80", 5, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_latin_1("\xFC\x80\x80\x80\x80\x80", 6, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_latin_1("\xFE\x80\x80\x80\x80\x80\x80", 7, ST::check_validity); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_latin_1("\xFF\x80\x80\x80\x80\x80\x80\x80", 8, ST::check_validity); }, ST::unicode_error);

    EXPECT_EQ(0, T_strcmp("?x", ST::utf8_to_latin_1("\x80x", 2, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp("?x", ST::utf8_to_latin_1("\xC0x", 2, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp("?xx", ST::utf8_to_latin_1("\xE0xx", 3, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp("?xxx", ST::utf8_to_latin_1("\xF0xxx", 4, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp("?????x", ST::utf8_to_latin_1("\xF8\x80\x80\x80\x80x", 6, ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp("?xxxx", ST::utf8_to_latin_1("\xF8xxxx", 5, ST::substitute_invalid).c_str()));

    // Out of Latin-1 range
    EXPECT_THROW({ (void)ST::utf8_to_latin_1("\xc4\x80", 2, ST::check_validity, false); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf8_to_latin_1("\xf4\x8f\xbf\xbf", 4, ST::check_validity, false); }, ST::unicode_error);
    EXPECT_EQ(0, T_strcmp("?x", ST::utf8_to_latin_1("\xc4\x80x", 3, ST::check_validity, true).c_str()));
    EXPECT_EQ(0, T_strcmp("?x", ST::utf8_to_latin_1("\xf4\x8f\xbf\xbfx", 5, ST::check_validity, true).c_str()));

    // Bad UTF-16 sequences
    const char16_t truncL[] = { 0xd800, 'x', 0 };
    const char16_t truncH[] = { 0xdc00, 'x', 0 };
    EXPECT_THROW({
        (void)ST::utf16_to_latin_1(truncL, text_size(truncL),
                                   ST::check_validity);
    }, ST::unicode_error);
    EXPECT_THROW({
        (void)ST::utf16_to_latin_1(truncH, text_size(truncH),
                                   ST::check_validity);
    }, ST::unicode_error);
    EXPECT_EQ(0, T_strcmp("?x", ST::utf16_to_latin_1(truncL, text_size(truncL),
                                                     ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp("?x", ST::utf16_to_latin_1(truncH, text_size(truncH),
                                                     ST::substitute_invalid).c_str()));

    const char16_t incompleteL[] = { 0xd800, 'x', 'x', 0 };
    const char16_t incompleteH[] = { 0xdc00, 'x', 'x', 0 };
    EXPECT_THROW({
        (void)ST::utf16_to_latin_1(incompleteL, text_size(incompleteL),
                                   ST::check_validity);
    }, ST::unicode_error);
    EXPECT_THROW({
        (void)ST::utf16_to_latin_1(incompleteH, text_size(incompleteH),
                                   ST::check_validity);
    }, ST::unicode_error);
    EXPECT_EQ(0, T_strcmp("?xx", ST::utf16_to_latin_1(incompleteL, text_size(incompleteL),
                                                      ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp("?xx", ST::utf16_to_latin_1(incompleteH, text_size(incompleteH),
                                                      ST::substitute_invalid).c_str()));

    const char16_t doubleL[] = { 0xd800, 0xdbff, 'x', 0 };
    const char16_t doubleH[] = { 0xdc00, 0xdfff, 'x', 0 };
    EXPECT_THROW({
        (void)ST::utf16_to_latin_1(doubleL, text_size(doubleL),
                                   ST::check_validity);
    }, ST::unicode_error);
    EXPECT_THROW({
        (void)ST::utf16_to_latin_1(doubleH, text_size(doubleH),
                                   ST::check_validity);
    }, ST::unicode_error);
    EXPECT_EQ(0, T_strcmp("??x", ST::utf16_to_latin_1(doubleL, text_size(doubleL),
                                                      ST::substitute_invalid).c_str()));
    EXPECT_EQ(0, T_strcmp("??x", ST::utf16_to_latin_1(doubleH, text_size(doubleH),
                                                      ST::substitute_invalid).c_str()));

    // Out of Latin-1 range
    EXPECT_THROW({ (void)ST::utf16_to_latin_1(u"\u0100", 1, ST::check_validity, false); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf16_to_latin_1(u"\U0010ffff", 2, ST::check_validity, false); }, ST::unicode_error);
    EXPECT_EQ(0, T_strcmp("?x", ST::utf16_to_latin_1(u"\u0100x", 2, ST::check_validity, true).c_str()));
    EXPECT_EQ(0, T_strcmp("?x", ST::utf16_to_latin_1(u"\U0010ffffx", 3, ST::check_validity, true).c_str()));

    // Out of range UTF-32
    const char32_t range32[] = { 0x110000, 'x', 0 };
    EXPECT_THROW({
        (void)ST::utf32_to_latin_1(range32, text_size(range32),
                                   ST::check_validity);
    }, ST::unicode_error);
    EXPECT_EQ(0, T_strcmp("?x", ST::utf32_to_latin_1(range32, text_size(range32),
                                                     ST::substitute_invalid).c_str()));

    // Out of Latin-1 range
    EXPECT_THROW({ (void)ST::utf32_to_latin_1(U"\u0100", 1, ST::check_validity, false); }, ST::unicode_error);
    EXPECT_THROW({ (void)ST::utf32_to_latin_1(U"\U0010ffff", 1, ST::check_validity, false); }, ST::unicode_error);
    EXPECT_EQ(0, T_strcmp("?x", ST::utf32_to_latin_1(U"\u0100x", 2, ST::check_validity, true).c_str()));
    EXPECT_EQ(0, T_strcmp("?x", ST::utf32_to_latin_1(U"\U0010ffffx", 2, ST::check_validity, true).c_str()));
}

TEST(string, conversion_errors)
{
    // The following should encode replacement characters for invalid chars
    const char32_t unicode_replacement[] = { 0xfffd, 0 };
    const char latin1_replacement[] = "?";

    // Character outside of Unicode specification range
    const char32_t too_big_c[] = { 0xffffff, 0 };
    EXPECT_THROW({ (void)ST::string::from_utf32(too_big_c, ST_AUTO_SIZE, ST::check_validity); },
                 ST::unicode_error);
    ST::utf32_buffer too_big = ST::string::from_utf32(too_big_c, ST_AUTO_SIZE,
                                                      ST::substitute_invalid).to_utf32();
    EXPECT_EQ(0, T_strcmp(unicode_replacement, too_big.data()));

    // Invalid surrogate pairs
    const char16_t incomplete_surr_c[] = { 0xd800, 0 };
    EXPECT_THROW({ (void)ST::string::from_utf16(incomplete_surr_c, ST_AUTO_SIZE, ST::check_validity); },
                 ST::unicode_error);
    ST::string incomplete_surr = ST::string::from_utf16(incomplete_surr_c, ST_AUTO_SIZE,
                                                        ST::substitute_invalid);
    EXPECT_EQ(0, T_strcmp(unicode_replacement, incomplete_surr.to_utf32().data()));

    const char32_t unicode_replacement2[] = { 0xfffd, 0xfffd, 0 };
    const char16_t double_low_c[] = { 0xd800, 0xd801, 0 };
    EXPECT_THROW({ (void)ST::string::from_utf16(double_low_c, ST_AUTO_SIZE, ST::check_validity); },
                 ST::unicode_error);
    ST::string double_low = ST::string::from_utf16(double_low_c, ST_AUTO_SIZE,
                                                   ST::substitute_invalid);
    EXPECT_EQ(0, T_strcmp(unicode_replacement2, double_low.to_utf32().data()));

    const char16_t double_high_c[] = { 0xdc00, 0xdc01, 0 };
    EXPECT_THROW({ (void)ST::string::from_utf16(double_high_c, ST_AUTO_SIZE, ST::check_validity); },
                 ST::unicode_error);
    ST::string double_high = ST::string::from_utf16(double_high_c, ST_AUTO_SIZE,
                                                    ST::substitute_invalid);
    EXPECT_EQ(0, T_strcmp(unicode_replacement2, double_high.to_utf32().data()));

    const char32_t unicode_replacement3[] = { 0xfffd, 0x20, 0 };
    const char16_t bad_combo_c[] = { 0xdc00, 0x20, 0 };
    EXPECT_THROW({ (void)ST::string::from_utf16(bad_combo_c, ST_AUTO_SIZE, ST::check_validity); },
                 ST::unicode_error);
    ST::string bad_combo = ST::string::from_utf16(bad_combo_c, ST_AUTO_SIZE,
                                                  ST::substitute_invalid);
    EXPECT_EQ(0, T_strcmp(unicode_replacement3, bad_combo.to_utf32().data()));

    // Latin-1 doesn't have \ufffd, so it uses '?' instead
    const char32_t non_latin1_c[] = { 0x1ff, 0 };
    EXPECT_THROW({ (void)ST::string::from_utf32(non_latin1_c).to_latin_1(false); }, ST::unicode_error);
    ST::char_buffer non_latin1 = ST::string::from_utf32(non_latin1_c).to_latin_1();
    EXPECT_STREQ(latin1_replacement, non_latin1.data());
}

TEST(string, concatenation)
{
    // If this changes, this test may need to be updated to match
    ASSERT_EQ(16, ST_MAX_SSO_LENGTH);

    ST::string expected_short = "xxxxyyy";
    ST::string input1 = "xxxx";
    ST::string input2 = "yyy";

    ST::string expected_med = "xxxxxxxxxxyyyyyyyyy";
    ST::string input3 = "xxxxxxxxxx";
    ST::string input4 = "yyyyyyyyy";

    ST::string expected_long = "xxxxxxxxxxxxxxxyyyyyyyyyyyyyyyy";
    ST::string expected_long2 = "yyyyyyyyyyyyyyyyxxxxxxxxxxxxxxx";
    ST::string input5 = "xxxxxxxxxxxxxxx";
    ST::string input6 = "yyyyyyyyyyyyyyyy";

    // ST::string + ST::string
    EXPECT_EQ(expected_short, input1 + input2);
    EXPECT_EQ(expected_med, input3 + input4);
    EXPECT_EQ(expected_long, input5 + input6);
    EXPECT_EQ(expected_long2, input6 + input5);
    EXPECT_EQ(input1, input1 + ST::string());
    EXPECT_EQ(input1, ST::string() + input1);
    EXPECT_EQ(input6, input6 + ST::string());
    EXPECT_EQ(input6, ST::string() + input6);

    // ST::string + const char*
    EXPECT_EQ(expected_short, input1 + input2.c_str());
    EXPECT_EQ(expected_short, input1.c_str() + input2);
    EXPECT_EQ(expected_med, input3 + input4.c_str());
    EXPECT_EQ(expected_med, input3.c_str() + input4);
    EXPECT_EQ(expected_long, input5 + input6.c_str());
    EXPECT_EQ(expected_long, input5.c_str() + input6);
    EXPECT_EQ(expected_long2, input6 + input5.c_str());
    EXPECT_EQ(expected_long2, input6.c_str() + input5);
    EXPECT_EQ(input1, input1 + "");
    EXPECT_EQ(input1, "" + input1);
    EXPECT_EQ(input6, input6 + "");
    EXPECT_EQ(input6, "" + input6);
}

TEST(string, char_concatenation)
{
    // If this changes, this test may need to be updated to match
    ASSERT_EQ(16, ST_MAX_SSO_LENGTH);

    ST::string input1 = "xxxx";
    ST::string input2 = "xxxxxxxxxxxxxxx";
    ST::string input3 = "xxxxxxxxxxxxxxxx";

    // ST::string + char
    EXPECT_EQ(ST_LITERAL("xxxxy"), input1 + 'y');
    EXPECT_EQ(ST_LITERAL("xxxxxxxxxxxxxxxy"), input2 + 'y');
    EXPECT_EQ(ST_LITERAL("xxxxxxxxxxxxxxxxy"), input3 + 'y');
    EXPECT_EQ(ST_LITERAL("yxxxx"), 'y' + input1);
    EXPECT_EQ(ST_LITERAL("yxxxxxxxxxxxxxxx"), 'y' + input2);
    EXPECT_EQ(ST_LITERAL("yxxxxxxxxxxxxxxxx"), 'y' + input3);

    EXPECT_EQ(ST::string(L"xxxx\u00ff"), input1 + char(0xff));
    EXPECT_EQ(ST::string(L"xxxxxxxxxxxxxxx\u00ff"), input2 + char(0xff));
    EXPECT_EQ(ST::string(L"xxxxxxxxxxxxxxxx\u00ff"), input3 + char(0xff));
    EXPECT_EQ(ST::string(L"\u00ffxxxx"), char(0xff) + input1);
    EXPECT_EQ(ST::string(L"\u00ffxxxxxxxxxxxxxxx"), char(0xff) + input2);
    EXPECT_EQ(ST::string(L"\u00ffxxxxxxxxxxxxxxxx"), char(0xff) + input3);

    // ST::string + char16_t
    EXPECT_EQ(ST::string(L"xxxx\u00ff"), input1 + char16_t(0xff));
    EXPECT_EQ(ST::string(L"xxxxxxxxxxxxxxx\u00ff"), input2 + char16_t(0xff));
    EXPECT_EQ(ST::string(L"xxxxxxxxxxxxxxxx\u00ff"), input3 + char16_t(0xff));
    EXPECT_EQ(ST::string(L"\u00ffxxxx"), char16_t(0xff) + input1);
    EXPECT_EQ(ST::string(L"\u00ffxxxxxxxxxxxxxxx"), char16_t(0xff) + input2);
    EXPECT_EQ(ST::string(L"\u00ffxxxxxxxxxxxxxxxx"), char16_t(0xff) + input3);

    EXPECT_EQ(ST::string(L"xxxx\u0100"), input1 + char16_t(0x100));
    EXPECT_EQ(ST::string(L"xxxxxxxxxxxxxxx\u0100"), input2 + char16_t(0x100));
    EXPECT_EQ(ST::string(L"xxxxxxxxxxxxxxxx\u0100"), input3 + char16_t(0x100));
    EXPECT_EQ(ST::string(L"\u0100xxxx"), char16_t(0x100) + input1);
    EXPECT_EQ(ST::string(L"\u0100xxxxxxxxxxxxxxx"), char16_t(0x100) + input2);
    EXPECT_EQ(ST::string(L"\u0100xxxxxxxxxxxxxxxx"), char16_t(0x100) + input3);

    const char32_t expect_wide1[] = { 0x78, 0x78, 0x78, 0x78, 0x10FFFF, 0 };
    const char32_t expect_wide2[] = {
        0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78,
        0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78,
        0x10FFFF, 0
    };
    const char32_t expect_wide3[] = {
        0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78,
        0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78,
        0x10FFFF, 0
    };

    const char32_t expect_wide4[] = { 0x10FFFF, 0x78, 0x78, 0x78, 0x78, 0 };
    const char32_t expect_wide5[] = {
        0x10FFFF,
        0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78,
        0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0
    };
    const char32_t expect_wide6[] = {
        0x10FFFF,
        0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78,
        0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0
    };

    // ST::string + char32_t
    EXPECT_EQ(ST::string::from_utf32(expect_wide1), input1 + char32_t(0x10ffff));
    EXPECT_EQ(ST::string::from_utf32(expect_wide2), input2 + char32_t(0x10ffff));
    EXPECT_EQ(ST::string::from_utf32(expect_wide3), input3 + char32_t(0x10ffff));
    EXPECT_EQ(ST::string::from_utf32(expect_wide4), char32_t(0x10ffff) + input1);
    EXPECT_EQ(ST::string::from_utf32(expect_wide5), char32_t(0x10ffff) + input2);
    EXPECT_EQ(ST::string::from_utf32(expect_wide6), char32_t(0x10ffff) + input3);

    // UTF-16 and UTF-32 are already tested, so just check the conversion from wchar_t
    EXPECT_EQ(ST::string(L"xxxx\u0100"), input1 + wchar_t(0x100));
    EXPECT_EQ(ST::string(L"xxxxxxxxxxxxxxx\u0100"), input2 + wchar_t(0x100));
    EXPECT_EQ(ST::string(L"xxxxxxxxxxxxxxxx\u0100"), input3 + wchar_t(0x100));
    EXPECT_EQ(ST::string(L"\u0100xxxx"), wchar_t(0x100) + input1);
    EXPECT_EQ(ST::string(L"\u0100xxxxxxxxxxxxxxx"), wchar_t(0x100) + input2);
    EXPECT_EQ(ST::string(L"\u0100xxxxxxxxxxxxxxxx"), wchar_t(0x100) + input3);
}

TEST(string, from_int)
{
    EXPECT_EQ(ST_LITERAL("0"), ST::string::from_int(0));
    EXPECT_EQ(ST_LITERAL("-80000"), ST::string::from_int(-80000));
    EXPECT_EQ(ST_LITERAL("80000"), ST::string::from_int(80000));
    EXPECT_EQ(ST_LITERAL("-13880"), ST::string::from_int(-80000, 16));
    EXPECT_EQ(ST_LITERAL("13880"), ST::string::from_int(80000, 16));
    EXPECT_EQ(ST_LITERAL("-234200"), ST::string::from_int(-80000, 8));
    EXPECT_EQ(ST_LITERAL("234200"), ST::string::from_int(80000, 8));

    EXPECT_EQ(ST_LITERAL("0"), ST::string::from_int(0LL));
    EXPECT_EQ(ST_LITERAL("-1000000000000"), ST::string::from_int(-1000000000000LL));
    EXPECT_EQ(ST_LITERAL("1000000000000"), ST::string::from_int(1000000000000LL));
    EXPECT_EQ(ST_LITERAL("-e8d4a51000"), ST::string::from_int(-1000000000000LL, 16));
    EXPECT_EQ(ST_LITERAL("e8d4a51000"), ST::string::from_int(1000000000000LL, 16));
    EXPECT_EQ(ST_LITERAL("-16432451210000"), ST::string::from_int(-1000000000000LL, 8));
    EXPECT_EQ(ST_LITERAL("16432451210000"), ST::string::from_int(1000000000000LL, 8));

    static const int int32_min = std::numeric_limits<int>::min();
    static const int int32_max = std::numeric_limits<int>::max();
    EXPECT_EQ(ST_LITERAL("-2147483648"), ST::string::from_int(int32_min));
    EXPECT_EQ(ST_LITERAL("2147483647"), ST::string::from_int(int32_max));
    EXPECT_EQ(ST_LITERAL("-80000000"), ST::string::from_int(int32_min, 16));
    EXPECT_EQ(ST_LITERAL("7fffffff"), ST::string::from_int(int32_max, 16, false));
    EXPECT_EQ(ST_LITERAL("7FFFFFFF"), ST::string::from_int(int32_max, 16, true));
    EXPECT_EQ(ST_LITERAL("-20000000000"), ST::string::from_int(int32_min, 8));
    EXPECT_EQ(ST_LITERAL("17777777777"), ST::string::from_int(int32_max, 8));
    EXPECT_EQ(ST_LITERAL("-10000000000000000000000000000000"), ST::string::from_int(int32_min, 2));
    EXPECT_EQ(ST_LITERAL("1111111111111111111111111111111"), ST::string::from_int(int32_max, 2));

    static const long long int64_min = std::numeric_limits<long long>::min();
    static const long long int64_max = std::numeric_limits<long long>::max();
    EXPECT_EQ(ST_LITERAL("-9223372036854775808"), ST::string::from_int(int64_min));
    EXPECT_EQ(ST_LITERAL("9223372036854775807"), ST::string::from_int(int64_max));
    EXPECT_EQ(ST_LITERAL("-8000000000000000"), ST::string::from_int(int64_min, 16));
    EXPECT_EQ(ST_LITERAL("7fffffffffffffff"), ST::string::from_int(int64_max, 16, false));
    EXPECT_EQ(ST_LITERAL("7FFFFFFFFFFFFFFF"), ST::string::from_int(int64_max, 16, true));
    EXPECT_EQ(ST_LITERAL("-1000000000000000000000"), ST::string::from_int(int64_min, 8));
    EXPECT_EQ(ST_LITERAL("777777777777777777777"), ST::string::from_int(int64_max, 8));
    EXPECT_EQ(ST_LITERAL("-1000000000000000000000000000000000000000000000000000000000000000"),
              ST::string::from_int(int64_min, 2));
    EXPECT_EQ(ST_LITERAL("111111111111111111111111111111111111111111111111111111111111111"),
              ST::string::from_int(int64_max, 2));
}

TEST(string, from_uint)
{
    EXPECT_EQ(ST_LITERAL("0"), ST::string::from_uint(0U));
    EXPECT_EQ(ST_LITERAL("80000"), ST::string::from_uint(80000U));
    EXPECT_EQ(ST_LITERAL("13880"), ST::string::from_uint(80000U, 16));
    EXPECT_EQ(ST_LITERAL("234200"), ST::string::from_uint(80000U, 8));

    EXPECT_EQ(ST_LITERAL("0"), ST::string::from_uint(0ULL));
    EXPECT_EQ(ST_LITERAL("1000000000000"), ST::string::from_uint(1000000000000ULL));
    EXPECT_EQ(ST_LITERAL("e8d4a51000"), ST::string::from_uint(1000000000000ULL, 16));
    EXPECT_EQ(ST_LITERAL("16432451210000"), ST::string::from_uint(1000000000000ULL, 8));

    static const unsigned int uint32_max = std::numeric_limits<unsigned int>::max();
    EXPECT_EQ(ST_LITERAL("4294967295"), ST::string::from_uint(uint32_max));
    EXPECT_EQ(ST_LITERAL("ffffffff"), ST::string::from_uint(uint32_max, 16, false));
    EXPECT_EQ(ST_LITERAL("FFFFFFFF"), ST::string::from_uint(uint32_max, 16, true));
    EXPECT_EQ(ST_LITERAL("37777777777"), ST::string::from_uint(uint32_max, 8));
    EXPECT_EQ(ST_LITERAL("11111111111111111111111111111111"), ST::string::from_uint(uint32_max, 2));

    static const unsigned long long uint64_max = std::numeric_limits<unsigned long long>::max();
    EXPECT_EQ(ST_LITERAL("18446744073709551615"), ST::string::from_uint(uint64_max));
    EXPECT_EQ(ST_LITERAL("ffffffffffffffff"), ST::string::from_uint(uint64_max, 16, false));
    EXPECT_EQ(ST_LITERAL("FFFFFFFFFFFFFFFF"), ST::string::from_uint(uint64_max, 16, true));
    EXPECT_EQ(ST_LITERAL("1777777777777777777777"), ST::string::from_uint(uint64_max, 8));
    EXPECT_EQ(ST_LITERAL("1111111111111111111111111111111111111111111111111111111111111111"),
              ST::string::from_uint(uint64_max, 2));
}

TEST(string, from_float)
{
    EXPECT_EQ(ST_LITERAL("0"), ST::string::from_float(0.0f));
    EXPECT_EQ(ST_LITERAL("0"), ST::string::from_double(0.0));

    EXPECT_EQ(ST_LITERAL("-16"), ST::string::from_float(-16.0f));
    EXPECT_EQ(ST_LITERAL("16"), ST::string::from_float(16.0f));
    EXPECT_EQ(ST_LITERAL("1.6"), ST::string::from_float(1.6f));
    EXPECT_EQ(ST_LITERAL("16384.5"), ST::string::from_float(16384.5f));
    EXPECT_EQ(ST_LITERAL("0.0078"), ST::string::from_float(0.0078f));

    EXPECT_EQ(ST_LITERAL("-16"), ST::string::from_double(-16.0));
    EXPECT_EQ(ST_LITERAL("16"), ST::string::from_double(16.0));
    EXPECT_EQ(ST_LITERAL("1.6"), ST::string::from_double(1.6));
    EXPECT_EQ(ST_LITERAL("16384.5"), ST::string::from_double(16384.5));
    EXPECT_EQ(ST_LITERAL("0.0078"), ST::string::from_double(0.0078));

    // Special values (Different CRTs have very different ways of representing
    // infinity and NaN textually :( )
    EXPECT_TRUE(ST::string::from_float(std::numeric_limits<float>::infinity())
                .find("inf", ST::case_insensitive) >= 0);
    EXPECT_TRUE(ST::string::from_double(std::numeric_limits<double>::infinity())
                .find("inf", ST::case_insensitive) >= 0);
    EXPECT_TRUE(ST::string::from_float(std::numeric_limits<float>::quiet_NaN())
                .find("nan", ST::case_insensitive) >= 0);
    EXPECT_TRUE(ST::string::from_double(std::numeric_limits<float>::quiet_NaN())
                .find("nan", ST::case_insensitive) >= 0);
}

TEST(string, from_bool)
{
    EXPECT_EQ(ST_LITERAL("false"), ST::string::from_bool(false));
    EXPECT_EQ(ST_LITERAL("true"), ST::string::from_bool(true));
    EXPECT_EQ(ST_LITERAL("true"), ST::string::from_bool((bool)16));
}

TEST(string, to_int)
{
    EXPECT_EQ(0, ST_LITERAL("0").to_int());
    EXPECT_EQ(0, ST_LITERAL("+0").to_int());
    EXPECT_EQ(0, ST_LITERAL("-0").to_int());

    EXPECT_EQ(0, ST_LITERAL("0").to_long_long());
    EXPECT_EQ(0, ST_LITERAL("+0").to_long_long());
    EXPECT_EQ(0, ST_LITERAL("-0").to_long_long());

    EXPECT_EQ(-80000, ST_LITERAL("-80000").to_int());
    EXPECT_EQ(80000, ST_LITERAL("80000").to_int());
    EXPECT_EQ(80000, ST_LITERAL("+80000").to_int());
    EXPECT_EQ(-80000, ST_LITERAL("-0x13880").to_int());
    EXPECT_EQ(80000, ST_LITERAL("0x13880").to_int());
    EXPECT_EQ(80000, ST_LITERAL("+0x13880").to_int());
    EXPECT_EQ(-80000, ST_LITERAL("-0234200").to_int());
    EXPECT_EQ(80000, ST_LITERAL("0234200").to_int());
    EXPECT_EQ(80000, ST_LITERAL("+0234200").to_int());
    EXPECT_EQ(-80000, ST_LITERAL("-13880").to_int(16));
    EXPECT_EQ(80000, ST_LITERAL("13880").to_int(16));
    EXPECT_EQ(80000, ST_LITERAL("+13880").to_int(16));
    EXPECT_EQ(-80000, ST_LITERAL("-234200").to_int(8));
    EXPECT_EQ(80000, ST_LITERAL("234200").to_int(8));
    EXPECT_EQ(80000, ST_LITERAL("+234200").to_int(8));

    EXPECT_EQ(-1000000000000LL, ST_LITERAL("-1000000000000").to_long_long());
    EXPECT_EQ(1000000000000LL, ST_LITERAL("1000000000000").to_long_long());
    EXPECT_EQ(1000000000000LL, ST_LITERAL("+1000000000000").to_long_long());
    EXPECT_EQ(-1000000000000LL, ST_LITERAL("-0xe8d4a51000").to_long_long());
    EXPECT_EQ(1000000000000LL, ST_LITERAL("0xe8d4a51000").to_long_long());
    EXPECT_EQ(1000000000000LL, ST_LITERAL("+0xe8d4a51000").to_long_long());
    EXPECT_EQ(-1000000000000LL, ST_LITERAL("-016432451210000").to_long_long());
    EXPECT_EQ(1000000000000LL, ST_LITERAL("016432451210000").to_long_long());
    EXPECT_EQ(1000000000000LL, ST_LITERAL("+016432451210000").to_long_long());
    EXPECT_EQ(-1000000000000LL, ST_LITERAL("-e8d4a51000").to_long_long(16));
    EXPECT_EQ(1000000000000LL, ST_LITERAL("e8d4a51000").to_long_long(16));
    EXPECT_EQ(1000000000000LL, ST_LITERAL("+e8d4a51000").to_long_long(16));
    EXPECT_EQ(-1000000000000LL, ST_LITERAL("-16432451210000").to_long_long(8));
    EXPECT_EQ(1000000000000LL, ST_LITERAL("16432451210000").to_long_long(8));
    EXPECT_EQ(1000000000000LL, ST_LITERAL("+16432451210000").to_long_long(8));

    static const int int32_min = std::numeric_limits<int>::min();
    static const int int32_max = std::numeric_limits<int>::max();
    EXPECT_EQ(int32_min, ST_LITERAL("-2147483648").to_int());
    EXPECT_EQ(int32_max, ST_LITERAL("2147483647").to_int());
    EXPECT_EQ(int32_max, ST_LITERAL("+2147483647").to_int());
    EXPECT_EQ(int32_min, ST_LITERAL("-0x80000000").to_int());
    EXPECT_EQ(int32_max, ST_LITERAL("0x7FFFFFFF").to_int());
    EXPECT_EQ(int32_max, ST_LITERAL("+0x7FFFFFFF").to_int());
    EXPECT_EQ(int32_min, ST_LITERAL("-020000000000").to_int());
    EXPECT_EQ(int32_max, ST_LITERAL("017777777777").to_int());
    EXPECT_EQ(int32_max, ST_LITERAL("+017777777777").to_int());
    EXPECT_EQ(int32_min, ST_LITERAL("-80000000").to_int(16));
    EXPECT_EQ(int32_max, ST_LITERAL("7FFFFFFF").to_int(16));
    EXPECT_EQ(int32_max, ST_LITERAL("+7FFFFFFF").to_int(16));
    EXPECT_EQ(int32_min, ST_LITERAL("-20000000000").to_int(8));
    EXPECT_EQ(int32_max, ST_LITERAL("17777777777").to_int(8));
    EXPECT_EQ(int32_max, ST_LITERAL("+17777777777").to_int(8));

    static const long long int64_min = std::numeric_limits<long long>::min();
    static const long long int64_max = std::numeric_limits<long long>::max();
    EXPECT_EQ(int64_min, ST_LITERAL("-9223372036854775808").to_long_long());
    EXPECT_EQ(int64_max, ST_LITERAL("9223372036854775807").to_long_long());
    EXPECT_EQ(int64_max, ST_LITERAL("+9223372036854775807").to_long_long());
    EXPECT_EQ(int64_min, ST_LITERAL("-0x8000000000000000").to_long_long());
    EXPECT_EQ(int64_max, ST_LITERAL("0x7FFFFFFFFFFFFFFF").to_long_long());
    EXPECT_EQ(int64_max, ST_LITERAL("+0x7FFFFFFFFFFFFFFF").to_long_long());
    EXPECT_EQ(int64_min, ST_LITERAL("-01000000000000000000000").to_long_long());
    EXPECT_EQ(int64_max, ST_LITERAL("0777777777777777777777").to_long_long());
    EXPECT_EQ(int64_max, ST_LITERAL("+0777777777777777777777").to_long_long());
    EXPECT_EQ(int64_min, ST_LITERAL("-8000000000000000").to_long_long(16));
    EXPECT_EQ(int64_max, ST_LITERAL("7FFFFFFFFFFFFFFF").to_long_long(16));
    EXPECT_EQ(int64_max, ST_LITERAL("+7FFFFFFFFFFFFFFF").to_long_long(16));
    EXPECT_EQ(int64_min, ST_LITERAL("-1000000000000000000000").to_long_long(8));
    EXPECT_EQ(int64_max, ST_LITERAL("777777777777777777777").to_long_long(8));
    EXPECT_EQ(int64_max, ST_LITERAL("+777777777777777777777").to_long_long(8));

    // Empty string is treated as zero for compatibility with strtol
    EXPECT_EQ(0, ST::string().to_int());
    EXPECT_EQ(0LL, ST::string().to_long_long());
}

TEST(string, to_int_check)
{
    ST::conversion_result result;
    (void) ST_LITERAL("0").to_int(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("100").to_int(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("+100").to_int(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("-100").to_int(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("0x1FF").to_int(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("+0x1FF").to_int(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("-0x1FF").to_int(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("1FF").to_int(result, 16);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("+1FF").to_int(result, 16);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("-1FF").to_int(result, 16);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("0100").to_int(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("+0100").to_int(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("-0100").to_int(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());

    (void) ST_LITERAL("1FF").to_int(result);
    EXPECT_TRUE(result.ok());
    EXPECT_FALSE(result.full_match());
    (void) ST_LITERAL("+1FF").to_int(result);
    EXPECT_TRUE(result.ok());
    EXPECT_FALSE(result.full_match());
    (void) ST_LITERAL("-1FF").to_int(result);
    EXPECT_TRUE(result.ok());
    EXPECT_FALSE(result.full_match());
    (void) ST_LITERAL("FF").to_int(result);
    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.full_match());
    (void) ST_LITERAL("+FF").to_int(result);
    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.full_match());
    (void) ST_LITERAL("-FF").to_int(result);
    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.full_match());
    (void) ST::string().to_int(result);
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(result.full_match());
}

TEST(string, to_uint)
{
    EXPECT_EQ(0U, ST_LITERAL("0").to_uint());
    EXPECT_EQ(0U, ST_LITERAL("+0").to_uint());
    EXPECT_EQ(0U, ST_LITERAL("-0").to_uint());

    EXPECT_EQ(0ULL, ST_LITERAL("0").to_ulong_long());
    EXPECT_EQ(0ULL, ST_LITERAL("+0").to_ulong_long());
    EXPECT_EQ(0ULL, ST_LITERAL("-0").to_ulong_long());

    EXPECT_EQ(80000U, ST_LITERAL("80000").to_uint());
    EXPECT_EQ(80000U, ST_LITERAL("+80000").to_uint());
    EXPECT_EQ(80000U, ST_LITERAL("0x13880").to_uint());
    EXPECT_EQ(80000U, ST_LITERAL("+0x13880").to_uint());
    EXPECT_EQ(80000U, ST_LITERAL("0234200").to_uint());
    EXPECT_EQ(80000U, ST_LITERAL("+0234200").to_uint());
    EXPECT_EQ(80000U, ST_LITERAL("13880").to_uint(16));
    EXPECT_EQ(80000U, ST_LITERAL("+13880").to_uint(16));
    EXPECT_EQ(80000U, ST_LITERAL("234200").to_uint(8));
    EXPECT_EQ(80000U, ST_LITERAL("+234200").to_uint(8));

    EXPECT_EQ(1000000000000ULL, ST_LITERAL("1000000000000").to_ulong_long());
    EXPECT_EQ(1000000000000ULL, ST_LITERAL("+1000000000000").to_ulong_long());
    EXPECT_EQ(1000000000000ULL, ST_LITERAL("0xe8d4a51000").to_ulong_long());
    EXPECT_EQ(1000000000000ULL, ST_LITERAL("+0xe8d4a51000").to_ulong_long());
    EXPECT_EQ(1000000000000ULL, ST_LITERAL("016432451210000").to_ulong_long());
    EXPECT_EQ(1000000000000ULL, ST_LITERAL("+016432451210000").to_ulong_long());
    EXPECT_EQ(1000000000000ULL, ST_LITERAL("e8d4a51000").to_ulong_long(16));
    EXPECT_EQ(1000000000000ULL, ST_LITERAL("+e8d4a51000").to_ulong_long(16));
    EXPECT_EQ(1000000000000ULL, ST_LITERAL("16432451210000").to_ulong_long(8));
    EXPECT_EQ(1000000000000ULL, ST_LITERAL("+16432451210000").to_ulong_long(8));

    static const unsigned int uint32_max = std::numeric_limits<unsigned int>::max();
    EXPECT_EQ(uint32_max, ST_LITERAL("4294967295").to_uint());
    EXPECT_EQ(uint32_max, ST_LITERAL("+4294967295").to_uint());
    EXPECT_EQ(uint32_max, ST_LITERAL("0xFFFFFFFF").to_uint());
    EXPECT_EQ(uint32_max, ST_LITERAL("+0xFFFFFFFF").to_uint());
    EXPECT_EQ(uint32_max, ST_LITERAL("037777777777").to_uint());
    EXPECT_EQ(uint32_max, ST_LITERAL("+037777777777").to_uint());

    static const unsigned long long uint64_max = std::numeric_limits<unsigned long long>::max();
    EXPECT_EQ(uint64_max, ST_LITERAL("18446744073709551615").to_ulong_long());
    EXPECT_EQ(uint64_max, ST_LITERAL("+18446744073709551615").to_ulong_long());
    EXPECT_EQ(uint64_max, ST_LITERAL("0xFFFFFFFFFFFFFFFF").to_ulong_long());
    EXPECT_EQ(uint64_max, ST_LITERAL("+0xFFFFFFFFFFFFFFFF").to_ulong_long());
    EXPECT_EQ(uint64_max, ST_LITERAL("01777777777777777777777").to_ulong_long());
    EXPECT_EQ(uint64_max, ST_LITERAL("+01777777777777777777777").to_ulong_long());

    // Empty string is treated as zero for compatibility with strtoul
    EXPECT_EQ(0U, ST::string().to_uint());
    EXPECT_EQ(0ULL, ST::string().to_ulong_long());
}

TEST(string, to_uint_check)
{
    ST::conversion_result result;
    (void) ST_LITERAL("0").to_uint(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("100").to_uint(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("0x1FF").to_uint(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("1FF").to_uint(result, 16);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("0100").to_uint(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());

    (void) ST_LITERAL("1FF").to_uint(result);
    EXPECT_TRUE(result.ok());
    EXPECT_FALSE(result.full_match());
    (void) ST_LITERAL("FF").to_uint(result);
    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.full_match());
    (void) ST::string().to_uint(result);
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(result.full_match());
}

TEST(string, to_float)
{
    EXPECT_EQ(0.0f, ST_LITERAL("0").to_float());
    EXPECT_EQ(0.0f, ST_LITERAL("+0").to_float());
    EXPECT_EQ(0.0f, ST_LITERAL("-0").to_float());

    EXPECT_EQ(0.0, ST_LITERAL("0").to_double());
    EXPECT_EQ(0.0, ST_LITERAL("+0").to_double());
    EXPECT_EQ(0.0, ST_LITERAL("-0").to_double());

    EXPECT_EQ(-16.0f, ST_LITERAL("-16").to_float());
    EXPECT_EQ(16.0f, ST_LITERAL("16").to_float());
    EXPECT_EQ(16.0f, ST_LITERAL("+16").to_float());
    EXPECT_EQ(-16.0f, ST_LITERAL("-16.0").to_float());
    EXPECT_EQ(16.0f, ST_LITERAL("16.0").to_float());
    EXPECT_EQ(16.0f, ST_LITERAL("+16.0").to_float());
    EXPECT_EQ(-16.0f, ST_LITERAL("-1.6e1").to_float());
    EXPECT_EQ(16.0f, ST_LITERAL("1.6e1").to_float());
    EXPECT_EQ(16.0f, ST_LITERAL("+1.6e1").to_float());

    EXPECT_EQ(-16.0, ST_LITERAL("-16").to_double());
    EXPECT_EQ(16.0, ST_LITERAL("16").to_double());
    EXPECT_EQ(16.0, ST_LITERAL("+16").to_double());
    EXPECT_EQ(-16.0, ST_LITERAL("-16.0").to_double());
    EXPECT_EQ(16.0, ST_LITERAL("16.0").to_double());
    EXPECT_EQ(16.0, ST_LITERAL("+16.0").to_double());
    EXPECT_EQ(-16.0, ST_LITERAL("-1.6e1").to_double());
    EXPECT_EQ(16.0, ST_LITERAL("1.6e1").to_double());
    EXPECT_EQ(16.0, ST_LITERAL("+1.6e1").to_double());

    // Empty string is treated as zero for compatibility with strtod
    EXPECT_EQ(0.0f, ST::string().to_float());
    EXPECT_EQ(0.0, ST::string().to_double());
}

TEST(string, to_float_check)
{
    ST::conversion_result result;
    (void) ST_LITERAL("0").to_float(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("0").to_double(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());

    (void) ST_LITERAL("16").to_float(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("+16").to_float(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("-16").to_float(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("16").to_double(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("+16").to_double(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("-16").to_double(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());

    (void) ST_LITERAL("16.0").to_float(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("+16.0").to_float(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("-16.0").to_float(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("16.0").to_double(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("+16.0").to_double(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("-16.0").to_double(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());

    (void) ST_LITERAL("1.6e1").to_float(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("+1.6e1").to_float(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("-1.6e1").to_float(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("1.6e1").to_double(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("+1.6e1").to_double(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("-1.6e1").to_double(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());

    (void) ST_LITERAL("16xx").to_float(result);
    EXPECT_TRUE(result.ok());
    EXPECT_FALSE(result.full_match());
    (void) ST_LITERAL("16xx").to_double(result);
    EXPECT_TRUE(result.ok());
    EXPECT_FALSE(result.full_match());
    (void) ST_LITERAL("xx").to_float(result);
    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.full_match());
    (void) ST_LITERAL("xx").to_double(result);
    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.full_match());

    (void) ST::string().to_float(result);
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST::string().to_double(result);
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(result.full_match());
}

TEST(string, to_bool)
{
    EXPECT_TRUE(ST_LITERAL("true").to_bool());
    EXPECT_TRUE(ST_LITERAL("TRUE").to_bool());
    EXPECT_FALSE(ST_LITERAL("false").to_bool());
    EXPECT_FALSE(ST_LITERAL("FALSE").to_bool());

    EXPECT_FALSE(ST_LITERAL("0").to_bool());
    EXPECT_TRUE(ST_LITERAL("1").to_bool());
    EXPECT_TRUE(ST_LITERAL("-1").to_bool());
    EXPECT_TRUE(ST_LITERAL("1000").to_bool());
    EXPECT_TRUE(ST_LITERAL("0x1000").to_bool());

    EXPECT_FALSE(ST_LITERAL("T").to_bool());
    EXPECT_FALSE(ST_LITERAL("trueXX").to_bool());

    EXPECT_FALSE(ST::string().to_bool());
}

TEST(string, to_bool_check)
{
    ST::conversion_result result;
    (void) ST_LITERAL("true").to_bool(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("TRUE").to_bool(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("false").to_bool(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("FALSE").to_bool(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());

    (void) ST_LITERAL("0").to_bool(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("1").to_bool(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("-1").to_bool(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("1000").to_bool(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("0x1000").to_bool(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());

    (void) ST_LITERAL("T").to_bool(result);
    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.full_match());
    (void) ST_LITERAL("trueXX").to_bool(result);
    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.full_match());

    (void) ST::string().to_bool(result);
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(result.full_match());
}

TEST(string, compare)
{
    // Same length, case sensitive
    EXPECT_EQ(0, ST_LITERAL("abc").compare("abc", ST::case_sensitive));
    EXPECT_GT(0, ST_LITERAL("abc").compare("abd", ST::case_sensitive));
    EXPECT_LT(0, ST_LITERAL("abc").compare("abb", ST::case_sensitive));
    EXPECT_GT(0, ST_LITERAL("abC").compare("abc", ST::case_sensitive));
    EXPECT_GT(0, ST_LITERAL("Abc").compare("abc", ST::case_sensitive));
    EXPECT_EQ(0, ST::string().compare("", ST::case_sensitive));

    // Same length, case insensitive
    EXPECT_EQ(0, ST_LITERAL("abc").compare("abc", ST::case_insensitive));
    EXPECT_EQ(0, ST_LITERAL("abc").compare("ABC", ST::case_insensitive));
    EXPECT_GT(0, ST_LITERAL("abc").compare("abD", ST::case_insensitive));
    EXPECT_LT(0, ST_LITERAL("abc").compare("abB", ST::case_insensitive));
    EXPECT_EQ(0, ST::string().compare("", ST::case_insensitive));

    // Mismatched length, case sensitive
    EXPECT_LT(0, ST_LITERAL("abc").compare("ab", ST::case_sensitive));
    EXPECT_GT(0, ST_LITERAL("abc").compare("abcd", ST::case_sensitive));
    EXPECT_LT(0, ST_LITERAL("abc").compare("", ST::case_sensitive));
    EXPECT_GT(0, ST_LITERAL("").compare("abc", ST::case_sensitive));

    // Mismatched length, case insensitive
    EXPECT_LT(0, ST_LITERAL("abc").compare("Ab", ST::case_insensitive));
    EXPECT_GT(0, ST_LITERAL("abc").compare("Abcd", ST::case_insensitive));
    EXPECT_LT(0, ST_LITERAL("abc").compare("", ST::case_insensitive));
    EXPECT_GT(0, ST::string().compare("abc", ST::case_insensitive));
}

TEST(string, compare_n)
{
    // Same length, case sensitive
    EXPECT_EQ(0, ST_LITERAL("abcXX").compare_n("abcYY", 3, ST::case_sensitive));
    EXPECT_GT(0, ST_LITERAL("abcXX").compare_n("abdYY", 3, ST::case_sensitive));
    EXPECT_LT(0, ST_LITERAL("abcXX").compare_n("abbYY", 3, ST::case_sensitive));
    EXPECT_GT(0, ST_LITERAL("abCXX").compare_n("abcYY", 3, ST::case_sensitive));
    EXPECT_GT(0, ST_LITERAL("AbcXX").compare_n("abcYY", 3, ST::case_sensitive));

    // Same length, case insensitive
    EXPECT_EQ(0, ST_LITERAL("abcXX").compare_n("abcYY", 3, ST::case_insensitive));
    EXPECT_EQ(0, ST_LITERAL("abcXX").compare_n("ABCYY", 3, ST::case_insensitive));
    EXPECT_GT(0, ST_LITERAL("abcXX").compare_n("abDYY", 3, ST::case_insensitive));
    EXPECT_LT(0, ST_LITERAL("abcXX").compare_n("abBYY", 3, ST::case_insensitive));

    // Mismatched length, case sensitive
    EXPECT_LT(0, ST_LITERAL("abc").compare_n("ab", 3, ST::case_sensitive));
    EXPECT_GT(0, ST_LITERAL("abc").compare_n("abcd", 4, ST::case_sensitive));
    EXPECT_LT(0, ST_LITERAL("abc").compare_n("", 3, ST::case_sensitive));
    EXPECT_GT(0, ST_LITERAL("").compare_n("abc", 3, ST::case_sensitive));

    // Mismatched length, case insensitive
    EXPECT_LT(0, ST_LITERAL("abc").compare_n("Ab", 3, ST::case_insensitive));
    EXPECT_GT(0, ST_LITERAL("abc").compare_n("Abcd", 4, ST::case_insensitive));
    EXPECT_LT(0, ST_LITERAL("abc").compare_n("", 3, ST::case_insensitive));
    EXPECT_GT(0, ST::string().compare_n("abc", 3, ST::case_insensitive));
}

TEST(string, find_char)
{
    // Available char, case sensitive
    EXPECT_EQ(0, ST_LITERAL("Aaaaaaaa").find('A', ST::case_sensitive));
    EXPECT_EQ(0, ST_LITERAL("AaaaAaaa").find('A', ST::case_sensitive));
    EXPECT_EQ(4, ST_LITERAL("aaaaAaaa").find('A', ST::case_sensitive));
    EXPECT_EQ(7, ST_LITERAL("aaaaaaaA").find('A', ST::case_sensitive));

    // Available char, case insensitive
    EXPECT_EQ(0, ST_LITERAL("Abbbbbbb").find('A', ST::case_insensitive));
    EXPECT_EQ(0, ST_LITERAL("AbbbAbbb").find('A', ST::case_insensitive));
    EXPECT_EQ(4, ST_LITERAL("bbbbAbbb").find('A', ST::case_insensitive));
    EXPECT_EQ(7, ST_LITERAL("bbbbbbbA").find('A', ST::case_insensitive));
    EXPECT_EQ(0, ST_LITERAL("abbbbbbb").find('A', ST::case_insensitive));
    EXPECT_EQ(0, ST_LITERAL("abbbabbb").find('A', ST::case_insensitive));
    EXPECT_EQ(4, ST_LITERAL("bbbbabbb").find('A', ST::case_insensitive));
    EXPECT_EQ(7, ST_LITERAL("bbbbbbba").find('A', ST::case_insensitive));
    EXPECT_EQ(0, ST_LITERAL("Abbbbbbb").find('a', ST::case_insensitive));
    EXPECT_EQ(0, ST_LITERAL("AbbbAbbb").find('a', ST::case_insensitive));
    EXPECT_EQ(4, ST_LITERAL("bbbbAbbb").find('a', ST::case_insensitive));
    EXPECT_EQ(7, ST_LITERAL("bbbbbbbA").find('a', ST::case_insensitive));

    // Unavailable char
    EXPECT_EQ(-1, ST_LITERAL("AaaaAaaa").find('C', ST::case_sensitive));
    EXPECT_EQ(-1, ST_LITERAL("caaacaaa").find('C', ST::case_sensitive));
    EXPECT_EQ(-1, ST_LITERAL("AaaaAaaa").find('C', ST::case_insensitive));

    // Empty string
    EXPECT_EQ(-1, ST::string().find('A', ST::case_sensitive));
    EXPECT_EQ(-1, ST::string().find('A', ST::case_insensitive));

    // Starting position, case senstive
    EXPECT_EQ( 4, ST_LITERAL("AaaaAaaa").find(1, 'A', ST::case_sensitive));
    EXPECT_EQ( 4, ST_LITERAL("AaaaAaaa").find(4, 'A', ST::case_sensitive));
    EXPECT_EQ(-1, ST_LITERAL("AaaaAaaa").find(5, 'A', ST::case_sensitive));
    EXPECT_EQ( 7, ST_LITERAL("AaaaAaaA").find(5, 'A', ST::case_sensitive));
    EXPECT_EQ(-1, ST_LITERAL("AaaaAaaa").find(7, 'A', ST::case_sensitive));
    EXPECT_EQ(-1, ST_LITERAL("AaaaAaaA").find(8, 'A', ST::case_sensitive));
    EXPECT_EQ(-1, ST_LITERAL("AaaaAaaA").find(100, 'A', ST::case_sensitive));

    // Starting position, case insenstive
    EXPECT_EQ( 4, ST_LITERAL("abbbabbb").find(1, 'A', ST::case_insensitive));
    EXPECT_EQ( 4, ST_LITERAL("abbbabbb").find(4, 'A', ST::case_insensitive));
    EXPECT_EQ(-1, ST_LITERAL("abbbabbb").find(5, 'A', ST::case_insensitive));
    EXPECT_EQ( 7, ST_LITERAL("abbbabba").find(5, 'A', ST::case_insensitive));
    EXPECT_EQ(-1, ST_LITERAL("abbbabbb").find(7, 'A', ST::case_insensitive));
    EXPECT_EQ(-1, ST_LITERAL("abbbabba").find(8, 'A', ST::case_insensitive));
    EXPECT_EQ(-1, ST_LITERAL("abbbabba").find(100, 'A', ST::case_insensitive));
}

TEST(string, find_last_char)
{
    // Available char, case sensitive
    EXPECT_EQ(0, ST_LITERAL("Aaaaaaaa").find_last('A', ST::case_sensitive));
    EXPECT_EQ(4, ST_LITERAL("AaaaAaaa").find_last('A', ST::case_sensitive));
    EXPECT_EQ(4, ST_LITERAL("aaaaAaaa").find_last('A', ST::case_sensitive));
    EXPECT_EQ(7, ST_LITERAL("aaaaaaaA").find_last('A', ST::case_sensitive));

    // Available char, case insensitive
    EXPECT_EQ(0, ST_LITERAL("Abbbbbbb").find_last('A', ST::case_insensitive));
    EXPECT_EQ(4, ST_LITERAL("AbbbAbbb").find_last('A', ST::case_insensitive));
    EXPECT_EQ(4, ST_LITERAL("bbbbAbbb").find_last('A', ST::case_insensitive));
    EXPECT_EQ(7, ST_LITERAL("bbbbbbbA").find_last('A', ST::case_insensitive));
    EXPECT_EQ(0, ST_LITERAL("abbbbbbb").find_last('A', ST::case_insensitive));
    EXPECT_EQ(4, ST_LITERAL("abbbabbb").find_last('A', ST::case_insensitive));
    EXPECT_EQ(4, ST_LITERAL("bbbbabbb").find_last('A', ST::case_insensitive));
    EXPECT_EQ(7, ST_LITERAL("bbbbbbba").find_last('A', ST::case_insensitive));
    EXPECT_EQ(0, ST_LITERAL("Abbbbbbb").find_last('a', ST::case_insensitive));
    EXPECT_EQ(4, ST_LITERAL("AbbbAbbb").find_last('a', ST::case_insensitive));
    EXPECT_EQ(4, ST_LITERAL("bbbbAbbb").find_last('a', ST::case_insensitive));
    EXPECT_EQ(7, ST_LITERAL("bbbbbbbA").find_last('a', ST::case_insensitive));

    // Unavailable char
    EXPECT_EQ(-1, ST_LITERAL("AaaaAaaa").find_last('C', ST::case_sensitive));
    EXPECT_EQ(-1, ST_LITERAL("caaacaaa").find_last('C', ST::case_sensitive));
    EXPECT_EQ(-1, ST_LITERAL("AaaaAaaa").find_last('C', ST::case_insensitive));

    // Empty string
    EXPECT_EQ(-1, ST::string().find_last('A', ST::case_sensitive));
    EXPECT_EQ(-1, ST::string().find_last('A', ST::case_insensitive));

    // End position, case senstive
    EXPECT_EQ( 0, ST_LITERAL("AaaaAaaa").find_last(4, 'A', ST::case_sensitive));
    EXPECT_EQ( 4, ST_LITERAL("AaaaAaaa").find_last(5, 'A', ST::case_sensitive));
    EXPECT_EQ( 4, ST_LITERAL("AaaaAaaA").find_last(7, 'A', ST::case_sensitive));
    EXPECT_EQ( 7, ST_LITERAL("AaaaAaaA").find_last(8, 'A', ST::case_sensitive));
    EXPECT_EQ( 7, ST_LITERAL("AaaaAaaA").find_last(100, 'A', ST::case_sensitive));
    EXPECT_EQ(-1, ST_LITERAL("aaaaAaaA").find_last(4, 'A', ST::case_sensitive));
    EXPECT_EQ(-1, ST_LITERAL("AaaaAaaA").find_last(0, 'A', ST::case_sensitive));

    // End position, case insenstive
    EXPECT_EQ( 0, ST_LITERAL("abbbabbb").find_last(4, 'A', ST::case_insensitive));
    EXPECT_EQ( 4, ST_LITERAL("abbbabbb").find_last(5, 'A', ST::case_insensitive));
    EXPECT_EQ( 4, ST_LITERAL("abbbabba").find_last(7, 'A', ST::case_insensitive));
    EXPECT_EQ( 7, ST_LITERAL("abbbabba").find_last(8, 'A', ST::case_insensitive));
    EXPECT_EQ( 7, ST_LITERAL("abbbabba").find_last(100, 'A', ST::case_insensitive));
    EXPECT_EQ(-1, ST_LITERAL("bbbbabba").find_last(4, 'A', ST::case_insensitive));
    EXPECT_EQ(-1, ST_LITERAL("abbbabba").find_last(0, 'A', ST::case_insensitive));
}

TEST(string, find)
{
    // Available string, case sensitive
    EXPECT_EQ(0, ST_LITERAL("ABCDabcd").find("ABCD", ST::case_sensitive));
    EXPECT_EQ(4, ST_LITERAL("abcdABCDABCDabcd").find("ABCD", ST::case_sensitive));
    EXPECT_EQ(4, ST_LITERAL("abcdABCDabcd").find("ABCD", ST::case_sensitive));
    EXPECT_EQ(4, ST_LITERAL("abcdABCD").find("ABCD", ST::case_sensitive));

    // Available string, case insensitive
    EXPECT_EQ(0, ST_LITERAL("ABCDxxxx").find("ABCD", ST::case_insensitive));
    EXPECT_EQ(4, ST_LITERAL("xxxxABCDABCDxxxx").find("ABCD", ST::case_insensitive));
    EXPECT_EQ(4, ST_LITERAL("xxxxABCDxxxx").find("ABCD", ST::case_insensitive));
    EXPECT_EQ(4, ST_LITERAL("xxxxABCD").find("ABCD", ST::case_insensitive));
    EXPECT_EQ(0, ST_LITERAL("abcdxxxx").find("ABCD", ST::case_insensitive));
    EXPECT_EQ(4, ST_LITERAL("xxxxabcdABCDxxxx").find("ABCD", ST::case_insensitive));
    EXPECT_EQ(4, ST_LITERAL("xxxxabcdxxxx").find("ABCD", ST::case_insensitive));
    EXPECT_EQ(4, ST_LITERAL("xxxxabcd").find("ABCD", ST::case_insensitive));
    EXPECT_EQ(0, ST_LITERAL("ABCDxxxx").find("abcd", ST::case_insensitive));
    EXPECT_EQ(4, ST_LITERAL("xxxxABCDabcdxxxx").find("abcd", ST::case_insensitive));
    EXPECT_EQ(4, ST_LITERAL("xxxxABCDxxxx").find("abcd", ST::case_insensitive));
    EXPECT_EQ(4, ST_LITERAL("xxxxABCD").find("abcd", ST::case_insensitive));

    // Unavailable string
    EXPECT_EQ(-1, ST_LITERAL("xxxx").find("ABCD", ST::case_sensitive));
    EXPECT_EQ(-1, ST_LITERAL("xxxx").find("ABCD", ST::case_insensitive));
    EXPECT_EQ(-1, ST_LITERAL("xxxxABC").find("ABCD", ST::case_sensitive));
    EXPECT_EQ(-1, ST_LITERAL("xxxxABC").find("ABCD", ST::case_insensitive));
    EXPECT_EQ(-1, ST_LITERAL("xxxxABCxxxx").find("ABCD", ST::case_sensitive));
    EXPECT_EQ(-1, ST_LITERAL("xxxxABCxxxx").find("ABCD", ST::case_insensitive));

    // Empty string
    EXPECT_EQ(-1, ST::string().find("AAAA", ST::case_sensitive));
    EXPECT_EQ(-1, ST::string().find("AAAA", ST::case_insensitive));
    EXPECT_EQ(-1, ST_LITERAL("xxxx").find("", ST::case_sensitive));
    EXPECT_EQ(-1, ST_LITERAL("xxxx").find((const char *)nullptr, ST::case_sensitive));
    EXPECT_EQ(-1, ST::string().find("", ST::case_sensitive));
    EXPECT_EQ(-1, ST::string().find((const char *)nullptr, ST::case_sensitive));

    // Unicode substring
    ST::string haystack;
    haystack = ST_LITERAL("xxxx") + ST::string::from_utf32(test_data);
    EXPECT_EQ(4, haystack.find(utf8_test_data, ST::case_sensitive));
    EXPECT_EQ(4, haystack.find(utf8_test_data, ST::case_insensitive));

    haystack = ST::string::from_utf32(test_data) + ST_LITERAL("xxxx");
    EXPECT_EQ(0, haystack.find(utf8_test_data, ST::case_sensitive));
    EXPECT_EQ(0, haystack.find(utf8_test_data, ST::case_insensitive));

    haystack = ST_LITERAL("xxxx") + ST::string::from_utf32(test_data) + ST_LITERAL("xxxx");
    EXPECT_EQ(4, haystack.find(utf8_test_data, ST::case_sensitive));
    EXPECT_EQ(4, haystack.find(utf8_test_data, ST::case_insensitive));

    // Starting position, case senstive
    EXPECT_EQ(-1, ST_LITERAL("ABCDabcd").find(1, "ABCD", ST::case_sensitive));
    EXPECT_EQ( 4, ST_LITERAL("abcdABCDABCDabcd").find(1, "ABCD", ST::case_sensitive));
    EXPECT_EQ( 4, ST_LITERAL("abcdABCDABCDabcd").find(4, "ABCD", ST::case_sensitive));
    EXPECT_EQ( 8, ST_LITERAL("abcdABCDABCDabcd").find(5, "ABCD", ST::case_sensitive));
    EXPECT_EQ(-1, ST_LITERAL("abcdABCDabcd").find(5, "ABCD", ST::case_sensitive));
    EXPECT_EQ(-1, ST_LITERAL("abcdABCDabcd").find(100, "ABCD", ST::case_sensitive));

    // Starting position, case insenstive
    EXPECT_EQ(-1, ST_LITERAL("abcdxxxx").find(1, "ABCD", ST::case_insensitive));
    EXPECT_EQ( 4, ST_LITERAL("xxxxabcdabcdxxxx").find(1, "ABCD", ST::case_insensitive));
    EXPECT_EQ( 4, ST_LITERAL("xxxxabcdabcdxxxx").find(4, "ABCD", ST::case_insensitive));
    EXPECT_EQ( 8, ST_LITERAL("xxxxabcdabcdxxxx").find(5, "ABCD", ST::case_insensitive));
    EXPECT_EQ(-1, ST_LITERAL("xxxxabcdxxxx").find(5, "ABCD", ST::case_insensitive));
    EXPECT_EQ(-1, ST_LITERAL("xxxxabcdxxxx").find(100, "ABCD", ST::case_insensitive));

    // Empty search string
    EXPECT_EQ(-1, ST_LITERAL("xxxx").find("", ST::case_sensitive));
    EXPECT_EQ(-1, ST_LITERAL("xxxx").find("", ST::case_insensitive));
    EXPECT_EQ(-1, ST_LITERAL("xxxx").find(4, "", ST::case_sensitive));
    EXPECT_EQ(-1, ST_LITERAL("xxxx").find(4, "", ST::case_insensitive));
}

TEST(string, find_last)
{
    // Available string, case sensitive
    EXPECT_EQ(0, ST_LITERAL("ABCDabcd").find_last("ABCD", ST::case_sensitive));
    EXPECT_EQ(8, ST_LITERAL("abcdABCDABCDabcd").find_last("ABCD", ST::case_sensitive));
    EXPECT_EQ(4, ST_LITERAL("abcdABCDabcd").find_last("ABCD", ST::case_sensitive));
    EXPECT_EQ(4, ST_LITERAL("abcdABCD").find_last("ABCD", ST::case_sensitive));

    // Available string, case insensitive
    EXPECT_EQ(0, ST_LITERAL("ABCDxxxx").find_last("ABCD", ST::case_insensitive));
    EXPECT_EQ(8, ST_LITERAL("xxxxABCDABCDxxxx").find_last("ABCD", ST::case_insensitive));
    EXPECT_EQ(4, ST_LITERAL("xxxxABCDxxxx").find_last("ABCD", ST::case_insensitive));
    EXPECT_EQ(4, ST_LITERAL("xxxxABCD").find_last("ABCD", ST::case_insensitive));
    EXPECT_EQ(0, ST_LITERAL("abcdxxxx").find_last("ABCD", ST::case_insensitive));
    EXPECT_EQ(8, ST_LITERAL("xxxxabcdABCDxxxx").find_last("ABCD", ST::case_insensitive));
    EXPECT_EQ(4, ST_LITERAL("xxxxabcdxxxx").find_last("ABCD", ST::case_insensitive));
    EXPECT_EQ(4, ST_LITERAL("xxxxabcd").find_last("ABCD", ST::case_insensitive));
    EXPECT_EQ(0, ST_LITERAL("ABCDxxxx").find_last("abcd", ST::case_insensitive));
    EXPECT_EQ(8, ST_LITERAL("xxxxABCDabcdxxxx").find_last("abcd", ST::case_insensitive));
    EXPECT_EQ(4, ST_LITERAL("xxxxABCDxxxx").find_last("abcd", ST::case_insensitive));
    EXPECT_EQ(4, ST_LITERAL("xxxxABCD").find_last("abcd", ST::case_insensitive));

    // Unavailable string
    EXPECT_EQ(-1, ST_LITERAL("xxxx").find_last("ABCD", ST::case_sensitive));
    EXPECT_EQ(-1, ST_LITERAL("xxxx").find_last("ABCD", ST::case_insensitive));
    EXPECT_EQ(-1, ST_LITERAL("xxxxABC").find_last("ABCD", ST::case_sensitive));
    EXPECT_EQ(-1, ST_LITERAL("xxxxABC").find_last("ABCD", ST::case_insensitive));
    EXPECT_EQ(-1, ST_LITERAL("xxxxABCxxxx").find_last("ABCD", ST::case_sensitive));
    EXPECT_EQ(-1, ST_LITERAL("xxxxABCxxxx").find_last("ABCD", ST::case_insensitive));

    // Empty string
    EXPECT_EQ(-1, ST::string().find_last("AAAA", ST::case_sensitive));
    EXPECT_EQ(-1, ST::string().find_last("AAAA", ST::case_insensitive));
    EXPECT_EQ(-1, ST_LITERAL("xxxx").find_last("", ST::case_sensitive));
    EXPECT_EQ(-1, ST_LITERAL("xxxx").find_last((const char *)nullptr, ST::case_sensitive));
    EXPECT_EQ(-1, ST::string().find_last("", ST::case_sensitive));
    EXPECT_EQ(-1, ST::string().find_last((const char *)nullptr, ST::case_sensitive));

    // Unicode substring
    ST::string haystack;
    haystack = ST_LITERAL("xxxx") + ST::string::from_utf32(test_data);
    EXPECT_EQ(4, haystack.find_last(utf8_test_data, ST::case_sensitive));
    EXPECT_EQ(4, haystack.find_last(utf8_test_data, ST::case_insensitive));

    haystack = ST::string::from_utf32(test_data) + ST_LITERAL("xxxx");
    EXPECT_EQ(0, haystack.find_last(utf8_test_data, ST::case_sensitive));
    EXPECT_EQ(0, haystack.find_last(utf8_test_data, ST::case_insensitive));

    haystack = ST_LITERAL("xxxx") + ST::string::from_utf32(test_data) + ST_LITERAL("xxxx");
    EXPECT_EQ(4, haystack.find_last(utf8_test_data, ST::case_sensitive));
    EXPECT_EQ(4, haystack.find_last(utf8_test_data, ST::case_insensitive));

    // Starting position, case senstive
    EXPECT_EQ(-1, ST_LITERAL("abcdABCD").find_last(4, "ABCD", ST::case_sensitive));
    EXPECT_EQ( 4, ST_LITERAL("abcdABCDABCDabcd").find_last(5, "ABCD", ST::case_sensitive));
    EXPECT_EQ( 4, ST_LITERAL("abcdABCDABCDabcd").find_last(8, "ABCD", ST::case_sensitive));
    EXPECT_EQ( 8, ST_LITERAL("abcdABCDABCDabcd").find_last(9, "ABCD", ST::case_sensitive));
    EXPECT_EQ(-1, ST_LITERAL("abcdABCDabcd").find_last(4, "ABCD", ST::case_sensitive));
    EXPECT_EQ(-1, ST_LITERAL("ABCDabcd").find_last(0, "ABCD", ST::case_sensitive));

    // Starting position, case insenstive
    EXPECT_EQ(-1, ST_LITERAL("xxxxabcd").find_last(4, "ABCD", ST::case_insensitive));
    EXPECT_EQ( 4, ST_LITERAL("xxxxabcdabcdxxxx").find_last(5, "ABCD", ST::case_insensitive));
    EXPECT_EQ( 4, ST_LITERAL("xxxxabcdabcdxxxx").find_last(8, "ABCD", ST::case_insensitive));
    EXPECT_EQ( 8, ST_LITERAL("xxxxabcdabcdxxxx").find_last(9, "ABCD", ST::case_insensitive));
    EXPECT_EQ(-1, ST_LITERAL("xxxxabcdxxxx").find_last(4, "ABCD", ST::case_insensitive));
    EXPECT_EQ(-1, ST_LITERAL("abcdxxxx").find_last(0, "ABCD", ST::case_insensitive));

    // Empty search string
    EXPECT_EQ(-1, ST_LITERAL("xxxx").find_last("", ST::case_sensitive));
    EXPECT_EQ(-1, ST_LITERAL("xxxx").find_last("", ST::case_insensitive));
    EXPECT_EQ(-1, ST_LITERAL("xxxx").find_last(0, "", ST::case_sensitive));
    EXPECT_EQ(-1, ST_LITERAL("xxxx").find_last(0, "", ST::case_insensitive));
}

TEST(string, trim)
{
    EXPECT_EQ(ST_LITERAL("xxx   "), ST_LITERAL("   xxx   ").trim_left(" \t\r\n"));
    EXPECT_EQ(ST_LITERAL("xxx\t"), ST_LITERAL("\txxx\t").trim_left(" \t\r\n"));
    EXPECT_EQ(ST_LITERAL("xxx\r\n"), ST_LITERAL("\r\nxxx\r\n").trim_left(" \t\r\n"));
    EXPECT_EQ(ST_LITERAL("   xxx   "), ST_LITERAL("   xxx   ").trim_left("abc"));
    EXPECT_EQ(ST_LITERAL("   xxx   "), ST_LITERAL("   xxx   ").trim_left("x"));

    EXPECT_EQ(ST_LITERAL("   xxx"), ST_LITERAL("   xxx   ").trim_right(" \t\r\n"));
    EXPECT_EQ(ST_LITERAL("\txxx"), ST_LITERAL("\txxx\t").trim_right(" \t\r\n"));
    EXPECT_EQ(ST_LITERAL("\r\nxxx"), ST_LITERAL("\r\nxxx\r\n").trim_right(" \t\r\n"));
    EXPECT_EQ(ST_LITERAL("   xxx   "), ST_LITERAL("   xxx   ").trim_right("abc"));
    EXPECT_EQ(ST_LITERAL("   xxx   "), ST_LITERAL("   xxx   ").trim_right("x"));

    EXPECT_EQ(ST_LITERAL("xxx"), ST_LITERAL("   xxx   ").trim(" \t\r\n"));
    EXPECT_EQ(ST_LITERAL("xxx"), ST_LITERAL("\txxx\t").trim(" \t\r\n"));
    EXPECT_EQ(ST_LITERAL("xxx"), ST_LITERAL("\r\nxxx\r\n").trim(" \t\r\n"));
    EXPECT_EQ(ST_LITERAL("   xxx   "), ST_LITERAL("   xxx   ").trim("abc"));
    EXPECT_EQ(ST_LITERAL("   xxx   "), ST_LITERAL("   xxx   ").trim("x"));
}

TEST(string, substrings)
{
    EXPECT_EQ(ST_LITERAL("AAA"), ST_LITERAL("AAA").left(3));
    EXPECT_EQ(ST_LITERAL("AAA"), ST_LITERAL("AAAxxxx").left(3));
    EXPECT_EQ(ST_LITERAL("A"), ST_LITERAL("A").left(3));
    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL("").left(3));

    EXPECT_EQ(ST_LITERAL("AAA"), ST_LITERAL("AAA").right(3));
    EXPECT_EQ(ST_LITERAL("AAA"), ST_LITERAL("xxxxAAA").right(3));
    EXPECT_EQ(ST_LITERAL("A"), ST_LITERAL("A").right(3));
    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL("").right(3));

    EXPECT_EQ(ST_LITERAL("AAA"), ST_LITERAL("AAAxxxx").substr(0, 3));
    EXPECT_EQ(ST_LITERAL("AAA"), ST_LITERAL("xxxxAAA").substr(4, 3));
    EXPECT_EQ(ST_LITERAL("AAA"), ST_LITERAL("xxAAAxx").substr(2, 3));

    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL("AAAA").substr(2, 0));
    EXPECT_EQ(ST_LITERAL("AA"), ST_LITERAL("AAAA").substr(2, 4));
    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL("AAAA").substr(6, 4));
    EXPECT_EQ(ST_LITERAL("AAAA"), ST_LITERAL("AAAA").substr(0, 4));
    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL("").substr(0, 4));

    // Negative indexes start from the right
    EXPECT_EQ(ST_LITERAL("AAA"), ST_LITERAL("xxxxAAA").substr(-3, 3));
    EXPECT_EQ(ST_LITERAL("AAA"), ST_LITERAL("xxAAAxx").substr(-5, 3));
    EXPECT_EQ(ST_LITERAL("AAA"), ST_LITERAL("xxxxAAA").substr(-3, 6));
    EXPECT_EQ(ST_LITERAL("AAA"), ST_LITERAL("AAAxxxx").substr(-10, 3));
    EXPECT_EQ(ST_LITERAL("AAA"), ST_LITERAL("AAA").substr(-10, 10));
}

TEST(string, starts_with)
{
    EXPECT_TRUE(ST_LITERAL("AAA").starts_with("AAA"));
    EXPECT_TRUE(ST_LITERAL("AAAxxx").starts_with("AAA"));
    EXPECT_TRUE(ST_LITERAL("AAAAA").starts_with("AAA"));
    EXPECT_FALSE(ST_LITERAL("xxx").starts_with("AAA"));
    EXPECT_FALSE(ST_LITERAL("AAxxx").starts_with("AAA"));
    EXPECT_FALSE(ST_LITERAL("xAAAxxx").starts_with("AAA"));
    EXPECT_FALSE(ST_LITERAL("").starts_with("AAA"));
    EXPECT_TRUE(ST_LITERAL("xxx").starts_with(""));
}

TEST(string, ends_with)
{
    EXPECT_TRUE(ST_LITERAL("AAA").ends_with("AAA"));
    EXPECT_TRUE(ST_LITERAL("xxxAAA").ends_with("AAA"));
    EXPECT_TRUE(ST_LITERAL("AAAAA").ends_with("AAA"));
    EXPECT_FALSE(ST_LITERAL("xxx").ends_with("AAA"));
    EXPECT_FALSE(ST_LITERAL("xxxAA").ends_with("AAA"));
    EXPECT_FALSE(ST_LITERAL("xxxAAAx").ends_with("AAA"));
    EXPECT_FALSE(ST_LITERAL("").ends_with("AAA"));
    EXPECT_TRUE(ST_LITERAL("xxx").ends_with(""));
}

TEST(string, before_after)
{
    EXPECT_EQ(ST_LITERAL("AAA"), ST_LITERAL("AAA;BBB").before_first(';'));
    EXPECT_EQ(ST_LITERAL("AAA"), ST_LITERAL("AAA##SEP##BBB").before_first("##SEP##"));
    EXPECT_EQ(ST_LITERAL("AAA"), ST_LITERAL("AAA;BBB;CCC").before_first(';'));
    EXPECT_EQ(ST_LITERAL("AAA"), ST_LITERAL("AAA##SEP##BBB##SEP##CCC").before_first("##SEP##"));
    EXPECT_EQ(ST_LITERAL("AAA"), ST_LITERAL("AAA").before_first(';'));
    EXPECT_EQ(ST_LITERAL("AAA"), ST_LITERAL("AAA").before_first("##SEP##"));
    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL(";").before_first(';'));
    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL("##SEP##").before_first("##SEP##"));
    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL("").before_first(';'));
    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL("").before_first("##SEP##"));

    EXPECT_EQ(ST_LITERAL("AAA"), ST_LITERAL("AAA;BBB").before_last(';'));
    EXPECT_EQ(ST_LITERAL("AAA"), ST_LITERAL("AAA##SEP##BBB").before_last("##SEP##"));
    EXPECT_EQ(ST_LITERAL("AAA;BBB"), ST_LITERAL("AAA;BBB;CCC").before_last(';'));
    EXPECT_EQ(ST_LITERAL("AAA##SEP##BBB"), ST_LITERAL("AAA##SEP##BBB##SEP##CCC").before_last("##SEP##"));
    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL("AAA").before_last(';'));
    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL("AAA").before_last("##SEP##"));
    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL(";").before_last(';'));
    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL("##SEP##").before_last("##SEP##"));
    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL("").before_last(';'));
    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL("").before_last("##SEP##"));

    EXPECT_EQ(ST_LITERAL("BBB"), ST_LITERAL("AAA;BBB").after_first(';'));
    EXPECT_EQ(ST_LITERAL("BBB"), ST_LITERAL("AAA##SEP##BBB").after_first("##SEP##"));
    EXPECT_EQ(ST_LITERAL("BBB;CCC"), ST_LITERAL("AAA;BBB;CCC").after_first(';'));
    EXPECT_EQ(ST_LITERAL("BBB##SEP##CCC"), ST_LITERAL("AAA##SEP##BBB##SEP##CCC").after_first("##SEP##"));
    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL("AAA").after_first(';'));
    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL("AAA").after_first("##SEP##"));
    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL(";").after_first(';'));
    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL("##SEP##").after_first("##SEP##"));
    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL("").after_first(';'));
    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL("").after_first("##SEP##"));

    EXPECT_EQ(ST_LITERAL("BBB"), ST_LITERAL("AAA;BBB").after_last(';'));
    EXPECT_EQ(ST_LITERAL("BBB"), ST_LITERAL("AAA##SEP##BBB").after_last("##SEP##"));
    EXPECT_EQ(ST_LITERAL("CCC"), ST_LITERAL("AAA;BBB;CCC").after_last(';'));
    EXPECT_EQ(ST_LITERAL("CCC"), ST_LITERAL("AAA##SEP##BBB##SEP##CCC").after_last("##SEP##"));
    EXPECT_EQ(ST_LITERAL("AAA"), ST_LITERAL("AAA").after_last(';'));
    EXPECT_EQ(ST_LITERAL("AAA"), ST_LITERAL("AAA").after_last("##SEP##"));
    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL(";").after_last(';'));
    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL("##SEP##").after_last("##SEP##"));
    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL("").after_last(';'));
    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL("").after_last("##SEP##"));
}

TEST(string, replace)
{
    EXPECT_EQ(ST_LITERAL("xxYYxx"), ST_LITERAL("xxAAxx").replace("A", "Y"));
    EXPECT_EQ(ST_LITERAL("xxAAxx"), ST_LITERAL("xxAAxx").replace("XX", "Y"));
    EXPECT_EQ(ST_LITERAL("xxxx"), ST_LITERAL("xxAAxx").replace("A", ""));
    EXPECT_EQ(ST_LITERAL("xxREPLACExx"), ST_LITERAL("xxFINDxx").replace("FIND", "REPLACE"));
    EXPECT_EQ(ST_LITERAL("xxREPLACExxREPLACExx"), ST_LITERAL("xxFINDxxFINDxx").replace("FIND", "REPLACE"));

    EXPECT_EQ(ST_LITERAL("YYxx"), ST_LITERAL("AAxx").replace("A", "Y"));
    EXPECT_EQ(ST_LITERAL("AAxx"), ST_LITERAL("AAxx").replace("XX", "Y"));
    EXPECT_EQ(ST_LITERAL("xx"), ST_LITERAL("AAxx").replace("A", ""));
    EXPECT_EQ(ST_LITERAL("REPLACExx"), ST_LITERAL("FINDxx").replace("FIND", "REPLACE"));
    EXPECT_EQ(ST_LITERAL("REPLACExxREPLACExx"), ST_LITERAL("FINDxxFINDxx").replace("FIND", "REPLACE"));

    EXPECT_EQ(ST_LITERAL("xxYY"), ST_LITERAL("xxAA").replace("A", "Y"));
    EXPECT_EQ(ST_LITERAL("xxAA"), ST_LITERAL("xxAA").replace("XX", "Y"));
    EXPECT_EQ(ST_LITERAL("xx"), ST_LITERAL("xxAA").replace("A", ""));
    EXPECT_EQ(ST_LITERAL("xxREPLACE"), ST_LITERAL("xxFIND").replace("FIND", "REPLACE"));
    EXPECT_EQ(ST_LITERAL("xxREPLACExxREPLACE"), ST_LITERAL("xxFINDxxFIND").replace("FIND", "REPLACE"));

    EXPECT_EQ(ST_LITERAL("YY"), ST_LITERAL("AA").replace("A", "Y"));
    EXPECT_EQ(ST_LITERAL("AA"), ST_LITERAL("AA").replace("XX", "Y"));
    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL("AA").replace("A", ""));
    EXPECT_EQ(ST_LITERAL("REPLACE"), ST_LITERAL("FIND").replace("FIND", "REPLACE"));
    EXPECT_EQ(ST_LITERAL("REPLACExxREPLACE"), ST_LITERAL("FINDxxFIND").replace("FIND", "REPLACE"));

    // Empty search string
    EXPECT_EQ(ST_LITERAL("AA"), ST_LITERAL("AA").replace("", "Y"));
}

TEST(string, case_conversion)
{
    /* Edge cases:
     * '@' = 'A' - 1
     * '[' = 'Z' + 1
     * '`' = 'a' - 1
     * '{' = 'z' + 1
     */

    EXPECT_EQ(ST_LITERAL("AAZZ"), ST_LITERAL("aazz").to_upper());
    EXPECT_EQ(ST_LITERAL("AAZZ"), ST_LITERAL("AAZZ").to_upper());
    EXPECT_EQ(ST_LITERAL("@AZ[`AZ{"), ST_LITERAL("@AZ[`az{").to_upper());
    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL("").to_upper());

    EXPECT_EQ(ST_LITERAL("aazz"), ST_LITERAL("aazz").to_lower());
    EXPECT_EQ(ST_LITERAL("aazz"), ST_LITERAL("AAZZ").to_lower());
    EXPECT_EQ(ST_LITERAL("@az[`az{"), ST_LITERAL("@AZ[`az{").to_lower());
    EXPECT_EQ(ST_LITERAL(""), ST_LITERAL("").to_lower());
}

TEST(string, tokenize)
{
    std::vector<ST::string> expected1;
    expected1.push_back("aaa");
    expected1.push_back("b");
    expected1.push_back("ccc");
    expected1.push_back("d");
    expected1.push_back("èèè");
    const ST::string input1("aaa\t\tb\n;ccc-d;èèè");
    EXPECT_EQ(expected1, input1.tokenize("\t\n-;"));

    std::vector<ST::string> expected2;
    expected2.push_back("aaa\t\tb\n");
    expected2.push_back("ccc-d");
    expected2.push_back("èèè");
    EXPECT_EQ(expected2, input1.tokenize(";"));

    std::vector<ST::string> expected3;
    expected3.push_back(input1);
    EXPECT_EQ(expected3, input1.tokenize("x"));

    const ST::string input2("\t;aaa\t\tb\n;ccc-d;èèè--");
    EXPECT_EQ(expected1, input2.tokenize("\t\n-;"));

    // tokenize will return an empty vector if there are no tokens in the input
    EXPECT_EQ(std::vector<ST::string>(), ST_LITERAL("\t;\n;").tokenize("\t\n-;"));
    EXPECT_EQ(std::vector<ST::string>(), ST_LITERAL("").tokenize("\t\n-;"));
}

TEST(string, split)
{
    std::vector<ST::string> expected1;
    expected1.push_back("aaa");
    expected1.push_back("b");
    expected1.push_back("ccc");
    expected1.push_back("d");
    expected1.push_back("èèè");
    const ST::string input1("aaa-b-ccc-d-èèè");
    EXPECT_EQ(expected1, input1.split("-"));
    EXPECT_EQ(expected1, input1.split("-", 4));
    EXPECT_EQ(expected1, input1.split("-", 10));

    const ST::string input2("aaa#SEP#b#SEP#ccc#SEP#d#SEP#èèè");
    EXPECT_EQ(expected1, input2.split("#SEP#"));
    EXPECT_EQ(expected1, input2.split("#SEP#", 4));
    EXPECT_EQ(expected1, input2.split("#SEP#", 10));

    std::vector<ST::string> expected2;
    expected2.push_back("aaa");
    expected2.push_back("b");
    expected2.push_back("ccc-d-èèè");
    EXPECT_EQ(expected2, input1.split("-", 2));

    std::vector<ST::string> expected3;
    expected3.push_back(input1);
    EXPECT_EQ(expected3, input1.split("-", 0));
    EXPECT_EQ(expected3, input1.split("x"));
    EXPECT_EQ(expected3, input1.split("x", 4));

    std::vector<ST::string> expected4;
    expected4.push_back("");
    expected4.push_back("aaa");
    expected4.push_back("b");
    expected4.push_back("ccc");
    expected4.push_back("d");
    expected4.push_back("èèè");
    expected4.push_back("");
    const ST::string input3("-aaa-b-ccc-d-èèè-");
    EXPECT_EQ(expected4, input3.split("-"));
    EXPECT_EQ(expected4, input3.split("-", 6));
    EXPECT_EQ(expected4, input3.split("-", 10));

    std::vector<ST::string> expected5;
    expected5.push_back("");
    expected5.push_back("");
    expected5.push_back("");
    expected5.push_back("");
    expected5.push_back("");
    const ST::string input4("----");
    EXPECT_EQ(expected5, input4.split("-"));
    EXPECT_EQ(expected5, input4.split("-", 4));
    EXPECT_EQ(expected5, input4.split("-", 10));

    std::vector<ST::string> expected6;
    expected6.push_back("");
    expected6.push_back("");
    expected6.push_back("--");
    EXPECT_EQ(expected6, input4.split("-", 2));

    std::vector<ST::string> expected7;
    expected7.push_back("");
    expected7.push_back("");
    expected7.push_back("");
    EXPECT_EQ(expected7, input4.split("--"));

    std::vector<ST::string> expected8;
    expected8.push_back("");
    expected8.push_back("--");
    EXPECT_EQ(expected8, input4.split("--", 1));

    // split never provides an empty vector, even for empty input
    std::vector<ST::string> expected9;
    expected9.push_back(ST::string());
    EXPECT_EQ(expected9, ST_LITERAL("").split("-"));
    EXPECT_EQ(expected9, ST_LITERAL("").split("-", 4));
}

TEST(string, split_char)
{
    std::vector<ST::string> expected1;
    expected1.push_back("aaa");
    expected1.push_back("b");
    expected1.push_back("ccc");
    expected1.push_back("d");
    expected1.push_back("èèè");
    const ST::string input1("aaa-b-ccc-d-èèè");
    EXPECT_EQ(expected1, input1.split('-'));
    EXPECT_EQ(expected1, input1.split('-', 4));
    EXPECT_EQ(expected1, input1.split('-', 10));

    std::vector<ST::string> expected2;
    expected2.push_back("aaa");
    expected2.push_back("b");
    expected2.push_back("ccc-d-èèè");
    EXPECT_EQ(expected2, input1.split('-', 2));

    std::vector<ST::string> expected3;
    expected3.push_back(input1);
    EXPECT_EQ(expected3, input1.split('-', 0));
    EXPECT_EQ(expected3, input1.split('x'));
    EXPECT_EQ(expected3, input1.split('x', 4));

    std::vector<ST::string> expected4;
    expected4.push_back("");
    expected4.push_back("aaa");
    expected4.push_back("b");
    expected4.push_back("ccc");
    expected4.push_back("d");
    expected4.push_back("èèè");
    expected4.push_back("");
    const ST::string input3("-aaa-b-ccc-d-èèè-");
    EXPECT_EQ(expected4, input3.split('-'));
    EXPECT_EQ(expected4, input3.split('-', 6));
    EXPECT_EQ(expected4, input3.split('-', 10));

    std::vector<ST::string> expected5;
    expected5.push_back("");
    expected5.push_back("");
    expected5.push_back("");
    expected5.push_back("");
    expected5.push_back("");
    const ST::string input4("----");
    EXPECT_EQ(expected5, input4.split('-'));
    EXPECT_EQ(expected5, input4.split('-', 4));
    EXPECT_EQ(expected5, input4.split('-', 10));

    std::vector<ST::string> expected6;
    expected6.push_back("");
    expected6.push_back("");
    expected6.push_back("--");
    EXPECT_EQ(expected6, input4.split('-', 2));

    // split never provides an empty vector, even for empty input
    std::vector<ST::string> expected9;
    expected9.push_back(ST::string());
    EXPECT_EQ(expected9, ST_LITERAL("").split('-'));
    EXPECT_EQ(expected9, ST_LITERAL("").split('-', 4));
}

TEST(string, fill)
{
    EXPECT_EQ(ST_LITERAL(""), ST::string::fill(0, 'a'));
    EXPECT_EQ(ST_LITERAL("aaaaa"), ST::string::fill(5, 'a'));
    EXPECT_EQ(ST_LITERAL("aaaaaaaaaaaaaaaaaaaa"), ST::string::fill(20, 'a'));
}

TEST(string, iterators)
{
    ST::string source_short("X");
    ST::string source_long("The quick brown fox jumps over the lazy dog.");
    ST::string empty;

    EXPECT_EQ('X', source_short.front());
    EXPECT_EQ('X', source_short.back());
    EXPECT_EQ('T', source_long.front());
    EXPECT_EQ('.', source_long.back());
    EXPECT_EQ('\0', empty.front());
    EXPECT_EQ('\0', empty.back());

    EXPECT_NE(source_short.begin(), source_short.end());
    EXPECT_NE(source_short.rbegin(), source_short.rend());
    EXPECT_NE(source_long.begin(), source_long.end());
    EXPECT_NE(source_long.rbegin(), source_long.rend());
    EXPECT_EQ(empty.begin(), empty.end());
    EXPECT_EQ(empty.rbegin(), empty.rend());

    std::string result;
    std::string forward = "The quick brown fox jumps over the lazy dog.";
    std::copy(source_long.begin(), source_long.end(), std::back_inserter(result));
    EXPECT_EQ(forward, result);

    result.clear();
    std::string reverse = ".god yzal eht revo spmuj xof nworb kciuq ehT";
    std::copy(source_long.rbegin(), source_long.rend(), std::back_inserter(result));
    EXPECT_EQ(reverse, result);

    result.clear();
    std::string x = "X";
    std::copy(source_short.begin(), source_short.end(), std::back_inserter(result));
    EXPECT_EQ(x, result);

    result.clear();
    std::copy(source_short.rbegin(), source_short.rend(), std::back_inserter(result));
    EXPECT_EQ(x, result);
}

TEST(string, udls)
{
    using namespace ST::literals;

    // Only need to test the UDL usage -- the rest is covered above
    EXPECT_EQ(ST_LITERAL(""), ""_st);
    EXPECT_EQ(ST_LITERAL("Test"), "Test"_st);
    EXPECT_EQ(ST_LITERAL("Test"), L"Test"_st);
    EXPECT_EQ(ST_LITERAL("Test"), u"Test"_st);
    EXPECT_EQ(ST_LITERAL("Test"), U"Test"_st);
#ifdef ST_HAVE_CXX20_CHAR8_TYPES
    EXPECT_EQ(ST_LITERAL("Test"), u8"Test"_st);
#endif
}
