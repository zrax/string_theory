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

#ifndef _ST_STRING_DARWIN_H
#define _ST_STRING_DARWIN_H

#include "st_string.h"

#ifdef ST_ENABLE_COREFOUNDATION
ST::string::string(CFStringRef cfstr, utf_validation_t validation)
{
    set(cfstr, validation);
}

ST::string::string(NSString* nsstr, utf_validation_t validation)
{
    set(nsstr, validation);
}


void ST::string::set(const CFStringRef str, utf_validation_t validation)
{
    CFRange range = CFRangeMake(0, CFStringGetLength(str));
    CFIndex strBufSz = 0;
    CFStringGetBytes(str, range, kCFStringEncodingUTF8, 0, false, nullptr, 0, &strBufSz);
    ST::char_buffer buffer;
    buffer.allocate(strBufSz);
    CFStringGetBytes(str, range, kCFStringEncodingUTF8, 0, false, (UInt8*)buffer.data(), strBufSz, nullptr);

    set(buffer, validation);
}

void ST::string::set(const NSString* nsstr, utf_validation_t validation)
{
    set(ST_BRIDGED_CAST(CFStringRef)nsstr, validation);
}

ST::string &ST::string::operator=(const CFStringRef cfstr)
{
    set(cfstr);
    return *this;
}

ST::string &ST::string::operator=(const NSString* nsstr)
{
    set(nsstr);
    return *this;
}


ST::string ST::string::from_CFString(const CFStringRef cfstr, utf_validation_t validation)
{
    ST::string str;
    str.set(cfstr, validation);
    return str;
}

ST::string ST::string::from_NSString(const NSString* nsstr, utf_validation_t validation)
{
    ST::string str;
    str.set(nsstr, validation);
    return str;
}


CFStringRef ST::string::to_CFString() const
{
    return CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8*)data(), size(), kCFStringEncodingUTF8, false);
}

NSString* ST::string::to_NSString() const
{
    return ST_BRIDGED_CAST(NSString*)to_CFString();
}

#endif // ST_ENABLE_COREFOUNDATION
#endif // _ST_STRING_DARWIN_H
