# string_theory Format String Reference

## Description

string_theory format strings use curly braces (`{` and `}`) to specify
placeholders for formatted objects.  Note that, in order to achieve better
performance, there is no way to re-order arguments -- they will always be
handled from left to right in order.

Within the curly braces, it is possible to specify additional formatting
options (see below).  If only `{}` is specified, the default options (no
padding or alignment, decimal numbers, etc.) are used.  If you need a literal
`{` character, it can be escaped by adding a second brace:

~~~c++
ST::format("This is a curly brace: {{;  This is a format: {}", 42);
~~~

Otherwise, any other characters outside of curly braces are literal text to
put directly into the output.

For details on how these format options get passed to the individual formatters,
see the [ST::format_spec](st_format_spec.md) structure documentation.


## Formatting options

Note:  These options may come in any order

| Option | Meaning |
|--------|---------|
| `<` | Align values to the left, adding padding characters on the right if the format width is wider than the actual formatted text. |
| `>` | Align values to the right, adding padding characters on the left if the format width is wider than the actual formatted text |
| `_` *&lt;char&gt;* | Use the character *&lt;char&gt;* as padding.  Note that any (single) 7-bit character can be used here, including curly braces and other "special" characters. |
| `0` | Use numeric padding.  This overrides the `<`, `>` and `_` options, and uses `0` for padding on the right, with special semantics for formatting numbers.  See [ST::format_spec::numeric_pad](st_format_spec.md#numeric_pad) for details |
| `#` | Use a numeric base-specific prefix for non-decimal number formatting.  See [ST::format_spec::class_prefix](st_format_spec.md#class_prefix) for details. |
| `+` | Force a sign for positive integers (including unsigned integers) in the output |
| `x` | Format integers as hexadecimal with lower-case digits `a` through `f` |
| `X` | Format integers as hexadecimal with upper-case digits `A` through `F` |
| `d` | Format integers as decimal (default except for character types) |
| `o` | Format integers as octal |
| `b` | Format integers as binary |
| `c` | Format integers as unicode characters in UTF-8 (default for character types) |
| `f` | Format floating-point numbers in fixed-point format |
| `e` | Format floating-point numbers in exponential format |
| `E` | Format floating-point numbers in exponential format using an upper-case `'E'` separator |
| *&lt;number&gt;* | Specifies the minimum width of the formatted output.  If the formatted text is shorter than this, it will be padded according to the specified pad character and alignment. |
| `.` *&lt;number&gt;* | Specifies the precision of the formatted floating point number, or the maximum number of characters to print from string formatters. |
