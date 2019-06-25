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

#include "st_codecs.h"
#include "st_assert.h"

#include <gtest/gtest.h>
#include <iostream>

#if defined(_MSC_VER) && (_MSC_VER < 1900)
#   define snprintf _snprintf
#   pragma warning(disable: 4996)
#endif

namespace ST
{
    // Teach GTest how to print an ST::string
    static void PrintTo(const ST::string &str, std::ostream *os)
    {
        *os << "ST::string{\"" << str.c_str() << "\"}";
    }

    // Teach GTest how to print an ST::char_buffer
    static void PrintTo(const ST::char_buffer &buffer, std::ostream *os)
    {
        *os << "ST::char_buffer{ ";
        char hex[8];
        for (size_t i = 0; i < buffer.size(); ++i) {
            unsigned int ch = static_cast<unsigned char>(buffer.data()[i]);
            snprintf(hex, sizeof(hex), "%02X ", ch);
            *os << hex;
        }
        *os << "}";
    }
}

static const unsigned char data_empty[] = { 0 };
static const unsigned char data_hex_ranges[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0xF0, 0xFF };
static const unsigned char data_base64_ranges[] = {
    /* Evaluates to the base64 numerals in order */
    0x00, 0x10, 0x83, 0x10, 0x51, 0x87, 0x20, 0x92,
    0x8b, 0x30, 0xd3, 0x8f, 0x41, 0x14, 0x93, 0x51,
    0x55, 0x97, 0x61, 0x96, 0x9b, 0x71, 0xd7, 0x9f,
    0x82, 0x18, 0xa3, 0x92, 0x59, 0xa7, 0xa2, 0x9a,
    0xab, 0xb2, 0xdb, 0xaf, 0xc3, 0x1c, 0xb3, 0xd3,
    0x5d, 0xb7, 0xe3, 0x9e, 0xbb, 0xf3, 0xdf, 0xbf
};
static const unsigned char data_1[] = { 0x01 };
static const unsigned char data_2[] = { 0x01, 0x02 };
static const unsigned char data_3[] = { 0x01, 0x02, 0x03 };
static const unsigned char data_4[] = { 0x01, 0x02, 0x03, 0x04 };
static const unsigned char data_15[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                        0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
static const unsigned char data_16[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                         0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10 };
static const unsigned char data_17[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                         0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
                                         0x11 };

#define cbuf(data) \
    ST::char_buffer(reinterpret_cast<const char *>(data), sizeof((data)))

#define empty_buf \
    ST::char_buffer(reinterpret_cast<const char *>(data_empty), 0)

TEST(codecs, hex_encode)
{
    EXPECT_EQ(ST::null, ST::hex_encode(data_empty, 0));
    EXPECT_EQ(ST::null, ST::hex_encode(nullptr, 0));
    EXPECT_EQ(ST_LITERAL("000102030405060708090a0b0c0d0e0f10f0ff"),
              ST::hex_encode(data_hex_ranges, sizeof(data_hex_ranges)));

    EXPECT_EQ(ST_LITERAL("01"), ST::hex_encode(data_1, sizeof(data_1)));
    EXPECT_EQ(ST_LITERAL("0102"), ST::hex_encode(data_2, sizeof(data_2)));
    EXPECT_EQ(ST_LITERAL("010203"), ST::hex_encode(data_3, sizeof(data_3)));
    EXPECT_EQ(ST_LITERAL("0102030405060708090a0b0c0d0e0f"),
              ST::hex_encode(data_15, sizeof(data_15)));
    EXPECT_EQ(ST_LITERAL("0102030405060708090a0b0c0d0e0f10"),
              ST::hex_encode(data_16, sizeof(data_16)));
    EXPECT_EQ(ST_LITERAL("0102030405060708090a0b0c0d0e0f1011"),
              ST::hex_encode(data_17, sizeof(data_17)));
}

TEST(codecs, hex_decode)
{
    EXPECT_EQ(empty_buf, ST::hex_decode(ST::null));
    EXPECT_EQ(cbuf(data_hex_ranges),
              ST::hex_decode(ST_LITERAL("000102030405060708090A0B0C0D0E0F10F0FF")));
    EXPECT_EQ(cbuf(data_hex_ranges),
              ST::hex_decode(ST_LITERAL("000102030405060708090a0b0c0d0e0f10f0ff")));

    EXPECT_EQ(cbuf(data_1), ST::hex_decode(ST_LITERAL("01")));
    EXPECT_EQ(cbuf(data_2), ST::hex_decode(ST_LITERAL("0102")));
    EXPECT_EQ(cbuf(data_3), ST::hex_decode(ST_LITERAL("010203")));
    EXPECT_EQ(cbuf(data_15),
              ST::hex_decode(ST_LITERAL("0102030405060708090A0B0C0D0E0F")));
    EXPECT_EQ(cbuf(data_16),
              ST::hex_decode(ST_LITERAL("0102030405060708090A0B0C0D0E0F10")));
    EXPECT_EQ(cbuf(data_17),
              ST::hex_decode(ST_LITERAL("0102030405060708090A0B0C0D0E0F1011")));
}

TEST(codecs, hex_decode_buffer)
{
    char buffer[64];

    EXPECT_EQ(0, ST::hex_decode(ST::null, nullptr, 0));
    EXPECT_EQ(0, ST::hex_decode(ST::null, buffer, sizeof(buffer)));
    EXPECT_EQ(empty_buf, ST::char_buffer(buffer, 0));

    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_hex_ranges)),
              ST::hex_decode(ST_LITERAL("000102030405060708090A0B0C0D0E0F10F0FF"), nullptr, 0));
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_hex_ranges)),
              ST::hex_decode(ST_LITERAL("000102030405060708090A0B0C0D0E0F10F0FF"), buffer, sizeof(buffer)));
    EXPECT_EQ(cbuf(data_hex_ranges), ST::char_buffer(buffer, sizeof(data_hex_ranges)));
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_hex_ranges)),
              ST::hex_decode(ST_LITERAL("000102030405060708090a0b0c0d0e0f10f0ff"), nullptr, 0));
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_hex_ranges)),
              ST::hex_decode(ST_LITERAL("000102030405060708090a0b0c0d0e0f10f0ff"), buffer, sizeof(buffer)));
    EXPECT_EQ(cbuf(data_hex_ranges), ST::char_buffer(buffer, sizeof(data_hex_ranges)));

    char tight_buffer[4];
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(tight_buffer)),
              ST::hex_decode(ST_LITERAL("01020304"), nullptr, 0));
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(tight_buffer)),
              ST::hex_decode(ST_LITERAL("01020304"), tight_buffer, sizeof(tight_buffer)));
    EXPECT_EQ(cbuf(data_4), ST::char_buffer(tight_buffer, sizeof(tight_buffer)));

    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_1)),
              ST::hex_decode(ST_LITERAL("01"), nullptr, 0));
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_1)),
              ST::hex_decode(ST_LITERAL("01"), buffer, sizeof(buffer)));
    EXPECT_EQ(cbuf(data_1), ST::char_buffer(buffer, sizeof(data_1)));
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_2)),
              ST::hex_decode(ST_LITERAL("0102"), nullptr, 0));
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_2)),
              ST::hex_decode(ST_LITERAL("0102"), buffer, sizeof(buffer)));
    EXPECT_EQ(cbuf(data_2), ST::char_buffer(buffer, sizeof(data_2)));
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_3)),
              ST::hex_decode(ST_LITERAL("010203"), nullptr, 0));
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_3)),
              ST::hex_decode(ST_LITERAL("010203"), buffer, sizeof(buffer)));
    EXPECT_EQ(cbuf(data_3), ST::char_buffer(buffer, sizeof(data_3)));

    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_15)),
              ST::hex_decode(ST_LITERAL("0102030405060708090A0B0C0D0E0F"), nullptr, 0));
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_15)),
              ST::hex_decode(ST_LITERAL("0102030405060708090A0B0C0D0E0F"), buffer, sizeof(buffer)));
    EXPECT_EQ(cbuf(data_15), ST::char_buffer(buffer, sizeof(data_15)));
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_16)),
              ST::hex_decode(ST_LITERAL("0102030405060708090A0B0C0D0E0F10"), nullptr, 0));
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_16)),
              ST::hex_decode(ST_LITERAL("0102030405060708090A0B0C0D0E0F10"), buffer, sizeof(buffer)));
    EXPECT_EQ(cbuf(data_16), ST::char_buffer(buffer, sizeof(data_16)));
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_17)),
              ST::hex_decode(ST_LITERAL("0102030405060708090A0B0C0D0E0F1011"), nullptr, 0));
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_17)),
              ST::hex_decode(ST_LITERAL("0102030405060708090A0B0C0D0E0F1011"), buffer, sizeof(buffer)));
    EXPECT_EQ(cbuf(data_17), ST::char_buffer(buffer, sizeof(data_17)));
}

void clean_assert(const char *, const char *, int, const char *message)
{
    fputs(message, stderr);
    fputs("\n", stderr);
    exit(0);
}

TEST(codecs, hex_codec_errors)
{
    ST::set_assert_handler(&clean_assert);
    EXPECT_EXIT(ST::hex_encode(nullptr, 1), ::testing::ExitedWithCode(0),
                "null data pointer passed to hex_encode");
    ST::set_default_assert_handler();

    EXPECT_THROW(ST::hex_decode(ST_LITERAL("1")), ST::codec_error);
    EXPECT_THROW(ST::hex_decode(ST_LITERAL("xF")), ST::codec_error);
    EXPECT_THROW(ST::hex_decode(ST_LITERAL("Fx")), ST::codec_error);
    EXPECT_THROW(ST::hex_decode(ST_LITERAL("\000F")), ST::codec_error);
    EXPECT_THROW(ST::hex_decode(ST_LITERAL("F\000")), ST::codec_error);

    char buffer[64];
    EXPECT_EQ(-1, ST::hex_decode(ST_LITERAL("1"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::hex_decode(ST_LITERAL("xF"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::hex_decode(ST_LITERAL("Fx"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::hex_decode(ST_LITERAL("\000F"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::hex_decode(ST_LITERAL("F\000"), buffer, sizeof(buffer)));

    char tight_buffer[3];
    EXPECT_EQ(-1, ST::hex_decode(ST_LITERAL("01020304"), tight_buffer, sizeof(tight_buffer)));
}

TEST(codecs, base64_encode)
{
    EXPECT_EQ(ST::null, ST::base64_encode(data_empty, 0));
    EXPECT_EQ(ST::null, ST::base64_encode(nullptr, 0));
    EXPECT_EQ(ST_LITERAL("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"),
              ST::base64_encode(data_base64_ranges, sizeof(data_base64_ranges)));

    EXPECT_EQ(ST_LITERAL("AQ=="), ST::base64_encode(data_1, sizeof(data_1)));
    EXPECT_EQ(ST_LITERAL("AQI="), ST::base64_encode(data_2, sizeof(data_2)));
    EXPECT_EQ(ST_LITERAL("AQID"), ST::base64_encode(data_3, sizeof(data_3)));
    EXPECT_EQ(ST_LITERAL("AQIDBAUGBwgJCgsMDQ4P"),
              ST::base64_encode(data_15, sizeof(data_15)));
    EXPECT_EQ(ST_LITERAL("AQIDBAUGBwgJCgsMDQ4PEA=="),
              ST::base64_encode(data_16, sizeof(data_16)));
    EXPECT_EQ(ST_LITERAL("AQIDBAUGBwgJCgsMDQ4PEBE="),
              ST::base64_encode(data_17, sizeof(data_17)));
}

TEST(codecs, base64_decode)
{
    EXPECT_EQ(empty_buf, ST::base64_decode(ST::null));
    EXPECT_EQ(cbuf(data_base64_ranges), ST::base64_decode(
        ST_LITERAL("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/")));

    EXPECT_EQ(cbuf(data_1), ST::base64_decode(ST_LITERAL("AQ==")));
    EXPECT_EQ(cbuf(data_2), ST::base64_decode(ST_LITERAL("AQI=")));
    EXPECT_EQ(cbuf(data_3), ST::base64_decode(ST_LITERAL("AQID")));
    EXPECT_EQ(cbuf(data_15),
              ST::base64_decode(ST_LITERAL("AQIDBAUGBwgJCgsMDQ4P")));
    EXPECT_EQ(cbuf(data_16),
              ST::base64_decode(ST_LITERAL("AQIDBAUGBwgJCgsMDQ4PEA==")));
    EXPECT_EQ(cbuf(data_17),
              ST::base64_decode(ST_LITERAL("AQIDBAUGBwgJCgsMDQ4PEBE=")));
}

TEST(codecs, base64_decode_buffer)
{
    char buffer[64];

    EXPECT_EQ(0, ST::base64_decode(ST::null, nullptr, 0));
    EXPECT_EQ(0, ST::base64_decode(ST::null, buffer, sizeof(buffer)));
    EXPECT_EQ(empty_buf, ST::char_buffer(buffer, 0));

    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_base64_ranges)),
        ST::base64_decode(ST_LITERAL("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"),
                          nullptr, 0));
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_base64_ranges)),
        ST::base64_decode(ST_LITERAL("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"),
                          buffer, sizeof(buffer)));
    EXPECT_EQ(cbuf(data_base64_ranges), ST::char_buffer(buffer, sizeof(data_base64_ranges)));

    char tight_buffer[4];
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(tight_buffer)),
              ST::base64_decode(ST_LITERAL("AQIDBA=="), nullptr, 0));
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(tight_buffer)),
              ST::base64_decode(ST_LITERAL("AQIDBA=="), tight_buffer, sizeof(tight_buffer)));
    EXPECT_EQ(cbuf(data_4), ST::char_buffer(tight_buffer, sizeof(tight_buffer)));

    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_1)),
              ST::base64_decode(ST_LITERAL("AQ=="), nullptr, 0));
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_1)),
              ST::base64_decode(ST_LITERAL("AQ=="), buffer, sizeof(buffer)));
    EXPECT_EQ(cbuf(data_1), ST::char_buffer(buffer, sizeof(data_1)));
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_2)),
              ST::base64_decode(ST_LITERAL("AQI="), nullptr, 0));
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_2)),
              ST::base64_decode(ST_LITERAL("AQI="), buffer, sizeof(buffer)));
    EXPECT_EQ(cbuf(data_2), ST::char_buffer(buffer, sizeof(data_2)));
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_3)),
              ST::base64_decode(ST_LITERAL("AQID"), nullptr, 0));
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_3)),
              ST::base64_decode(ST_LITERAL("AQID"), buffer, sizeof(buffer)));
    EXPECT_EQ(cbuf(data_3), ST::char_buffer(buffer, sizeof(data_3)));

    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_15)),
              ST::base64_decode(ST_LITERAL("AQIDBAUGBwgJCgsMDQ4P"), nullptr, 0));
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_15)),
              ST::base64_decode(ST_LITERAL("AQIDBAUGBwgJCgsMDQ4P"), buffer, sizeof(buffer)));
    EXPECT_EQ(cbuf(data_15), ST::char_buffer(buffer, sizeof(data_15)));
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_16)),
              ST::base64_decode(ST_LITERAL("AQIDBAUGBwgJCgsMDQ4PEA=="), nullptr, 0));
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_16)),
              ST::base64_decode(ST_LITERAL("AQIDBAUGBwgJCgsMDQ4PEA=="), buffer, sizeof(buffer)));
    EXPECT_EQ(cbuf(data_16), ST::char_buffer(buffer, sizeof(data_16)));
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_17)),
              ST::base64_decode(ST_LITERAL("AQIDBAUGBwgJCgsMDQ4PEBE="), nullptr, 0));
    EXPECT_EQ(static_cast<ST_ssize_t>(sizeof(data_17)),
              ST::base64_decode(ST_LITERAL("AQIDBAUGBwgJCgsMDQ4PEBE="), buffer, sizeof(buffer)));
    EXPECT_EQ(cbuf(data_17), ST::char_buffer(buffer, sizeof(data_17)));
}

TEST(codecs, base64_codec_errors)
{
    ST::set_assert_handler(&clean_assert);
    EXPECT_EXIT(ST::base64_encode(nullptr, 1), ::testing::ExitedWithCode(0),
                "null data pointer passed to base64_encode");
    ST::set_default_assert_handler();

    EXPECT_THROW(ST::base64_decode(ST_LITERAL("A")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("AB")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("ABC")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("!ABC")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("A!BC")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("AB!C")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("ABC!")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("\000ABC")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("A\000BC")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("AB\000C")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("ABC\000")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("=ABC")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("A=BC")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("AB=C")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("A===")), ST::codec_error);

    EXPECT_THROW(ST::base64_decode(ST_LITERAL("++++++++A")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("++++++++AB")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("++++++++ABC")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("!ABC++++++++")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("A!BC++++++++")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("AB!C++++++++")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("ABC!++++++++")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("\000ABC++++++++")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("A\000BC++++++++")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("AB\000C++++++++")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("ABC\000++++++++")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("=ABC++++++++")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("A=BC++++++++")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("AB=C++++++++")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("ABC=++++++++")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("++++=ABC++++")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("++++A=BC++++")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("++++AB=C++++")), ST::codec_error);
    EXPECT_THROW(ST::base64_decode(ST_LITERAL("++++ABC=++++")), ST::codec_error);

    char buffer[64];
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("A"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("AB"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("ABC"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("!ABC"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("A!BC"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("AB!C"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("ABC!"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("\000ABC"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("A\000BC"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("AB\000C"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("ABC\000"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("=ABC"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("A=BC"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("AB=C"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("A==="), buffer, sizeof(buffer)));

    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("++++++++A"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("++++++++AB"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("++++++++ABC"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("!ABC++++++++"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("A!BC++++++++"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("AB!C++++++++"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("ABC!++++++++"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("\000ABC++++++++"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("A\000BC++++++++"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("AB\000C++++++++"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("ABC\000++++++++"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("=ABC++++++++"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("A=BC++++++++"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("AB=C++++++++"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("ABC=++++++++"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("++++=ABC++++"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("++++A=BC++++"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("++++AB=C++++"), buffer, sizeof(buffer)));
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("++++ABC=++++"), buffer, sizeof(buffer)));

    char tight_buffer[3];
    EXPECT_EQ(-1, ST::base64_decode(ST_LITERAL("AQIDBA=="), tight_buffer, sizeof(tight_buffer)));
}
