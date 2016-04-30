# ST::conversion_result

## Headers
~~~c++
#include <string_theory/string>
~~~

## Public Functions

|    |   |
|---:|---|
|    | [conversion_result](#ctor_0)() noexcept |
| bool | [ok](#ok)() const noexcept |
| bool | [full_match](#full_match)() const noexcept |

# Details

The ST::conversion_result type stores the result of a conversion operation.
Specifically, this is used by the [ST::string](st_string.md) numeric conversion
functions, in the variants that can report their result.


## Member Documentation

<a name="ctor_0"></a>
### ST::conversion_result::conversion_result()
~~~c++
explicit conversion_result() noexcept
~~~

Default constructor.  This will construct a result that has neither the "ok"
nor the "full_match" status set.

------

<a name="full_match"></a>
### bool ST::conversion_result::full_match() const
~~~c++
bool full_match() const noexcept
~~~

Returns `true` if the entire string was matched.  This may be true if the
whole string was a valid number (in which case [ok](#ok)() will also be `true`),
or if the input string was empty (in which case [ok](#ok)() will be `false`).

------

<a name="ok"></a>
### bool ST::conversion_result::ok() const
~~~c++
bool ok() const noexcept
~~~

Returns `true` if the conversion was successful.  More specifically, this is
`true` if the conversion found one or more characters to convert from the
beginning of the string.
