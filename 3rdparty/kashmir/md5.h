// md5.h -- hash algorithm described in IETF RFC 1321

// Copyright (C) 2010 Kenneth Laskoski

/** @file md5.h
    @brief hash algorithm described in IETF RFC 1321
    @author Copyright (C) 2010 Kenneth Laskoski

    Use, modification, and distribution are subject to the
    Boost Software License, Version 1.0. See accompanying file
    LICENSE_1_0.txt or <http://www.boost.org/LICENSE_1_0.txt>.
*/
/*
    Relevant copyright information is provided below and may not be removed from this file. 
    Derived from the RSA Data Security, Inc. MD5 Message-Digest Algorithm.

    MD5C.C - RSA Data Security, Inc., MD5 message-digest algorithm

    Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All rights 
    reserved.

    License to copy and use this software is granted provided that it is 
    identified as the "RSA Data Security, Inc. MD5 Message-Digest Algorithm" in 
    all material mentioning or referencing this software or this function. 

    License is also granted to make and use derivative works provided that such 
    works are identified as "derived from the RSA Data Security, Inc. MD5 
    Message-Digest Algorithm" in all material mentioning or referencing the 
    derived work. 

    RSA Data Security, Inc. makes no representations concerning either the 
    merchantability of this software or the suitability of this software for 
    any particular purpose. It is provided "as is" without express or implied 
    warranty of any kind. 

    These notices must be retained in any copies of any part of this 
    documentation and/or software. 
*/

#ifndef KL_MD5_H
#define KL_MD5_H 

#include "array.h"

#include <istream>
#include <ostream>
#include <sstream>

#include <cstddef>

#include <stdexcept>
#include <algorithm>

#include "iostate.h"

#include <cstring>

namespace kashmir {
namespace md5 {

/** @class md5_t
    @brief This class implements the hash algorithm described in
    
    - IETF RFC 1321 - available at http://tools.ietf.org/html/rfc1321

    This document documents the code below.
*/

class md5_t
{
    // an MD5 is a string of 16 octets (128 bits)
    // we use an unpacked representation, value_type may be larger than 8 bits,
    // in which case every input operation must assert data[i] < 256 for i < 16
    // note even char may be more than 8 bits in some particular platform
    typedef unsigned char value_type;
    typedef std::size_t size_type;

    enum { size = 16, string_size = 32 };

    typedef array<value_type, size> data_type;
    data_type data;

public:
    // uninitialized memory
    md5_t();
    ~md5_t() {}

    // copy and assignment
    md5_t(const md5_t& rhs) : data(rhs.data) {}

    md5_t& operator=(const md5_t& rhs)
    {
        data = rhs.data;
        return *this;
    }

    // initialization from C string
    explicit md5_t(const char* string)
    {
        std::stringstream stream(string);
        get(stream);
    }

    // test for nil value
    bool is_nil() const
    {
        for (size_type i = 0; i < size; ++i)
            if (data[i])
                return false;
        return true;
    }

    // safe bool idiom
    typedef data_type md5_t::*bool_type; 

    operator bool_type() const
    {
        return is_nil() ? 0 : &md5_t::data;
    }

    friend bool operator==(const md5_t& lhs, const md5_t& rhs);
    friend bool operator<(const md5_t& lhs, const md5_t& rhs);

    // stream operators
    template<class char_t, class char_traits>
    std::basic_ostream<char_t, char_traits>& put(std::basic_ostream<char_t, char_traits>& os) const;

    template<class char_t, class char_traits>
    std::basic_istream<char_t, char_traits>& get(std::basic_istream<char_t, char_traits>& is);

private:
    unsigned state[4];
    std::size_t count[2];
    unsigned char buffer[64];

    void update(void *input, std::size_t input_length);
    void transform(void *input);

    void encode(unsigned char *output, unsigned *input, std::size_t len)
    {
        unsigned i, j;
        for (i = 0, j = 0; j < len; i++, j += 4)
        {
            output[j] = input[i] & 0xff;
            output[j+1] = (input[i] >> 8) & 0xff;
            output[j+2] = (input[i] >> 16) & 0xff;
            output[j+3] = (input[i] >> 24) & 0xff;
        }
    }

    void decode(unsigned *output, unsigned char *input, std::size_t len)
    {
         unsigned i, j;
         for (i = 0, j = 0; j < len; i++, j += 4)
            output[i] = input[j] | input[j+1] << 8 | input[j+2] << 16 | input[j+3] << 24;
    }
};

inline md5_t::md5_t()
{
    count[0] = 0;
    count[1] = 0;

    state[0] = 0x67452301;
    state[1] = 0xefcdab89;
    state[2] = 0x98badcfe;
    state[3] = 0x10325476;
}

// comparison operators define a total order
inline bool operator==(const md5_t& lhs, const md5_t& rhs)
{
    return lhs.data == rhs.data;
}

inline bool operator<(const md5_t& lhs, const md5_t& rhs)
{
    return lhs.data < rhs.data;
}

inline bool operator>(const md5_t& lhs, const md5_t& rhs) { return (rhs < lhs); }
inline bool operator<=(const md5_t& lhs, const md5_t& rhs) { return !(rhs < lhs); }
inline bool operator>=(const md5_t& lhs, const md5_t& rhs) { return !(lhs < rhs); }
inline bool operator!=(const md5_t& lhs, const md5_t& rhs) { return !(lhs == rhs); }

template<class char_t, class char_traits>
std::basic_ostream<char_t, char_traits>& md5_t::put(std::basic_ostream<char_t, char_traits>& os) const
{
    if (!os.good())
        return os;

    const typename std::basic_ostream<char_t, char_traits>::sentry ok(os);
    if (ok)
    {
        ios_flags_saver flags(os);
        basic_ios_fill_saver<char_t, char_traits> fill(os);

        const std::streamsize width = os.width(0);
        const std::streamsize mysize = string_size;

        // right padding
        if (flags.value() & (std::ios_base::right | std::ios_base::internal))
            for (std::streamsize i = width; i > mysize; --i)
                os << fill.value();

        os << std::hex;
        os.fill(os.widen('0'));

        for (size_t i = 0; i < 16; ++i)
        {
            os.width(2);
            os << static_cast<unsigned>(data[i]);
        }

        // left padding
        if (flags.value() & std::ios_base::left)
            for (std::streamsize i = width; i > mysize; --i)
                os << fill.value();
    }

    return os;
}

template<class char_t, class char_traits>
std::basic_istream<char_t, char_traits>& md5_t::get(std::basic_istream<char_t, char_traits>& is)
{
    if (!is.good())
        return is;

    const typename std::basic_istream<char_t, char_traits>::sentry ok(is);
    if (ok)
    {
        char_t hexdigits[16];
        char_t* const npos = hexdigits+16;

        typedef std::ctype<char_t> facet_t;
        const facet_t& facet = std::use_facet<facet_t>(is.getloc());

        const char* tmp = "0123456789abcdef";
        facet.widen(tmp, tmp+16, hexdigits);

        char_t c;
        char_t* f;
        for (size_t i = 0; i < size; ++i)
        {
            is >> c;
            c = facet.tolower(c);

            f = std::find(hexdigits, npos, c);
            if (f == npos)
            {
                is.setstate(std::ios_base::failbit);
                break;
            }

            data[i] = static_cast<value_type>(std::distance(hexdigits, f));

            is >> c;
            c = facet.tolower(c);

            f = std::find(hexdigits, npos, c);
            if (f == npos)
            {
                is.setstate(std::ios_base::failbit);
                break;
            }

            data[i] <<= 4;
            data[i] |= static_cast<value_type>(std::distance(hexdigits, f));
        }

        if (!is)
            throw std::runtime_error("failed to extract valid md5 from stream.");
    }

    return is;
}

template<class char_t, class char_traits>
inline std::basic_ostream<char_t, char_traits>& operator<<(std::basic_ostream<char_t, char_traits>& os, const md5_t& md5)
{
    return md5.put(os);
}

template<class char_t, class char_traits>
inline std::basic_istream<char_t, char_traits>& operator>>(std::basic_istream<char_t, char_traits>& is, md5_t& md5)
{
    return md5.get(is);
}

// Although we could use C++ style constants, defines are actually better,
// since they let us easily evade scope clashes.

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

// F, G, H and I are basic MD5 functions.
template<class T> T F(T x, T y, T z) { return (x & y) | (~x & z); } 
template<class T> T G(T x, T y, T z) { return (x & z) | (y & ~z); }
template<class T> T H(T x, T y, T z) { return x ^ y ^ z; }
template<class T> T I(T x, T y, T z) { return y ^ (x | ~z); }

// left rotation
template<class T> T rotate_left(T x, T y) { return x << y | x >> (32 - y); }

// FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
// Rotation is separate from addition to prevent recomputation.
template<class T>
void FF(T& a, T b, T c, T d, T x, T s, T ac){
    a += F(b, c, d) + x + ac;
    a = rotate_left(a, s) +b;
}

template<class T>
void GG(T& a, T b, T c, T d, T x, T s, T ac){
    a += G(b, c, d) + x + ac;
    a = rotate_left(a, s) +b;
}

template<class T>
void HH(T& a, T b, T c, T d, T x, T s, T ac){
    a += H(b, c, d) + x + ac;
    a = rotate_left(a, s) +b;
}

template<class T>
void II(T& a, T b, T c, T d, T x, T s, T ac){
    a += I(b, c, d) + x + ac;
    a = rotate_left(a, s) +b;
}

inline void md5_t::transform(void *input)
{
    unsigned a = state[0], b = state[1], c = state[2], d = state[3], x[16];

    decode (x, block, 64);

    /* Round 1 */
    FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
    FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
    FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
    FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
    FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
    FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
    FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
    FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
    FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
    FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
    FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
    FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
    FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
    FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
    FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
    FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

    /* Round 2 */
    GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
    GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
    GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
    GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
    GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
    GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
    GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
    GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
    GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
    GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
    GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
    GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
    GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
    GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
    GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
    GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

    /* Round 3 */
    HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
    HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
    HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
    HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
    HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
    HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
    HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
    HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
    HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
    HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
    HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
    HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
    HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
    HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
    HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
    HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

    /* Round 4 */
    II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
    II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
    II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
    II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
    II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
    II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
    II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
    II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
    II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
    II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
    II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
    II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
    II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
    II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
    II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
    II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;

    /* Zeroize sensitive information. */
    std::memset (x, 0, sizeof(x));
}

// Constants for MD5Transform routine.
inline void md5_t::update(void *input, std::size_t input_length)
{
    // Compute number of bytes mod 64
    std::size_t index = (count[0] >> 3) & 0x3F;

    // Update number of bits
    std::size_t length = input_length << 3;
    count[0] += length;
    count[1] += count[0] < length;      // carry
    count[1] += input_length >> 29;

    std::size_t space = 64 - index;
    
    // Transform as many times as possible.
    std::size_t i = 0;
    if (input_length >= space)
    {
        // fill the rest of the buffer and transform
        std::memcpy(buffer+index, input, space);
        transform(buffer);

        // now, transform each 64-byte piece of the input, bypassing the buffer
        for (i = space; i+63 < input_length; i += 64)
            transform(input+i);

        index = 0;  // so we can buffer remaining
    }

    // and here we do the buffering:
    memcpy(buffer+index, input+i, input_length-i);
}

md5_t hash(const char *input, std::size_t input_length)
{
    md5_t val;
    return val;
}

md5_t hash(std::istream& s)
{
    md5_t val;
/*
    unsigned char buffer[1024];
    int len;

    while (stream.good()){
        stream.read(buffer, 1024); // note that return value of read is unusable.
        len=stream.gcount();
        update(buffer, len);
    }

*/
    return val;
}

} // namespace kashmir::md5

using md5::md5_t;

} // namespace kashmir

#endif
