# string_theory assert handlers

## Headers
~~~c++
#include <string_theory/assert>
~~~

## Public Types

|    |   |
|---:|---|
| typedef | [assert_handler_t](#assert_handler_t) |

## Public Functions

|    |   |
|---:|---|
| void | [set_assert_handler](#set_assert_handler)(assert_handler_t handler) noexcept |
| void | [set_default_assert_handler](#set_default_assert_handler)() noexcept |


## Type Documentation

<a name="assert_handler_t"></a>
### typedef ST::assert_handler_t
~~~c++
namespace ST
{
    typedef std::function<void (const char *condition_str,
                                const char *filename, int line,
                                const char *message)> assert_handler_t
}
~~~

C++ function object to be called when the string_theory assert handler is
called for a failed assertion.  This may either hold a function pointer or
a C++11 lambda expression.

The function may wish to display `message` to the user, as this is the message
that indicates what sort of failure occurred.  In addition, the C++ code which
evaluated falsely is provided in `condition_str`, as well as the `filename`
and `line` number within the string_theory source code which triggered the
assertion.

It is *strongly* recommended to abort the process in this function.  While it
is not strictly necessary, failing to do so may cause parts of string_theory
to execute with undefined behavior, potentially leading to other crashes or
even unusual and unexpected results.

**See also** [ST::set_assert_handler](#set_assert_handler)(assert_handler_t)

------

## Function Documentation

<a name="set_assert_handler"></a>
### void ST::set_assert_handler([ST::assert_handler_t](#assert_handler_t) handler)
~~~c++
namespace ST
{
    set_assert_handler(assert_handler_t handler) noexcept;
}
~~~

Sets the assert handler to `handler`.  This function will be called whenever a
string_theory assertion fails for any reason.

Note that while it is not possible to compile string_theory without asserts,
it is possible to handle these assertion failures with your own handler (for
example, writing to a log or executing cleanup code prior to termination).

While it is possible to use this function to disable asserts in string_theory,
it is not recommended as doing so may cause parts of string_theory to execute
with undefined behavior, potentially leading to other crashes or even unusual
and unexpected results.

**See also** [ST::assert_handler_t](#assert_handler_t)

------

<a name="set_default_assert_handler"></a>
### void ST::set_default_assert_handler()
~~~c++
namespace ST
{
    set_default_assert_handler() noexcept;
}
~~~

Reset the string_theory assert handler back to its default implementation.
The default implementation simply writes the filename, line number, and
message text to stderr, and then calls `abort()`.
