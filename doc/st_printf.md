# ST::printf

## Headers
~~~c++
#include <string_theory/stdio>
~~~

## Public Functions

|    |   |
|---:|---|
| template &lt;typename type_T, typename... args_T&gt; void | [ST::printf](#printf_1)(const char \*fmt_str, type_T value, args_T ...args) |
| template &lt;typename type_T, typename... args_T&gt; void | [ST::printf](#printf_1)(FILE \*out_file, const char \*fmt_str, type_T value, args_T ...args) |


## Function Documentation

<a name="printf_1"></a>
### template &lt;typename type_T, typename... args_T&gt; void ST::printf(const char \*fmt_str, type_T value, args_T ...args)
~~~c++
namespace ST
{
    template <typename type_T, typename... args_T>
    void printf(const char *fmt_str, type_T value, args_T ...args)
}
~~~

Writes formatted text to the C `stdout` stream.

------

<a name="printf_2"></a>
### template &lt;typename type_T, typename... args_T&gt; void ST::printf(FILE \*out_file, const char \*fmt_str, type_T value, args_T ...args)
~~~c++
namespace ST
{
    template <typename type_T, typename... args_T>
    string printf(FILE *out_file, const char *fmt_str,
                  type_T value, args_T ...args);
}
~~~

Writes formatted text to the C `FILE` stream `out_file`.
