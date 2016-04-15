# ST::buffer&lt;char_T&gt;

## Headers
~~~c++
#include <string_theory/char_buffer>
~~~

## Public Functions

|    |   |
|---:|---|
|    | [buffer](#ctor_0)() noexcept |
|    | [buffer](#ctor_1)(const null_t &) noexcept |
|    | [buffer](#ctor_2)(const buffer&lt;char_T&gt; &copy) |
|    | [buffer](#ctor_3)(buffer&lt;char_T&gt; &&move) noexcept |
|    | [buffer](#ctor_4)(const char_T \*data, size_t size) |
|    | [~buffer](#dtor)() noexcept |
| buffer&lt;char_T&gt; & | [operator=](#operator_eq_1)(const null_t &) noexcept |
| buffer&lt;char_T&gt; & | [operator=](#operator_eq_2)(const buffer&lt;char_T&gt; &copy) |
| buffer&lt;char_T&gt; & | [operator=](#operator_eq_3)(buffer&lt;char_T&gt; &&move) noexcept |
| bool | [operator==](#operator_iseq_1)(const null_t &) const noexcept |
| bool | [operator==](#operator_iseq_2)(const buffer&lt;char_T&gt; &other) const noexcept |
| bool | [operator!=](#operator_isneq_1)(const null_t &) const noexcept |
| bool | [operator!=](#operator_isneq_2)(const buffer&lt;char_T&gt; &other) const noexcept |
| const char_T \* | [data](#data)() const noexcept |
| size_t | [size](#size)() const noexcept |
| bool | [is_empty](#is_empty)() const noexcept |
|    | [operator const char_T \*](#operator_const_char_p)() const noexcept |
| char_T \* | [create_writable_buffer](#create_writable_buffer)(size_t size) |

## Static Public Members

|    |   |
|---:|---|
| size_t | [strlen](#strlen)(const char_T \*buffer) |

## Related Non-Members

|    |   |
|---:|---|
| struct | [null_t](#null_t) |
| typedef | [char_buffer](#char_buffer) |
| typedef | [wchar_buffer](#wchar_buffer) |
| typedef | [utf16_buffer](#utf16_buffer) |
| typedef | [utf32_buffer](#utf32_buffer) |
| bool | [operator==](#operator_iseq_3)(const null_t &, const buffer&lt;char_T&gt; &right) noexcept |
| bool | [operator!=](#operator_isneq_3)(const null_t &, const buffer&lt;char_T&gt; &right) noexcept |

## Details

The ST::buffer<char_T> class provides basic storage for a contiguous sequence
of characters.  This is used internally by [ST::string](st_string.md), as well
as a storage area for the various UTF conversion results.  It can also be used
by itself as a way to store characters in various encodings; for example,
[ST::wchar_buffer](#wchar_buffer) objects may provide convenient storage for
Win32 APIs which use the wchar_t variants.

Note that ST::buffer objects are meant to be used primarily as storage, not for
string manipulation.  You should convert the buffer to an ST::string in order
to use the normal string methods on the data.  However, when constructing
strings from various sources (such as reading from a file), it may be more
convenient to create the buffer with the
[create_writable_buffer](#create_writable_buffer)() method, and then fill in
the data directly before passing it off to an ST::string.

With the exception of [create_writable_buffer](#create_writable_buffer)(),
ST::buffer objects are intended to be immutable and re-entrant.  Do not use
create_writable_buffer() or the pointer that it returns on a buffer that may
be accessible from multiple threads.

**Implementation Note:** ST::buffer is a short-string optimized data type.
This means that buffers smaller than the buffer size (currently 15 `char_T`
code units, plus one for the nul-terminator) will exist purely on the stack
with no associated heap data.


## Member Documentation

<a name="ctor_0"></a>
### ST::buffer::buffer()
~~~c++
buffer() noexcept
~~~

Default constructor for character buffers.  Creates an empty buffer.

**See also** [is_empty](#is_empty)()

------

<a name="ctor_1"></a>
### ST::buffer::buffer(const [null_t](#null_t) &)
~~~c++
buffer(const null_t &) noexcept
~~~

Shortcut constructor for empty character buffers.  This is equivalent to the
empty constructor.

**See also** [buffer](#ctor_0)(), [operator=](#operator_eq_1)(const null_t &),
[is_empty](#is_empty)()

------

<a name="ctor_2"></a>
### ST::buffer::buffer(const buffer&lt;char_T&gt; &copy)
~~~c++
buffer(const buffer<char_T> &copy)
~~~

Copy constructor.  Creates a new buffer with the same contents as `copy`.

**See also** [operator=](#operator_eq_2)(const buffer&lt;char_T&gt; &)

------

<a name="ctor_3"></a>
### ST::buffer::buffer(buffer&lt;char_T&gt; &&move)
~~~c++
buffer(buffer<char_T> &&move)
~~~

Move constructor.  Moves the contents of `move` into this object.

**See also** [operator=](#operator_eq_3)(buffer&lt;char_T&gt; &&)

------

<a name="ctor_4"></a>
### ST::buffer::buffer(const char_T \*data, size_t size)
~~~c++
buffer(const char_T *data, size_t size)
~~~

Creates a new buffer with a copy of the first `size` characters pointed to by
`data`.  This will make a *copy* of the data buffer, so the original is safe
to delete or modify after the object is constructed.

**See also** [create_writable_buffer](#create_writable_buffer)(size_t)

------

<a name="dtor"></a>
### ST::buffer::~buffer()
~~~c++
~buffer() noexcept
~~~

Destroys the buffer, freeing any associated memory.

------

<a name="create_writable_buffer"></a>
### char_T *ST::buffer::create_writable_buffer(size_t size)
~~~c++
char_T *create_writable_buffer(size_t size)
~~~

This method creates a new (uninitialized) storage buffer for `size` + 1
`char_T` characters, destroying the previous buffer contents (if any).  This
method is primarily intended for use in constructing string buffers from
external sources that can populate an array of characters directly.  It is
therefore more efficient than the [ST::buffer](#ctor_4)(const char_T \*, size_t)
constructor, since create_writable_buffer returns a pointer than can be written
to in-place rather than needing to make a copy of it.

**NOTE**: It is important to nul-terminate the returned buffer, as it will not
be done for you.

**Example**

~~~c++
ST::string read_string(FILE *stream)
{
    ST::char_buffer result;
    uint32_t size;
    size_t nread = fread(&size, sizeof(size), 1, stream);
    assert(nread == sizeof(size));
    char *buf = result.create_writable_buffer(size);
    nread = fread(buf, sizeof(char), size, stream);

    // Be sure to null-terminate the buffer returned by create_writable_buffer!
    buf[size] = 0;
    return ST::string::from_utf8(result);
}
~~~

------

<a name="data"></a>
### const char_T \*ST::buffer::data() const
~~~c++
const char_T *data() const noexcept
~~~

Returns a pointer to the first stored character in the buffer.

------

<a name="is_empty"></a>
### bool ST::buffer::is_empty() const
~~~c++
bool is_empty() const noexcept
~~~

Returns true if this buffer is empty (has no characters).  Note that even
for an empty buffer, the first character pointed to by [data](#data)() can
be accessed, and should be the nul character (`'\0'`).

**See also** [size](#size)()

------

<a name="operator_eq_1"></a>
### ST::buffer&lt;char_T&gt; &ST::buffer::operator=(const [null_t](#null_t) &)
~~~c++
buffer<char_T> &operator=(const null_t &) noexcept
~~~

Assigns an empty buffer to this object.  Equivalent to:
~~~c++
ST::char_buffer buf;
buf = ST::char_buffer();
~~~

**See also** [buffer](#ctor_0)(), [buffer](#ctor_1)(const null_t &)

------

<a name="operator_eq_2"></a>
### ST::buffer&lt;char_T&gt; &ST::buffer::operator=(const buffer&lt;char_T&gt; &copy)
~~~c++
buffer<char_T> &operator=(const buffer<char_T> &copy)
~~~

Copy the contents of `copy` into the current buffer object.

**See also** [buffer](#ctor_2)(const buffer&lt;char_T&gt; &copy)

------

<a name="operator_eq_3"></a>
### ST::buffer&lt;char_T&gt; &ST::buffer::operator=(buffer&lt;char_T&gt; &&move)
~~~c++
buffer<char_T> &operator=(buffer<char_T> &&move) noexcept
~~~

Move the contents of `move` into the current buffer object.

**See also** [buffer](#ctor_3)(buffer&lt;char_T&gt; &&move)

------

<a name="operator_iseq_1"></a>
### bool ST::buffer::operator==(const [null_t](#null_t) &) const
~~~c++
bool operator==(const null_t &) const noexcept
~~~

Returns true if this buffer is empty.

**See also** [is_empty](#is_empty)(),
[operator==](#operator_iseq_3)(const null_t &, const buffer&lt;char_T&gt; &rhs)

------

<a name="operator_iseq_2"></a>
### bool ST::buffer::operator==(const buffer&lt;char_T&gt; &other) const
~~~c++
bool operator==(const buffer<char_T> &other) const noexcept
~~~

Returns true if the contents of this buffer are identical to the contents of
`other`.  This comparison is safe on buffers with embedded nul characters.

------

<a name="operator_isneq_1"></a>
### bool ST::buffer::operator!=(const [null_t](#null_t) &) const
~~~c++
bool operator!=(const null_t &) const noexcept
~~~

Returns true if this buffer is not empty.

**See also** [is_empty](#is_empty)(),
[operator!=](#operator_isneq_3)(const null_t &, const buffer&lt;char_T&gt; &rhs)

------

<a name="operator_isneq_2"></a>
### bool ST::buffer::operator!=(const buffer&lt;char_T&gt; &other) const
~~~c++
bool operator!=(const buffer<char_T> &other) const noexcept
~~~

Returns true if the contents of this buffer are different from the contents of
`other`.  This comparison is safe on buffers with embedded nul characters.

------

<a name="operator_const_char_p"></a>
### ST::buffer::operator const char_T \*() const
~~~c++
operator const char_T *() const noexcept
~~~

Allows for easy conversion of ST::buffer objects to functions which expect
raw `char_T` pointers.  This function is equivalent to [data](#data)().

**Example**
~~~c++
#include <windows.h>

ST::string path = "C:\\Some\\Filename.txt";
ST::wchar_buffer wpath = path.to_wchar();
HANDLE hFile = CreateFileW(wpath, ...);
~~~

**See also** [data](#data)()

------

<a name="size"></a>
### size_t ST::buffer::size() const
~~~c++
size_t size() const noexcept
~~~

Returns the number of `char_T` character units stored in the buffer, not
including the terminating nul (`'\0'`) character.

------

<a name="strlen"></a>
### size_t ST::buffer::strlen(const char_T \*buffer)
~~~c++
static size_t strlen(const char_T *buffer)
~~~

Returns the number of characters up to but not including the first
nul-terminator pointed to by `buffer`.  This is equivalent to the C standard
fucntion `strlen()`, except that it works on arbitrary character types.

------


## Related Non-Member Documentation

<a name="char_buffer"></a>
### typedef char_buffer
~~~c++
namespace ST
{
    typedef buffer<char> char_buffer;
}
~~~

------

<a name="null_t"></a>
### struct null_t
~~~c++
namespace ST
{
    struct null_t { };
    extern const null_t null;
}
~~~

This is a special type designed to provide a clean and efficient way to
construct (or default) empty strings and buffers.  Using it is equivalent
to using the default constructor, but may be shorter to type or more clear
in some situations.  Note that for ST::string objects, defaulting to ST::null
will often produce a faster construction than defaulting to `""`.

**Examples**
~~~c++
// The following two declarations are equivalent:
ST::string foo = ST::null;
ST::string foo;

void bar(const ST::string &s = ST::null);
~~~

------

<a name="operator_iseq_3"></a>
### bool operator==(const [null_t](#null_t) &, const buffer&lt;char_T&gt; &right)
~~~c++
bool operator==(const null_t &, const buffer<char_T> &right) noexcept
~~~

Returns true if `right` is empty.

**See also** [is_empty](#is_empty)(), [operator==](#operator_iseq_2)(const null_t &)

------

<a name="operator_isneq_3"></a>
### bool operator!=(const [null_t](#null_t) &, const buffer&lt;char_T&gt; &right)
~~~c++
bool operator!=(const null_t &, const buffer<char_T> &right) noexcept
~~~

Returns true if `right` is not empty.

**See also** [is_empty](#is_empty)(), [operator!=](#operator_isneq_2)(const null_t &)

------

<a name="utf16_buffer"></a>
### typedef utf16_buffer
~~~c++
namespace ST
{
    typedef buffer<char16_t> utf16_buffer;
}
~~~

------

<a name="utf32_buffer"></a>
### typedef utf32_buffer
~~~c++
namespace ST
{
    typedef buffer<char32_t> utf32_buffer;
}
~~~

------

<a name="wchar_buffer"></a>
### typedef wchar_buffer
~~~c++
namespace ST
{
    typedef buffer<wchar_t> wchar_buffer;
}
~~~
