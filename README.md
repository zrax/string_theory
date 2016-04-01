String Theory
=============

Introduction
------------

String Theory is a flexible C++11 library for string manipulation and storage.
It stores data internally as UTF-8, for ease of use with exiting C/C++ APIs.
It can also handle conversion to and from UTF-16 and UTF-32, and has a variety
of methods for easier text manipulation.

In addition, if your compiler supports it, String Theory includes a powerful
type-safe string formatter (`ST::format`), which can be extended with custom
type formatters by end-user code.

Why another string library?
---------------------------

String Theory was originally developed to replace the half-dozen or so string
types and string manipulation mini-libraries in the [Plasma game
engine](http://github.com/H-uru/Plasma).  Because of the state of the code, it
was designed primarily to reduce coding errors, provide an easy-to-use set of
manipulation functionality with minimal surprises, handle Unicode text without
a lot of overhead, and have reasonable performance.  Many existing string
libraries provide some subset of those features, but were hard to integrate
well with Plasma, or didn't meet all of our needs.  Therefore, plString (and
later plFormat) were born.  After it had matured a while, it seemed that other
projects could benefit from the string library, and so it was ported out into
its own stand-alone library, which is String Theory.

String Theory's features
------------------------

String Theory is designed to provide:
- Minimal surprises.  Strings are immutable objects, so you never have to worry
  whether your `.replace()` will create a copy or modify the original -- it
  will always return a copy unless the new string is identical.
- UTF-8 by default.  You don't have to remember what encoding your string data
  came in as; by the time `ST::string` is constructed, its data is assumed to
  already be in the UTF-8 encoding.  This also allows easy re-use by other
  character-based APIs, since you don't have to first down-convert the string
  data from UTF-16 or UTF-32 in order to use it.
- Easy conversion to Unicode formats.  String theory provides conversion
  between UTF-8, UTF-16, UTF-32 and Latin-1.  In addition, it can check raw
  character input for validity with several mechanisms (assert, C++ exceptions,
  replacement of invalid characters, or just ignore).
- Type-safe formatting.  `sprintf` and friends are notoriously unsafe, and
  are one of the most common sources of bugs in string code.  `ST::format` uses
  C++11's variadic templates to provide a type-safe way to format strings.
  String Theory also provides a mechanism to create custom formatters for
  end-user code, in order to extend `ST::format`'s capabilities.
- Reasonable performance.  String theory is optimized to be reasonably fast
  on a variety of compilers and systems.  For `ST::string`, this ends up being
  a bit slower than C++'s `std::string` due to the extra encoding work.
  However, in my tests `ST::string_stream` tends to be faster or at least on
  par with `std::string`, and `ST::format` generally clocks in significantly
  faster than `boost::format` (albeit slower than non-type-safe formatters
  like `sprintf`).
- Cross Platform.  String Theory is supported on any platform that provides a
  reasonably modern C++ compiler.  You can use it with older compilers as well,
  but note that the feature set may become somewhat reduced by doing so.
- Minimal dependencies.  Currently, String Theory has no run-time dependencies
  aside from the C/C++ standard libraries and runtime.  Additional tools may
  however be necessary for building String Theory or its documentation.
- Well tested.  String Theory comes with a suite of unit tests to ensure it
  works as designed.

What String Theory is NOT
-------------------------

- A full Unicode library.  If you need more Unicode support than just basic
  UTF data conversion, you probably want to use something like
  [ICU](http://icu-project.org) instead.
- A faster version of `std::string`.  String Theory was never designed to be
  faster than STL, and because of its design goal to always use UTF-8 data
  internally, it may be slower for some use cases.
- A regular expression library.  C++11 provides a regex library which should
  be usable with `ST::string`, and I don't have a compelling reason at this
  point to introduce another regular expression library to String Theory.
- A library for working with theoretical physics.  Just in case you got this
  far and were still uncertain :).

Contributing to String Theory
-----------------------------

String Theory is Open Source software, licensed under the
[MIT license](https://opensource.org/licenses/MIT).  Contributions are welcome,
and are accepted as issues or pull requests on GitHub:
http://github.com/zrax/string_theory.

Some ideas for areas to contribute:
- Fix bugs (See http://github.com/zrax/string_theory/issues)
- Write documentation
- Translate documentation to other languages
- Write more unit tests
