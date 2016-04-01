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
    EXPECT_EQ(ST::string::null, ST::string(""));
    EXPECT_EQ(ST::string::null, ST::string{});

    EXPECT_EQ(0, ST::string::null.size());
    EXPECT_TRUE(ST::string::null.is_empty());

    // Short and Long string length
    EXPECT_EQ(4, ST::string("1234").size());
    EXPECT_EQ(32, ST::string("12345678901234567890123456789012").size());

    // ST::string stores data as UTF-8 internally
    EXPECT_EQ(utf8_test_data_length, ST::string(utf8_test_data).size());
}

TEST(string, stack_construction)
{
    char stack_buf[256];
    strcpy(stack_buf, "Test");
    ST::string test(stack_buf);

    EXPECT_EQ(ST::string("Test"), test);
    EXPECT_EQ(strlen("Test"), test.size());

    wchar_t wstack_buf[256];
    wcscpy(wstack_buf, L"Test");
    ST::string wtest(wstack_buf);

    EXPECT_EQ(ST::string(L"Test"), wtest);
    EXPECT_EQ(strlen("Test"), wtest.size());

    strcpy(stack_buf, "operator=");
    test = stack_buf;

    EXPECT_EQ(ST::string("operator="), test);
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
    EXPECT_EQ(0, empty.size());
    EXPECT_EQ(0, T_strcmp(empty.c_str(), ""));

    const char32_t empty_data[] = { 0 };
    empty = ST::string::from_utf32(empty_data);
    EXPECT_EQ(0, empty.size());
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
    EXPECT_EQ(0, empty.size());
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
    EXPECT_EQ(0, empty.size());
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
    EXPECT_EQ(0, empty.size());
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

TEST(string, to_int)
{
    EXPECT_EQ(0, ST::string("0").to_int());
    EXPECT_EQ(0, ST::string("+0").to_int());
    EXPECT_EQ(0, ST::string("-0").to_int());

#ifdef ST_HAVE_INT64
    EXPECT_EQ(0, ST::string("0").to_int64());
    EXPECT_EQ(0, ST::string("+0").to_int64());
    EXPECT_EQ(0, ST::string("-0").to_int64());
#endif

    EXPECT_EQ(-80000, ST::string("-80000").to_int());
    EXPECT_EQ(80000, ST::string("80000").to_int());
    EXPECT_EQ(80000, ST::string("+80000").to_int());
    EXPECT_EQ(-80000, ST::string("-0x13880").to_int());
    EXPECT_EQ(80000, ST::string("0x13880").to_int());
    EXPECT_EQ(80000, ST::string("+0x13880").to_int());
    EXPECT_EQ(-80000, ST::string("-0234200").to_int());
    EXPECT_EQ(80000, ST::string("0234200").to_int());
    EXPECT_EQ(80000, ST::string("+0234200").to_int());
    EXPECT_EQ(-80000, ST::string("-13880").to_int(16));
    EXPECT_EQ(80000, ST::string("13880").to_int(16));
    EXPECT_EQ(80000, ST::string("+13880").to_int(16));
    EXPECT_EQ(-80000, ST::string("-234200").to_int(8));
    EXPECT_EQ(80000, ST::string("234200").to_int(8));
    EXPECT_EQ(80000, ST::string("+234200").to_int(8));

#ifdef ST_HAVE_INT64
    EXPECT_EQ(-1000000000000LL, ST::string("-1000000000000").to_int64());
    EXPECT_EQ(1000000000000LL, ST::string("1000000000000").to_int64());
    EXPECT_EQ(1000000000000LL, ST::string("+1000000000000").to_int64());
    EXPECT_EQ(-1000000000000LL, ST::string("-0xe8d4a51000").to_int64());
    EXPECT_EQ(1000000000000LL, ST::string("0xe8d4a51000").to_int64());
    EXPECT_EQ(1000000000000LL, ST::string("+0xe8d4a51000").to_int64());
    EXPECT_EQ(-1000000000000LL, ST::string("-016432451210000").to_int64());
    EXPECT_EQ(1000000000000LL, ST::string("016432451210000").to_int64());
    EXPECT_EQ(1000000000000LL, ST::string("+016432451210000").to_int64());
    EXPECT_EQ(-1000000000000LL, ST::string("-e8d4a51000").to_int64(16));
    EXPECT_EQ(1000000000000LL, ST::string("e8d4a51000").to_int64(16));
    EXPECT_EQ(1000000000000LL, ST::string("+e8d4a51000").to_int64(16));
    EXPECT_EQ(-1000000000000LL, ST::string("-16432451210000").to_int64(8));
    EXPECT_EQ(1000000000000LL, ST::string("16432451210000").to_int64(8));
    EXPECT_EQ(1000000000000LL, ST::string("+16432451210000").to_int64(8));
#endif

    static const int int32_min = std::numeric_limits<int>::min();
    static const int int32_max = std::numeric_limits<int>::max();
    EXPECT_EQ(int32_min, ST::string("-2147483648").to_int());
    EXPECT_EQ(int32_max, ST::string("2147483647").to_int());
    EXPECT_EQ(int32_max, ST::string("+2147483647").to_int());
    EXPECT_EQ(int32_min, ST::string("-0x80000000").to_int());
    EXPECT_EQ(int32_max, ST::string("0x7FFFFFFF").to_int());
    EXPECT_EQ(int32_max, ST::string("+0x7FFFFFFF").to_int());
    EXPECT_EQ(int32_min, ST::string("-020000000000").to_int());
    EXPECT_EQ(int32_max, ST::string("017777777777").to_int());
    EXPECT_EQ(int32_max, ST::string("+017777777777").to_int());
    EXPECT_EQ(int32_min, ST::string("-80000000").to_int(16));
    EXPECT_EQ(int32_max, ST::string("7FFFFFFF").to_int(16));
    EXPECT_EQ(int32_max, ST::string("+7FFFFFFF").to_int(16));
    EXPECT_EQ(int32_min, ST::string("-20000000000").to_int(8));
    EXPECT_EQ(int32_max, ST::string("17777777777").to_int(8));
    EXPECT_EQ(int32_max, ST::string("+17777777777").to_int(8));

#ifdef ST_HAVE_INT64
    static const long long int64_min = std::numeric_limits<long long>::min();
    static const long long int64_max = std::numeric_limits<long long>::max();
    EXPECT_EQ(int64_min, ST::string("-9223372036854775808").to_int64());
    EXPECT_EQ(int64_max, ST::string("9223372036854775807").to_int64());
    EXPECT_EQ(int64_max, ST::string("+9223372036854775807").to_int64());
    EXPECT_EQ(int64_min, ST::string("-0x8000000000000000").to_int64());
    EXPECT_EQ(int64_max, ST::string("0x7FFFFFFFFFFFFFFF").to_int64());
    EXPECT_EQ(int64_max, ST::string("+0x7FFFFFFFFFFFFFFF").to_int64());
    EXPECT_EQ(int64_min, ST::string("-01000000000000000000000").to_int64());
    EXPECT_EQ(int64_max, ST::string("0777777777777777777777").to_int64());
    EXPECT_EQ(int64_max, ST::string("+0777777777777777777777").to_int64());
    EXPECT_EQ(int64_min, ST::string("-8000000000000000").to_int64(16));
    EXPECT_EQ(int64_max, ST::string("7FFFFFFFFFFFFFFF").to_int64(16));
    EXPECT_EQ(int64_max, ST::string("+7FFFFFFFFFFFFFFF").to_int64(16));
    EXPECT_EQ(int64_min, ST::string("-1000000000000000000000").to_int64(8));
    EXPECT_EQ(int64_max, ST::string("777777777777777777777").to_int64(8));
    EXPECT_EQ(int64_max, ST::string("+777777777777777777777").to_int64(8));
#endif

    // Empty string is treated as zero for compatibility with strtol
    EXPECT_EQ(0, ST::string::null.to_int());
#ifdef ST_HAVE_INT64
    EXPECT_EQ(0, ST::string::null.to_int64());
#endif
}

TEST(string, to_uint)
{
    EXPECT_EQ(0, ST::string("0").to_uint());
    EXPECT_EQ(0, ST::string("+0").to_uint());
    EXPECT_EQ(0, ST::string("-0").to_uint());

#ifdef ST_HAVE_INT64
    EXPECT_EQ(0, ST::string("0").to_uint64());
    EXPECT_EQ(0, ST::string("+0").to_uint64());
    EXPECT_EQ(0, ST::string("-0").to_uint64());
#endif

    EXPECT_EQ(80000, ST::string("80000").to_uint());
    EXPECT_EQ(80000, ST::string("+80000").to_uint());
    EXPECT_EQ(80000, ST::string("0x13880").to_uint());
    EXPECT_EQ(80000, ST::string("+0x13880").to_uint());
    EXPECT_EQ(80000, ST::string("0234200").to_uint());
    EXPECT_EQ(80000, ST::string("+0234200").to_uint());
    EXPECT_EQ(80000, ST::string("13880").to_uint(16));
    EXPECT_EQ(80000, ST::string("+13880").to_uint(16));
    EXPECT_EQ(80000, ST::string("234200").to_uint(8));
    EXPECT_EQ(80000, ST::string("+234200").to_uint(8));

#ifdef ST_HAVE_INT64
    EXPECT_EQ(1000000000000ULL, ST::string("1000000000000").to_uint64());
    EXPECT_EQ(1000000000000ULL, ST::string("+1000000000000").to_uint64());
    EXPECT_EQ(1000000000000ULL, ST::string("0xe8d4a51000").to_uint64());
    EXPECT_EQ(1000000000000ULL, ST::string("+0xe8d4a51000").to_uint64());
    EXPECT_EQ(1000000000000ULL, ST::string("016432451210000").to_uint64());
    EXPECT_EQ(1000000000000ULL, ST::string("+016432451210000").to_uint64());
    EXPECT_EQ(1000000000000ULL, ST::string("e8d4a51000").to_uint64(16));
    EXPECT_EQ(1000000000000ULL, ST::string("+e8d4a51000").to_uint64(16));
    EXPECT_EQ(1000000000000ULL, ST::string("16432451210000").to_uint64(8));
    EXPECT_EQ(1000000000000ULL, ST::string("+16432451210000").to_uint64(8));
#endif

    static const unsigned int uint32_max = std::numeric_limits<unsigned int>::max();
    EXPECT_EQ(uint32_max, ST::string("4294967295").to_uint());
    EXPECT_EQ(uint32_max, ST::string("+4294967295").to_uint());
    EXPECT_EQ(uint32_max, ST::string("0xFFFFFFFF").to_uint());
    EXPECT_EQ(uint32_max, ST::string("+0xFFFFFFFF").to_uint());
    EXPECT_EQ(uint32_max, ST::string("037777777777").to_uint());
    EXPECT_EQ(uint32_max, ST::string("+037777777777").to_uint());

#ifdef ST_HAVE_INT64
    static const unsigned long long uint64_max = std::numeric_limits<unsigned long long>::max();
    EXPECT_EQ(uint64_max, ST::string("18446744073709551615").to_uint64());
    EXPECT_EQ(uint64_max, ST::string("+18446744073709551615").to_uint64());
    EXPECT_EQ(uint64_max, ST::string("0xFFFFFFFFFFFFFFFF").to_uint64());
    EXPECT_EQ(uint64_max, ST::string("+0xFFFFFFFFFFFFFFFF").to_uint64());
    EXPECT_EQ(uint64_max, ST::string("01777777777777777777777").to_uint64());
    EXPECT_EQ(uint64_max, ST::string("+01777777777777777777777").to_uint64());
#endif

    // Empty string is treated as zero for compatibility with strtoul
    EXPECT_EQ(0, ST::string::null.to_uint());
#ifdef ST_HAVE_INT64
    EXPECT_EQ(0, ST::string::null.to_uint64());
#endif
}

TEST(string, to_float)
{
    EXPECT_EQ(0.0f, ST::string("0").to_float());
    EXPECT_EQ(0.0f, ST::string("+0").to_float());
    EXPECT_EQ(0.0f, ST::string("-0").to_float());

    EXPECT_EQ(0.0, ST::string("0").to_double());
    EXPECT_EQ(0.0, ST::string("+0").to_double());
    EXPECT_EQ(0.0, ST::string("-0").to_double());

    EXPECT_EQ(-16.0f, ST::string("-16").to_float());
    EXPECT_EQ(16.0f, ST::string("16").to_float());
    EXPECT_EQ(16.0f, ST::string("+16").to_float());
    EXPECT_EQ(-16.0f, ST::string("-16.0").to_float());
    EXPECT_EQ(16.0f, ST::string("16.0").to_float());
    EXPECT_EQ(16.0f, ST::string("+16.0").to_float());
    EXPECT_EQ(-16.0f, ST::string("-1.6e1").to_float());
    EXPECT_EQ(16.0f, ST::string("1.6e1").to_float());
    EXPECT_EQ(16.0f, ST::string("+1.6e1").to_float());

    EXPECT_EQ(-16.0, ST::string("-16").to_double());
    EXPECT_EQ(16.0, ST::string("16").to_double());
    EXPECT_EQ(16.0, ST::string("+16").to_double());
    EXPECT_EQ(-16.0, ST::string("-16.0").to_double());
    EXPECT_EQ(16.0, ST::string("16.0").to_double());
    EXPECT_EQ(16.0, ST::string("+16.0").to_double());
    EXPECT_EQ(-16.0, ST::string("-1.6e1").to_double());
    EXPECT_EQ(16.0, ST::string("1.6e1").to_double());
    EXPECT_EQ(16.0, ST::string("+1.6e1").to_double());

    EXPECT_TRUE(std::isinf(ST::string("INF").to_float()));
    EXPECT_TRUE(std::isnan(ST::string("NAN").to_float()));

    EXPECT_TRUE(std::isinf(ST::string("INF").to_double()));
    EXPECT_TRUE(std::isnan(ST::string("NAN").to_double()));

    // Empty string is treated as zero for compatibility with strtod
    EXPECT_EQ(0.0f, ST::string::null.to_float());
    EXPECT_EQ(0.0, ST::string::null.to_double());
}

TEST(string, to_bool)
{
    EXPECT_TRUE(ST::string("true").to_bool());
    EXPECT_TRUE(ST::string("TRUE").to_bool());
    EXPECT_FALSE(ST::string("false").to_bool());
    EXPECT_FALSE(ST::string("FALSE").to_bool());

    EXPECT_FALSE(ST::string("0").to_bool());
    EXPECT_TRUE(ST::string("1").to_bool());
    EXPECT_TRUE(ST::string("-1").to_bool());
    EXPECT_TRUE(ST::string("1000").to_bool());
    EXPECT_TRUE(ST::string("0x1000").to_bool());

    EXPECT_FALSE(ST::string::null.to_bool());
}

TEST(string, compare)
{
    // Same length, case sensitive
    EXPECT_EQ(0, ST::string("abc").compare("abc", ST::case_sensitive));
    EXPECT_GT(0, ST::string("abc").compare("abd", ST::case_sensitive));
    EXPECT_LT(0, ST::string("abc").compare("abb", ST::case_sensitive));
    EXPECT_GT(0, ST::string("abC").compare("abc", ST::case_sensitive));
    EXPECT_GT(0, ST::string("Abc").compare("abc", ST::case_sensitive));
    EXPECT_EQ(0, ST::string().compare("", ST::case_sensitive));

    // Same length, case insensitive
    EXPECT_EQ(0, ST::string("abc").compare("abc", ST::case_insensitive));
    EXPECT_EQ(0, ST::string("abc").compare("ABC", ST::case_insensitive));
    EXPECT_GT(0, ST::string("abc").compare("abD", ST::case_insensitive));
    EXPECT_LT(0, ST::string("abc").compare("abB", ST::case_insensitive));
    EXPECT_EQ(0, ST::string().compare("", ST::case_insensitive));

    // Mismatched length, case sensitive
    EXPECT_LT(0, ST::string("abc").compare("ab", ST::case_sensitive));
    EXPECT_GT(0, ST::string("abc").compare("abcd", ST::case_sensitive));
    EXPECT_LT(0, ST::string("abc").compare("", ST::case_sensitive));
    EXPECT_GT(0, ST::string("").compare("abc", ST::case_sensitive));

    // Mismatched length, case insensitive
    EXPECT_LT(0, ST::string("abc").compare("Ab", ST::case_insensitive));
    EXPECT_GT(0, ST::string("abc").compare("Abcd", ST::case_insensitive));
    EXPECT_LT(0, ST::string("abc").compare("", ST::case_insensitive));
    EXPECT_GT(0, ST::string().compare("abc", ST::case_insensitive));
}

TEST(string, compare_n)
{
    // Same length, case sensitive
    EXPECT_EQ(0, ST::string("abcXX").compare_n("abcYY", 3, ST::case_sensitive));
    EXPECT_GT(0, ST::string("abcXX").compare_n("abdYY", 3, ST::case_sensitive));
    EXPECT_LT(0, ST::string("abcXX").compare_n("abbYY", 3, ST::case_sensitive));
    EXPECT_GT(0, ST::string("abCXX").compare_n("abcYY", 3, ST::case_sensitive));
    EXPECT_GT(0, ST::string("AbcXX").compare_n("abcYY", 3, ST::case_sensitive));

    // Same length, case insensitive
    EXPECT_EQ(0, ST::string("abcXX").compare_n("abcYY", 3, ST::case_insensitive));
    EXPECT_EQ(0, ST::string("abcXX").compare_n("ABCYY", 3, ST::case_insensitive));
    EXPECT_GT(0, ST::string("abcXX").compare_n("abDYY", 3, ST::case_insensitive));
    EXPECT_LT(0, ST::string("abcXX").compare_n("abBYY", 3, ST::case_insensitive));

    // Mismatched length, case sensitive
    EXPECT_LT(0, ST::string("abc").compare_n("ab", 3, ST::case_sensitive));
    EXPECT_GT(0, ST::string("abc").compare_n("abcd", 4, ST::case_sensitive));
    EXPECT_LT(0, ST::string("abc").compare_n("", 3, ST::case_sensitive));
    EXPECT_GT(0, ST::string("").compare_n("abc", 3, ST::case_sensitive));

    // Mismatched length, case insensitive
    EXPECT_LT(0, ST::string("abc").compare_n("Ab", 3, ST::case_insensitive));
    EXPECT_GT(0, ST::string("abc").compare_n("Abcd", 4, ST::case_insensitive));
    EXPECT_LT(0, ST::string("abc").compare_n("", 3, ST::case_insensitive));
    EXPECT_GT(0, ST::string().compare_n("abc", 3, ST::case_insensitive));
}

TEST(string, find_char)
{
    // Available char, case sensitive
    EXPECT_EQ(0, ST::string("Aaaaaaaa").find('A', ST::case_sensitive));
    EXPECT_EQ(0, ST::string("AaaaAaaa").find('A', ST::case_sensitive));
    EXPECT_EQ(4, ST::string("aaaaAaaa").find('A', ST::case_sensitive));
    EXPECT_EQ(7, ST::string("aaaaaaaA").find('A', ST::case_sensitive));

    // Available char, case insensitive
    EXPECT_EQ(0, ST::string("Abbbbbbb").find('A', ST::case_insensitive));
    EXPECT_EQ(0, ST::string("AbbbAbbb").find('A', ST::case_insensitive));
    EXPECT_EQ(4, ST::string("bbbbAbbb").find('A', ST::case_insensitive));
    EXPECT_EQ(7, ST::string("bbbbbbbA").find('A', ST::case_insensitive));
    EXPECT_EQ(0, ST::string("abbbbbbb").find('A', ST::case_insensitive));
    EXPECT_EQ(0, ST::string("abbbabbb").find('A', ST::case_insensitive));
    EXPECT_EQ(4, ST::string("bbbbabbb").find('A', ST::case_insensitive));
    EXPECT_EQ(7, ST::string("bbbbbbba").find('A', ST::case_insensitive));
    EXPECT_EQ(0, ST::string("Abbbbbbb").find('a', ST::case_insensitive));
    EXPECT_EQ(0, ST::string("AbbbAbbb").find('a', ST::case_insensitive));
    EXPECT_EQ(4, ST::string("bbbbAbbb").find('a', ST::case_insensitive));
    EXPECT_EQ(7, ST::string("bbbbbbbA").find('a', ST::case_insensitive));

    // Unavailable char
    EXPECT_EQ(-1, ST::string("AaaaAaaa").find('C', ST::case_sensitive));
    EXPECT_EQ(-1, ST::string("caaacaaa").find('C', ST::case_sensitive));
    EXPECT_EQ(-1, ST::string("AaaaAaaa").find('C', ST::case_insensitive));

    // Empty string
    EXPECT_EQ(-1, ST::string().find('A', ST::case_sensitive));
    EXPECT_EQ(-1, ST::string().find('A', ST::case_insensitive));
}

TEST(string, find_last_char)
{
    // Available char, case sensitive
    EXPECT_EQ(0, ST::string("Aaaaaaaa").find_last('A', ST::case_sensitive));
    EXPECT_EQ(4, ST::string("AaaaAaaa").find_last('A', ST::case_sensitive));
    EXPECT_EQ(4, ST::string("aaaaAaaa").find_last('A', ST::case_sensitive));
    EXPECT_EQ(7, ST::string("aaaaaaaA").find_last('A', ST::case_sensitive));

    // Available char, case insensitive
    EXPECT_EQ(0, ST::string("Abbbbbbb").find_last('A', ST::case_insensitive));
    EXPECT_EQ(4, ST::string("AbbbAbbb").find_last('A', ST::case_insensitive));
    EXPECT_EQ(4, ST::string("bbbbAbbb").find_last('A', ST::case_insensitive));
    EXPECT_EQ(7, ST::string("bbbbbbbA").find_last('A', ST::case_insensitive));
    EXPECT_EQ(0, ST::string("abbbbbbb").find_last('A', ST::case_insensitive));
    EXPECT_EQ(4, ST::string("abbbabbb").find_last('A', ST::case_insensitive));
    EXPECT_EQ(4, ST::string("bbbbabbb").find_last('A', ST::case_insensitive));
    EXPECT_EQ(7, ST::string("bbbbbbba").find_last('A', ST::case_insensitive));
    EXPECT_EQ(0, ST::string("Abbbbbbb").find_last('a', ST::case_insensitive));
    EXPECT_EQ(4, ST::string("AbbbAbbb").find_last('a', ST::case_insensitive));
    EXPECT_EQ(4, ST::string("bbbbAbbb").find_last('a', ST::case_insensitive));
    EXPECT_EQ(7, ST::string("bbbbbbbA").find_last('a', ST::case_insensitive));

    // Unavailable char
    EXPECT_EQ(-1, ST::string("AaaaAaaa").find_last('C', ST::case_sensitive));
    EXPECT_EQ(-1, ST::string("caaacaaa").find_last('C', ST::case_sensitive));
    EXPECT_EQ(-1, ST::string("AaaaAaaa").find_last('C', ST::case_insensitive));

    // Empty string
    EXPECT_EQ(-1, ST::string().find_last('A', ST::case_sensitive));
    EXPECT_EQ(-1, ST::string().find_last('A', ST::case_insensitive));
}

TEST(string, find)
{
    // Available string, case sensitive
    EXPECT_EQ(0, ST::string("ABCDabcd").find("ABCD", ST::case_sensitive));
    EXPECT_EQ(4, ST::string("abcdABCDABCDabcd").find("ABCD", ST::case_sensitive));
    EXPECT_EQ(4, ST::string("abcdABCDabcd").find("ABCD", ST::case_sensitive));
    EXPECT_EQ(4, ST::string("abcdABCD").find("ABCD", ST::case_sensitive));

    // Available string, case insensitive
    EXPECT_EQ(0, ST::string("ABCDxxxx").find("ABCD", ST::case_insensitive));
    EXPECT_EQ(4, ST::string("xxxxABCDABCDxxxx").find("ABCD", ST::case_insensitive));
    EXPECT_EQ(4, ST::string("xxxxABCDxxxx").find("ABCD", ST::case_insensitive));
    EXPECT_EQ(4, ST::string("xxxxABCD").find("ABCD", ST::case_insensitive));
    EXPECT_EQ(0, ST::string("abcdxxxx").find("ABCD", ST::case_insensitive));
    EXPECT_EQ(4, ST::string("xxxxabcdABCDxxxx").find("ABCD", ST::case_insensitive));
    EXPECT_EQ(4, ST::string("xxxxabcdxxxx").find("ABCD", ST::case_insensitive));
    EXPECT_EQ(4, ST::string("xxxxabcd").find("ABCD", ST::case_insensitive));
    EXPECT_EQ(0, ST::string("ABCDxxxx").find("abcd", ST::case_insensitive));
    EXPECT_EQ(4, ST::string("xxxxABCDabcdxxxx").find("abcd", ST::case_insensitive));
    EXPECT_EQ(4, ST::string("xxxxABCDxxxx").find("abcd", ST::case_insensitive));
    EXPECT_EQ(4, ST::string("xxxxABCD").find("abcd", ST::case_insensitive));

    // Unavailable string
    EXPECT_EQ(-1, ST::string("xxxx").find("ABCD", ST::case_sensitive));
    EXPECT_EQ(-1, ST::string("xxxx").find("ABCD", ST::case_insensitive));

    // Empty string
    EXPECT_EQ(-1, ST::string().find("AAAA", ST::case_sensitive));
    EXPECT_EQ(-1, ST::string().find("AAAA", ST::case_insensitive));

    // Unicode substring
    ST::string haystack;
    haystack = ST::string("xxxx") + ST::string::from_utf32(test_data);
    EXPECT_EQ(4, haystack.find(utf8_test_data, ST::case_sensitive));
    EXPECT_EQ(4, haystack.find(utf8_test_data, ST::case_insensitive));

    haystack = ST::string::from_utf32(test_data) + ST::string("xxxx");
    EXPECT_EQ(0, haystack.find(utf8_test_data, ST::case_sensitive));
    EXPECT_EQ(0, haystack.find(utf8_test_data, ST::case_insensitive));

    haystack = ST::string("xxxx") + ST::string::from_utf32(test_data) + ST::string("xxxx");
    EXPECT_EQ(4, haystack.find(utf8_test_data, ST::case_sensitive));
    EXPECT_EQ(4, haystack.find(utf8_test_data, ST::case_insensitive));
}

TEST(string, trim)
{
    EXPECT_EQ(ST::string("xxx   "), ST::string("   xxx   ").trim_left(" \t\r\n"));
    EXPECT_EQ(ST::string("xxx\t"), ST::string("\txxx\t").trim_left(" \t\r\n"));
    EXPECT_EQ(ST::string("xxx\r\n"), ST::string("\r\nxxx\r\n").trim_left(" \t\r\n"));
    EXPECT_EQ(ST::string("   xxx   "), ST::string("   xxx   ").trim_left("abc"));
    EXPECT_EQ(ST::string("   xxx   "), ST::string("   xxx   ").trim_left("x"));

    EXPECT_EQ(ST::string("   xxx"), ST::string("   xxx   ").trim_right(" \t\r\n"));
    EXPECT_EQ(ST::string("\txxx"), ST::string("\txxx\t").trim_right(" \t\r\n"));
    EXPECT_EQ(ST::string("\r\nxxx"), ST::string("\r\nxxx\r\n").trim_right(" \t\r\n"));
    EXPECT_EQ(ST::string("   xxx   "), ST::string("   xxx   ").trim_right("abc"));
    EXPECT_EQ(ST::string("   xxx   "), ST::string("   xxx   ").trim_right("x"));

    EXPECT_EQ(ST::string("xxx"), ST::string("   xxx   ").trim(" \t\r\n"));
    EXPECT_EQ(ST::string("xxx"), ST::string("\txxx\t").trim(" \t\r\n"));
    EXPECT_EQ(ST::string("xxx"), ST::string("\r\nxxx\r\n").trim(" \t\r\n"));
    EXPECT_EQ(ST::string("   xxx   "), ST::string("   xxx   ").trim("abc"));
    EXPECT_EQ(ST::string("   xxx   "), ST::string("   xxx   ").trim("x"));
}

TEST(string, substrings)
{
    EXPECT_EQ(ST::string("AAA"), ST::string("AAA").left(3));
    EXPECT_EQ(ST::string("AAA"), ST::string("AAAxxxx").left(3));
    EXPECT_EQ(ST::string("A"), ST::string("A").left(3));
    EXPECT_EQ(ST::string(""), ST::string("").left(3));

    EXPECT_EQ(ST::string("AAA"), ST::string("AAA").right(3));
    EXPECT_EQ(ST::string("AAA"), ST::string("xxxxAAA").right(3));
    EXPECT_EQ(ST::string("A"), ST::string("A").right(3));
    EXPECT_EQ(ST::string(""), ST::string("").right(3));

    EXPECT_EQ(ST::string("AAA"), ST::string("AAAxxxx").substr(0, 3));
    EXPECT_EQ(ST::string("AAA"), ST::string("xxxxAAA").substr(4, 3));
    EXPECT_EQ(ST::string("AAA"), ST::string("xxAAAxx").substr(2, 3));

    EXPECT_EQ(ST::string(""), ST::string("AAAA").substr(2, 0));
    EXPECT_EQ(ST::string("AA"), ST::string("AAAA").substr(2, 4));
    EXPECT_EQ(ST::string(""), ST::string("AAAA").substr(6, 4));
    EXPECT_EQ(ST::string("AAAA"), ST::string("AAAA").substr(0, 4));
    EXPECT_EQ(ST::string(""), ST::string("").substr(0, 4));

    // Negative indexes start from the right
    EXPECT_EQ(ST::string("AAA"), ST::string("xxxxAAA").substr(-3, 3));
    EXPECT_EQ(ST::string("AAA"), ST::string("xxAAAxx").substr(-5, 3));
    EXPECT_EQ(ST::string("AAA"), ST::string("xxxxAAA").substr(-3, 6));
    EXPECT_EQ(ST::string("AAA"), ST::string("AAAxxxx").substr(-10, 3));
    EXPECT_EQ(ST::string("AAA"), ST::string("AAA").substr(-10, 10));
}

TEST(string, starts_with)
{
    EXPECT_TRUE(ST::string("AAA").starts_with("AAA"));
    EXPECT_TRUE(ST::string("AAAxxx").starts_with("AAA"));
    EXPECT_TRUE(ST::string("AAAAA").starts_with("AAA"));
    EXPECT_FALSE(ST::string("xxx").starts_with("AAA"));
    EXPECT_FALSE(ST::string("AAxxx").starts_with("AAA"));
    EXPECT_FALSE(ST::string("xAAAxxx").starts_with("AAA"));
    EXPECT_FALSE(ST::string("").starts_with("AAA"));
    EXPECT_TRUE(ST::string("xxx").starts_with(""));
}

TEST(string, ends_with)
{
    EXPECT_TRUE(ST::string("AAA").ends_with("AAA"));
    EXPECT_TRUE(ST::string("xxxAAA").ends_with("AAA"));
    EXPECT_TRUE(ST::string("AAAAA").ends_with("AAA"));
    EXPECT_FALSE(ST::string("xxx").ends_with("AAA"));
    EXPECT_FALSE(ST::string("xxxAA").ends_with("AAA"));
    EXPECT_FALSE(ST::string("xxxAAAx").ends_with("AAA"));
    EXPECT_FALSE(ST::string("").ends_with("AAA"));
    EXPECT_TRUE(ST::string("xxx").ends_with(""));
}

TEST(string, before_after)
{
    EXPECT_EQ(ST::string("AAA"), ST::string("AAA;BBB").before_first(';'));
    EXPECT_EQ(ST::string("AAA"), ST::string("AAA##SEP##BBB").before_first("##SEP##"));
    EXPECT_EQ(ST::string("AAA"), ST::string("AAA;BBB;CCC").before_first(';'));
    EXPECT_EQ(ST::string("AAA"), ST::string("AAA##SEP##BBB##SEP##CCC").before_first("##SEP##"));
    EXPECT_EQ(ST::string("AAA"), ST::string("AAA").before_first(';'));
    EXPECT_EQ(ST::string("AAA"), ST::string("AAA").before_first("##SEP##"));
    EXPECT_EQ(ST::string(""), ST::string(";").before_first(';'));
    EXPECT_EQ(ST::string(""), ST::string("##SEP##").before_first("##SEP##"));
    EXPECT_EQ(ST::string(""), ST::string("").before_first(';'));
    EXPECT_EQ(ST::string(""), ST::string("").before_first("##SEP##"));

    EXPECT_EQ(ST::string("AAA"), ST::string("AAA;BBB").before_last(';'));
    EXPECT_EQ(ST::string("AAA"), ST::string("AAA##SEP##BBB").before_last("##SEP##"));
    EXPECT_EQ(ST::string("AAA;BBB"), ST::string("AAA;BBB;CCC").before_last(';'));
    EXPECT_EQ(ST::string("AAA##SEP##BBB"), ST::string("AAA##SEP##BBB##SEP##CCC").before_last("##SEP##"));
    EXPECT_EQ(ST::string(""), ST::string("AAA").before_last(';'));
    EXPECT_EQ(ST::string(""), ST::string("AAA").before_last("##SEP##"));
    EXPECT_EQ(ST::string(""), ST::string(";").before_last(';'));
    EXPECT_EQ(ST::string(""), ST::string("##SEP##").before_last("##SEP##"));
    EXPECT_EQ(ST::string(""), ST::string("").before_last(';'));
    EXPECT_EQ(ST::string(""), ST::string("").before_last("##SEP##"));

    EXPECT_EQ(ST::string("BBB"), ST::string("AAA;BBB").after_first(';'));
    EXPECT_EQ(ST::string("BBB"), ST::string("AAA##SEP##BBB").after_first("##SEP##"));
    EXPECT_EQ(ST::string("BBB;CCC"), ST::string("AAA;BBB;CCC").after_first(';'));
    EXPECT_EQ(ST::string("BBB##SEP##CCC"), ST::string("AAA##SEP##BBB##SEP##CCC").after_first("##SEP##"));
    EXPECT_EQ(ST::string(""), ST::string("AAA").after_first(';'));
    EXPECT_EQ(ST::string(""), ST::string("AAA").after_first("##SEP##"));
    EXPECT_EQ(ST::string(""), ST::string(";").after_first(';'));
    EXPECT_EQ(ST::string(""), ST::string("##SEP##").after_first("##SEP##"));
    EXPECT_EQ(ST::string(""), ST::string("").after_first(';'));
    EXPECT_EQ(ST::string(""), ST::string("").after_first("##SEP##"));

    EXPECT_EQ(ST::string("BBB"), ST::string("AAA;BBB").after_last(';'));
    EXPECT_EQ(ST::string("BBB"), ST::string("AAA##SEP##BBB").after_last("##SEP##"));
    EXPECT_EQ(ST::string("CCC"), ST::string("AAA;BBB;CCC").after_last(';'));
    EXPECT_EQ(ST::string("CCC"), ST::string("AAA##SEP##BBB##SEP##CCC").after_last("##SEP##"));
    EXPECT_EQ(ST::string("AAA"), ST::string("AAA").after_last(';'));
    EXPECT_EQ(ST::string("AAA"), ST::string("AAA").after_last("##SEP##"));
    EXPECT_EQ(ST::string(""), ST::string(";").after_last(';'));
    EXPECT_EQ(ST::string(""), ST::string("##SEP##").after_last("##SEP##"));
    EXPECT_EQ(ST::string(""), ST::string("").after_last(';'));
    EXPECT_EQ(ST::string(""), ST::string("").after_last("##SEP##"));
}

TEST(string, replace)
{
    EXPECT_EQ(ST::string("xxYYxx"), ST::string("xxAAxx").replace("A", "Y"));
    EXPECT_EQ(ST::string("xxAAxx"), ST::string("xxAAxx").replace("XX", "Y"));
    EXPECT_EQ(ST::string("xxxx"), ST::string("xxAAxx").replace("A", ""));
    EXPECT_EQ(ST::string("xxREPLACExx"), ST::string("xxFINDxx").replace("FIND", "REPLACE"));
    EXPECT_EQ(ST::string("xxREPLACExxREPLACExx"), ST::string("xxFINDxxFINDxx").replace("FIND", "REPLACE"));

    EXPECT_EQ(ST::string("YYxx"), ST::string("AAxx").replace("A", "Y"));
    EXPECT_EQ(ST::string("AAxx"), ST::string("AAxx").replace("XX", "Y"));
    EXPECT_EQ(ST::string("xx"), ST::string("AAxx").replace("A", ""));
    EXPECT_EQ(ST::string("REPLACExx"), ST::string("FINDxx").replace("FIND", "REPLACE"));
    EXPECT_EQ(ST::string("REPLACExxREPLACExx"), ST::string("FINDxxFINDxx").replace("FIND", "REPLACE"));

    EXPECT_EQ(ST::string("xxYY"), ST::string("xxAA").replace("A", "Y"));
    EXPECT_EQ(ST::string("xxAA"), ST::string("xxAA").replace("XX", "Y"));
    EXPECT_EQ(ST::string("xx"), ST::string("xxAA").replace("A", ""));
    EXPECT_EQ(ST::string("xxREPLACE"), ST::string("xxFIND").replace("FIND", "REPLACE"));
    EXPECT_EQ(ST::string("xxREPLACExxREPLACE"), ST::string("xxFINDxxFIND").replace("FIND", "REPLACE"));

    EXPECT_EQ(ST::string("YY"), ST::string("AA").replace("A", "Y"));
    EXPECT_EQ(ST::string("AA"), ST::string("AA").replace("XX", "Y"));
    EXPECT_EQ(ST::string(""), ST::string("AA").replace("A", ""));
    EXPECT_EQ(ST::string("REPLACE"), ST::string("FIND").replace("FIND", "REPLACE"));
    EXPECT_EQ(ST::string("REPLACExxREPLACE"), ST::string("FINDxxFIND").replace("FIND", "REPLACE"));
}

TEST(string, case_conversion)
{
    /* Edge cases:
     * '@' = 'A' - 1
     * '[' = 'Z' + 1
     * '`' = 'a' - 1
     * '{' = 'z' + 1
     */

    EXPECT_EQ(ST::string("AAZZ"), ST::string("aazz").to_upper());
    EXPECT_EQ(ST::string("AAZZ"), ST::string("AAZZ").to_upper());
    EXPECT_EQ(ST::string("@AZ[`AZ{"), ST::string("@AZ[`az{").to_upper());
    EXPECT_EQ(ST::string(""), ST::string("").to_upper());

    EXPECT_EQ(ST::string("aazz"), ST::string("aazz").to_lower());
    EXPECT_EQ(ST::string("aazz"), ST::string("AAZZ").to_lower());
    EXPECT_EQ(ST::string("@az[`az{"), ST::string("@AZ[`az{").to_lower());
    EXPECT_EQ(ST::string(""), ST::string("").to_lower());
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
    EXPECT_EQ(std::vector<ST::string>{}, ST::string("\t;\n;").tokenize("\t\n-;"));
    EXPECT_EQ(std::vector<ST::string>{}, ST::string("").tokenize("\t\n-;"));
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
    EXPECT_EQ(expected9, ST::string("").split("-"));
    EXPECT_EQ(expected9, ST::string("").split("-", 4));
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
    EXPECT_EQ(expected9, ST::string("").split('-'));
    EXPECT_EQ(expected9, ST::string("").split('-', 4));
}

TEST(string, fill)
{
    EXPECT_EQ(ST::string(""), ST::string::fill(0, 'a'));
    EXPECT_EQ(ST::string("aaaaa"), ST::string::fill(5, 'a'));
    EXPECT_EQ(ST::string("aaaaaaaaaaaaaaaaaaaa"), ST::string::fill(20, 'a'));
}
