# Getting Started with string_theory

## Building string_theory

string_theory uses the [CMake](http://cmake.org) build system.  To get started
on a Unix-like system (Linux, Mac, MSYS, etc), you can just build string_theory
from the command line after installing CMake:

~~~bash
$ cd /path/to/string_theory
$ mkdir build && cd build
$ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local ..
$ make && sudo make install
~~~

On Windows, you can also generate Visual Studio projects, either from the
command line or from the CMake GUI:

~~~bat
> cd C:\Path\to\string_theory
> mkdir build-msvc
> cmake -G "Visual Studio 14 2015" ..
(Open the generated .sln file and build the Release target)
~~~

If you want to build string_theory as a static library, you can also add
`-DST_BUILD_STATIC=ON` to the CMake command line (or set it in the GUI).


## Using string_theory in your project

If your project is built using CMake, it should be easy to add string_theory:

~~~CMake
# CMakeLists.txt
# ...
find_package(string_theory REQUIRED)
include_directories("${STRING_THEORY_INCLUDE_DIRS}")
# ...
add_executable(myapp ${SOURCES} ${HEADERS})
target_link_libraries(myapp ${STRING_THEORY_LIBRARIES})
~~~

Then be sure to specify or locate the cmake module path when configuring your
project (e.g. `-Dstring_theory_DIR=/usr/local/lib/cmake/string_theory`).

When using other build systems, you can just specify string_theory's include
and library paths for your build system.  For GNU makefiles, this may look
something like:

~~~make
STRING_THEORY_DIR = /usr/local
CPPFLAGS += -I$(STRING_THEORY_DIR)/include
LDFLAGS += -L$(STRING_THEORY_DIR)/lib -lstring_theory
~~~

If you create a useful module for your favorite build system, you can also
contribute the necessary files to string_theory via a Pull Request.


## Examples

### Basic ST::string features

~~~c++
#include <string_theory/string>
#include <string_theory/exceptions>
#include <string_theory/stdio>

int main(int argc, char *argv[])
{
    ST::string greeting = ST_LITERAL("Hello");

    // Literals can also use the _st user-literal operator if your compiler
    // supports user-defined literals
    ST::string name = "world"_st;
    if (argc > 1) {
        try {
            // Alternatively, you can assume UTF-8 and substitute invalid
            // byte sequences with
            // ST::string::from_utf8(argv[1], ST_SIZE_AUTO, ST::substitute_invalid)
            // The above method will also avoid throwing any exceptions.
            name = ST::string::from_utf8(argv[1]);
        } catch (ST::unicode_error) {
            name = ST::string::from_latin_1(argv[1]);
        }
    }
    ST::printf("{}, {}\n", greeting, name);

    return 0;
}
~~~

### String formatting

~~~c++
#include <string_theory/format>

extern void log_line(const ST::string &line);

ST::string status_line(const ST::string &filename, int file_num, int total_files)
{
    // "Processing file 004: Foobar.txt          - 12.5%"
    return ST::format("Processing file {>03}: {<20} - {4.1f}%", file_num,
                      filename, double(file_num) / double(total_files));
}

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i) {
        log_line(status_line(argv[i]));
        // Process file...
    }
    return 0;
}
~~~

### Codecs

~~~c++
#include <string_theory/codecs>
#include <string_theory/stdio>

int main(int argc, char *argv)
{
    if (argc < 2) {
        ST::printf(stderr, "Usage: {} <mode> input\n", argv[0]);
        fputs("<mode>:\n", stderr);
        fputs("  base64_encode\n", stderr);
        fputs("  base64_decode\n", stderr);
        fputs("  hex_encode\n", stderr);
        fputs("  hex_decode\n", stderr);
        return 1;
    }

    ST::string mode = argv[1];
    ST::string input = argv[2];
    ST::string result;
    if (mode == "base64_encode") {
        result = ST::base64_encode(input.to_utf8());
    } else if (mode == "base64_decode") {
        ST::char_buffer data = ST::base64_decode(input);
        result = ST::string::from_utf8(data);
    } else if (mode == "hex_encode") {
        result = ST::hex_encode(input.to_utf8());
    } else if (mode == "hex_decode") {
        ST::char_buffer data = ST::hex_decode(input);
        result = ST::string::from_utf8()
    } else {
        fputs("ERROR: Unsupported mode.\n", stderr);
        return 2;
    }
    puts(result.c_str());

    return 0;
}
~~~

### Custom formatter

~~~c++
/* point3d.h */
#include <string_theory/formatter>

struct Point3D { double x, y, z; };

// This version will apply the specified floating point formatting rules
// to each of the rendered values x,y,z
inline ST_FORMAT_TYPE(const Point3D &)
{
    output.append("Point3D{");
    ST_FORMAT_FORWARD(value.x);
    output.append(",");
    ST_FORMAT_FORWARD(value.y);
    output.append(",");
    ST_FORMAT_FORWARD(value.z);
    output.append("}");
}

// Could also format recursively.  This will instead treat the upper-level
// formatting rules as a single string formatter.  For example:
inline ST_FORMAT_TYPE(const Point3D &)
{
    ST_FORMAT_FORWARD(ST::format("Point3D{{{.2f},{.2f},{.2f}}", value.x, value.y, value.z));
}
~~~

~~~c++
// Note:  Include order is important here.  Custom formatters MUST be declared
// before the format or stdio header is included!
#include "point3d.h"
#include <string_theory/stdio>

int main(int argc, char *argv[])
{
    Point3D point{12.4, 42.0, -6.3};

    ST::printf("It's located at {}\n", point);

    return 0;
}
~~~

### Custom format writer

~~~c++
#include <string_theory/formatter>
#include "my_logger.h"

class my_log_writer : public ST::format_writer
{
public:
    my_log_writer(const char *format_str, my_logger *logger)
        : ST::format_writer(format_str), m_logger(logger) { }

    my_log_writer &append(const char *data, size_t size = ST_AUTO_SIZE) override
    {
        if (size == ST_AUTO_SIZE)
            size = ST::char_buffer::strlen(data);
        m_logger->write_log(data, size);
        return *this;
    }

    my_log_writer &append_char(char ch, size_t count = 1) ST_OVERRIDE
    {
        ST::string buffer = ST::string::fill(count, ch);
        m_logger->write_log(buffer.c_str(), count);
        return *this;
    }
};

inline void _impl_log_format(my_log_writer &data)
{
    data.finalize();
}

template <typename type_T, typename... args_T>
void _impl_log_format(my_log_writer &data, type_T value, args_T ...args)
{
    ST::format_spec format = data.fetch_next_format();
    ST_INVOKE_FORMATTER(format, data, value);
    _impl_log_format(data, args...);
}

template <typename type_T, typename... args_T>
void log_format(my_logger *logger, const char *fmt_str, type_T value, args_T ...args)
{
    my_log_writer data(fmt_str, logger);
    ST::format_spec format = data.fetch_next_format();
    ST_INVOKE_FORMATTER(format, data, value);
    _impl_log_format(data, args...);
}

int main(int argc, char *argv[])
{
    my_logger *logger = get_logger();
    log_format(logger, "Running {}\n", argv[0]);

    return 0;
}
~~~
