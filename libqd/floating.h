/**
    Copyright 2019 Fornux Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef FLOATING_H
#define FLOATING_H


///#include "qd.h"

#include <math.h>

#include <cmath>
#include <limits>
#include <iostream>


typedef long long integer;

class floating //: public qd_real
{
public:
    typedef long double type;

    static bool fast;

#if __cplusplus > 201100L
    constexpr static const type _e = 2.7182818284590452353602874713526625L;
    constexpr static const type _pi = 3.1415926535897932384626433832795029L;
#else
    static const type _e = 2.7182818284590452353602874713526625L;
    static const type _pi = 3.1415926535897932384626433832795029L;
#endif

    static const type _nan;
    static const type _inf;

    /** overriding fmtflags */
    static const std::ios_base::fmtflags showmagnitude = std::ios_base::fmtflags(1L << 19);
    static const std::ios_base::fmtflags imperialsystem = std::ios_base::fmtflags(1L << 20);

	floating();
    floating(type const & a);
    floating(floating const & a);

    operator type & ();
    operator type const & () const;

    bool isinf() const;
    bool isnan() const;
    bool is_positive() const;

    floating operator - () const;

	friend floating sin(const floating &a);
	friend floating cos(const floating &a);
	friend floating tan(const floating &a);

	friend floating asin(const floating &a);
	friend floating acos(const floating &a);
	friend floating atan(const floating &a);
	friend floating atan2(const floating &y, const floating &x);

	friend floating sinh(const floating &a);
	friend floating cosh(const floating &a);
	friend floating tanh(const floating &a);

	friend floating asinh(const floating &a);
	friend floating acosh(const floating &a);
	friend floating atanh(const floating &a);

	friend floating exp(const floating &a);
	friend floating log(const floating &a);
	friend floating log10(const floating &a);

	friend floating abs(const floating &a);
	friend floating sqrt(const floating &a);
	friend floating floor(const floating &a);
	friend floating pow(const floating &a, const floating &n);
	friend floating drem(const floating &a, const floating &b);

	friend floating operator + (const floating &a, const floating &b);
	friend floating operator - (const floating &a, const floating &b);
	friend floating operator * (const floating &a, const floating &b);
	friend floating operator / (const floating &a, const floating &b);

    friend bool operator < (const floating &a, const floating &b);

    friend std::ostream &operator<<(std::ostream &s, const floating &a);
    friend std::istream &operator>>(std::istream &s, floating &a);

private:
    type v;
};


inline floating::floating() {}
inline floating::floating(floating::type const & a) : v(a) {}
inline floating::floating(floating const & a) : v(a.v) {}

inline floating::operator type & () { return v; }
inline floating::operator type const & () const { return v; }

inline bool floating::isinf() const { return std::isinf(v); }
inline bool floating::isnan() const { return std::isnan(v); }
inline bool floating::is_positive() const { return v >= 0.0L; }

inline floating floating::operator - () const { return - v; }

inline floating sin(const floating &a) { return std::sin(a.v); }
inline floating cos(const floating &a) { return std::cos(a.v); }
inline floating tan(const floating &a) { return std::tan(a.v); }

inline floating asin(const floating &a) { return std::asin(a.v); }
inline floating acos(const floating &a) { return std::acos(a.v); }
inline floating atan(const floating &a) { return std::atan(a.v); }
inline floating atan2(const floating &a, const floating &b) { return std::atan2(a.v, b.v); }

inline floating sinh(const floating &a) { return std::sinh(a.v); }
inline floating cosh(const floating &a) { return std::cosh(a.v); }
inline floating tanh(const floating &a) { return std::tanh(a.v); }

#if __cplusplus > 201100L
inline floating asinh(const floating &a) { return std::log(a.v + std::sqrt(a.v * a.v + 1.0L)); }
inline floating acosh(const floating &a) { return std::log(a.v + std::sqrt(a.v * a.v - 1.0L)); }
inline floating atanh(const floating &a) { return std::log((1.0L + a.v) / (1.0L - a.v)); }
#else
inline floating asinh(const floating &a) { return ::asinhl(a.v); }
inline floating acosh(const floating &a) { return ::acoshl(a.v); }
inline floating atanh(const floating &a) { return ::atanhl(a.v); }
#endif

inline floating exp(const floating &a) { return std::exp(a.v); }
inline floating log(const floating &a) { return std::log(a.v); }
inline floating log10(const floating &a) { return std::log10(a.v); }

inline floating abs(const floating &a) { return std::fabs(a.v); }
inline floating sqrt(const floating &a) { return std::sqrt(a.v); }
#if __cplusplus > 201100L
inline floating floor(const floating &a) { return (floating::type)(integer)(a.v); }
#else
inline floating floor(const floating &a) { return std::floor(a.v); }
#endif
inline floating pow(const floating &a, const floating &b) { return std::pow(a.v, b.v); }
inline floating drem(const floating &a, const floating &b) { return std::fmod(a.v, b.v); }

inline floating operator + (const floating &a, const floating &b) { return a.v + b.v; }
inline floating operator - (const floating &a, const floating &b) { return a.v - b.v; }
inline floating operator * (const floating &a, const floating &b) { return a.v * b.v; }
inline floating operator / (const floating &a, const floating &b) { return a.v / b.v; }

inline bool operator < (const floating &a, const floating &b) { return a.v < b.v; }

inline std::ostream &operator<<(std::ostream &s, const floating &a) { return s << a.v; }
inline std::istream &operator>>(std::istream &s, floating &a) { return s >> a.v; }

#endif
