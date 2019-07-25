/*  Copyright (c) 2016 Michael Hansen

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE. */

#include "st_string.h"

#include <cstdlib>
#include <cstring>

int ST::string::to_int(int base) const noexcept
{
    return static_cast<int>(strtol(c_str(), nullptr, base));
}

int ST::string::to_int(ST::conversion_result &result, int base) const noexcept
{
    if (empty()) {
        result.m_flags = ST::conversion_result::result_full_match;
        return 0;
    }

    char *end;
    int value = static_cast<int>(strtol(c_str(), &end, base));
    result.m_flags = 0;
    if (end != c_str())
        result.m_flags |= ST::conversion_result::result_ok;
    if (end == c_str() + size())
        result.m_flags |= ST::conversion_result::result_full_match;
    return value;
}

unsigned int ST::string::to_uint(int base) const noexcept
{
    return static_cast<unsigned int>(strtoul(c_str(), nullptr, base));
}

unsigned int ST::string::to_uint(ST::conversion_result &result, int base)
    const noexcept
{
    if (empty()) {
        result.m_flags = ST::conversion_result::result_full_match;
        return 0;
    }

    char *end;
    unsigned int value = static_cast<unsigned int>(strtoul(c_str(), &end, base));
    result.m_flags = 0;
    if (end != c_str())
        result.m_flags |= ST::conversion_result::result_ok;
    if (end == c_str() + size())
        result.m_flags |= ST::conversion_result::result_full_match;
    return value;
}

float ST::string::to_float() const noexcept
{
    return static_cast<float>(strtof(c_str(), nullptr));
}

float ST::string::to_float(ST::conversion_result &result) const noexcept
{
    if (empty()) {
        result.m_flags = ST::conversion_result::result_full_match;
        return 0;
    }

    char *end;
    float value = strtof(c_str(), &end);
    result.m_flags = 0;
    if (end != c_str())
        result.m_flags |= ST::conversion_result::result_ok;
    if (end == c_str() + size())
        result.m_flags |= ST::conversion_result::result_full_match;
    return value;
}

double ST::string::to_double() const noexcept
{
    return strtod(c_str(), nullptr);
}

double ST::string::to_double(ST::conversion_result &result) const noexcept
{
    if (empty()) {
        result.m_flags = ST::conversion_result::result_full_match;
        return 0;
    }

    char *end;
    double value = strtod(c_str(), &end);
    result.m_flags = 0;
    if (end != c_str())
        result.m_flags |= ST::conversion_result::result_ok;
    if (end == c_str() + size())
        result.m_flags |= ST::conversion_result::result_full_match;
    return value;
}

#ifdef ST_HAVE_INT64
int64_t ST::string::to_int64(int base) const noexcept
{
    return static_cast<int64_t>(strtoll(c_str(), nullptr, base));
}

int64_t ST::string::to_int64(ST::conversion_result &result, int base)
    const noexcept
{
    char *end;
    int64_t value = static_cast<int64_t>(strtoll(c_str(), &end, base));
    result.m_flags = 0;
    if (end != c_str())
        result.m_flags = ST::conversion_result::result_ok;
    if (end == c_str() + size())
        result.m_flags |= ST::conversion_result::result_full_match;
    return value;
}

uint64_t ST::string::to_uint64(int base) const noexcept
{
    return static_cast<uint64_t>(strtoull(c_str(), nullptr, base));
}

uint64_t ST::string::to_uint64(ST::conversion_result &result, int base)
    const noexcept
{
    char *end;
    uint64_t value = static_cast<uint64_t>(strtoull(c_str(), &end, base));
    result.m_flags = 0;
    if (end != c_str())
        result.m_flags = ST::conversion_result::result_ok;
    if (end == c_str() + size())
        result.m_flags |= ST::conversion_result::result_full_match;
    return value;
}
#endif

static int _compare_cs(const char *left, const char *right, size_t fsize) noexcept
{
    return std::char_traits<char>::compare(left, right, fsize);
}

static int _compare_cs(const char *left, size_t lsize,
                       const char *right, size_t rsize) noexcept
{
    return ST::char_buffer::compare(left, lsize, right, rsize);
}

static int _compare_cs(const char *left, size_t lsize,
                       const char *right, size_t rsize, size_t maxlen) noexcept
{
    return ST::char_buffer::compare(left, lsize, right, rsize, maxlen);
}

static int _compare_ci(const char *left, const char *right, size_t fsize) noexcept
{
    while (fsize--) {
        const char cl = _ST_PRIVATE::cl_fast_lower(*left++);
        const char cr = _ST_PRIVATE::cl_fast_lower(*right++);
        if (cl != cr)
            return cl - cr;
    }
    return 0;
}

static int _compare_ci(const char *left, size_t lsize,
                       const char *right, size_t rsize) noexcept
{
    const size_t cmplen = std::min(lsize, rsize);
    const int cmp = _compare_ci(left, right, cmplen);
    return cmp ? cmp : lsize - rsize;
}

static int _compare_ci(const char *left, size_t lsize,
                       const char *right, size_t rsize, size_t maxlen) noexcept
{
    lsize = std::min(lsize, maxlen);
    rsize = std::min(rsize, maxlen);
    return _compare_ci(left, lsize, right, rsize);
}

int ST::string::compare(const string &str, case_sensitivity_t cs) const noexcept
{
    return (cs == case_sensitive) ? _compare_cs(c_str(), size(), str.c_str(), str.size())
                                  : _compare_ci(c_str(), size(), str.c_str(), str.size());
}

int ST::string::compare(const char *str, case_sensitivity_t cs) const noexcept
{
    const size_t rsize = str ? std::char_traits<char>::length(str) : 0;
    return (cs == case_sensitive) ? _compare_cs(c_str(), size(), str ? str : "", rsize)
                                  : _compare_ci(c_str(), size(), str ? str : "", rsize);
}

int ST::string::compare_n(const string &str, size_t count,
                          case_sensitivity_t cs) const noexcept
{
    return (cs == case_sensitive) ? _compare_cs(c_str(), size(), str.c_str(), str.size(), count)
                                  : _compare_ci(c_str(), size(), str.c_str(), str.size(), count);
}

int ST::string::compare_n(const char *str, size_t count,
                          case_sensitivity_t cs) const noexcept
{
    const size_t rsize = str ? std::char_traits<char>::length(str) : 0;
    return (cs == case_sensitive) ? _compare_cs(c_str(), size(), str ? str : "", rsize, count)
                                  : _compare_ci(c_str(), size(), str ? str : "", rsize, count);
}

const char *_ST_PRIVATE::find_cs(const char *haystack, const char *needle)
{
    return strstr(haystack, needle);
}

const char *_ST_PRIVATE::find_ci(const char *haystack, const char *needle)
{
    // The "easy" way
    size_t sublen = std::char_traits<char>::length(needle);
    const char *cp = haystack;
    const char *ep = cp + std::char_traits<char>::length(haystack);
    while (cp + sublen <= ep) {
        if (_compare_ci(cp, needle, sublen) == 0)
            return cp;
        ++cp;
    }
    return nullptr;
}

const char *_ST_PRIVATE::find_ci(const char *haystack, size_t size, char ch)
{
    const char *cp = haystack;
    const char *ep = haystack + size;
    const int lch = _ST_PRIVATE::cl_fast_lower(static_cast<char>(ch));
    while (cp < ep) {
        if (_ST_PRIVATE::cl_fast_lower(*cp) == lch)
            return cp;
        ++cp;
    }
    return nullptr;
}

ST_ssize_t ST::string::find(size_t start, char ch, case_sensitivity_t cs)
    const noexcept
{
    if (start >= size())
        return -1;

    const char *cp = (cs == case_sensitive)
            ? _ST_PRIVATE::find_cs(c_str() + start, size() - start, ch)
            : _ST_PRIVATE::find_ci(c_str() + start, size() - start, ch);
    return cp ? (cp - c_str()) : -1;
}

ST_ssize_t ST::string::find(size_t start, const char *substr, case_sensitivity_t cs)
    const noexcept
{
    if (!substr || !substr[0] || start >= size())
        return -1;

    const char *cp = (cs == case_sensitive)
            ? _ST_PRIVATE::find_cs(c_str() + start, substr)
            : _ST_PRIVATE::find_ci(c_str() + start, substr);
    return cp ? (cp - c_str()) : -1;
}

ST_ssize_t ST::string::find_last(size_t max, char ch, case_sensitivity_t cs) const noexcept
{
    if (empty())
        return -1;

    const char *endp = c_str() + (max > size() ? size() : max);

    const char *start = c_str();
    const char *found = nullptr;
    for ( ;; ) {
        const char *cp = (cs == case_sensitive)
                ? _ST_PRIVATE::find_cs(start, endp - start, ch)
                : _ST_PRIVATE::find_ci(start, endp - start, ch);
        if (!cp || cp >= endp)
            break;
        found = cp;
        start = cp + 1;
    }
    return found ? (found - c_str()) : -1;
}

ST_ssize_t ST::string::find_last(size_t max, const char *substr, case_sensitivity_t cs)
    const noexcept
{
    if (!substr || !substr[0] || empty())
        return -1;

    const char *endp = c_str() + (max > size() ? size() : max);

    const char *start = c_str();
    const char *found = nullptr;
    for ( ;; ) {
        const char *cp = (cs == case_sensitive)
                ? _ST_PRIVATE::find_cs(start, substr)
                : _ST_PRIVATE::find_ci(start, substr);
        if (!cp || cp >= endp)
            break;
        found = cp;
        start = cp + 1;
    }
    return found ? (found - c_str()) : -1;
}

bool ST::string::starts_with(const ST::string &prefix, case_sensitivity_t cs) const noexcept
{
    if (prefix.size() > size())
        return false;
    return compare_n(prefix, prefix.size(), cs) == 0;
}

bool ST::string::starts_with(const char *prefix, case_sensitivity_t cs) const noexcept
{
    size_t count = prefix ? std::char_traits<char>::length(prefix) : 0;
    if (count > size())
        return false;
    return compare_n(prefix, count, cs) == 0;
}

bool ST::string::ends_with(const ST::string &suffix, case_sensitivity_t cs) const noexcept
{
    if (suffix.size() > size())
        return false;

    size_t start = size() - suffix.size();
    return (cs == case_sensitive)
            ? _compare_cs(c_str() + start, suffix.c_str(), suffix.size()) == 0
            : _compare_ci(c_str() + start, suffix.c_str(), suffix.size()) == 0;
}

bool ST::string::ends_with(const char *suffix, case_sensitivity_t cs) const noexcept
{
    size_t count = suffix ? std::char_traits<char>::length(suffix) : 0;
    if (count > size())
        return false;

    size_t start = size() - count;
    return (cs == case_sensitive)
            ? _compare_cs(c_str() + start, suffix ? suffix : "", count) == 0
            : _compare_ci(c_str() + start, suffix ? suffix : "", count) == 0;
}

template <size_t Size>
struct _fnv_constants { };

template <>
struct _fnv_constants<4>
{
    static constexpr size_t offset_basis = 0x811c9dc5UL;
    static constexpr size_t prime = 0x01000193UL;
};

template<>
struct _fnv_constants<8>
{
    static constexpr size_t offset_basis = 0xcbf29ce484222325ULL;
    static constexpr size_t prime = 0x00000100000001b3ULL;
};

typedef _fnv_constants<sizeof(size_t)> fnv_constants;

size_t ST::hash::operator()(const string &str) const noexcept
{
    /* FNV-1a hash.  See http://isthe.com/chongo/tech/comp/fnv/ for details */
    size_t hash = fnv_constants::offset_basis;
    const char *cp = str.c_str();
    const char *ep = cp + str.size();
    while (cp < ep) {
        hash ^= static_cast<size_t>(*cp++);
        hash *= fnv_constants::prime;
    }
    return hash;
}

size_t ST::hash_i::operator()(const string &str) const noexcept
{
    /* FNV-1a hash.  See http://isthe.com/chongo/tech/comp/fnv/ for details */
    size_t hash = fnv_constants::offset_basis;
    const char *cp = str.c_str();
    const char *ep = cp + str.size();
    while (cp < ep) {
        hash ^= static_cast<size_t>(_ST_PRIVATE::cl_fast_lower(*cp++));
        hash *= fnv_constants::prime;
    }
    return hash;
}
