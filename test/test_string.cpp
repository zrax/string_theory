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
static const size_t utf8_test_data_length = sizeof(utf8_test_data) - 1;

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

/* Utility for comparing char32_t buffers */
template <typename char_T>
static int T_strcmp(const char_T *left, const char_T *right)
{
    for ( ;; ) {
        if (*left != *right)
            return *left - *right;
        if (*left == 0)
            return (*right == 0) ? 0 : -1;
        if (*right == 0)
            return 1;

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
    // Special string null constant
    EXPECT_EQ(ST::string::null, ST_LITERAL(""));
    EXPECT_EQ(ST::string::null, ST::string{});

    EXPECT_EQ(0U, ST::string::null.size());
    EXPECT_TRUE(ST::string::null.is_empty());

    // Short and Long string length
    EXPECT_EQ(4U, ST_LITERAL("1234").size());
    EXPECT_EQ(32U, ST_LITERAL("12345678901234567890123456789012").size());

    // ST::string stores data as UTF-8 internally
    EXPECT_EQ(utf8_test_data_length, ST::string(utf8_test_data).size());
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

TEST(string, utf8)
{
    // From UTF-8 to ST::string
    ST::string from_utf8 = ST::string::from_utf8(utf8_test_data);
    EXPECT_STREQ(utf8_test_data, from_utf8.c_str());
    EXPECT_EQ(utf8_test_data_length, from_utf8.size());
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

TEST(string, utf16)
{
    // From UTF-16 to ST::string
    ST::string from_utf16 = ST::string::from_utf16(utf16_test_data);
    EXPECT_EQ(utf8_test_data_length, from_utf16.size());
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

TEST(string, latin_1)
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

TEST(string, wchar)
{
    // UTF-8 and UTF-16 are already tested, so just make sure we test
    // wchar_t and L"" conversions

    const wchar_t wtext[] = L"\x20\x7f\xff\u0100\uffff";
    const char32_t unicode_text[] = { 0x20, 0x7f, 0xff, 0x100, 0xffff, 0 };
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

TEST(string, conversion_errors)
{
    // The following should encode replacement characters for invalid chars
    const char32_t unicode_replacement[] = { 0xfffd, 0 };
    const char latin1_replacement[] = "?";

    // Character outside of Unicode specification range
    const char32_t too_big_c[] = { 0xffffff, 0 };
    EXPECT_THROW(ST::string::from_utf32(too_big_c, ST_AUTO_SIZE, ST::check_validity),
                 ST::unicode_error);
    ST::utf32_buffer too_big = ST::string::from_utf32(too_big_c, ST_AUTO_SIZE,
                                                      ST::substitute_invalid).to_utf32();
    EXPECT_EQ(0, T_strcmp(unicode_replacement, too_big.data()));

    // Invalid surrogate pairs
    const char16_t incomplete_surr_c[] = { 0xd800, 0 };
    EXPECT_THROW(ST::string::from_utf16(incomplete_surr_c, ST_AUTO_SIZE, ST::check_validity),
                 ST::unicode_error);
    ST::string incomplete_surr = ST::string::from_utf16(incomplete_surr_c, ST_AUTO_SIZE,
                                                        ST::substitute_invalid);
    EXPECT_EQ(0, T_strcmp(unicode_replacement, incomplete_surr.to_utf32().data()));

    const char16_t double_low_c[] = { 0xd800, 0xd801, 0 };
    EXPECT_THROW(ST::string::from_utf16(double_low_c, ST_AUTO_SIZE, ST::check_validity),
                 ST::unicode_error);
    ST::string double_low = ST::string::from_utf16(double_low_c, ST_AUTO_SIZE,
                                                   ST::substitute_invalid);
    EXPECT_EQ(0, T_strcmp(unicode_replacement, double_low.to_utf32().data()));

    const char16_t bad_combo_c[] = { 0xdc00, 0x20, 0 };
    EXPECT_THROW(ST::string::from_utf16(bad_combo_c, ST_AUTO_SIZE, ST::check_validity),
                 ST::unicode_error);
    ST::string bad_combo = ST::string::from_utf16(bad_combo_c, ST_AUTO_SIZE,
                                                  ST::substitute_invalid);
    EXPECT_EQ(0, T_strcmp(unicode_replacement, bad_combo.to_utf32().data()));

    // Latin-1 doesn't have \ufffd, so it uses '?' instead
    const char32_t non_latin1_c[] = { 0x1ff, 0 };
    EXPECT_THROW(ST::string::from_utf32(non_latin1_c).to_latin_1(ST::check_validity),
                 ST::unicode_error);
    ST::char_buffer non_latin1 = ST::string::from_utf32(non_latin1_c).to_latin_1();
    EXPECT_STREQ(latin1_replacement, non_latin1.data());
}

TEST(string, concatenation)
{
    // If this changes, this test may need to be updated to match
    ASSERT_EQ(16, ST_SHORT_STRING_LEN);

    ST::string expected_short = "xxxxyyy";
    ST::string input1 = "xxxx";
    ST::string input2 = "yyy";

    ST::string expected_long = "xxxxxxxxxxyyyyyyyyy";
    ST::string input3 = "xxxxxxxxxx";
    ST::string input4 = "yyyyyyyyy";

    // ST::string + ST::string
    EXPECT_EQ(expected_short, input1 + input2);
    EXPECT_EQ(expected_long, input3 + input4);
    EXPECT_EQ(input1, input1 + ST::string());
    EXPECT_EQ(input1, ST::string() + input1);

    // ST::string + const char*
    EXPECT_EQ(expected_short, input1 + input2.c_str());
    EXPECT_EQ(expected_short, input1.c_str() + input2);
    EXPECT_EQ(expected_long, input3 + input4.c_str());
    EXPECT_EQ(expected_long, input3.c_str() + input4);
    EXPECT_EQ(input1, input1 + "");
    EXPECT_EQ(input1, "" + input1);
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

#ifdef ST_HAVE_INT64
    EXPECT_EQ(ST_LITERAL("0"), ST::string::from_int64(0));
    EXPECT_EQ(ST_LITERAL("-1000000000000"), ST::string::from_int64(-1000000000000LL));
    EXPECT_EQ(ST_LITERAL("1000000000000"), ST::string::from_int64(1000000000000LL));
    EXPECT_EQ(ST_LITERAL("-e8d4a51000"), ST::string::from_int64(-1000000000000LL, 16));
    EXPECT_EQ(ST_LITERAL("e8d4a51000"), ST::string::from_int64(1000000000000LL, 16));
    EXPECT_EQ(ST_LITERAL("-16432451210000"), ST::string::from_int64(-1000000000000LL, 8));
    EXPECT_EQ(ST_LITERAL("16432451210000"), ST::string::from_int64(1000000000000LL, 8));
#endif

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

#ifdef ST_HAVE_INT64
    static const long long int64_min = std::numeric_limits<long long>::min();
    static const long long int64_max = std::numeric_limits<long long>::max();
    EXPECT_EQ(ST_LITERAL("-9223372036854775808"), ST::string::from_int64(int64_min));
    EXPECT_EQ(ST_LITERAL("9223372036854775807"), ST::string::from_int64(int64_max));
    EXPECT_EQ(ST_LITERAL("-8000000000000000"), ST::string::from_int64(int64_min, 16));
    EXPECT_EQ(ST_LITERAL("7fffffffffffffff"), ST::string::from_int64(int64_max, 16, false));
    EXPECT_EQ(ST_LITERAL("7FFFFFFFFFFFFFFF"), ST::string::from_int64(int64_max, 16, true));
    EXPECT_EQ(ST_LITERAL("-1000000000000000000000"), ST::string::from_int64(int64_min, 8));
    EXPECT_EQ(ST_LITERAL("777777777777777777777"), ST::string::from_int64(int64_max, 8));
    EXPECT_EQ(ST_LITERAL("-1000000000000000000000000000000000000000000000000000000000000000"),
              ST::string::from_int64(int64_min, 2));
    EXPECT_EQ(ST_LITERAL("111111111111111111111111111111111111111111111111111111111111111"),
              ST::string::from_int64(int64_max, 2));
#endif
}

TEST(string, from_uint)
{
    EXPECT_EQ(ST_LITERAL("0"), ST::string::from_uint(0));
    EXPECT_EQ(ST_LITERAL("80000"), ST::string::from_uint(80000));
    EXPECT_EQ(ST_LITERAL("13880"), ST::string::from_uint(80000, 16));
    EXPECT_EQ(ST_LITERAL("234200"), ST::string::from_uint(80000, 8));

#ifdef ST_HAVE_INT64
    EXPECT_EQ(ST_LITERAL("0"), ST::string::from_uint64(0));
    EXPECT_EQ(ST_LITERAL("1000000000000"), ST::string::from_uint64(1000000000000ULL));
    EXPECT_EQ(ST_LITERAL("e8d4a51000"), ST::string::from_uint64(1000000000000ULL, 16));
    EXPECT_EQ(ST_LITERAL("16432451210000"), ST::string::from_uint64(1000000000000ULL, 8));
#endif

    static const int uint32_max = std::numeric_limits<unsigned int>::max();
    EXPECT_EQ(ST_LITERAL("4294967295"), ST::string::from_uint(uint32_max));
    EXPECT_EQ(ST_LITERAL("ffffffff"), ST::string::from_uint(uint32_max, 16, false));
    EXPECT_EQ(ST_LITERAL("FFFFFFFF"), ST::string::from_uint(uint32_max, 16, true));
    EXPECT_EQ(ST_LITERAL("37777777777"), ST::string::from_uint(uint32_max, 8));
    EXPECT_EQ(ST_LITERAL("11111111111111111111111111111111"), ST::string::from_uint(uint32_max, 2));

#ifdef ST_HAVE_INT64
    static const unsigned long long uint64_max = std::numeric_limits<unsigned long long>::max();
    EXPECT_EQ(ST_LITERAL("18446744073709551615"), ST::string::from_uint64(uint64_max));
    EXPECT_EQ(ST_LITERAL("ffffffffffffffff"), ST::string::from_uint64(uint64_max, 16, false));
    EXPECT_EQ(ST_LITERAL("FFFFFFFFFFFFFFFF"), ST::string::from_uint64(uint64_max, 16, true));
    EXPECT_EQ(ST_LITERAL("1777777777777777777777"), ST::string::from_uint64(uint64_max, 8));
    EXPECT_EQ(ST_LITERAL("1111111111111111111111111111111111111111111111111111111111111111"),
              ST::string::from_uint64(uint64_max, 2));
#endif
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

    EXPECT_EQ(ST_LITERAL("inf"), ST::string::from_float(INFINITY));
    EXPECT_EQ(ST_LITERAL("inf"), ST::string::from_double(INFINITY));
    EXPECT_EQ(ST_LITERAL("nan"), ST::string::from_float(NAN));
    EXPECT_EQ(ST_LITERAL("nan"), ST::string::from_double(NAN));
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

#ifdef ST_HAVE_INT64
    EXPECT_EQ(0, ST_LITERAL("0").to_int64());
    EXPECT_EQ(0, ST_LITERAL("+0").to_int64());
    EXPECT_EQ(0, ST_LITERAL("-0").to_int64());
#endif

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

#ifdef ST_HAVE_INT64
    EXPECT_EQ(-1000000000000LL, ST_LITERAL("-1000000000000").to_int64());
    EXPECT_EQ(1000000000000LL, ST_LITERAL("1000000000000").to_int64());
    EXPECT_EQ(1000000000000LL, ST_LITERAL("+1000000000000").to_int64());
    EXPECT_EQ(-1000000000000LL, ST_LITERAL("-0xe8d4a51000").to_int64());
    EXPECT_EQ(1000000000000LL, ST_LITERAL("0xe8d4a51000").to_int64());
    EXPECT_EQ(1000000000000LL, ST_LITERAL("+0xe8d4a51000").to_int64());
    EXPECT_EQ(-1000000000000LL, ST_LITERAL("-016432451210000").to_int64());
    EXPECT_EQ(1000000000000LL, ST_LITERAL("016432451210000").to_int64());
    EXPECT_EQ(1000000000000LL, ST_LITERAL("+016432451210000").to_int64());
    EXPECT_EQ(-1000000000000LL, ST_LITERAL("-e8d4a51000").to_int64(16));
    EXPECT_EQ(1000000000000LL, ST_LITERAL("e8d4a51000").to_int64(16));
    EXPECT_EQ(1000000000000LL, ST_LITERAL("+e8d4a51000").to_int64(16));
    EXPECT_EQ(-1000000000000LL, ST_LITERAL("-16432451210000").to_int64(8));
    EXPECT_EQ(1000000000000LL, ST_LITERAL("16432451210000").to_int64(8));
    EXPECT_EQ(1000000000000LL, ST_LITERAL("+16432451210000").to_int64(8));
#endif

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

#ifdef ST_HAVE_INT64
    static const long long int64_min = std::numeric_limits<long long>::min();
    static const long long int64_max = std::numeric_limits<long long>::max();
    EXPECT_EQ(int64_min, ST_LITERAL("-9223372036854775808").to_int64());
    EXPECT_EQ(int64_max, ST_LITERAL("9223372036854775807").to_int64());
    EXPECT_EQ(int64_max, ST_LITERAL("+9223372036854775807").to_int64());
    EXPECT_EQ(int64_min, ST_LITERAL("-0x8000000000000000").to_int64());
    EXPECT_EQ(int64_max, ST_LITERAL("0x7FFFFFFFFFFFFFFF").to_int64());
    EXPECT_EQ(int64_max, ST_LITERAL("+0x7FFFFFFFFFFFFFFF").to_int64());
    EXPECT_EQ(int64_min, ST_LITERAL("-01000000000000000000000").to_int64());
    EXPECT_EQ(int64_max, ST_LITERAL("0777777777777777777777").to_int64());
    EXPECT_EQ(int64_max, ST_LITERAL("+0777777777777777777777").to_int64());
    EXPECT_EQ(int64_min, ST_LITERAL("-8000000000000000").to_int64(16));
    EXPECT_EQ(int64_max, ST_LITERAL("7FFFFFFFFFFFFFFF").to_int64(16));
    EXPECT_EQ(int64_max, ST_LITERAL("+7FFFFFFFFFFFFFFF").to_int64(16));
    EXPECT_EQ(int64_min, ST_LITERAL("-1000000000000000000000").to_int64(8));
    EXPECT_EQ(int64_max, ST_LITERAL("777777777777777777777").to_int64(8));
    EXPECT_EQ(int64_max, ST_LITERAL("+777777777777777777777").to_int64(8));
#endif

    // Empty string is treated as zero for compatibility with strtol
    EXPECT_EQ(0, ST::string::null.to_int());
#ifdef ST_HAVE_INT64
    EXPECT_EQ(0, ST::string::null.to_int64());
#endif
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
    (void) ST::string::null.to_int(result);
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(result.full_match());
}

TEST(string, to_uint)
{
    EXPECT_EQ(0U, ST_LITERAL("0").to_uint());
    EXPECT_EQ(0U, ST_LITERAL("+0").to_uint());
    EXPECT_EQ(0U, ST_LITERAL("-0").to_uint());

#ifdef ST_HAVE_INT64
    EXPECT_EQ(0U, ST_LITERAL("0").to_uint64());
    EXPECT_EQ(0U, ST_LITERAL("+0").to_uint64());
    EXPECT_EQ(0U, ST_LITERAL("-0").to_uint64());
#endif

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

#ifdef ST_HAVE_INT64
    EXPECT_EQ(1000000000000ULL, ST_LITERAL("1000000000000").to_uint64());
    EXPECT_EQ(1000000000000ULL, ST_LITERAL("+1000000000000").to_uint64());
    EXPECT_EQ(1000000000000ULL, ST_LITERAL("0xe8d4a51000").to_uint64());
    EXPECT_EQ(1000000000000ULL, ST_LITERAL("+0xe8d4a51000").to_uint64());
    EXPECT_EQ(1000000000000ULL, ST_LITERAL("016432451210000").to_uint64());
    EXPECT_EQ(1000000000000ULL, ST_LITERAL("+016432451210000").to_uint64());
    EXPECT_EQ(1000000000000ULL, ST_LITERAL("e8d4a51000").to_uint64(16));
    EXPECT_EQ(1000000000000ULL, ST_LITERAL("+e8d4a51000").to_uint64(16));
    EXPECT_EQ(1000000000000ULL, ST_LITERAL("16432451210000").to_uint64(8));
    EXPECT_EQ(1000000000000ULL, ST_LITERAL("+16432451210000").to_uint64(8));
#endif

    static const unsigned int uint32_max = std::numeric_limits<unsigned int>::max();
    EXPECT_EQ(uint32_max, ST_LITERAL("4294967295").to_uint());
    EXPECT_EQ(uint32_max, ST_LITERAL("+4294967295").to_uint());
    EXPECT_EQ(uint32_max, ST_LITERAL("0xFFFFFFFF").to_uint());
    EXPECT_EQ(uint32_max, ST_LITERAL("+0xFFFFFFFF").to_uint());
    EXPECT_EQ(uint32_max, ST_LITERAL("037777777777").to_uint());
    EXPECT_EQ(uint32_max, ST_LITERAL("+037777777777").to_uint());

#ifdef ST_HAVE_INT64
    static const unsigned long long uint64_max = std::numeric_limits<unsigned long long>::max();
    EXPECT_EQ(uint64_max, ST_LITERAL("18446744073709551615").to_uint64());
    EXPECT_EQ(uint64_max, ST_LITERAL("+18446744073709551615").to_uint64());
    EXPECT_EQ(uint64_max, ST_LITERAL("0xFFFFFFFFFFFFFFFF").to_uint64());
    EXPECT_EQ(uint64_max, ST_LITERAL("+0xFFFFFFFFFFFFFFFF").to_uint64());
    EXPECT_EQ(uint64_max, ST_LITERAL("01777777777777777777777").to_uint64());
    EXPECT_EQ(uint64_max, ST_LITERAL("+01777777777777777777777").to_uint64());
#endif

    // Empty string is treated as zero for compatibility with strtoul
    EXPECT_EQ(0U, ST::string::null.to_uint());
#ifdef ST_HAVE_INT64
    EXPECT_EQ(0U, ST::string::null.to_uint64());
#endif
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
    (void) ST::string::null.to_uint(result);
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

    EXPECT_TRUE(std::isinf(ST_LITERAL("inf").to_float()));
    EXPECT_TRUE(std::isnan(ST_LITERAL("nan").to_float()));

    EXPECT_TRUE(std::isinf(ST_LITERAL("inf").to_double()));
    EXPECT_TRUE(std::isnan(ST_LITERAL("nan").to_double()));

    // Empty string is treated as zero for compatibility with strtod
    EXPECT_EQ(0.0f, ST::string::null.to_float());
    EXPECT_EQ(0.0, ST::string::null.to_double());
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

    (void) ST_LITERAL("INF").to_float(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("NAN").to_float(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("INF").to_double(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST_LITERAL("NAN").to_double(result);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.full_match());

    (void) ST_LITERAL("16xx").to_float(result);
    EXPECT_TRUE(result.ok());
    EXPECT_FALSE(result.full_match());
    (void) ST_LITERAL("INFxx").to_float(result);
    EXPECT_TRUE(result.ok());
    EXPECT_FALSE(result.full_match());
    (void) ST_LITERAL("NANxx").to_float(result);
    EXPECT_TRUE(result.ok());
    EXPECT_FALSE(result.full_match());
    (void) ST_LITERAL("16xx").to_double(result);
    EXPECT_TRUE(result.ok());
    EXPECT_FALSE(result.full_match());
    (void) ST_LITERAL("INFxx").to_double(result);
    EXPECT_TRUE(result.ok());
    EXPECT_FALSE(result.full_match());
    (void) ST_LITERAL("NANxx").to_double(result);
    EXPECT_TRUE(result.ok());
    EXPECT_FALSE(result.full_match());

    (void) ST_LITERAL("xx").to_float(result);
    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.full_match());
    (void) ST_LITERAL("xx").to_double(result);
    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.full_match());

    (void) ST::string::null.to_float(result);
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(result.full_match());
    (void) ST::string::null.to_double(result);
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

    EXPECT_FALSE(ST::string::null.to_bool());
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

    // Empty string
    EXPECT_EQ(-1, ST::string().find("AAAA", ST::case_sensitive));
    EXPECT_EQ(-1, ST::string().find("AAAA", ST::case_insensitive));

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
    EXPECT_EQ(std::vector<ST::string>{}, ST_LITERAL("\t;\n;").tokenize("\t\n-;"));
    EXPECT_EQ(std::vector<ST::string>{}, ST_LITERAL("").tokenize("\t\n-;"));
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
    expected9.push_back(ST::string::null);
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
    expected9.push_back(ST::string::null);
    EXPECT_EQ(expected9, ST_LITERAL("").split('-'));
    EXPECT_EQ(expected9, ST_LITERAL("").split('-', 4));
}

TEST(string, fill)
{
    EXPECT_EQ(ST_LITERAL(""), ST::string::fill(0, 'a'));
    EXPECT_EQ(ST_LITERAL("aaaaa"), ST::string::fill(5, 'a'));
    EXPECT_EQ(ST_LITERAL("aaaaaaaaaaaaaaaaaaaa"), ST::string::fill(20, 'a'));
}
