# ST::unicode_error

## Headers
~~~c++
#include <string_theory/exceptions>
~~~

## Inherits
- [std::runtime_error](http://en.cppreference.com/w/cpp/error/runtime_error)

## Public Functions

|    |   |
|---:|---|
|    | [unicode_error](#ctor_0)(const char \*message) noexcept |

# Details

The ST::unicode_error exception is thrown when an error occurs during string
data conversions when using the [ST::check_validity](st_string.md#utf_validation_t)
validation option.  This exception can be caught to report invalid data to
the user, or retry the conversion from Latin-1, for example.


## Member Documentation

<a name="ctor_0"></a>
### ST::unicode_error::unicode_error(const char \*message)
~~~c++
explicit unicode_error(const char *message) noexcept
~~~

Sets the exception message (returned by `what()`) to `message`.
