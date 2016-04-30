# ST::format_writer

## Headers
~~~c++
#include <string_theory/formatter>
~~~

## Public Functions

|    |   |
|---:|---|
|    | [format_writer](#ctor_0)(const char \*format) noexcept |
| virtual | [~format_writer](#dtor)() noexcept |
| virtual format_writer & | [append](#append)(const char \*data, size_t size = ST_AUTO_SIZE) = 0 |
| virtual format_writer & | [append_char](#append_char)(char ch, size_t count = 1) = 0 |
| ST::format_spec | [fetch_next_format](#fetch_next_format)() |
| void | [finalize](#finalize)() |

## Details

The `format_writer` class is used by the formatting functions to do the actual
work of writing the formatted output.  This may mean writing data to a buffer
in memory, to a file, etc.

When implementing a new formatter, you'll need to create a subclass of
ST::format_writer which implements the [append](#append)() and
[append_char](#append_char)() functions.  Implementing the formatter itself
will also need to call [fetch_next_format](#fetch_next_format)() and
[finalize](#finalize)() when appropriate to advance through the format
specifier and pass the data off to the formatters.

For an example of subclassing `format_writer` to implement a custom format
backend, see the [custom format writer](getting-started.md#custom-format-writer)
example on the Getting Started guide.


## Member Documentation

<a name="ctor_0"></a>
### ST::format_writer::format_writer(const char \*format)
~~~c++
format_writer(const char *format) noexcept
~~~

Constructs the format_writer with the specified format string.  This will
start the formatter at the beginning of `format`, but will not yet write
any data to the output.

------

<a name="dtor"></a>
### ST::format_writer::~format_writer() [virtual]
~~~c++
virtual ~format_writer() noexcept
~~~

Virtual destructor for format writers.  The base class does nothing, but it
is provided in case a subclass needs to do work in the destructor.

------

<a name="append"></a>
### ST::format_writer &ST::format_writer::append(const char \*data, size_t size = ST_AUTO_SIZE) [pure virtual]
~~~c++
virtual format_writer & append(const char *data, size_t size = ST_AUTO_SIZE) = 0
~~~

Called by the formatters to write `size` bytes of data starting from `data`
to the output.  If `size` is `ST_SIZE_AUTO`, the implementation should compute
the size of `data` with `strlen()` or equivalent.

Implementations should return a reference to this object (i.e. `return *this`)
after handling the input data.

------

<a name="append_char"></a>
### ST::format_writer &ST::format_writer::append_char(char ch, size_t count = 1) [pure virtual]
~~~c++
virtual format_writer &append_char(char ch, size_t count = 1) = 0
~~~

Called by formatters to write `count` copies of the ASCII character `ch` to
the output.

Implementations should return a reference to this object (i.e. `return *this`)
after handling the input characters.

------

<a name="fetch_next_format"></a>
### [ST::format_spec](st_format_spec.md) ST::format_writer::fetch_next_format()
~~~c++
ST::format_spec fetch_next_format()
~~~

This should be called by format implementations to advance to the next
user-specified format spec.  This call may write data to the output if there
are other characters in the input before the next format specifier.  If there
are no more format specifiers in the input string, this will call the
string_theory assert handler to raise the error back to the application.

------

<a name="finalize"></a>
### void ST::format_writer::finalize()
~~~c++
void finalize()
~~~

This should be called by format implementations to consume the remaining
characters in the format string.  This call may write data to the output if
there are more characters in the format string before the end.  If there are
still additional unhandled format specifiers in the format string, this will
call the string_theory assert handler to raise the error back to the
application.
