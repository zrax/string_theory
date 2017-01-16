# string_theory base64 codec

## Headers
~~~c++
#include <string_theory/codecs>
~~~

## Public Functions

|    |   |
|---:|---|
| ST::string | [ST::base64_encode](#base64_encode_1)(const void \*data, size_t size) |
| ST::string | [ST::base64_encode](#base64_encode_2)(const char_buffer &data) |
| ST::char_buffer | [ST::base64_decode](#base64_decode_1)(const string &base64) |
| ST_ssize_t | [ST::base64_decode](#base64_decode_2)(const string &base64, void \*output, size_t output_size) noexcept |


## Function Documentation

<a name="base64_decode_1"></a>
### [ST::char_buffer](st_buffer.md) ST::base64_decode(const [ST::string](st_string.md) &base64)
~~~c++
namespace ST
{
    char_buffer base64_decode(const string &base64);
}
~~~

Decode the base64 string in `base64` to a buffer.  If the input string contains
invalid base64 data, an [ST::codec_error](st_codec_error.md) is thrown with
a message indicating the type of failure.

------

<a name="base64_decode_2"></a>
### ST_ssize_t ST::base64_decode(const [ST::string](st_string.md) &base64, void \*output, size_t output_size)
~~~c++
namespace ST
{
    ST_ssize_t base64_decode(const string &base64, void *output, size_t output_size) noexcept;
}
~~~

If `output` is NULL, this function ignores `output_size` and returns the
necessary size to store the entire decoded data from `base64`.  If the data
in `base64` is not correctly padded, this will return `-1` instead.  However,
no further checking of the input is done in this mode.

If `output` is not NULL, this will convert the base64 data in `base64` and
write it to `output`.  If `output_size` is too small for the entire conversion,
or if any part of the conversion fails, this will return `-1` and `output`
will be left in an indeterminate state.

------

<a name="base64_encode_1"></a>
### [ST::string](st_string.md) ST::base64_encode(const void \*data, size_t size)
~~~c++
namespace ST
{
    string base64_encode(const void *data, size_t size);
}
~~~

Encode the first `size` bytes of `data` to a Base64 string representation.
If `size` is `0`, data may be NULL; otherwise, `data` must be non-NULL.

------

<a name="base64_encode_2"></a>
### [ST::string](st_string.md) ST::base64_encode(const [ST::char_buffer](st_buffer.md) &data)
~~~c++
namespace ST
{
    string base64_encode(const char_buffer &data);
}
~~~

Encode all data stored in `data` to a Base64 string representation.
