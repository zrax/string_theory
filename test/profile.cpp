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
#include "st_stdio.h"

#ifdef ST_PROFILE_HAVE_BOOST
#   include <boost/format.hpp>
#endif
#ifdef ST_PROFILE_HAVE_QSTRING
#   include <QString>
#endif

#ifndef M_PI
#   define M_PI (3.14159265358979)
#endif

#ifdef _MSC_VER
#   define snprintf _snprintf
#endif

volatile const char *V;
#define NO_OPTIMIZE(x) V = reinterpret_cast<const char *>(x)

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
        NO_OPTIMIZE(s.c_str());
    });

    _measure("Empty ST::string", []() {
        ST::string s;
        NO_OPTIMIZE(s.c_str());
    });

#ifdef ST_PROFILE_HAVE_QSTRING
    _measure("Empty QString", []() {
        QString s;
        NO_OPTIMIZE(s.data());
    });
#endif

    _measure("Short std::string", []() {
        std::string short_str("Short");
        NO_OPTIMIZE(short_str.c_str());
    });

    _measure("Short ST::string", []() {
        ST::string short_str("Short");
        NO_OPTIMIZE(short_str.c_str());
    });

    _measure("Short ST::string (assume_valid)", []() {
        ST::string short_str("Short", ST_AUTO_SIZE, ST::assume_valid);
        NO_OPTIMIZE(short_str.c_str());
    });

    _measure("Short ST::string (literal)", []() {
        ST::string short_str = ST_LITERAL("Short");
        NO_OPTIMIZE(short_str.c_str());
    });

#ifdef ST_PROFILE_HAVE_QSTRING
    _measure("Short QString", []() {
        QString short_str("Short");
        NO_OPTIMIZE(short_str.data());
    });

    _measure("Short QString (literal)", []() {
        QString short_str = QStringLiteral("Short");
        NO_OPTIMIZE(short_str.data());
    });
#endif

    _measure("Long std::string", []() {
        std::string long_str("This is a long string.  Testing the excessively long long string.");
        NO_OPTIMIZE(long_str.c_str());
    });

    _measure("Long ST::string", []() {
        ST::string long_str("This is a long string.  Testing the excessively long long string.");
        NO_OPTIMIZE(long_str.c_str());
    });

    _measure("Long ST::string (assume_valid)", []() {
        ST::string long_str("This is a long string.  Testing the excessively long long string.",
                            ST_AUTO_SIZE, ST::assume_valid);
        NO_OPTIMIZE(long_str.c_str());
    });

    _measure("Long ST::string (literal)", []() {
        ST::string long_str = ST_LITERAL("This is a long string.  Testing the excessively long long string.");
        NO_OPTIMIZE(long_str.c_str());
    });

#ifdef ST_PROFILE_HAVE_QSTRING
    _measure("Long QString", []() {
        QString long_str("This is a long string.  Testing the excessively long long string.");
        NO_OPTIMIZE(long_str.data());
    });

    _measure("Long QString (literal)", []() {
        QString long_str = QStringLiteral("This is a long string.  Testing the excessively long long string.");
        NO_OPTIMIZE(long_str.data());
    });
#endif

    std::string _ss1("Short");
    _measure("Copy short std::string", [&_ss1]() {
        std::string copy = _ss1;
        NO_OPTIMIZE(copy.c_str());
    });

    ST::string _st1("Short");
    _measure("Copy short ST::string", [&_st1]() {
        ST::string copy = _st1;
        NO_OPTIMIZE(copy.c_str());
    });

#ifdef ST_PROFILE_HAVE_QSTRING
    QString _qs1("Short");
    _measure("Copy short QString", [&_qs1]() {
        QString copy = _qs1;
        NO_OPTIMIZE(copy.data());
    });
#endif

    std::string _ss2("This is a long string.  Testing the excessively long long string.");
    _measure("Copy long std::string", [&_ss2]() {
        std::string copy = _ss2;
        NO_OPTIMIZE(copy.c_str());
    });

    ST::string _st2("This is a long string.  Testing the excessively long long string.");
    _measure("Copy long ST::string", [&_st2]() {
        ST::string copy = _st2;
        NO_OPTIMIZE(copy.c_str());
    });

#ifdef ST_PROFILE_HAVE_QSTRING
    QString _qs2("This is a long string.  Testing the excessively long long string.");
    _measure("Copy long QString", [&_qs2]() {
        QString copy = _qs2;
        NO_OPTIMIZE(copy.data());
    });
#endif

    std::string _ss3[] = {"Piece 1", "Piece 2", "Piece 3"};
    _measure("std::string (+)", [&_ss3]() {
        std::string result = _ss3[0] + _ss3[1] + _ss3[2];
        NO_OPTIMIZE(result.c_str());
    });

    ST::string _st3[] = {"Piece 1", "Piece 2", "Piece 3"};
    _measure("ST::string (+)", [&_st3]() {
        ST::string result = _st3[0] + _st3[1] + _st3[2];
        NO_OPTIMIZE(result.c_str());
    });

#ifdef ST_PROFILE_HAVE_QSTRING
    QString _qs3[] = {"Piece 1", "Piece 2", "Piece 3"};
    _measure("QString (+)", [&_qs3]() {
        QString result = _qs3[0] + _qs3[1] + _qs3[2];
        NO_OPTIMIZE(result.data());
    });
#endif

    _measure("static snprintf", []() {
        char buffer[256];
        snprintf(buffer, 256, "This %d is %6.2f a %s test %c.", 42, M_PI, "<Singin' in the rain>", '?');
        NO_OPTIMIZE(buffer);
    });

    _measure("dynamic snprintf", []() {
        int len = snprintf(nullptr, 0, "This %d is %6.2f a %s test %c.", 42, M_PI, "<Singin' in the rain>", '?');
        char *buffer = new char[len + 1];
        snprintf(buffer, len, "This %d is %6.2f a %s test %c.", 42, M_PI, "<Singin' in the rain>", '?');
        NO_OPTIMIZE(buffer);
        delete[] buffer;
    });

    _measure("ST::format", []() {
        ST::string foo = ST::format("This {} is {6.2f} a {} test {}.", 42, M_PI, "<Singin' in the rain>", '?');
        NO_OPTIMIZE(foo.c_str());
    });

#ifdef ST_PROFILE_HAVE_BOOST
    _measure("boost::format", []() {
        std::string foo = (boost::format("This %1% is %2$6.2f a %3% test %4%.")
                           % 42 % M_PI % "<Singin' in the rain>" % '?').str();
        NO_OPTIMIZE(foo.c_str());
    });
#endif

#ifdef ST_PROFILE_HAVE_QSTRING
    _measure("QString::arg", []() {
        QString foo = QString("This %1 is %2 a %3 test %4.")
                      .arg(42).arg(M_PI, 6, 'f', 2).arg("<Singin' in the rain>").arg('?');
        NO_OPTIMIZE(foo.data());
    });
#endif

#ifdef WIN32
    FILE *devnull = fopen("nul", "w");
#else
    FILE *devnull = fopen("/dev/null", "w");
#endif
    if (devnull) {
        _measure("printf", [devnull]() {
            fprintf(devnull, "This %d is %6.2f a %s test %c.", 42, M_PI,
                             "<Singin' in the rain>", '?');
        });

        _measure("ST::printf", [devnull]() {
            ST::printf(devnull, "This {} is {6.2f} a {} test {}.", 42, M_PI,
                                "<Singin' in the rain>", '?');
        });

        fclose(devnull);
    }

    _measure("std::stringstream (~format)", []() {
        std::stringstream ss;
        ss << "This " << 42 << " is " << M_PI << " a "
           << "<Singin' in the rain>" << " test " << '?' << ".";
        NO_OPTIMIZE(ss.str().c_str());
    });

    _measure("ST::string_stream (~format)", []() {
        ST::string_stream ss;
        ss << "This " << 42 << " is " << M_PI << " a "
           << "<Singin' in the rain>" << " test " << '?' << ".";
        NO_OPTIMIZE(ss.to_string().c_str());
    });

    return 0;
}
