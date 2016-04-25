# ST::format_spec

## Headers
~~~c++
#include <string_theory/formatter>
~~~

## Public Types

|    |   |
|---:|---|
| enum | [alignment_t](#alignment_t) |
| enum | [digit_class_t](#digit_class_t) |
| enum | [float_class_t](#float_class_t) |

## Public Functions

|    |   |
|---:|---|
|    | [format_spec](#ctor_0)() noexcept |

## Public Members

|    |   |
|---:|---|
| int | [minimum_length](#minimum_length) |
| int | [precision](#precision) |
| alignment_t | [alignment](#alignment) |
| digit_class_t | [digit_class](#digit_class) |
| float_class_t | [float_class](#float_class) |
| char | [pad](#pad) |
| bool | [always_signed](#always_signed) |
| bool | [class_prefix](#class_prefix) |
| bool | [numeric_pad](#numeric_pad) |

## Details

The ST::format_spec structure defines the parameters for formatting an object
with string_theory's formatters.  It is usually populated by the format call
itself (see the [Format string reference](format-strings.md) guide for
details), but it can also be modified if necessary within the type formatter.
Mostly, it is useful for deciding how to format something based on the user's
requested formatting rules.


## Member Type Documentation

<a name="alignment_t"></a>
### enum ST::alignment_t
~~~c++
enum alignment_t
{
    align_default,
    align_left,
    align_right
};
~~~

Indicates the text alignment for objects.

- **align_default:** Use the formatter-specific default alignment for the
  given data.  For built-in formatters, this is right for numbers and left
  for everything else.
- **align_left:** Align text to the left of the reserved format space.
- **align_right:** Align text to the right of the reserved format space.

------

<a name="digit_class_t"></a>
### enum ST::digit_class_t
~~~c++
enum digit_class_t
{
    digit_default,
    digit_dec,
    digit_hex,
    digit_hex_upper,
    digit_oct,
    digit_bin,
    digit_char
};
~~~

Specifies the number base to use when formatting numeric objects.

- **digit_default:** Use the default digit class.  This is `digit_dec` for
  integers and `digit_char` for character types.
- **digit_dec:** Format number in decimal (base 10).
- **digit_hex:** Format number in lower-case hexadecimal.
- **digit_hex_upper:** Format number in upper-case hexadecimal.
- **digit_oct:** Format number in octal (base 8).
- **digit_bin:** Format number in binary.
- **digit_char:** Format number as a single Unicode character (encoded as UTF-8).

------

<a name="float_class_t"></a>
### enum ST::float_class_t
~~~c++
enum float_class_t
{
    float_default,
    float_fixed,
    float_exp,
    float_exp_upper
};
~~~

Specifies the formatting type to use when formatting floating-point numbers.

- **float_default:** Auto-determine the formatting rules.  This is equivalent to
  the `'%g'` printf-style format.
- **float_fixed:** Format number in fixed-point.  This is equivalent to the
  `'%f'` printf-style format.
- **float_exp:** Format number in exponent notation.  This is equivalent to
  the `'%e'` printf-style format.
- **float_exp_upper:** Identical to `float_exp`, except that an upper-case 'E'
  is used to separate the base from the exponent.

------


## Member Documentation

<a name="ctor_0"></a>
### ST::format_spec::format_spec()
~~~c++
format_spec() noexcept
~~~

Default constructor.  Sets all properties to their default values.

------

<a name="alignment"></a>
### [ST::alignment_t](#alignment_t) ST::format_spec::alignment
~~~c++
alignment_t alignment
~~~

Which direction to align the text.  Note that this value is only meaningful
if the actual length of the formatted text is shorter than the specified
[minimum_length](#minimum_length).  For the case of `align_default`, it is
up to the formatter to decide which direction to align the text.  For the
built-in formatters, `align_right` is used for numerics, and `align_left` is
used for everything else.

Note that when [numeric_pad](#numeric_pad) is `true`, this value will not be
used, since numeric padding is always on the left side of the number.

------

<a name="always_signed"></a>
### bool ST::format_spec::always_signed
~~~c++
bool always_signed
~~~

If this property is `true`, then a `'+'` should be added to the left of numeric
formats for positive and unsigned values.

------

<a name="class_prefix"></a>
### bool ST::format_spec::class_prefix
~~~c++
bool class_prefix
~~~

If this property is `true`, then a [digit_class](#digit_class)-specific prefix
should be added to the left of numeric formats.

The following prefixes are added:
- **digit_bin:** `'0b'`
- **digit_hex:** `'0x'`
- **digit_hex_upper:** `'0X'`
- **digit_oct:** `'0'`
- ***Others:*** (no prefix)

------

<a name="digit_class"></a>
### [ST::digit_class_t](#digit_class_t) ST::format_spec::digit_class
~~~c++
digit_class_t digit_class
~~~

How to format integer and character objects.  For details, see the
[digit_class_t](#digit_class_t) enumeration documentation.

------

<a name="float_class"></a>
### [ST::float_class_t](#float_class_t) ST::format_spec::float_class
~~~c++
float_class_t float_class
~~~

How to render floating point numbers.  For details, see the
[float_class_t](#float_class_t) enumeration documentation.

------

<a name="minimum_length"></a>
### int ST::format_spec::minimum_length
~~~c++
int minimum_length
~~~

The minimum number of characters to use in formatting the object.  If the
actual formatted length is less than `minimum_length`, the extra space will
be filled in with [pad](#pad) characters.

------

<a name="numeric_pad"></a>
### bool ST::format_spec::numeric_pad
~~~c++
bool numeric_pad
~~~

If this property is `true`, then numeric rules should be used when applying
the [pad](#pad) character (usually `'0'`) to the left side of the format.
Specifically:
- Other prefixes (like the [class_prefix](#class_prefix) and the sign) should
  be placed *before* the padding.
- Alignment is always right -- the zeros should always be on the left side of
  the formatted digits.

------

<a name="pad"></a>
### char ST::format_spec::pad
~~~c++
char pad
~~~

The character to use when padding formatted strings.  If the resulting format
is shorter than [minimum_length](#minimum_length), this (ASCII) character will
be used to fill in from either the left or right, depending on the value of
the [alignment](#alignment) property.

------

<a name="precision"></a>
### int ST::format_spec::precision
~~~c++
int precision
~~~

The precision to use when formatting floating-point numbers.  That is, the
number of digits after the radix for `float_fixed` and `float_exp` numbers, or
the total number of significant digits for `float_default` numbers.
