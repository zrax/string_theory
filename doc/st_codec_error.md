# ST::codec_error

## Headers
~~~c++
#include <string_theory/exceptions>
~~~

## Inherits
- [std::runtime_error](http://en.cppreference.com/w/cpp/error/runtime_error)

## Public Functions

|    |   |
|---:|---|
|    | [codec_error](#ctor_0)(const char \*message) noexcept |

# Details

The ST::codec_error exception is thrown when an error occurs decoding a
[hex](st_hex.md) or [base64](st_base64.md) string from their respective codecs.


## Member Documentation

<a name="ctor_0"></a>
### ST::codec_error::codec_error(const char \*message)
~~~c++
explicit codec_error(const char *message) noexcept
~~~

Sets the exception message (returned by `what()`) to `message`.
