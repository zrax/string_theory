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

#include "st_stringstream.h"
#include "st_assert.h"

#include <gtest/gtest.h>

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

TEST(regress, string_stream_move)
{
    ST::string_stream ss;
    ss << "Hello";
    EXPECT_EQ(5U, ss.size());
    EXPECT_EQ(ST_LITERAL("Hello"), ss.to_string());

    ST::string_stream ss2 = std::move(ss);
    EXPECT_EQ(5U, ss2.size());
    EXPECT_EQ(ST_LITERAL("Hello"), ss2.to_string());
}

TEST(regress, surrogate_replacement)
{
    // string_theory 1.x would eat the trailing character after an incomplete
    // or invalid UTF-16 surrogate pair
    const char replacement_text[] = "A\xef\xbf\xbdz";
    const char16_t incomplete_surrogate_padded[] = { 0x41, 0xd800, 0x7a, 0 };
    ST::string incomplete_surr = ST::string::from_utf16(incomplete_surrogate_padded,
                                                        ST_AUTO_SIZE,
                                                        ST::substitute_invalid);
    EXPECT_STREQ(replacement_text, incomplete_surr.c_str());

    // Ensure that a valid surrogate after an incomplete one still gets converted
    const char replacement_text2[] = "A\xef\xbf\xbd\xf4\x8f\xbf\xbfz";
    const char16_t surrogate_dupe[] = { 0x41, 0xdbff, 0xdbff, 0xdfff, 0x7a, 0};
    ST::string surr_dupe = ST::string::from_utf16(surrogate_dupe, ST_AUTO_SIZE,
                                                  ST::substitute_invalid);
    EXPECT_STREQ(replacement_text2, surr_dupe.c_str());

    // string_theory 1.x would use a 4-byte representation for \ufffd, even
    // though canonically it should use 3 bytes.
    const char replacement_text3[] = "A\xef\xbf\xbd\xef\xbf\xbdz";
    const char16_t bad_surrogate_padded[] = { 0x41, 0xd800, 0xd801, 0x7a, 0 };
    ST::string bad_surr = ST::string::from_utf16(bad_surrogate_padded, ST_AUTO_SIZE,
                                                 ST::substitute_invalid);
    EXPECT_STREQ(replacement_text3, bad_surr.c_str());
}

TEST(regress, null_init)
{
    ST::string dont_crash = (const char *)ST_NULLPTR;
    EXPECT_TRUE(dont_crash.empty());

    dont_crash = (const char *)ST_NULLPTR;
    dont_crash = ST::string::from_wchar((const wchar_t *)ST_NULLPTR);
    dont_crash = ST::string::from_utf16((const char16_t *)ST_NULLPTR);
    dont_crash = ST::string::from_utf32((const char32_t *)ST_NULLPTR);
}
