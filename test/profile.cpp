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

// This test is not meant to cover all actual uses; it's just a test to get
// a rough idea of how ST::string and ST::format hold up against other string
// and formatting APIs performance-wise.

#include <chrono>
#include <functional>
#include <sstream>
#include <cmath>

#include "st_format.h"

#ifdef ST_PROFILE_HAVE_BOOST
#   include <boost/format.hpp>
#endif

#ifndef M_PI
#   define M_PI (3.14159265358979)
#endif

static void _measure(const char *title, std::function<void()> fun)
{
    auto clk = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < 100000; ++i)
        fun();

    auto dur = std::chrono::high_resolution_clock::now() - clk;
    puts(ST::format("{32}: {6.2f} ms", title,
         std::chrono::duration<double, std::milli>(dur).count()).c_str());
}

int main(int, char **)
{
    _measure("Nothing", []() { });

    _measure("Empty std::string", []() {
        std::string s;
        volatile const char *V = s.c_str();
    });

    _measure("Empty ST::string", []() {
        ST::string s;
        volatile const char *V = s.c_str();
    });

    _measure("Short std::string", []() {
        std::string short_str("Short");
        volatile const char *V = short_str.c_str();
    });

    _measure("Short ST::string", []() {
        ST::string short_str("Short");
        volatile const char *V = short_str.c_str();
    });

    _measure("Short ST::string (assume_valid)", []() {
        ST::string short_str("Short", ST_AUTO_SIZE, ST::assume_valid);
        volatile const char *V = short_str.c_str();
    });

    _measure("Short ST::string (literal)", []() {
        ST::string short_str = ST_LITERAL("Short");
        volatile const char *V = short_str.c_str();
    });

    _measure("Long std::string", []() {
        std::string long_str("This is a long string.  Testing the excessively long long string.");
        volatile const char *V = long_str.c_str();
    });

    _measure("Long ST::string", []() {
        ST::string long_str("This is a long string.  Testing the excessively long long string.");
        volatile const char *V = long_str.c_str();
    });

    _measure("Long ST::string (assume_valid)", []() {
        ST::string long_str("This is a long string.  Testing the excessively long long string.",
                            ST_AUTO_SIZE, ST::assume_valid);
        volatile const char *V = long_str.c_str();
    });

    _measure("Long ST::string (literal)", []() {
        ST::string long_str = ST_LITERAL("This is a long string.  Testing the excessively long long string.");
        volatile const char *V = long_str.c_str();
    });

    std::string _xc1("Short");
    _measure("Copy short std::string", [&_xc1]() {
        std::string copy = _xc1;
        volatile const char *V = copy.c_str();
    });

    ST::string _xc2("Short");
    _measure("Copy short ST::string", [&_xc2]() {
        ST::string copy = _xc2;
        volatile const char *V = copy.c_str();
    });

    std::string _xc3("This is a long string.  Testing the excessively long long string.");
    _measure("Copy long std::string", [&_xc3]() {
        std::string copy = _xc3;
        volatile const char *V = copy.c_str();
    });

    ST::string _xc4("This is a long string.  Testing the excessively long long string.");
    _measure("Copy long ST::string", [&_xc4]() {
        ST::string copy = _xc4;
        volatile const char *V = copy.c_str();
    });

    _measure("static snprintf", []() {
        char buffer[256];
        snprintf(buffer, 256, "This %d is %6.2f a %s test %c.", 42, M_PI, "<Singin' in the rain>", '?');
        volatile const char *V = buffer;
    });

    _measure("dynamic snprintf", []() {
        int len = snprintf(nullptr, 0, "This %d is %6.2f a %s test %c.", 42, M_PI, "<Singin' in the rain>", '?');
        char *buffer = new char[len + 1];
        snprintf(buffer, len, "This %d is %6.2f a %s test %c.", 42, M_PI, "<Singin' in the rain>", '?');
        volatile const char *V = buffer;
        delete[] buffer;
    });

    _measure("ST::format", []() {
        ST::string foo = ST::format("This {} is {6.2f} a {} test {}.", 42, M_PI, "<Singin' in the rain>", '?');
        volatile const char *V = foo.c_str();
    });

#ifdef ST_PROFILE_HAVE_BOOST
    _measure("boost::format", []() {
        std::string foo = (boost::format("This %1% is %2$6.2f a %3% test %4%.")
                           % 42 % M_PI % "<Singin' in the rain>" % '?').str();
        volatile const char *V = foo.c_str();
    });
#endif

    _measure("std::stringstream (~format)", []() {
        std::stringstream ss;
        ss << "This " << 42 << " is " << M_PI << " a "
           << "<Singin' in the rain>" << " test " << '?' << ".";
        volatile const char *V = ss.str().c_str();
    });

    _measure("ST::string_stream (~format)", []() {
        ST::string_stream ss;
        ss << "This " << 42 << " is " << M_PI << " a "
           << "<Singin' in the rain>" << " test " << '?' << ".";
        volatile const char *V = ss.to_string().c_str();
    });

    return 0;
}
