# string_theory formatter functions

## Headers
~~~c++
#include <string_theory/formatter>
~~~

## Public Functions

|    |   |
|---:|---|
| void | [ST::format_string](#format_string)(const format_spec &format, format_writer &output, const char *text, size_t size, alignment_t default_alignment = align_left) |

## Macros

|   |
|---|
| [ST_DECL_FORMAT_TYPE](#ST_DECL_FORMAT_TYPE)(type_T) |
| [ST_FORMAT_TYPE](#ST_FORMAT_TYPE)(type_T) |
| [ST_FORMAT_FORWARD](#ST_FORMAT_FORWARD)(fwd_value) |
| [ST_INVOKE_FORMATTER](#ST_INVOKE_FORMATTER)(format, output, value) |

## Function Documentation

<a name="format_string"></a>
### void ST::format_string(const [ST::format_spec](st_format_spec.md) &format, [ST::format_writer](st_format_writer.md) &output, const char \*text, size_t size, [ST::alignment_t](st_format_spec.md#alignment_t) alignment = [ST::align_left](st_format_spec.md#alignment_t))
~~~c++
namespace ST
{
    void format_string(const format_spec &format, format_writer &output,
                       const char *text, size_t size,
                       alignment_t default_alignment = align_left);
}
~~~

Helper function for writing string data to a [format_writer](st_format_writer.md).
This can be called directly by formatters to handle properties like alignment
and padding for string data formatted through other means.  If the `size` and
`default_alignment` parameters are set to defaults, this is essentially
equivalent to [ST_FORMAT_FORWARD](#ST_FORMAT_FORWARD) with the string data.

**See also** [ST_FORMAT_FORWARD](#ST_FORMAT_FORWARD)(fwd_value)

------

## Macro Documentation

<a name="ST_DECL_FORMAT_TYPE"></a>
### ST_DECL_FORMAT_TYPE(type_T)
~~~c++
#define ST_DECL_FORMAT_TYPE(type_T) ...
~~~

Forward-declare a custom formatter.  Note that this macro should be in the
include chain AFTER `&lt;string_theory/formatter&gt;` is included, but BEFORE
any format implementations (like `&lt;string_theory/format`) are included.
However, it only needs to appear in the include chain if a caller needs to
format the specified `type_T`.

**See also** [ST_FORMAT_TYPE](#ST_FORMAT_TYPE)(type_T)

------

<a name="ST_FORMAT_FORWARD"></a>
### ST_FORMAT_FORWARD(fwd_value)
~~~c++
#define ST_FORMAT_FORWARD(fwd_value) ...
~~~

Forward a value to another formatter.  Note that the type of `fwd_value` must
resolve to only one formatter, so a cast may be needed.

**Example**
~~~c++
ST_FORMAT_TYPE(const my_type &)
{
    // Assumes my_type::to_string() resolves to a string type we can format
    ST_FORMAT_FORWARD(value.to_string());
}
~~~

------

<a name="ST_FORMAT_TYPE"></a>
### ST_FORMAT_TYPE(type_T)
~~~c++
#define ST_FORMAT_TYPE(type_T) ...
~~~

Provides the necessary function header for implementing a custom formatter for
the type `type_T`.  Note that `type_T` should be a fully specified type, but
should not include the variable name.

When writing a custom formatter using this header, the following parameters
are available for use:
- const [ST::format_spec](st_format_spec.md) &format:  The format spec to be
  used for formatting the object.
- [ST::format_writer](st_format_writer.md) &output:  The output object for
  writing formatted text data.
- `type_T` value:  The value object to format.

If this macro is used in a source file, there should be a matching
[ST_DECL_FORMAT_TYPE](#ST_DECL_FORMAT_TYPE)(type_T) in a header somewhere
in order for it to be used by format engines.  Alternatively, if you wish to
declare the whole formatter in a header, this can be used directly with the
`inline` specifier.

**See also** [ST_DECL_FORMAT_TYPE](#ST_DECL_FORMAT_TYPE)(type_T),
[Custom formatter example](getting-started.md#custom-formatter)

------

<a name="ST_INVOKE_FORMATTER"></a>
### ST_INVOKE_FORMATTER(format, output, value)
~~~c++
#define ST_INVOKE_FORMATTER ...
~~~

This macro should be called by custom format engines to invoke the formatter
for a given object.  It evaluates to the function call defined by either
[ST_DECL_FORMAT_TYPE](#ST_DECL_FORMAT_TYPE)() or [ST_FORMAT_TYPE](#ST_FORMAT_TYPE).
