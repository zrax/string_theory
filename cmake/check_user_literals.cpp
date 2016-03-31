/*  This file is part of string_theory.

    string_theory is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    string_theory is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with string_theory.  If not, see <http://www.gnu.org/licenses/>.  */

#include <cstddef>

size_t operator"" _size(const char *, size_t size) { return size; }
size_t operator"" _size(const wchar_t *, size_t size) { return size; }
size_t operator"" _size(const char16_t *, size_t size) { return size; }
size_t operator"" _size(const char32_t *, size_t size) { return size; }

int main(int, char **)
{
    size_t x = "foobar"_size;
    return 0;
}
