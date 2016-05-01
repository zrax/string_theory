# ST::format

## Headers
~~~c++
#include <string_theory/format>
~~~

## Public Functions

|    |   |
|---:|---|
| template &lt;typename type_T, typename... args_T&gt; string | [format](#format_1)(const char \*fmt_str, type_T value, args_T ...args) |
| template &lt;typename type_T, typename... args_T&gt; string | [format](#format_2)(utf_validation_t validation, const char \*fmt_str, type_T value, args_T ...args) |
| template &lt;typename type_T, typename... args_T&gt; string | [format_latin_1](#format_latin_1)(const char \*fmt_str, type_T value, args_T ...args) |


## Function Documentation

<a name="format_1"></a>
### template &lt;typename type_T, typename... args_T&gt; [ST::string](st_string.md) ST::format(const char \*fmt_str, type_T value, args_T ...args)
~~~c++
namespace ST
{
    template <typename type_T, typename... args_T>
    string format(const char *fmt_str, type_T value, args_T ...args)
}
~~~

Creates a formatted [ST::string](st_string.md) object with the specified
formatting and data.  This function assumes UTF-8 and uses the default
verification (specified by [ST_DEFAULT_VALIDATION](st_string.md#ST_DEFAULT_VALIDATION))
to check the resulting string data.

------

<a name="format_2"></a>
### template &lt;typename type_T, typename... args_T&gt; [ST::string](st_string.md) ST::format([ST::utf_validation_t](st_string.md#utf_validation_t) validation, const char \*fmt_str, type_T value, args_T ...args)
~~~c++
namespace ST
{
    template <typename type_T, typename... args_T>
    string format(utf_validation_t validation, const char *fmt_str,
                  type_T value, args_T ...args);
}
~~~

Creates a formatted [ST::string](st_string.md) object with the specified
formatting and data.  This function assumes UTF-8 and uses the `validation`
setting to check the resulting string data.

------

<a name="format_latin_1"></a>
### template &lt;typename type_T, typename... args_T&gt; [ST::string](st_string.md) ST::format_latin_1(const char \*fmt_str, type_T value, args_T ...args)
~~~c++
namespace ST
{
    template <typename type_T, typename... args_T>
    string format_latin_1(const char *fmt_str, type_T value, args_T ...args);
}
~~~

Creates a formatted [ST::string](st_string.md) object with the specified
formatting and data.  This function assumes Latin-1 data, which means that
no validation is done on the input.
