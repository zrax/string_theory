# string_theory hex codec

## Headers
~~~c++
#include <string_theory/codecs>
~~~

## Public Functions

|    |   |
|---:|---|
| ST::string | [ST::hex_encode](#hex_encode_1)(const void \*data, size_t size) |
| ST::string | [ST::hex_encode](#hex_encode_2)(const char_buffer &data) |
| ST::char_buffer | [ST::hex_decode](#hex_decode_1)(const string &hex) |
| ST_ssize_t | [ST::hex_decode](#hex_decode_2)(const string &hex, void \*output, size_t output_size) noexcept |


## Function Documentation

<a name="hex_decode_1"></a>
### [ST::char_buffer](st_buffer.md) ST::hex_decode(const [ST::string](st_string.md) &hex)
~~~c++
namespace ST
{
    char_buffer hex_decode(const string &hex);
}
~~~

Decode the hexadecimal string (upper or lower case) in `hex` to a buffer.  If
the input string contains invalid hex data, an
[ST::codec_error](st_codec_error.md) is thrown with a message indicating the
type of failure.

------

<a name="hex_decode_2"></a>
### ST_ssize_t ST::hex_decode(const [ST::string](st_string.md) &hex, void \*output, size_t output_size)
~~~c++
namespace ST
{
    ST_ssize_t hex_decode(const string &hex, void *output, size_t output_size) noexcept;
}
~~~

If `output` is NULL, this function ignores `output_size` and returns the
necessary size to store the entire decoded data from `hex`.  If the data
in `hex` is not a multiple of two characters, this will return `-1` instead.
However, no further checking of the input is done in this mode.

If `output` is not NULL, this will convert the hexadecimal string data in `hex`
(upper or lower case) and write it to `output`.  If `output_size` is too small
for the entire conversion, or if any part of the conversion fails, this will
return `-1` and `output` will be left in an indeterminate state.

------

<a name="hex_encode_1"></a>
### [ST::string](st_string.md) ST::hex_encode(const void \*data, size_t size)
~~~c++
namespace ST
{
    string hex_encode(const void *data, size_t size);
}
~~~

Encode the first `size` bytes of `data` to a lower-case hexadecimal string
representation.

------

<a name="hex_encode_2"></a>
### [ST::string](st_string.md) ST::hex_encode(const [ST::char_buffer](st_buffer.md) &data)
~~~c++
namespace ST
{
    string hex_encode(const char_buffer &data);
}
~~~

Encode all data stored in `data` to a lower-case hexadecimal string
representation.
