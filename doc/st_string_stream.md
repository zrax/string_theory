# ST::string_stream

## Headers
~~~c++
#include <string_theory/string_stream>
~~~

## Public Functions

|    |   |
|---:|---|
|    | [string_stream](#ctor_0)() noexcept |
|    | [string_stream](#ctor_1)(string_stream &&move) noexcept |
|    | [~string_stream](#dtor)() noexcept |
| string_stream & | [operator=](#operator_eq)(string_stream &&move) noexcept |
| string_stream & | [append](#append)(const char \*data, size_t size = ST_AUTO_SIZE) |
| string_stream & | [append_char](#append_char)(char ch, size_t count = 1) |
| string_stream & | [operator&lt;&lt;](#operator_shl_1)(const char \*text) |
| string_stream & | [operator&lt;&lt;](#operator_shl_2)(int num) |
| string_stream & | [operator&lt;&lt;](#operator_shl_3)(unsigned int num) |
| string_stream & | [operator&lt;&lt;](#operator_shl_4)(long num) |
| string_stream & | [operator&lt;&lt;](#operator_shl_5)(unsigned long num) |
| string_stream & | [operator&lt;&lt;](#operator_shl_6)(int64_t num) |
| string_stream & | [operator&lt;&lt;](#operator_shl_7)(uint64_t num) |
| string_stream & | [operator&lt;&lt;](#operator_shl_8)(float num) |
| string_stream & | [operator&lt;&lt;](#operator_shl_9)(double num) |
| string_stream & | [operator&lt;&lt;](#operator_shl_10)(char ch) |
| string_stream & | [operator&lt;&lt;](#operator_shl_11)(const string &text) |
| const char \* | [raw_buffer](#raw_buffer)() const noexcept |
| size_t | [size](#size)() const noexcept |
| string | [to_string](#to_string)(bool utf8_encoded = true, utf_validation_t validation = assert_validity) const |
| void | [truncate](#truncate)(size_t size = 0) noexcept |
| void | [erase](#erase)(size_t count) noexcept |

## Details

ST::string_stream provides an efficient way to append text and numbers (with
limited formatting options) to a memory buffer.  The buffer will grow
automatically as more text is added.  If the data stored in the stream is short
enough, it will remain completely on the stack.

The string_stream is particularly useful for building strings incrementally
through various pieces that have some sort of logic or looping involved, as it
is much more efficient than adding strings with `operator+` or `operator+=`.

Example:

~~~c++
// Slow way -- allocates several "fit length" strings:
ST::string s1 = "There are " + ST::string::from_int(count) + " items in boxes (";
for (auto box : boxes)
    s1 += box + ", ";
s1 = s1.left(s1.size() - 2);  // Erase last comma
s1 += ")";

// Fast way, construct only one string.
ST::string_stream ss;
ss << "There are " << count << " items in boxes ";
for (auto box : boxes)
    ss << box << ", ";
s1.erase(2);  // Erase last comma
ss << ")";
ST::string s2 = ss.to_string();
~~~

Note that for more control over formatting, you should use [ST::format](st_format.md)
instead.  However, because ST::string_stream can take [ST::string](st_string.md)
objects, it is possible to combine ST::format with ST::string_stream:

~~~c++
ST::string_stream ss;
ss << "Some fancy formatting " << ST::format("{<40x}", number);
~~~


## Member Documentation

<a name="ctor_0"></a>
### ST::string_stream::string_stream()
~~~c++
string_stream() noexcept
~~~

Default constructor.  Creates an empty stream (size() == `0`).

------

<a name="ctor_1"></a>
### ST::string_stream::string_stream(ST::string_stream &&move)
~~~c++
string_stream(string_stream &&move) noexcept
~~~

Move constructor.  Moves the stream content from `move` into this string_stream.

**See also** [operator=](#operator_eq)(string_stream &&)

------

<a name="dtor"></a>
### ST::string_stream::~string_stream()
~~~c++
~string_stream() noexcept
~~~

Destructor.  Destroys the string_stream, freeing any allocated memory.

------

<a name="append"></a>
### ST::string_stream &ST::string_stream::append(const char \*data, size_t size = ST_AUTO_SIZE)
~~~c++
string_stream &append(const char *data, size_t size = ST_AUTO_SIZE)
~~~

Append the first `size` bytes of the string data from `data` to the end of
this stream.  If `size` is `ST_AUTO_SIZE`, then the length of `data` is
determined with `strlen`.

------

<a name="append_char"></a>
### ST::string_stream &ST::string_stream::append_char(char ch, size_t count = 1)
~~~c++
string_stream &append(char ch, size_t count = 1)
~~~

Append `count` copies of the character `ch` to the end of this stream.

------

<a name="erase"></a>
### void ST::string_stream::erase(size_t count)
~~~c++
void erase(size_t count) noexcept
~~~

Remove `count` bytes from the end of the stream.  If `count` is greater than
the current stream size, this will truncate the stream to `0` bytes.  This
will not cause a buffer reallocation, so it is free to use.

**See also** [size](#size)(), [truncate](#truncate)(size_t)

------

<a name="operator_eq"></a>
### ST::string_stream &ST::string_stream::operator=(ST::string_stream &&move)
~~~c++
string_stream &operator=(string_stream &&move) noexcept
~~~

Move operator.  Moves the string content from `move` into this string_stream.

**See also** [string_stream](#ctor_0)(string_stream &&)

------

<a name="operator_shl_1"></a>
### ST::string_stream &ST::string_stream::operator&lt;&lt;(const char \*text)
~~~c++
string_stream &operator<<(const char *text)
~~~

Append the C-string `text` to the end of this stream.  The length of `text`
is determined with `strlen`, so it should be nul-terminated.

------

<a name="operator_shl_2"></a>
### ST::string_stream &ST::string_stream::operator&lt;&lt;(int num)
~~~c++
string_stream &operator<<(int num)
~~~

Append a string representation of `num` in signed decimal notation to the end
of this stream.

------

<a name="operator_shl_3"></a>
### ST::string_stream &ST::string_stream::operator&lt;&lt;(unsigned int num)
~~~c++
string_stream &operator<<(unsigned int num)
~~~

Append a string representation of `num` in unsigned decimal notation to the
end of this stream.

------

<a name="operator_shl_4"></a>
### ST::string_stream &ST::string_stream::operator&lt;&lt;(long num)
~~~c++
string_stream &operator<<(long num)
~~~

Append a string representation of `num` in signed decimal notation to the end
of this stream.

------

<a name="operator_shl_5"></a>
### ST::string_stream &ST::string_stream::operator&lt;&lt;(unsigned long num)
~~~c++
string_stream &operator<<(unsigned long num)
~~~

Append a string representation of `num` in unsigned decimal notation to the
end of this stream.

------

<a name="operator_shl_6"></a>
### ST::string_stream &ST::string_stream::operator&lt;&lt;(int64_t num)
~~~c++
string_stream &operator<<(int64_t num)
~~~

Append a string representation of `num` in signed decimal notation to the end
of this stream.

------

<a name="operator_shl_7"></a>
### ST::string_stream &ST::string_stream::operator&lt;&lt;(uint64_t num)
~~~c++
string_stream &operator<<(uint64_t num)
~~~

Append a string representation of `num` in unsigned decimal notation to the
end of this stream.

------

<a name="operator_shl_8"></a>
### ST::string_stream &ST::string_stream::operator&lt;&lt;(float num)
~~~c++
string_stream &operator<<(float num)
~~~

Append a string representation of `num` in floating-point decimal notation
(using the `"%g"` printf-style formatting rules) to the end of this stream.

------

<a name="operator_shl_9"></a>
### ST::string_stream &ST::string_stream::operator&lt;&lt;(double num)
~~~c++
string_stream &operator<<(double num)
~~~

Append a string representation of `num` in floating-point decimal notation
(using the `"%g"` printf-style formatting rules) to the end of this stream.

------

<a name="operator_shl_10"></a>
### ST::string_stream &ST::string_stream::operator&lt;&lt;(char num)
~~~c++
string_stream &operator<<(char ch)
~~~

Append the ASCII character `ch` to the end of this stream.

------

<a name="operator_shl_11"></a>
### ST::string_stream &ST::string_stream::operator&lt;&lt;(const [ST::string](st_string.md) &text)
~~~c++
string_stream &operator<<(const string &text)
~~~

Append the contents of `text` to the end of this stream.

------

<a name="raw_buffer"></a>
### const char \*ST::string_stream::raw_buffer() const
~~~c++
const char *raw_buffer() const noexcept
~~~

Return a pointer to the beginning of the stream data.  Note that this buffer
is NOT nul-terminated, so it should not be used directly in functions that
expect C-style strings.

**See also** [size](#size)(), [to_string](#to_string)(bool, utf_validation_t)

------

<a name="size"></a>
### size_t ST::string_stream::size() const
~~~c++
size_t size() const noexcept
~~~

Returns the size of the data currently stored in the stream, in bytes.

**See also** [raw_buffer](#raw_buffer)()

------

<a name="to_string"></a>
### [ST::string](st_string.md) ST::string_stream::to_string(bool utf8_encoded = true, [ST::utf_validation_t](st_string.md#utf_validation_t) validation = ST::assert_validity) const
~~~c++
string to_string(bool utf8_encoded = true, utf_validation_t validation = assert_validity) const
~~~

Construct an [ST::string](st_string.md) object from the string data.  This will
return a copy of the data in the string (properly nul-terminated), so you can
continue to manipulate the string_stream after creating a string.

If `utf8_encoded` is `true`, the data in the stream is converted with
[ST::string::from_utf8](st_string.md#from_utf8_1) with the specified
`validation`.  If `false`, the data is converted as Latin-1 with
[ST::string::from_latin_1](st_string.md#from_latin_1_1), and `validation` is
ignored.

**See also** [raw_buffer](#raw_buffer)()

------

<a name="truncate"></a>
### void ST::string_stream::truncate(size_t size = 0)
~~~c++
void truncate(size_t size = 0) noexcept
~~~

This will reset the stream's size to `size`.  If `size` is greater than the
current stream size, this method does nothing.  It will not reallocate its
internal buffer, so this may be useful when building multiple strings using
the same string_stream object to construct multiple strings.

**See also** [size](#size)(), [erase](#erase)(size_t)
