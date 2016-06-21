# ST::string

## Headers
~~~c++
#include <string_theory/string>
~~~

## Public Types

|    |   |
|---:|---|
| enum | [case_sensitivity_t](#case_sensitivity_t) |
| enum | [utf_validation_t](#utf_validation_t) |

## Public Functions

|    |   |
|---:|---|
|    | [string](#ctor_0)() noexcept |
|    | [string](#ctor_1)(const null_t &) noexcept |
|    | [string](#ctor_2)(const char \*cstr, size_t size = ST_AUTO_SIZE, utf_validation_t validation = ST_DEFAULT_VALIDATION) |
|    | [string](#ctor_3)(const wchar_t \*wstr, size_t size = ST_AUTO_SIZE, utf_validation_t validation = ST_DEFAULT_VALIDATION) |
|    | [string](#ctor_4)(const string &copy) |
|    | [string](#ctor_5)(string &&move) noexcept |
|    | [string](#ctor_6)(const char_buffer &init, utf_validation_t validation = ST_DEFAULT_VALIDATION) |
|    | [string](#ctor_7)(char_buffer &&init, utf_validation_t validation = ST_DEFAULT_VALIDATION) |
|    | [string](#ctor_8)(const utf16_buffer &init, utf_validation_t validation = ST_DEFAULT_VALIDATION) |
|    | [string](#ctor_9)(const utf32_buffer &init, utf_validation_t validation = ST_DEFAULT_VALIDATION) |
|    | [string](#ctor_10)(const wchar_buffer &init, utf_validation_t validation = ST_DEFAULT_VALIDATION) |
|    | [string](#ctor_11)(const std::string &init, utf_validation_t validation = ST_DEFAULT_VALIDATION) |
|    | [string](#ctor_12)(const std::wstring &init, utf_validation_t validation = ST_DEFAULT_VALIDATION) |
|    | [string](#ctor_13)(const std::filesystem::path &init) |
| void | [set](#set_1)(const null_t &) noexcept |
| void | [set](#set_2)(const char \*cstr, size_t size = ST_AUTO_SIZE, utf_validation_t validation = ST_DEFAULT_VALIDATION) |
| void | [set](#set_3)(const wchar_t \*wstr, size_t size = ST_AUTO_SIZE, utf_validation_t validation = ST_DEFAULT_VALIDATION) |
| void | [set](#set_4)(const string &copy) |
| void | [set](#set_5)(string &&move) noexcept |
| void | [set](#set_6)(const char_buffer &init, utf_validation_t validation = ST_DEFAULT_VALIDATION) |
| void | [set](#set_7)(char_buffer &&init, utf_validation_t validation = ST_DEFAULT_VALIDATION) |
| void | [set](#set_8)(const utf16_buffer &init, utf_validation_t validation = ST_DEFAULT_VALIDATION) |
| void | [set](#set_9)(const utf32_buffer &init, utf_validation_t validation = ST_DEFAULT_VALIDATION) |
| void | [set](#set_10)(const wchar_buffer &init, utf_validation_t validation = ST_DEFAULT_VALIDATION) |
| void | [set](#set_11)(const std::string &init, utf_validation_t validation = ST_DEFAULT_VALIDATION) |
| void | [set](#set_12)(const std::wstring &init, utf_validation_t validation = ST_DEFAULT_VALIDATION) |
| void | [set](#set_13)(const std::filesystem::path &init) |
| string & | [operator=](#operator_eq_1)(const null_t &) noexcept |
| string & | [operator=](#operator_eq_2)(const char *cstr) |
| string & | [operator=](#operator_eq_3)(const wchar_t *wstr) |
| string & | [operator=](#operator_eq_4)(const string &copy) |
| string & | [operator=](#operator_eq_5)(string &&move) noexcept |
| string & | [operator=](#operator_eq_6)(const char_buffer &init) |
| string & | [operator=](#operator_eq_7)(char_buffer &&init) |
| string & | [operator=](#operator_eq_8)(const utf16_buffer &init) |
| string & | [operator=](#operator_eq_9)(const utf32_buffer &init) |
| string & | [operator=](#operator_eq_10)(const wchar_buffer &init) |
| string & | [operator=](#operator_eq_11)(const std::string &init) |
| string & | [operator=](#operator_eq_12)(const std::wstring &init) |
| string & | [operator=](#operator_eq_13)(const std::filesystem::path &init) |
| string & | [operator+=](#operator_pluseq_1)(const char *cstr) |
| string & | [operator+=](#operator_pluseq_2)(const wchar_t *wstr) |
| string & | [operator+=](#operator_pluseq_3)(const string &other) |
| const char * | [c_str](#c_str)(const char \*substitute = "") const noexcept |
| char | [char_at](#char_at)(size_t position) const noexcept |
| char_buffer | [to_utf8](#to_utf8)() const noexcept |
| utf16_buffer | [to_utf16](#to_utf16)() const |
| utf32_buffer | [to_utf32](#to_utf32)() const |
| wchar_buffer | [to_wchar](#to_wchar)() const |
| char_buffer | [to_latin_1](#to_latin_1)(utf_validation_t validation = substitute_invalid) const |
| std::string | [to_std_string](#to_std_string)(bool utf8 = true, utf_validation_t validation = substitute_invalid) const |
| std::wstring | [to_std_wstring](#to_std_wstring)(bool utf8 = true, utf_validation_t validation = substitute_invalid) const |
| std::filesystem::path | [to_path](#to_path)() const |
| size_t | [size](#size)() const noexcept |
| bool | [is_empty](#is_empty)() const noexcept |
| int | [to_int](#to_int_1)(int base = 0) const noexcept |
| int | [to_int](#to_int_2)(conversion_result &result, int base = 0) const noexcept |
| unsigned int | [to_uint](#to_uint_1)(int base = 0) const noexcept |
| unsigned int | [to_uint](#to_uint_2)(conversion_result &result, int base = 0) const noexcept |
| float | [to_float](#to_float_1)() const noexcept |
| float | [to_float](#to_float_2)(conversion_result &result) const noexcept |
| double | [to_double](#to_double_1)() const noexcept |
| double | [to_double](#to_double_2)(conversion_result &result) const noexcept |
| int64_t | [to_int64](#to_int64_1)(int base = 0) const noexcept |
| int64_t | [to_int64](#to_int64_2)(conversion_result &result, int base = 0) const noexcept |
| uint64_t | [to_uint64](#to_uint64_1)(int base = 0) const noexcept |
| uint64_t | [to_uint64](#to_uint64_2)(conversion_result &result, int base = 0) const noexcept |
| bool | [to_bool](#to_bool_1)() const noexcept |
| bool | [to_bool](#to_bool_2)(conversion_result &result) const noexcept |
| int | [compare](#compare_1)(const string &str, case_sensitivity_t cs = case_sensitive) const noexcept |
| int | [compare](#compare_2)(const char \*str, case_sensitivity_t cs = case_sensitive) const noexcept |
| int | [compare_n](#compare_n_1)(const string &str, size_t count, case_sensitivity_t cs = case_sensitive) const noexcept |
| int | [compare_n](#compare_n_2)(const char *str, size_t count, case_sensitivity_t cs = case_sensitive) const noexcept |
| int | [compare_i](#compare_i_1)(const string &str) const noexcept |
| int | [compare_i](#compare_i_2)(const char \*str) const noexcept |
| int | [compare_ni](#compare_ni_1)(const string &str, size_t count) const noexcept |
| int | [compare_ni](#compare_ni_2)(const char \*str, size_t count) const noexcept |
| bool | [operator&lt;](#operator_less)(const string &other) const noexcept |
| bool | [operator==](#operator_iseq_1)(const null_t &) const noexcept |
| bool | [operator==](#operator_iseq_2)(const string &other) const noexcept |
| bool | [operator==](#operator_iseq_3)(const char \*other) const noexcept |
| bool | [operator!=](#operator_isneq_1)(const null_t &) const noexcept |
| bool | [operator!=](#operator_isneq_2)(const string &other) const noexcept |
| bool | [operator!=](#operator_isneq_3)(const char \*other) const noexcept |
| ST_ssize_t | [find](#find_1)(char ch, case_sensitivity_t cs = case_sensitive) const noexcept |
| ST_ssize_t | [find](#find_2)(const char *substr, case_sensitivity_t cs = case_sensitive) const noexcept |
| ST_ssize_t | [find](#find_3)(const string &substr, case_sensitivity_t cs = case_sensitive) const noexcept |
| ST_ssize_t | [find_last](#find_last_1)(char ch, case_sensitivity_t cs = case_sensitive) const noexcept |
| ST_ssize_t | [find_last](#find_last_2)(const char \*substr, case_sensitivity_t cs = case_sensitive) const noexcept |
| ST_ssize_t | [find_last](#find_last_3)(const string &substr, case_sensitivity_t cs = case_sensitive) const noexcept |
| bool | [contains](#contains_1)(char ch, case_sensitivity_t cs = case_sensitive) const noexcept |
| bool | [contains](#contains_2)(const char \*substr, case_sensitivity_t cs = case_sensitive) const noexcept |
| bool | [contains](#contains_3)(const string &substr, case_sensitivity_t cs = case_sensitive) const noexcept |
| string | [trim_left](#trim_left)(const char \*charset = ST_WHITESPACE) const |
| string | [trim_right](#trim_right)(const char \*charset = ST_WHITESPACE) const |
| string | [trim](#trim)(const char *charset = ST_WHITESPACE) const |
| string | [substr](#substr)(ST_ssize_t start, size_t size = ST_AUTO_SIZE) const |
| string | [left](#left)(size_t size) const |
| string | [right](#right)(size_t size) const |
| bool | [starts_with](#starts_with_1)(const string &prefix, case_sensitivity_t cs = case_sensitive) const |
| bool | [starts_with](#starts_with_2)(const char \*prefix, case_sensitivity_t cs = case_sensitive) const |
| bool | [ends_with](#ends_with_1)(const string &suffix, case_sensitivity_t cs = case_sensitive) const |
| bool | [ends_with](#ends_with_2)(const char \*suffix, case_sensitivity_t cs = case_sensitive) const |
| string | [before_first](#before_first_1)(char sep, case_sensitivity_t cs = case_sensitive) const |
| string | [before_first](#before_first_2)(const char \*sep, case_sensitivity_t cs = case_sensitive) const |
| string | [before_first](#before_first_3)(const string &sep, case_sensitivity_t cs = case_sensitive) const |
| string | [after_first](#after_first_1)(char sep, case_sensitivity_t cs = case_sensitive) const |
| string | [after_first](#after_first_2)(const char \*sep, case_sensitivity_t cs = case_sensitive) const |
| string | [after_first](#after_first_3)(const string &sep, case_sensitivity_t cs = case_sensitive) const |
| string | [before_last](#before_last_1)(char sep, case_sensitivity_t cs = case_sensitive) const |
| string | [before_last](#before_last_2)(const char \*sep, case_sensitivity_t cs = case_sensitive) const |
| string | [before_last](#before_last_3)(const string &sep, case_sensitivity_t cs = case_sensitive) const |
| string | [after_last](#after_last_1)(char sep, case_sensitivity_t cs = case_sensitive) const |
| string | [after_last](#after_last_2)(const char \*sep, case_sensitivity_t cs = case_sensitive) const |
| string | [after_last](#after_last_3)(const string &sep, case_sensitivity_t cs = case_sensitive) const |
| string | [replace](#replace_1)(const char \*from, const char \*to, case_sensitivity_t cs = case_sensitive, utf_validation_t validation = ST_DEFAULT_VALIDATION) const |
| string | [replace](#replace_2)(const string &from, const char \*to, case_sensitivity_t cs = case_sensitive, utf_validation_t validation = ST_DEFAULT_VALIDATION) const |
| string | [replace](#replace_3)(const char \*from, const string &to, case_sensitivity_t cs = case_sensitive, utf_validation_t validation = ST_DEFAULT_VALIDATION) const |
| string | [replace](#replace_4)(const string &from, const string &to, case_sensitivity_t cs = case_sensitive, utf_validation_t validation = ST_DEFAULT_VALIDATION) const |
| string | [to_upper](#to_upper)() const |
| string | [to_lower](#to_lower)() const |
| std::vector&lt;string&gt; | [split](#split_1)(char split_char, size_t max_splits = ST_AUTO_SIZE, case_sensitivity_t cs = case_sensitive) const |
| std::vector&lt;string&gt; | [split](#split_2)(const char \*splitter, size_t max_splits = ST_AUTO_SIZE, case_sensitivity_t cs = case_sensitive) const |
| std::vector&lt;string&gt; | [split](#split_3)(const string &splitter, size_t max_splits = ST_AUTO_SIZE, case_sensitivity_t cs = case_sensitive) const |
| std::vector&lt;string&gt; | [tokenize](#tokenize)(const char \*delims = ST_WHITESPACE) const |

## Static Public Members

|    |   |
|---:|---|
| string | [from_utf8](#from_utf8_1)(const char \*utf8, size_t size = ST_AUTO_SIZE, utf_validation_t validation = ST_DEFAULT_VALIDATION) |
| string | [from_utf16](#from_utf16_1)(const char16_t \*utf16, size_t size = ST_AUTO_SIZE, utf_validation_t validation = ST_DEFAULT_VALIDATION) |
| string | [from_utf32](#from_utf32_1)(const char32_t \*utf32, size_t size = ST_AUTO_SIZE, utf_validation_t validation = ST_DEFAULT_VALIDATION) |
| string | [from_wchar](#from_wchar_1)(const wchar_t \*wstr, size_t size = ST_AUTO_SIZE, utf_validation_t validation = ST_DEFAULT_VALIDATION) |
| string | [from_latin_1](#from_latin_1_1)(const char \*astr, size_t size = ST_AUTO_SIZE) |
| string | [from_utf8](#from_utf8_2)(const char_buffer &utf8, utf_validation_t validation = ST_DEFAULT_VALIDATION) |
| string | [from_utf16](#from_utf16_2)(const utf16_buffer &utf16, utf_validation_t validation = ST_DEFAULT_VALIDATION) |
| string | [from_utf32](#from_utf32_2)(const utf32_buffer &utf32, utf_validation_t validation = ST_DEFAULT_VALIDATION) |
| string | [from_wchar](#from_wchar_2)(const wchar_buffer &wstr, utf_validation_t validation = ST_DEFAULT_VALIDATION) |
| string | [from_latin_1](#from_latin_1_2)(const char_buffer &astr) |
| string | [from_std_string](#from_std_string)(const std::string &sstr, validation_t validation = ST_DEFAULT_VALIDATION) |
| string | [from_std_wstring](#from_std_wstring)(const std::wstring &wstr, validation_t validation = ST_DEFAULT_VALIDATION) |
| string | [from_path](#from_path)(const std::filesystem::path &path) |
| string | [from_int](#from_int)(int value, int base = 10, bool upper_case = false) |
| string | [from_uint](#from_uint)(unsigned int value, int base = 10, bool upper_case = false) |
| string | [from_float](#from_float)(float value, char format='g') |
| string | [from_double](#from_double)(double value, char format='g') |
| string | [from_int64](#from_int64)(int64_t value, int base = 10, bool upper_case = false) |
| string | [from_uint64](#from_uint64)(uint64_t value, int base = 10, bool upper_case = false) |
| string | [from_bool](#from_bool)(bool value) |
| string | [fill](#fill)(size_t count, char c) |

## Related Non-Members

|    |   |
|---:|---|
| struct | [hash](#hash) |
| struct | [hash_i](#hash_i) |
| struct | [less_i](#less_i) |
| struct | [equal_i](#equal_i) |
| string | [operator+](#operator_plus_1)(const string &left, const string &right) |
| string | [operator+](#operator_plus_2)(const string &left, const char \*right) |
| string | [operator+](#operator_plus_3)(const char \*left, const string &right) |
| string | [operator+](#operator_plus_4)(const string &left, const wchar_t \*right) |
| string | [operator+](#operator_plus_5)(const wchar_t \*left, const string &right) |
| bool | [operator==](#operator_iseq_4)(const null_t &, const string &right) noexcept |
| bool | [operator!=](#operator_isneq_4)(const null_t &, const string &right) noexcept |
| string | [operator"" _st](#operator_st_1)(const char \*str, size_t size) |
| string | [operator"" _st](#operator_st_2)(const wchar_t \*str, size_t size) |
| string | [operator"" _st](#operator_st_3)(const char16_t \*str, size_t size) |
| string | [operator"" _st](#operator_st_4)(const char32_t \*str, size_t size) |

## Macros

|   |
|---|
| [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION) |
| [ST_LITERAL](#ST_LITERAL)(str) |
| [ST_WHITESPACE](#ST_WHITESPACE) |

## Details

ST::string provides storage and manipulation tools for Unicode strings.  The
string data is stored internally as UTF-8 (in a [ST::char_buffer](st_buffer.md)
object).  This makes it easier for dealing with streams and files already in
UTF-8 encoding, but means that many unicode characters make take up more than
one code point (byte) in the string object.

ST::string objects can be easily converted to/from a few other encodings,
including UTF-16, UTF-32 (UCS4), and wchar_t arrays (assumed to be either
UTF-16 or UTF-32 depending on the platform).

With the exception of `operator=` and `operator+=` overloads, ST::string
objects are immutable.  All operations which manipulate the string data
(including `operator=` and `operator+=`) will create a new string buffer with
a copy of the necessary data.  This means that all ST::string members are
re-entrant.  Furthermore, the buffers returned by [to_utf8](#to_utf8)() and
[c_str](#c_str)() are accessors to ST::string's internal storage, meaning
they do not have to be stored externally in order to remain valid.

### Faster string Literals

Although it's perfectly valid to create strings with normal C string literals,
ST::string provides some helpers which skip ST::string's validation and checks
when you know the input data is a string literal already encoded as valid
UTF-8 bytes.

~~~c++
// The following are equivalent, but the second line may be faster
ST::string greeting = "Hello";
ST::string greeting_2 = ST_LITERAL("Hello");

// If you compiler supports user-defined literals, the second greeting can
// be written more concisely as
ST::string greeting_3 = "Hello"_st;
~~~

### Dealing with non-unicode data

ST::string will by default check that its data is valid UTF-8.  If it finds
any input which it can't encode, it will either report the error or substitute
it with a substitute character (U+FFFD), depending on the conversion options.

It is also possible to tell ST::string to skip its checks, if you know the
input data is already valid UTF-8.  However, passing invalid UTF-8 data into
an ST::string with ST::assume_valid is undefined behavior, and may have
unexpected results and bugs.

Finally, it is possible to treat input as Latin-1 (ISO-8859-1) data, which
always succeeds.  However, passing UTF-8 data as Latin-1 may result in
the individual UTF-8 bytes showing up as Latin-1 character sequences.

~~~c++
char *bad_input = "...";

// This will throw ST::unicode_error with a message about what was wrong if
// the conversion encounters any sequences it can't decode.
ST::string str1(bad_input, ST_AUTO_SIZE, ST::check_validity);

// This will call ST_ASSERT() with a message about what was wrong if
// the conversion encounters any sequences it can't decode.
ST::string str2(bad_input, ST_AUTO_SIZE, ST::assert_validity);

// This will replace any character sequences it can't decode with the Unicode
// substitute character (U+FFFD).
ST::string str3(bad_input, ST_AUTO_SIZE, ST::substitute_invalid);

// This will assume the input is already valid UTF-8.  Passing invalid UTF-8
// data with ST::assume_valid is undefined behavior, and may have unexpected
// results and bugs.
ST::string str4(bad_input, ST_AUTO_SIZE, ST::assume_valid);

// Conversion always succeeds; treat data as Latin-1
ST::string str5 = ST::string::from_latin_1(bad_input);
~~~


## Member Type Documentation

<a name="case_sensitivity_t"></a>
### enum ST::case_sensitivity_t
~~~c++
enum case_sensitivity_t
{
    case_sensitive,
    case_insensitive
};
~~~

Indicates the case sensitivity for various find and comparison operations.

- **case_sensitive:** Consider upper- and lower-case characters as different
  when doing string comparisons and searches.
- **case_insensitive:** Consider upper- and lower-case characters as equal
  when doing string comparisons and searches.

------

<a name="utf_validation_t"></a>
### enum ST::utf_validation_t
~~~c++
enum utf_validation_t
{
    assume_valid,
    substitute_invalid,
    check_validity,
    assert_validity
};
~~~

Options for dealing with invalid character sequences in encoding/decoding
operations.

- **assume_valid:** Don't do any checking or substitution.  Only use this value
  if you are certain the data is already correct for the target encoding.
- **substitute_invalid:** Replace invalid sequences with a substitute.  For
  conversions to Unicode encodings, this will use the Unicode replacement
  character (U+FFFD).  For conversions to Latin-1, this will use `'?'`.
- **check_validity:** Throw a [ST::unicode_error](st_unicode_error.md)
  exception if any invalid sequences are encountered in the source data.  This
  is the default for most conversions.
- **assert_validity:** Call the string_theory assert handler if any invalid
  sequences are encountered in the source data.

------


## Member Documentation

<a name="ctor_0"></a>
### ST::string::string()
~~~c++
string() noexcept
~~~

Default constructor for strings.  Creates an empty string.

**See also** [is_empty](#is_empty)()

------

<a name="ctor_1"></a>
### ST::string::string(const [ST::null_t](st_buffer.md#null_t) &)
~~~c++
string(const null_t &) noexcept
~~~

Shortcut constructor for empty strings.  This is equivalent to the empty
constructor.

**See also** [string](#ctor_0)(), [operator=](#operator_eq_1)(const null_t &),
[is_empty](#is_empty)()

------

<a name="ctor_2"></a>
### ST::string::string(const char \*cstr, size_t size = ST_AUTO_SIZE, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION))
~~~c++
string(const char *cstr, size_t size = ST_AUTO_SIZE, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Construct a string from the first `size` bytes of the string pointed to by
`cstr`.  If `size` is `ST_AUTO_SIZE`, the length of the input will be
determined with [ST::char_buffer::strlen](st_buffer.md#strlen)().  The data
pointed to by `cstr` is expected to be encoded as UTF-8.

**See also** [from_utf8](#from_utf8_1)(const char \*, size_t, utf_validation_t)

------

<a name="ctor_3"></a>
### ST::string::string(const wchar_t \*cstr, size_t size = ST_AUTO_SIZE, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION))
~~~c++
string(const wchar_t *wstr, size_t size = ST_AUTO_SIZE, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Construct a string from the first `size` wide characters of the string pointed
to by `wstr`.  If `size` is `ST_AUTO_SIZE`, the length of the input will be
determined with [ST::wchar_buffer::strlen](st_buffer.md#strlen)().  The data
pointed to by `wstr` is expected to be encoded as either UTF-16 or UTF-32,
depending on your platform's wchar_t support.

**See also** [from_wchar](#from_wchar_1)(const wchar_t \*, size_t, utf_validation_t)

------

<a name="ctor_4"></a>
### ST::string::string(const ST::string &copy)
~~~c++
string(const string &copy)
~~~

Construct a string whose contents are a copy of `copy`.

**See also** [operator=](#operator_eq_4)(const string &)

------

<a name="ctor_5"></a>
### ST::string::string(ST::string &&move)
~~~c++
string(string &&move) noexcept
~~~

Move the contents of `move` into this string object.

**See also** [operator=](#operator_eq_5)(string &&)

------

<a name="ctor_6"></a>
### ST::string::string(const [ST::char_buffer](st_buffer.md) &init, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION))
~~~c++
string(const char_buffer &init, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Construct a string from the contents of `init`.  The data stored in `init` is
expected to be encoded as UTF-8.

**See also** [from_utf8](#from_utf8_2)(const char_buffer &, utf_validation_t)

------

<a name="ctor_7"></a>
### ST::string::string([ST::char_buffer](st_buffer.md) &&init, [uST::tf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION))
~~~c++
string(char_buffer &&init, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Move the contents of `init` into this string's internal UTF-8 buffer. The data
stored in `init` will still be checked according to `validation`, and is
expected to be encoded as UTF-8.

**See also** [operator=](#operator_eq_7)(char_buffer &&)

------

<a name="ctor_8"></a>
### ST::string::string(const [ST::utf16_buffer](st_buffer.md) &&init, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION))
~~~c++
string(const utf16_buffer &init, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Construct a string from the UTF-16 data provided in `init`.

**See also** [from_utf16](#from_utf16_2)(const utf16_buffer &, utf_validation_t)

------

<a name="ctor_9"></a>
### ST::string::string(const [ST::utf32_buffer](st_buffer.md) &&init, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION))
~~~c++
string(const utf32_buffer &init, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Construct a string from the UTF-32 / UCS-4 data provided in `init`.

**See also** [from_utf32](#from_utf32_2)(const utf32_buffer &, utf_validation_t)

------

<a name="ctor_10"></a>
### ST::string::string(const [ST::wchar_buffer](st_buffer.md) &&init, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION))
~~~c++
string(const wchar_buffer &init, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Construct a string from the wide character data provided in `init`.  The data
provided in `init` is expected to be encoded as either UTF-16 or UTF-32,
depending on your platform's wchar_t support.

**See also** [from_wchar](#from_wchar_2)(const wchar_buffer &, utf_validation_t)

------

<a name="ctor_11"></a>
### ST::string::string(const std::string &init, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION))
~~~c++
string(const std::string &init, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Construct a string from the the contents of `init`.  The string data is
expected to be encoded as UTF-8.

------

<a name="ctor_12"></a>
### ST::string::string(const std::wstring &init, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION))
~~~c++
string(const std::wstring &init, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Construct a string from the the contents of `init`.  The string data
in `init` is expected to be encoded as either UTF-16 or UTF-32, depending
on your platform's wchar_t support.

------

<a name="ctor_13"></a>
### ST::string::string(const std::filesystem::path &init)
~~~c++
string(const std::filesystem::path &init)
~~~

Construct a string from the the filesystem path in `init`.

------

<a name="after_first_1"></a>
### ST::string ST::string::after_first(char sep, [ST::case_sensitivity_t](#case_sensitivity_t) cs = case_sensitive) const
~~~c++
string after_first(char sep, case_sensitivity_t cs = case_sensitive) const
~~~

Returns the part of this string after the first instance of `sep` found
within the string.  If `sep` is not found in the string, an empty string
is returned.

------

<a name="after_first_2"></a>
### ST::string ST::string::after_first(const char \*sep, [ST::case_sensitivity_t](#case_sensitivity_t) cs = case_sensitive) const
~~~c++
string after_first(const char *sep, case_sensitivity_t cs = case_sensitive) const
~~~

Returns the part of this string after the first instance of `sep` found
within the string.  If `sep` is not found in the string, an empty string
is returned.

------

<a name="after_first_3"></a>
### ST::string ST::string::after_first(const ST::string &sep, [ST::case_sensitivity_t](#case_sensitivity_t) cs = case_sensitive) const
~~~c++
string after_first(const string &sep, case_sensitivity_t cs = case_sensitive) const
~~~

Returns the part of this string after the first instance of `sep` found
within the string.  If `sep` is not found in the string, an empty string
is returned.

------

<a name="after_last_1"></a>
### ST::string ST::string::after_last(char sep, [ST::case_sensitivity_t](#case_sensitivity_t) cs = case_sensitive) const
~~~c++
string after_last(char sep, case_sensitivity_t cs = case_sensitive) const
~~~

Returns the part of this string after the last instance of `sep` found
within the string.  If `sep` is not found in the string, the whole string
is returned.

------

<a name="after_last_2"></a>
### ST::string ST::string::after_last(const char \*sep, [ST::case_sensitivity_t](#case_sensitivity_t) cs = case_sensitive) const
~~~c++
string after_last(const char *sep, case_sensitivity_t cs = case_sensitive) const
~~~

Returns the part of this string after the last instance of `sep` found
within the string.  If `sep` is not found in the string, the whole string
is returned.

------

<a name="after_last_3"></a>
### ST::string ST::string::after_last(const string &sep, [ST::case_sensitivity_t](#case_sensitivity_t) cs = case_sensitive) const
~~~c++
string after_last(const string &sep, case_sensitivity_t cs = case_sensitive) const
~~~

Returns the part of this string after the last instance of `sep` found
within the string.  If `sep` is not found in the string, the whole string
is returned.

------

<a name="before_first_1"></a>
### ST::string ST::string::before_first(char sep, [ST::case_sensitivity_t](#case_sensitivity_t) cs = case_sensitive) const
~~~c++
string before_first(char sep, case_sensitivity_t cs = case_sensitive) const
~~~

Returns the part of this string before the first instance of `sep` found
within the string.  If `sep` is not found in the string, the whole string
is returned.

------

<a name="before_first_2"></a>
### ST::string ST::string::before_first(const char \*sep, [ST::case_sensitivity_t](#case_sensitivity_t) cs = case_sensitive) const
~~~c++
string before_first(const char *sep, case_sensitivity_t cs = case_sensitive) const
~~~

Returns the part of this string before the first instance of `sep` found
within the string.  If `sep` is not found in the string, the whole string
is returned.

------

<a name="before_first_3"></a>
### ST::string ST::string::before_first(const ST::string &sep, [ST::case_sensitivity_t](#case_sensitivity_t) cs = case_sensitive) const
~~~c++
string before_first(const string &sep, case_sensitivity_t cs = case_sensitive) const
~~~

Returns the part of this string before the first instance of `sep` found
within the string.  If `sep` is not found in the string, the whole string
is returned.

------

<a name="before_last_1"></a>
### ST::string ST::string::before_last(char sep, [ST::case_sensitivity_t](#case_sensitivity_t) cs = case_sensitive) const
~~~c++
string before_last(char sep, case_sensitivity_t cs = case_sensitive) const
~~~

Returns the part of this string before the last instance of `sep` found
within the string.  If `sep` is not found in the string, an empty string
is returned.

------

<a name="before_last_2"></a>
### ST::string ST::string::before_last(const char \*sep, [ST::case_sensitivity_t](#case_sensitivity_t) cs = case_sensitive) const
~~~c++
string before_last(const char *sep, case_sensitivity_t cs = case_sensitive) const
~~~

Returns the part of this string before the last instance of `sep` found
within the string.  If `sep` is not found in the string, an empty string
is returned.

------

<a name="before_last_3"></a>
### ST::string ST::string::before_last(const string &sep, [ST::case_sensitivity_t](#case_sensitivity_t) cs = case_sensitive) const
~~~c++
string before_last(const string &sep, case_sensitivity_t cs = case_sensitive) const
~~~

Returns the part of this string before the last instance of `sep` found
within the string.  If `sep` is not found in the string, an empty string
is returned.

------

<a name="c_str"></a>
### const char \*ST::string::c_str(const char \*substitute = "") const
~~~c++
const char *c_str(const char *substitute = "") const noexcept
~~~

Returns a pointer to the stored UTF-8 string data.  This buffer should always
be nul-terminated, so it's safe to use in functions which require C-style
string buffers.  If this string is empty, the pointer provided in `substitute`
will be returned instead.

**See also** [is_empty](#is_empty)()

------

<a name="char_at"></a>
### char ST::string::char_at(size_t position) const
~~~c++
char char_at(size_t position) const noexcept
~~~

Returns the UTF-8 code unit (byte) at the specified position.  Note that this
may return a byte in the middle of a UTF-8 multi-byte sequence!  The
position is not bounds-checked, so accessing positions outside the range
[0, [size](#size)()+1] will result in undefined behavior.

**See also** [c_str](#c_str)()

------

<a name="compare_1"></a>
### int ST::string::compare(const ST::string &str, [ST::case_sensitivity_t](#case_sensitivity_t) cs = ST::case_sensitive) const
~~~c++
int compare(const string &str, case_sensitivity_t cs = case_sensitive) const noexcept
~~~

Compare this string to `str`, in a manner similar to `strcmp`.  That is to
say, if this string is "less than" `str`, this function returns a negative
value.  If this string is "greater than" `str`, this function returns a
positive value.  If this string and `str` are equal, this returns `0`.

Set `cs` to `case_insensitive` in order to perform a case_insensitive
comparison.

**See also** [operator==](#operator_iseq_2)(const string &)

------

<a name="compare_2"></a>
### int ST::string::compare(const char \*str, [ST::case_sensitivity_t](#case_sensitivity_t) cs = ST::case_sensitive) const
~~~c++
int compare(const char *str, case_sensitivity_t cs = case_sensitive) const noexcept
~~~

Compare this string to `str`, in a manner similar to `strcmp`.  That is to
say, if this string is "less than" `str`, this function returns a negative
value.  If this string is "greater than" `str`, this function returns a
positive value.  If this string and `str` are equal, this returns `0`.

Set `cs` to `case_insensitive` in order to perform a case_insensitive
comparison.

**See also** [operator==](#operator_iseq_3)(const char \*)

------

<a name="compare_i_1"></a>
### int ST::string::compare_i(const ST::string &str) const
~~~c++
int compare_i(const string &str) const noexcept
~~~

Convenience comparison function.  Equivalent to
[compare](#compare_1)(str, ST::case_insensitive).

**See also** [compare](#compare_1)(const string &, case_sensitivity_t)

------

<a name="compare_i_2"></a>
### int ST::string::compare_i(const char \*str) const
~~~c++
int compare_i(const char *str) const noexcept
~~~

Convenience comparison function.  Equivalent to
[compare](#compare_2)(str, ST::case_insensitive).

**See also** [compare](#compare_2)(const char \*, case_sensitivity_t)

------

<a name="compare_n_1"></a>
### int ST::string::compare_n(const ST::string &str, size_t count, [ST::case_sensitivity_t](#case_sensitivity_t) cs = ST::case_sensitive) const
~~~c++
int compare(const string &str, size_t count, case_sensitivity_t cs = case_sensitive) const noexcept
~~~

Compare up to the first `count` bytes of this string to `str`, in a manner
similar to `strncmp`.

Set `cs` to `case_insensitive` in order to perform a case_insensitive
comparison.

**See also** [compare](#compare_1)(const string &, case_sensitivity_t)

------

<a name="compare_n_2"></a>
### int ST::string::compare_n(const char \*str, size_t count, [ST::case_sensitivity_t](#case_sensitivity_t) cs = ST::case_sensitive) const
~~~c++
int compare(const char *str, size_t count, case_sensitivity_t cs = case_sensitive) const noexcept
~~~

Compare up to the first `count` bytes of this string to `str`, in a manner
similar to `strncmp`.

Set `cs` to `case_insensitive` in order to perform a case_insensitive
comparison.

**See also** [compare](#compare_2)(const char \*, case_sensitivity_t)

------

<a name="compare_ni_1"></a>
### int ST::string::compare_ni(const ST::string &str, size_t count) const
~~~c++
int compare_ni(const string &str, size_t count) const noexcept
~~~

Convenience comparison function.  Equivalent to
[compare](#compare_n_1)(str, count, ST::case_insensitive).

**See also** [compare_n](#compare_n_1)(const string &, size_t, case_sensitivity_t)

------

<a name="compare_ni_2"></a>
### int ST::string::compare_ni(const char \*str, size_t count) const
~~~c++
int compare_ni(const char *str, size_t count) const noexcept
~~~

Convenience comparison function.  Equivalent to
[compare](#compare_n_2)(str, size, ST::case_insensitive).

**See also** [compare_n](#compare_n_2)(const char \*, size_t, case_sensitivity_t)

------

<a name="contains_1"></a>
### bool ST::string::contains(char ch, [ST::case_sensitivity_t](#case_sensitivity_t) cs = ST::case_sensitive) const
~~~c++
bool contains(char ch, case_sensitivity_t cs = case_sensitive) const noexcept
~~~

Returns `true` if `ch` is contained anywhere in this string.

**See also** [find](#find_1)(char, case_sensitivity_t)

------

<a name="contains_2"></a>
### bool ST::string::contains(const char \*substr, [ST::case_sensitivity_t](#case_sensitivity_t) cs = ST::case_sensitive) const
~~~c++
bool contains(const char *substr, case_sensitivity_t cs = case_sensitive) const noexcept
~~~

Returns `true` if `substr` is contained anywhere in this string.

**See also** [find](#find_2)(const char \*, case_sensitivity_t)

------

<a name="contains_3"></a>
### bool ST::string::contains(const ST::string &substr, [ST::case_sensitivity_t](#case_sensitivity_t) cs = ST::case_sensitive) const
~~~c++
bool contains(const string &substr, case_sensitivity_t cs = case_sensitive) const noexcept
~~~

Returns `true` if `substr` is contained anywhere in this string.

**See also** [find](#find_3)(const string &, case_sensitivity_t)

------

<a name="ends_with_1"></a>
### bool ST::string::ends_with(const ST::string &suffix, [ST::case_sensitivity_t](#case_sensitivity_t) cs = ST::case_sensitive) const
~~~c++
bool ends_with(const string &suffix, case_sensitivity_t cs = case_sensitive) const
~~~

Return `true` if this string ends with `suffix`.

------

<a name="ends_with_2"></a>
### bool ST::string::ends_with(const char \*suffix, [ST::case_sensitivity_t](#case_sensitivity_t) cs = ST::case_sensitive) const
~~~c++
bool ends_with(const char *suffix, case_sensitivity_t cs = case_sensitive) const
~~~

Return `true` if this string ends with `suffix`.

------

<a name="fill"></a>
### ST::string ST::string::fill(size_t count, char c) [static]
~~~c++
static string fill(size_t count, char c)
~~~

Create a string which is pre-populated with `count` copies of the character
in `c`.

------

<a name="find_1"></a>
### ST_ssize_t ST::string::find(char ch, [ST::case_sensitivity_t](#case_sensitivity_t) cs = ST::case_sensitive) const
~~~c++
ST_ssize_t find(char ch, case_sensitivity_t cs = case_sensitive) const noexcept
~~~

Find the first instance of `ch` within the string, and return its byte
position.  If `ch` isn't found, this returns `-1`.

------

<a name="find_2"></a>
### ST_ssize_t ST::string::find(const char \*substr, [ST::case_sensitivity_t](#case_sensitivity_t) cs = ST::case_sensitive) const
~~~c++
ST_ssize_t find(const char *substr, case_sensitivity_t cs = case_sensitive) const noexcept
~~~

Find the first instance of `substr` within the string, and return its byte
position.  If `substr` isn't found, this returns `-1`.

------

<a name="find_3"></a>
### ST_ssize_t ST::string::find(const ST::string &substr, [ST::case_sensitivity_t](#case_sensitivity_t) cs = ST::case_sensitive) const
~~~c++
ST_ssize_t find(const string &substr, case_sensitivity_t cs = case_sensitive) const noexcept
~~~

Find the first instance of `substr` within the string, and return its byte
position.  If `substr` isn't found, this returns `-1`.

------

<a name="find_last_1"></a>
### ST_ssize_t ST::string::find_last(char ch, [ST::case_sensitivity_t](#case_sensitivity_t) cs = ST::case_sensitive) const
~~~c++
ST_ssize_t find_last(char ch, case_sensitivity_t cs = case_sensitive) const noexcept
~~~

Find the last instance of `ch` within the string, and return its byte
position.  If `ch` isn't found, this returns `-1`.

------

<a name="find_last_2"></a>
### ST_ssize_t ST::string::find_last(const char \*substr, [ST::case_sensitivity_t](#case_sensitivity_t) cs = ST::case_sensitive) const
~~~c++
ST_ssize_t find_last(const char *substr, case_sensitivity_t cs = case_sensitive) const noexcept
~~~

Find the last instance of `substr` within the string, and return its byte
position.  If `substr` isn't found, this returns `-1`.

------

<a name="find_last_3"></a>
### ST_ssize_t ST::string::find_last(const ST::string &substr, [ST::case_sensitivity_t](#case_sensitivity_t) cs = ST::case_sensitive) const
~~~c++
ST_ssize_t find_last(const string &substr, case_sensitivity_t cs = case_sensitive) const noexcept
~~~

Find the last instance of `substr` within the string, and return its byte
position.  If `substr` isn't found, this returns `-1`.

------

<a name="from_bool"></a>
### ST::string ST::string::from_bool(bool value) [static]
~~~c++
static string from_bool(bool value)
~~~

Creates the string literal `"true"` or `"false"`, depending on `value`.

------

<a name="from_double"></a>
### ST::string ST::string::from_double(double value, char format = 'g') [static]
~~~c++
static string from_double(double value, char format = 'g')
~~~

Create a string representation of the floating-point number in `value`.
The format character has the same meaning as printf's floating point formats,
and should be one of `'e'`, `'f'` or `'g'`.

------

<a name="from_float"></a>
### ST::string ST::string::from_float(float value, char format = 'g') [static]
~~~c++
static string from_float(float value, char format = 'g')
~~~

Create a string representation of the floating-point number in `value`.
The format character has the same meaning as printf's floating point formats,
and should be one of `'e'`, `'f'` or `'g'`.

------

<a name="from_int"></a>
### ST::string ST::string::from_int(int value, int base = 10, bool upper_case = false) [static]
~~~c++
static string from_int(int value, int base = 10, bool upper_case = false)
~~~

Create a string representation of the integer in `value`.

------

<a name="from_int64"></a>
### ST::string ST::string::from_int64(int64_t value, int base = 10, bool upper_case = false) [static]
~~~c++
static string from_int64(int64_t value, int base = 10, bool upper_case = false)
~~~

Create a string representation of the integer in `value`.

------

<a name="from_latin_1_1"></a>
### ST::string ST::string::from_latin_1(const char \*astr, size_t size = ST_AUTO_SIZE) [static]
~~~c++
static string from_latin_1(const char *astr, size_t size = ST_AUTO_SIZE)
~~~

Construct a string from the first `size` bytes of the Latin-1 / ISO-8859-1
string data in `astr`.  If `size` is `ST_AUTO_SIZE`, the length of the input
will be determined with [ST::char_buffer::strlen](st_buffer.md#strlen)().

------

<a name="from_latin_1_2"></a>
### ST::string ST::string::from_latin_1(const [ST::char_buffer](st_buffer.md) &astr) [static]
~~~c++
static string from_latin_1(const char_buffer &astr)
~~~

Construct a string from the Latin-1 / ISO-8859-1 string data in `astr`.

------

<a name="from_std_string"></a>
### ST::string ST::string::from_std_string(const std::string &sstr, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION)) [static]
~~~c++
static string from_std_string(const std::string &sstr, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Construct a string from a std::string.  The string is expected to be encoded
as UTF-8.

------

<a name="from_std_wstring"></a>
### ST::string ST::string::from_std_wstring(const std::string &wstr, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION)) [static]
~~~c++
static string from_std_wstring(const std::wstring &wstr, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Construct a string from a std::wstring.  The string is expected to be encoded
as either UTF-16 or UTF-32 / UCS-4 depending on your platform's wchar_t
support.

------

<a name="from_path"></a>
### ST::string ST::string::from_path(const std::filesystem::path &path) [static]
~~~c++
static string from_path(const std::filesystem::path &path)
~~~

Construct a string from a filesystem path, using the system's default encoding.

------

<a name="from_uint"></a>
### ST::string ST::string::from_uint(unsigned int value, int base = 10, bool upper_case = false) [static]
~~~c++
static string from_int(unsigned int value, int base = 10, bool upper_case = false)
~~~

Create a string representation of the unsigned integer in `value`.

------

<a name="from_uint64"></a>
### ST::string ST::string::from_uint64(uint64_t value, int base = 10, bool upper_case = false) [static]
~~~c++
static string from_uint64(uint64_t value, int base = 10, bool upper_case = false)
~~~

Create a string representation of the unsigned integer in `value`.

------

<a name="from_utf8_1"></a>
### ST::string ST::string::from_utf8(const char \*utf8, size_t size = ST_AUTO_SIZE, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION)) [static]
~~~c++
static string from_utf8(const char *utf8, size_t size = ST_AUTO_SIZE, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Construct a string from the first `size` bytes of the UTF-8 string data in
`utf8`.  If `size` is `ST_AUTO_SIZE`, the length of the input will be
determined with [ST::char_buffer::strlen](st_buffer.md#strlen)().

------

<a name="from_utf8_2"></a>
### ST::string ST::string::from_utf8(const [ST::char_buffer](st_buffer.md) &utf8, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION)) [static]
~~~c++
static string from_utf8(const char_buffer &utf8, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Construct a string from the UTF-8 string data in `utf8`.

------

<a name="from_utf16_1"></a>
### ST::string ST::string::from_utf16(const char16_t \*utf16, size_t size = ST_AUTO_SIZE, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION)) [static]
~~~c++
static string from_utf16(const char16_t *utf16, size_t size = ST_AUTO_SIZE, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Construct a string from the first `size` characters of the UTF-16 string data
in `utf16`.  If `size` is `ST_AUTO_SIZE`, the length of the input will be
determined with [ST::utf16_buffer::strlen](st_buffer.md#strlen)().

------

<a name="from_utf16_2"></a>
### ST::string ST::string::from_utf16(const [ST::utf16_buffer](st_buffer.md) &utf16, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION)) [static]
~~~c++
static string from_utf16(const utf16_buffer &utf8, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Construct a string from the UTF-16 string data in `utf16`.

------

<a name="from_utf32_1"></a>
### ST::string ST::string::from_utf32(const char32_t \*utf32, size_t size = ST_AUTO_SIZE, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION)) [static]
~~~c++
static string from_utf32(const char32_t *utf32, size_t size = ST_AUTO_SIZE, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Construct a string from the first `size` characters of the UTF-32 string data
in `utf32`.  If `size` is `ST_AUTO_SIZE`, the length of the input will be
determined with [ST::utf32_buffer::strlen](st_buffer.md#strlen)().

------

<a name="from_utf32_2"></a>
### ST::string ST::string::from_utf32(const [ST::utf32_buffer](st_buffer.md) &utf32, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION)) [static]
~~~c++
static string from_utf32(const utf32_buffer &utf8, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Construct a string from the UTF-32 string data in `utf32`.

------

<a name="from_wchar_1"></a>
### ST::string ST::string::from_wchar(const wchar_t \*wstr, size_t size = ST_AUTO_SIZE, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION)) [static]
~~~c++
static string from_wchar(const wchar_t *wstr, size_t size = ST_AUTO_SIZE, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Construct a string from the first `size` wide characters of the wide string
data in `wstr`.  If `size` is `ST_AUTO_SIZE`, the length of the input will be
determined with [ST::wchar_buffer::strlen](st_buffer.md#strlen)().

Note that the data is expected to be either UTF-16 or UTF-32 encoded,
depending on your platform's wchar_t support.

------

<a name="from_wchar_2"></a>
### ST::string ST::string::from_wchar(const [ST::wchar_buffer](st_buffer.md) &wstr, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION)) [static]
~~~c++
static string from_wchar(const wchar_buffer &wstr, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Construct a string from the wide character string data in `wstr`.

Note that the data is expected to be either UTF-16 or UTF-32 encoded,
depending on your platform's wchar_t support.

------

<a name="is_empty"></a>
### bool ST::string::is_empty() const
~~~c++
bool is_empty() const noexcept
~~~

Returns `true` if this string is empty (i.e. its size is 0).  Note that even
for an empty string, the first character pointed to by [c_str](#c_str)() can
be accessed, and should be the nul character (`'\0'`).

**See also** [size](#size)()

------

<a name="left"></a>
### ST::string ST::string::left(size_t size) const
~~~c++
string left(size_t size) const
~~~

Convenience function to extract a substring from the left side of the string.
This is equivalent to [substr](#substr)(0, size).

**See also** [substr](#substr)(ST_ssize_t, size_t)

------

<a name="operator_eq_1"></a>
### ST::string &ST::string::operator=(const [ST::null_t](st_buffer.md#null_t) &)
~~~c++
string &operator=(const null_t &) noexcept
~~~

Shortcut operator=() overload to reset the string to the empty string.

**See also** [string](#ctor_0)(), [set](#set_1)(const null_t &),
[is_empty](#is_empty)()

------

<a name="operator_eq_2"></a>
### ST::string &ST::string::operator=(const char \*cstr)
~~~c++
void operator=(const char *cstr)
~~~

Set the string content to the contents of the string pointed to by `cstr`.
This is equivalent to [set](#set_2)(cstr).

**See also** [set](#set_2)(const char \*, size_t, utf_validation_t)

------

<a name="operator_eq_3"></a>
### ST::string &ST::string::operator=(const wchar_t \*cstr)
~~~c++
string &operator=(const wchar_t *wstr)
~~~

Set the string content from the wide string pointed to by `wstr`.  This is
equivalent to [set](#set_3)(wstr).

**See also** [set](#set_3)(const wchar_t \*, size_t, utf_validation_t)

------

<a name="operator_eq_4"></a>
### ST::string &ST::string::operator=(const ST::string &copy)
~~~c++
string &operator=(const string &copy)
~~~

Set the string to the same value as `copy`.

**See also** [set](#set_4)(const string &)

------

<a name="operator_eq_5"></a>
### ST::string &ST::string::operator=(ST::string &&move)
~~~c++
string &operator=(string &&move) noexcept
~~~

Move the contents of `move` into this string object.

**See also** [set](#set_5)(string &&)

------

<a name="operator_eq_6"></a>
### ST::string &ST::string::operator=(const [ST::char_buffer](st_buffer.md) &init)
~~~c++
string &operator=(const char_buffer &init)
~~~

Set the string from the contents of `init`.  The data stored in `init` is
expected to be encoded as UTF-8.

**See also** [set](#set_6)(const char_buffer &, utf_validation_t)

------

<a name="operator_eq_7"></a>
### ST::string &ST::string::operator=([ST::char_buffer](st_buffer.md) &&init)
~~~c++
string &operator=(char_buffer &&init)
~~~

Move the contents of `init` into this string's internal UTF-8 buffer. The data
stored in `init` will still be checked according to `validation`, and is
expected to be encoded as UTF-8.

**See also** [set](#set_7)(char_buffer &&)

------

<a name="operator_eq_8"></a>
### ST::string &ST::string::operator=(const [ST::utf16_buffer](st_buffer.md) &init)
~~~c++
string &operator=(const utf16_buffer &init)
~~~

Set the string content from the UTF-16 data provided in `init`.

**See also** [set](#set_8)(const utf16_buffer &, utf_validation_t)

------

<a name="operator_eq_9"></a>
### ST::string &ST::string::operator=(const [ST::utf32_buffer](st_buffer.md) &init)
~~~c++
string &operator=(const utf32_buffer &init)
~~~

Set the string content from the UTF-32 / UCS-4 data provided in `init`.

**See also** [set](#est_9)(const utf32_buffer &, utf_validation_t)

------

<a name="operator_eq_10"></a>
### ST::string &ST::string::operator=(const [ST::wchar_buffer](st_buffer.md) &init)
~~~c++
string &operator=(const wchar_buffer &init)
~~~

Set the string content from the wide character data provided in `init`.
The data provided in `init` is expected to be encoded as either UTF-16 or
UTF-32, depending on your platform's wchar_t support.

**See also** [set](#set_10)(const wchar_buffer &, utf_validation_t)

------

<a name="operator_eq_11"></a>
### ST::string &ST::string::operator=(const std::string &init)
~~~c++
string &operator=(const std::string &init)
~~~

Set the string content from the string in `init`.  The string data is
expected to be encoded as UTF-8.

------

<a name="operator_eq_12"></a>
### ST::string &ST::string::operator=(const std::wstring &init)
~~~c++
string &operator=(const std::wstring &init)
~~~

Set the string content from the wide string in `init`. The string data is
expected to be encoded as either UTF-16 or UTF-32, depending on your
platform's wchar_t support.

------

<a name="operator_eq_13"></a>
### ST::string &ST::string::operator=(const std::filesystem::path &init)
~~~c++
string &operator=(const std::filesystem::path &init)
~~~

Set the string content from the filesystem path in `init`.

------

<a name="operator_pluseq_1"></a>
### ST::string &ST::string::operator+=(const char \*cstr)
~~~c++
string &operator+=(const char *cstr)
~~~

Append the contents of `cstr` to the end of this string.  The input is
expected to be encoded as UTF-8.

------

<a name="operator_pluseq_2"></a>
### ST::string &ST::string::operator+=(const wchar_t \*wstr)
~~~c++
string &operator+=(const wchar_t *wstr)
~~~

Append the contents of `wstr` to the end of this string.  The input is
converted to UTF-8 in the same manner as [from_wchar](#from_wchar_1)(const
wchar_t \*).

**See also** [from_wchar](#from_wchar_1)(const wchar_t \*, size_t, utf_validation_t)

------

<a name="operator_pluseq_3"></a>
### ST::string &ST::string::operator+=(const ST::string &other)
~~~c++
string &operator+=(const string &other)
~~~

Append the contents of `other` to the end of this string.

------

<a name="operator_less"></a>
### bool ST::string::operator&lt;(const ST::string &other) const
~~~c++
bool operator<(const string &other) const noexcept
~~~

Convenience operator.  This is provided to work with `std::less` for STL-style
containers.  For more control over string comparisons, see the
[compare](#compare_1)() functions.

**See also** [compare](#compare_1)(const string &, case_sensitivity_t),
[compare](#compare_2)(const char \*, case_sensitivity_t), struct [less_i](#less_i)

------

<a name="operator_iseq_1"></a>
### bool ST::string::operator==(const [ST::null_t](st_buffer.md#null_t) &) const
~~~c++
bool operator==(const null_t &) const noexcept
~~~

Returns true if this string is empty.

**See also** [is_empty](#is_empty)()

------

<a name="operator_iseq_2"></a>
### bool ST::string::operator==(const ST::string &other) const
~~~c++
bool operator==(const string &other) const noexcept
~~~

Convenience operator.  This is equivalent to checking
[compare](#compare_1)(other, ST::case_sensitive) == `0`

**See also** [compare](#compare_1)(const string &, case_sensitivity_t)

------

<a name="operator_iseq_3"></a>
### bool ST::string::operator==(const char \*other) const
~~~c++
bool operator==(const char *other) const noexcept
~~~

Convenience operator.  This is equivalent to checking
[compare](#compare_2)(other, ST::case_sensitive) == `0`

**See also** [compare](#compare_2)(const char \*, case_sensitivity_t)

------

<a name="operator_isneq_1"></a>
### bool ST::string::operator!=(const [ST::null_t](st_buffer.md#null_t) &) const
~~~c++
bool operator!=(const null_t &) const noexcept
~~~

Returns true if this string is not empty.

**See also** [is_empty](#is_empty)()

------

<a name="operator_isneq_2"></a>
### bool ST::string::operator!=(const ST::string &other) const
~~~c++
bool operator!=(const string &other) const noexcept
~~~

Convenience operator.  This is equivalent to checking
[compare](#compare_1)(other, ST::case_sensitive) != `0`

**See also** [compare](#compare_1)(const string &, case_sensitivity_t)

------

<a name="operator_isneq_3"></a>
### bool ST::string::operator!=(const char \*other) const
~~~c++
bool operator!=(const char *other) const noexcept
~~~

Convenience operator.  This is equivalent to checking
[compare](#compare_2)(other, ST::case_sensitive) != `0`

**See also** [compare](#compare_2)(const char \*, case_sensitivity_t)

------

<a name="replace_1"></a>
### ST::string ST::string::replace(const char \*from, const char \*to, [ST::case_sensitivity_t](#case_sensitivity_t) cs = ST::case_sensitive, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION)) const
~~~c++
string replace(const char *from, const char *to, case_sensitivity_t cs = case_sensitive, utf_validation_t validation = ST_DEFAULT_VALIDATION) const
~~~

Return a string which has all instances of the string `from` replaced with
the string in `to`.

------

<a name="replace_2"></a>
### ST::string ST::string::replace(const string &from, const char \*to, [ST::case_sensitivity_t](#case_sensitivity_t) cs = ST::case_sensitive, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION)) const
~~~c++
string replace(const string &from, const char *to, case_sensitivity_t cs = case_sensitive, utf_validation_t validation = ST_DEFAULT_VALIDATION) const
~~~

Return a string which has all instances of the string `from` replaced with
the string in `to`.

------

<a name="replace_3"></a>
### ST::string ST::string::replace(const char \*from, const string &to, [ST::case_sensitivity_t](#case_sensitivity_t) cs = ST::case_sensitive, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION)) const
~~~c++
string replace(const char *from, const string &to, case_sensitivity_t cs = case_sensitive, utf_validation_t validation = ST_DEFAULT_VALIDATION) const
~~~

Return a string which has all instances of the string `from` replaced with
the string in `to`.

------

<a name="replace_4"></a>
### ST::string ST::string::replace(const ST::string &from, const ST::string &to, [ST::case_sensitivity_t](#case_sensitivity_t) cs = ST::case_sensitive, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION)) const
~~~c++
string replace(const string &from, const string &to, case_sensitivity_t cs = case_sensitive, utf_validation_t validation = ST_DEFAULT_VALIDATION) const
~~~

Return a string which has all instances of the string `from` replaced with
the string in `to`.

------

<a name="right"></a>
### ST::string ST::string::right(size_t size) const
~~~c++
string right(size_t size) const
~~~

Convenience function to extract a substring from the right side of the string.
This is equivalent to [substr](#substr)(-size).

**See also** [substr](#substr)(ST_ssize_t, size_t)

------

<a name="set_1"></a>
### void ST::string::set(const [ST::null_t](st_buffer.md#null_t) &)
~~~c++
void set(const null_t &) noexcept
~~~

Shortcut set() overload to reset the string to the empty string.

**See also** [string](#ctor_0)(), [operator=](#operator_eq_1)(const null_t &),
[is_empty](#is_empty)()

------

<a name="set_2"></a>
### void ST::string::set(const char \*cstr, size_t size = ST_AUTO_SIZE, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION))
~~~c++
void set(const char *cstr, size_t size = ST_AUTO_SIZE, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Set the string content to the first `size` bytes of the string pointed to by
`cstr`.  If `size` is `ST_AUTO_SIZE`, the length of the input will be
determined with [ST::char_buffer::strlen](st_buffer.md#strlen)().  The data
pointed to by `cstr` is expected to be encoded as UTF-8.

**See also** [from_utf8](#from_utf8_1)(const char \*, size_t, utf_validation_t)

------

<a name="set_3"></a>
### void ST::string::set(const wchar_t \*cstr, size_t size = ST_AUTO_SIZE, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION))
~~~c++
void set(const wchar_t *wstr, size_t size = ST_AUTO_SIZE, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Set the string content from the first `size` wide characters of the string
pointed to by `wstr`.  If `size` is `ST_AUTO_SIZE`, the length of the input
will be determined with [ST::wchar_buffer::strlen](st_buffer.md#strlen)().
The data pointed to by `wstr` is expected to be encoded as either UTF-16 or
UTF-32, depending on your platform's wchar_t support.

**See also** [from_wchar](#from_wchar_1)(const wchar_t \*, size_t, utf_validation_t)

------

<a name="set_4"></a>
### void ST::string::set(const ST::string &copy)
~~~c++
void set(const string &copy)
~~~

Set the string to the same value as `copy`.

**See also** [operator=](#operator_eq_4)(const string &)

------

<a name="set_5"></a>
### void ST::string::set(ST::string &&move)
~~~c++
void set(string &&move) noexcept
~~~

Move the contents of `move` into this string object.

**See also** [operator=](#operator_eq_5)(string &&)

------

<a name="set_6"></a>
### void ST::string::set(const [ST::char_buffer](st_buffer.md) &init, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION))
~~~c++
void set(const char_buffer &init, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Set the string from the contents of `init`.  The data stored in `init` is
expected to be encoded as UTF-8.

**See also** [from_utf8](#from_utf8_2)(const char_buffer &, utf_validation_t)

------

<a name="set_7"></a>
### void ST::string::set([ST::char_buffer](st_buffer.md) &&init, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION))
~~~c++
void set(char_buffer &&init, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Move the contents of `init` into this string's internal UTF-8 buffer. The data
stored in `init` will still be checked according to `validation`, and is
expected to be encoded as UTF-8.

**See also** [operator=](#operator_eq_7)(char_buffer &&)

------

<a name="set_8"></a>
### void ST::string::set(const [ST::utf16_buffer](st_buffer.md) &init, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION))
~~~c++
void set(const utf16_buffer &init, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Set the string content from the UTF-16 data provided in `init`.

**See also** [from_utf16](#from_utf16_2)(const utf16_buffer &, utf_validation_t)

------

<a name="set_9"></a>
### void ST::string::set(const [ST::utf32_buffer](st_buffer.md) &init, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION))
~~~c++
void set(const utf32_buffer &init, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Set the string content from the UTF-32 / UCS-4 data provided in `init`.

**See also** [from_utf32](#from_utf32_2)(const utf32_buffer &, utf_validation_t)

------

<a name="set_10"></a>
### void ST::string::set(const [ST::wchar_buffer](st_buffer.md) &init, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION))
~~~c++
void set(const wchar_buffer &init, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Set the string content from the wide character data provided in `init`.
The data provided in `init` is expected to be encoded as either UTF-16 or
UTF-32, depending on your platform's wchar_t support.

**See also** [from_wchar](#from_wchar_2)(const wchar_buffer &, utf_validation_t)

------

<a name="set_11"></a>
### void ST::string::set(const std::string &init, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION))
~~~c++
void set(const std::string &init, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Set the string content from the string provided in `init`.  The string is
expected to be encoded as UTF-8.

**See also** [from_wchar](#from_wchar_2)(const wchar_buffer &, utf_validation_t)

------

<a name="set_12"></a>
### void ST::string::set(const std::wstring &init, [ST::utf_validation_t](#utf_validation_t) validation = [ST_DEFAULT_VALIDATION](#ST_DEFAULT_VALIDATION))
~~~c++
void set(const std::wstring &init, utf_validation_t validation = ST_DEFAULT_VALIDATION)
~~~

Set the string content from the wide string data provided in `init`.
The string is expected to be encoded as either UTF-16 or UTF-32, depending
on your platform's wchar_t support.

------

<a name="set_13"></a>
### void ST::string::set(const std::filesystem::path &init)
~~~c++
void set(const std::filesystem::path &init)
~~~

Set the string content from the filesystem path in `init`.

------

<a name="size"></a>
### size_t ST::string::size() const
~~~c++
size_t size() const noexcept
~~~

Returns the size (in bytes) of the string data, not including the
nul-terminator.

**See also** [ST::buffer::size](st_buffer.md#size)()

------

<a name="split_1"></a>
### std::vector&lt;ST::string&gt; ST::string::split(char split_char, size_t max_splits = ST_AUTO_SIZE, [ST::case_sensitivity_t](#case_sensitivity_t) cs = ST::case_sensitive) const
~~~c++
std::vector<string> split(char split_char, size_t max_splits = ST_AUTO_SIZE, case_sensitivity_t cs = case_sensitive) const
~~~

Split the string into pieces separated by `split_char`.  If `max_splits` is not
`ST_AUTO_SIZE` and there are more than `max_splits` separators in the string,
the extras will be preserved in the final element of the returned vector.
Specifically, the maximum size of the returned vector is `max_splits` + `1`
elements.

Note that adjacent separators are treated individually:  Two instances of
`split_char` next to each other will result in an empty string in the result.
If this string is empty, a vector with a single empty string element will be
returned.

**See also** [tokenize](#tokenize)(const char \*)

------

<a name="split_2"></a>
### std::vector&lt;ST::string&gt; ST::string::split(const char \*splitter, size_t max_splits = ST_AUTO_SIZE, [ST::case_sensitivity_t](#case_sensitivity_t) cs = ST::case_sensitive) const
~~~c++
std::vector<string> split(const char *splitter, size_t max_splits = ST_AUTO_SIZE, case_sensitivity_t cs = case_sensitive) const
~~~

Split the string into pieces separated by `splitter`.  If `max_splits` is not
`ST_AUTO_SIZE` and there are more than `max_splits` separators in the string,
the extras will be preserved in the final element of the returned vector.
Specifically, the maximum size of the returned vector is `max_splits` + `1`
elements.

Note that adjacent separators are treated individually:  Two instances of
`splitter` next to each other will result in an empty string in the result.
If this string is empty, a vector with a single empty string element will be
returned.

**See also** [tokenize](#tokenize)(const char \*)

------

<a name="split_3"></a>
### std::vector&lt;ST::string&gt; ST::string::split(const ST::string &splitter, size_t max_splits = ST_AUTO_SIZE, [ST::case_sensitivity_t](#case_sensitivity_t) cs = ST::case_sensitive) const
~~~c++
std::vector<string> split(const string &splitter, size_t max_splits = ST_AUTO_SIZE, case_sensitivity_t cs = case_sensitive) const
~~~

Split the string into pieces separated by `splitter`.  If `max_splits` is not
`ST_AUTO_SIZE` and there are more than `max_splits` separators in the string,
the extras will be preserved in the final element of the returned vector.
Specifically, the maximum size of the returned vector is `max_splits` + `1`
elements.

Note that adjacent separators are treated individually:  Two instances of
`splitter` next to each other will result in an empty string in the result.
If this string is empty, a vector with a single empty string element will be
returned.

**See also** [tokenize](#tokenize)(const char \*)

------

<a name="tokenize"></a>
### std::vector&lt;ST::string&gt; ST::string::tokenize(const char \*delims = [ST_WHITESPACE](#ST_WHITESPACE)) const
~~~c++
std::vector<string> tokenize(const char *delims = ST_WHITESPACE) const
~~~

Split the string into pieces separated by any of the characters in `delims`.
Any sequence of adjacent delimiters will be treated as a single separator,
meaning that no elements of the returned vector will be empty.  If this
string is empty, an empty vector will be returned.

**See also** [split](#split)(char, size_t, case_sensitivity_t),
[split](#split)(const char \*, size_t, case_sensitivity_t),
[split](#split)(const string &, size_t, case_sensitivity_t)

------

<a name="starts_with_1"></a>
### bool ST::string::starts_with(const ST::string &prefix, [ST::case_sensitivity_t](#case_sensitivity_t) cs = ST::case_sensitive) const
~~~c++
bool starts_with(const string &prefix, case_sensitivity_t cs = case_sensitive) const
~~~

Return `true` if this string starts with `prefix`.  Equivalent to
[compare_n_1](#compare_n)(prefix, prefix.size(), cs) == `0`.

**See also** [compare_n_1](#compare_n)(const string &, case_sensitivity_t)

------

<a name="starts_with_2"></a>
### bool ST::string::starts_with(const char \*prefix, [ST::case_sensitivity_t](#case_sensitivity_t) cs = ST::case_sensitive) const
~~~c++
bool starts_with(const char *prefix, case_sensitivity_t cs = case_sensitive) const
~~~

Return `true` if this string starts with `prefix`.  Equivalent to
[compare_n_2](#compare_n)(prefix, strlen(prefix), cs) == `0`.

**See also** [compare_n_2](#compare_n)(const char \*, case_sensitivity_t)

------

<a name="substr"></a>
### ST::string ST::string::substr(ST_ssize_t start, size_t size = ST_AUTO_SIZE) const
~~~c++
string substr(ST_ssize_t start, size_t size = ST_AUTO_SIZE) const
~~~

Return a string whose contents are a copy of at most `size` bytes from this
string, starting at position `start`.  If `size` is `ST_AUTO_SIZE` or `start` +
`size` is greather than the size of the string, this will return the remainder
of the string from the starting position. If `start` is negative, then the
starting position is relative to the end of the string.

------

<a name="to_bool_1"></a>
### bool ST::string::to_bool() const
~~~c++
bool to_bool() const noexcept
~~~

Convert the string to a boolean.  If the string is either "true" or "false"
(case insensitive), those values are converted to the respective boolean
values.  Otherwise, this behaves like [to_int](#to_int_1)(), where a non-zero
result is treated as `true`.

Note that this function has no way of reporting errors.  An empty string will
return `false`, as will any string which cannot be converted to based on the
rules described above.

**See also** [to_bool](#to_bool_2)(conversion_result &)

------

<a name="to_bool_2"></a>
### bool ST::string::to_bool([ST::conversion_result](st_conversion_result.md) &result) const
~~~c++
bool to_bool(conversion_result &result) const noexcept
~~~

Convert the string to a boolean.  If the string is either "true" or "false"
(case insensitive), those values are converted to the respective boolean
values.  Otherwise, this behaves like [to_int](#to_int_1)(), where a non-zero
result is treated as `true`.  The result of the conversion is stored in
`result`.

------

<a name="to_double_1"></a>
### double ST::string::to_double() const
~~~c++
double to_double() const noexcept
~~~

Convert the string to a double-precision floating-point number, in a manner
similar to `strtod`.

Note that this function has no way of reporting errors.  An empty string will
return `0`, and a string with other characters not in the specified `base`
will get partially converted, up to the first invalid character.

**See also** [to_double](#to_double_2)(conversion_result &)

------

<a name="to_double_2"></a>
### float ST::string::to_double([ST::conversion_result](st_conversion_result.md) &result) const
~~~c++
double to_double(conversion_result &result) const noexcept
~~~

Convert the string to a double-precision floating-point number, in a manner
similar to `strtod`.  The result of the conversion is stored in `result`.

------

<a name="to_float_1"></a>
### float ST::string::to_float() const
~~~c++
float to_float() const noexcept
~~~

Convert the string to a single-precision floating-point number, in a manner
similar to `strtod`.

Note that this function has no way of reporting errors.  An empty string will
return `0`, and a string with other characters not in the specified `base`
will get partially converted, up to the first invalid character.

**See also** [to_float](#to_float_2)(conversion_result &)

------

<a name="to_float_2"></a>
### float ST::string::to_float([ST::conversion_result](st_conversion_result.md) &result) const
~~~c++
float to_float(conversion_result &result) const noexcept
~~~

Convert the string to a single-precision floating-point number, in a manner
similar to `strtod`.  The result of the conversion is stored in `result`.

------

<a name="to_int_1"></a>
### int ST::string::to_int(int base = 0) const
~~~c++
int to_int(int base = 0) const noexcept
~~~

Convert the string to an integer.  If `base` is `0`, this function will try
to guess the base in a similar manner to `strtol`.

Note that this function has no way of reporting errors.  An empty string will
return `0`, and a string with other characters not in the specified `base`
will get partially converted, up to the first invalid character.

**See also** [to_int](#to_int_2)(conversion_result &, int)

------

<a name="to_int_2"></a>
### int ST::string::to_int([ST::conversion_result](st_conversion_result.md) &result, int base = 0) const
~~~c++
int to_int(conversion_result &result, int base = 0) const noexcept
~~~

Convert the string to an integer.  If `base` is `0`, this function will try
to guess the base in a similar manner to `strtol`.  The result of the
conversion is stored in `result`, so that validity checking can be done.

------

<a name="to_int64_1"></a>
### int64_t ST::string::to_int64(int base = 0) const
~~~c++
int64_t to_int64(int base = 0) const noexcept
~~~

Convert the string to a 64-bit integer.  If `base` is `0`, this function will
try to guess the base in a similar manner to `strtoll`.

Note that this function has no way of reporting errors.  An empty string will
return `0`, and a string with other characters not in the specified `base`
will get partially converted, up to the first invalid character.

**See also** [to_int64](#to_int64_2)(conversion_result &, int)

------

<a name="to_int64_2"></a>
### int64_t ST::string::to_int64([ST::conversion_result](st_conversion_result.md) &result, int base = 0) const
~~~c++
int64_t to_int64(conversion_result &result, int base = 0) const noexcept
~~~

Convert the string to a 64-bit integer.  If `base` is `0`, this function will
try to guess the base in a similar manner to `strtoll`.  The result of the
conversion is stored in `result`, so that validity checking can be done.

------

<a name="to_latin_1"></a>
### [ST::char_buffer](st_buffer.md) ST::string::to_latin_1([ST::utf_validation_t](#utf_validation_t) validation = ST::substitute_invalid) const
~~~c++
char_buffer to_latin_1(utf_validation_t validation = substitute_invalid) const
~~~

Convert the string content to Latin-1 / ISO-8859-1.  Any characters outside
of the Latin-1 range will be handled according to `validation`.

**See also** enum [utf_validation_t](#utf_validation_t)

------

<a name="to_lower"></a>
### ST::string ST::string::to_lower() const
~~~c++
string to_lower() const
~~~

Returns a copy of this string with all ANSI letters converted to lower-case.

------

<a name="to_std_string"></a>
### std::string ST::string::to_std_string(bool utf8 = true, [ST::utf_validation_t](#utf_validation_t) validation = ST::substitute_invalid) const
~~~c++
std::string to_std_string(bool utf8 = true, utf_validation_t validation = substitute_invalid) const
~~~

Convert the string content to a std::string.  If `utf8` is true, the data will
be converted as UTF-8; otherwise, it will be converted as Latin-1.

**See also** [to_utf8](to_utf8)(), [to_latin_1](to_latin_1)(utf_validation_t)

------

<a name="to_std_wstring"></a>
### std::wstring ST::string::to_std_wstring() const
~~~c++
std::wstring to_std_wstring() const
~~~

Convert the string content to a std::wstring.  The string will be encoded
as either UTF-16 or UTF-32 / UCS-4 depending on your platform's wchar_t
support.

------

<a name="to_path"></a>
### std::filesystem::path ST::string::to_path() const
~~~c++
std::filesystem::path to_path() const
~~~

Convert the string to a filesystem path using the system's default path
encoding.

------

<a name="to_uint_1"></a>
### unsigned int ST::string::to_uint(int base = 0) const
~~~c++
unsigned int to_uint(int base = 0) const noexcept
~~~

Convert the string to an unsigned integer.  If `base` is `0`, this function
will try to guess the base in a similar manner to `strtoul`.

Note that this function has no way of reporting errors.  An empty string will
return `0`, and a string with other characters not in the specified `base`
will get partially converted, up to the first invalid character.

**See also** [to_uint](#to_uint_2)(conversion_result, int)

------

<a name="to_uint_2"></a>
### unsigned int ST::string::to_uint([ST::conversion_result](st_conversion_result.md) &result, int base = 0) const
~~~c++
unsigned int to_uint(conversion_result &result, int base = 0) const noexcept
~~~

Convert the string to an unsigned integer.  If `base` is `0`, this function
will try to guess the base in a similar manner to `strtoul`.  The result of the
conversion is stored in `result`, so that validity checking can be done.

------

<a name="to_uint64_1"></a>
### uint64_t ST::string::to_uint64(int base = 0) const
~~~c++
uint64_t to_uint64(int base = 0) const noexcept
~~~

Convert the string to an unsigned 64-bit integer.  If `base` is `0`, this
function will try to guess the base in a similar manner to `strtoull`.

Note that this function has no way of reporting errors.  An empty string will
return `0`, and a string with other characters not in the specified `base`
will get partially converted, up to the first invalid character.

**See also** [to_uint64](#to_uint64_2)(conversion_result, int)

------

<a name="to_uint64_2"></a>
### uint64_t ST::string::to_uint64([ST::conversion_result](st_conversion_result.md) &result, int base = 0) const
~~~c++
uint64_t to_uint64(conversion_result &result, int base = 0) const noexcept
~~~

Convert the string to an unsigned 64-bit integer.  If `base` is `0`, this
function will try to guess the base in a similar manner to `strtoull`.  The
result of the conversion is stored in `result`, so that validity checking can
be done.

------

<a name="to_upper"></a>
### ST::string ST::string::to_upper() const
~~~c++
string to_upper() const
~~~

Returns a copy of this string with all ANSI letters converted to upper-case.

------

<a name="to_utf8"></a>
### [ST::char_buffer](st_buffer.md) ST::string::to_utf8() const
~~~c++
char_buffer to_utf8() const noexcept
~~~

Return a copy of the UTF-8 string data buffer.

------

<a name="to_utf16"></a>
### [ST::utf16_buffer](st_buffer.md) ST::string::to_utf16() const
~~~c++
utf16_buffer to_utf16() const
~~~

Convert the string content to UTF-16.

------

<a name="to_utf32"></a>
### [ST::utf32_buffer](st_buffer.md) ST::string::to_utf32() const
~~~c++
utf32_buffer to_utf32() const
~~~

Convert the string content to UTF-32 / UCS-4.

------

<a name="to_wchar"></a>
### [ST::wchar_buffer](st_buffer.md) ST::string::to_wchar() const
~~~c++
wchar_buffer to_wchar() const
~~~

Convert the string content to a wchar_t buffer.  The buffer is encoded either
as UTF-16 or UTF-32 / UCS-4, depending on your platform's wchar_t support.

------

<a name="trim"></a>
### ST::string ST::string::trim(const char \*charset = [ST_WHITESPACE](#ST_WHITESPACE)) const
~~~c++
string trim(const char *charset = ST_WHITESPACE) const
~~~

Return a string which has any characters found in `charset` removed from both
the left and right sides of the string.

------

<a name="trim_left"></a>
### ST::string ST::string::trim_left(const char \*charset = [ST_WHITESPACE](#ST_WHITESPACE)) const
~~~c++
string trim_left(const char *charset = ST_WHITESPACE) const
~~~

Return a string which has any characters found in `charset` removed from the
left side of the string.

------

<a name="trim_right"></a>
### ST::string ST::string::trim_right(const char \*charset = [ST_WHITESPACE](#ST_WHITESPACE)) const
~~~c++
string trim_right(const char *charset = ST_WHITESPACE) const
~~~

Return a string which has any characters found in `charset` removed from the
right side of the string.

------


## Related Non-Member Documentation

<a name="equal_i"></a>
### struct ST::equal_i
~~~c++
struct equal_i
{
    bool operator()(const string &left, const string &right) const noexcept
};
~~~

Functor object which returns true for case-insensitive string comparisons
where the `left` string is equal to the `right` string.  This is designed
for STL-style containers which need case-insensitive ordering.

**See also** struct [hash_i](#hash_i), struct [less_i](#less_i),
struct [equal_i](#equal_i), [operator==](#operator_iseq_2)()

------

<a name="hash"></a>
### struct ST::hash
~~~c++
struct hash
{
    size_t operator()(const string &str) const noexcept;

protected:
      virtual unsigned char fetch_char(const string &str, size_t index);
};
~~~

Functor object which returns a reasonable hash for the provided string.  This
is designed for STL-style containers which use hashing for indexes, e.g.
`std::unordered_map`.

**See also** struct [hash_i](#hash_i), struct [less](#less)

------

<a name="hash_i"></a>
### struct ST::hash_i
~~~c++
struct hash_i : public hash
{
protected:
      unsigned char fetch_char(const string &str, size_t index) override;
};
~~~

Functor object which returns a reasonable case-insensitive hash for the
provided string.  This is designed for STL-style containers which use hashing
for indexes, e.g. `std::unordered_map`.

**See also** struct [hash](#hash), struct [less_i](#less_i),
struct [equal_i](#equal_i)

------

<a name="less_i"></a>
### struct ST::less_i
~~~c++
struct less_i
{
    bool operator()(const string &left, const string &right) const noexcept
};
~~~

Functor object which returns true for case-insensitive string comparisons
where the `left` string is less than the `right` string.  This is designed
for STL-style containers which need case-insensitive ordering.

**See also** struct [hash_i](#hash_i), struct [less_i](#less_i),
struct [equal_i](#equal_i), [operator&lt;](#operator_less)()

------

<a name="operator_plus_1"></a>
### ST::string ST::operator+(const ST::string &left, const ST::string &right)
~~~c++
string operator+(const string &left, const string &right)
~~~

Returns a string which is the concatenation of `left` and `right`.

**See also** [operator+=](#operator_pluseq_3)(const string &)

------

<a name="operator_plus_2"></a>
### ST::string ST::operator+(const ST::string &left, const char \*right)
~~~c++
string operator+(const string &left, const char *right)
~~~

Returns a string which is the concatenation of `left` and `right`.

**See also** [operator+=](#operator_pluseq_1)(const char \*)

------

<a name="operator_plus_3"></a>
### ST::string ST::operator+(const char \*left, const ST::string &right)
~~~c++
string operator+(const char *left, const string &right)
~~~

Returns a string which is the concatenation of `left` and `right`.

**See also** [operator+=](#operator_pluseq_1)(const char \*)

------

<a name="operator_plus_4"></a>
### ST::string ST::operator+(const ST::string &left, const wchar_t \*right)
~~~c++
string operator+(const string &left, const wchar_t *right)
~~~

Returns a string which is the concatenation of `left` and `right`.

**See also** [operator+=](#operator_pluseq_2)(const wchar_t \*)

------

<a name="operator_plus_5"></a>
### ST::string ST::operator+(const wchar_t \*left, const ST::string &right)
~~~c++
string operator+(const wchar_t *left, const string &right)
~~~

Returns a string which is the concatenation of `left` and `right`.

**See also** [operator+=](#operator_pluseq_2)(const wchar_t \*)

------

<a name="operator_iseq_4"></a>
### bool ST::operator==(const [ST::null_t](st_buffer.md#null_t) &, const ST::string &right)
~~~c++
bool operator==(const null_t &, const string &right) noexcept
~~~

Return true if `right` is empty.

**See also** [is_empty](#is_empty)()

------

<a name="operator_isneq_4"></a>
### bool ST::operator!=(const [ST::null_t](st_buffer.md#null_t) &, const ST::string &right)
~~~c++
bool operator!=(const null_t &, const string &right) noexcept
~~~

Return true if `right` is not empty.

**See also** [is_empty](#is_empty)()

------

<a name="operator_st_1"></a>
### ST::string operator"" _st(const char \*str, size_t size)
~~~c++
ST::string operator"" _st(const char *str, size_t size)
~~~

User-defined literal operator to convert string literals to ST::string
objects efficiently.  The string literal should be encoded as UTF-8.

Because this operator assumes UTF-8 data, it is as efficient as using
[ST_LITERAL](#ST_LITERAL)() to construct the string.

**See also** [ST_LITERAL](#ST_LITERAL)(str),
[from_utf8](#from_utf8_1)(const char \*, size_t, utf_validation_t)

------

<a name="operator_st_2"></a>
### ST::string operator"" _st(const wchar_t \*str, size_t size)
~~~c++
ST::string operator"" _st(const wchar_t *str, size_t size)
~~~

User-defined literal operator to convert string literals to ST::string
objects efficiently.  The string literal should be encoded as either UTF-16
or UTF-32, depending on your platform's wchar_t support.

**See also** [from_wchar](#from_wchar_1)(const wchar_t \*, size_t, utf_validation_t)

------

<a name="operator_st_3"></a>
### ST::string operator"" _st(const char16_t \*str, size_t size)
~~~c++
ST::string operator"" _st(const char16_t *str, size_t size)
~~~

User-defined literal operator to convert string literals to ST::string
objects efficiently.  The string literal should be encoded as UTF-16.

**See also** [from_utf16](#from_utf16_1)(const char16_t \*, size_t, utf_validation_t)

------

<a name="operator_st_4"></a>
### ST::string operator"" _st(const char32_t \*str, size_t size)
~~~c++
ST::string operator"" _st(const char32_t *str, size_t size)
~~~

User-defined literal operator to convert string literals to ST::string
objects efficiently.  The string literal should be encoded as UTF-32.

**See also** [from_utf32](#from_utf32_1)(const char32_t \*, size_t, utf_validation_t)

------


## Macro Documentation

<a name="ST_DEFAULT_VALIDATION"></a>
### ST_DEFAULT_VALIDATION
~~~c++
#ifndef ST_DEFAULT_VALIDATION
#   define ST_DEFAULT_VALIDATION ST::check_validity
#endif
~~~

The default checking type for methods which do validity checking.  It is
possible to override the default by defining an alternate ST_DEFAULT_VALIDATION
before including `<string_theory/string>`, however it is generally recommended
to leave the default and explicitly set other values in method calls that need
different behavior.

**See also** enum [utf_validation_t](#utf_validation_t)

------

<a name="ST_LITERAL"></a>
### ST_LITERAL(str)
~~~c++
#define ST_LITERAL(str)  ...
~~~

Generate an ST::string from static UTF-8 data in an efficient manner.  This
bypasses the normal constructor size and validity checks to construct the
string object directly (at compile time), making it much faster than the
[string](#ctor_2)() or [from_utf8](#from_utf8_1)() constructors.

**See also** [operator"" _st](#operator_st_1)(const char \*str, size_t size)

------

<a name="ST_WHITESPACE"></a>
### ST_WHITESPACE
~~~c++
#define ST_WHITESPACE " \t\r\n"
~~~

A default set of whitespace characters, useful for trimming and tokenizing
strings.

**See also** [trim](#trim)(const char \*), [tokenize](#tokenize)(const char \*)
