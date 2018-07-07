String Theory
=============

<a href="https://travis-ci.org/zrax/string_theory">
  <img alt="Travis Build Status"
       src="https://travis-ci.org/zrax/string_theory.svg?branch=master"/>
</a>
<a href="https://ci.appveyor.com/project/zrax/string-theory/branch/master">
  <img alt="AppVeyor Build Status"
       src="https://ci.appveyor.com/api/projects/status/48di5o8n0btl58c7/branch/master?svg=true"/>
</a>
<a href="https://scan.coverity.com/projects/zrax-string_theory">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/8580/badge.svg"/>
</a>

Introduction
------------

String Theory is a flexible C++11 library for string manipulation and storage.
It stores data internally as UTF-8, for ease of use with exiting C/C++ APIs.
It can also handle conversion to and from UTF-16 and UTF-32, and has a variety
of methods for easier text manipulation.

In addition, if your compiler supports it, String Theory includes a powerful
type-safe string formatter (`ST::format`), which can be extended with custom
type formatters by end-user code.

You can find the [full documentation](https://github.com/zrax/string_theory/wiki)
online at https://github.com/zrax/string_theory/wiki.

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
  par with `std::stringstream`, and `ST::format` generally clocks in
  significantly faster than `boost::format` (albeit slower than non-type-safe
  formatters like `sprintf`).
- Reentrance.  Another side-effect of immutable strings is that ST::string is
  a fully reentrant string object with no locking necessary.
- Cross Platform.  String Theory is supported on any platform that provides a
  reasonably modern C++ compiler.  You can use it with older compilers as well,
  but note that the feature set may become somewhat reduced by doing so.
- Minimal dependencies.  Currently, String Theory has no run-time dependencies
  aside from the C/C++ standard libraries and runtime.  Additional tools may
  however be necessary for building String Theory or its tests.
- Well tested.  String Theory comes with a suite of unit tests to ensure it
  works as designed.

What String Theory is NOT
-------------------------

- A full Unicode library.  If you need more Unicode support than just basic
  UTF data conversion, you probably want to use something like
  [ICU](http://icu-project.org) instead.
- A faster version of `std::string`.  String Theory was never designed to be
  faster than STL, and because of its design goal to always use UTF-8 data
  internally, it may be slower for some use cases.  However, practical tests
  have shown that `ST::string` performs at least on par with STL in many use
  cases, and `ST::format` is usually significantly faster than other type-safe
  alternatives such as `boost::format`.
- A regular expression library.  C++11 provides a regex library which should
  be usable with `ST::string`, and I don't have a compelling reason at this
  point to introduce another regular expression library to String Theory.
- A library for working with theoretical physics.  Just in case you got this
  far and were still uncertain :).

Platform Support
----------------

string_theory supports a variety of platforms and compilers.  As of January
2018, string_theory is tested and working on:
- GCC 7 (Arch Linux x86_64 and ARMv7)
- GCC 5 (Ubuntu 16.04 x86_64)
- GCC 4.8 (Ubuntu 14.04 i686)
- GCC 4.6 (Ubuntu 12.04 i686)
- GCC 4.4 (CentOS 6 x86_64)
- Clang 5 (Arch Linux x86_64 and ARMv7)
- Clang 3.8 (Ubuntu 16.04 x86_64)
- Clang 3.4 (Ubuntu 14.04 i686)
- AppleClang 8.0 (OSX El Capitan)
- AppleClang 6.0 (OSX Mavericks)
- MSVC 2017 (x64 and x86)
- MSVC 2015 (x64 and x86)
- MSVC 2013 (x64 and x86)
- MSVC 2012 (x86)
- MSVC 2010 (x86)
- MinGW-w64 GCC 7 (x86_64 and i686)
- MinGW-w64 GCC 4.9 (i686)

Note that `ST::format` and friends may require newer compilers than the base
string_theory due to the C++11 features it depends on.  Specifically, you'll
need at least GCC 4.5, Clang 3.1 or MSVC 2013 (or the equivalent from another
compiler vendor) to use it.

Contributing to String Theory
-----------------------------

String Theory is Open Source software, licensed under the
[MIT license](https://opensource.org/licenses/MIT).  Contributions are welcome,
and may be submitted as issues and/or pull requests on GitHub:
http://github.com/zrax/string_theory.

Some ideas for areas to contribute:
- Report and/or fix bugs (See http://github.com/zrax/string_theory/issues)
- Write and/or fix [documentation](https://github.com/zrax/string_theory/wiki)
- Translate documentation to other languages
- Write more unit tests
