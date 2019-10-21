/**
    Copyright 2019 Fornux Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "floating.h"

#include "math.h"


#if defined(_WIN32) && !defined(__MINGW32__)
inline real asinhl(real const & a)
{
	return (a < 0 ? -1 : 1) * logl(fabsl(a) + sqrtl(1 + a * a));
}

inline real acoshl(real const & a)
{
	return logl(a + sqrtl(a * a - 1));
}

inline real atanhl(real const & a)
{
	return logl(sqrtl((1 + a) / (1 - a)));
}
#endif


bool floating::fast = false;

#if __cplusplus > 201100L
constexpr const floating::type floating::_e;
constexpr const floating::type floating::_pi;
#else
const floating::type floating::_e;
const floating::type floating::_pi;
#endif

const floating::type floating::_nan = std::numeric_limits<type>::quiet_NaN();
const floating::type floating::_inf = std::numeric_limits<type>::infinity();


#if 0
floating floating::operator - () const { if (floating::fast) return - x[0]; else return - (qd_real const &)(* this); }

floating sin(const floating &a) { if (floating::fast) return sinl(a.x[0]); else return sin((qd_real const &)(a)); }
floating cos(const floating &a) { if (floating::fast) return cosl(a.x[0]); else return cos((qd_real const &)(a)); }
floating tan(const floating &a) { if (floating::fast) return tanl(a.x[0]); else return tan((qd_real const &)(a)); }

floating asin(const floating &a) { if (floating::fast) return asinl(a.x[0]); else return asin((qd_real const &)(a)); }
floating acos(const floating &a) { if (floating::fast) return acosl(a.x[0]); else return acos((qd_real const &)(a)); }
floating atan(const floating &a) { if (floating::fast) return atanl(a.x[0]); else return atan((qd_real const &)(a)); }
floating atan2(const floating &a, const floating &b) { if (floating::fast) return atan2l(a.x[0], b.x[0]); else return atan2((qd_real const &)(a), (qd_real const &)(b)); }

floating sinh(const floating &a) { if (floating::fast) return sinhl(a.x[0]); else return sinhl((qd_real const &)(a)); }
floating cosh(const floating &a) { if (floating::fast) return coshl(a.x[0]); else return coshl((qd_real const &)(a)); }
floating tanh(const floating &a) { if (floating::fast) return tanhl(a.x[0]); else return tanhl((qd_real const &)(a)); }

floating asinh(const floating &a) { if (floating::fast) return asinhl(a.x[0]); else return asinh((qd_real const &)(a)); }
floating acosh(const floating &a) { if (floating::fast) return acoshl(a.x[0]); else return acosh((qd_real const &)(a)); }
floating atanh(const floating &a) { if (floating::fast) return atanhl(a.x[0]); else return atanh((qd_real const &)(a)); }

floating exp(const floating &a) { if (floating::fast) return expl(a.x[0]); else return exp((qd_real const &)(a)); }
floating log(const floating &a) { if (floating::fast) return logl(a.x[0]); else return log((qd_real const &)(a)); }
floating log10(const floating &a) { if (floating::fast) return log10l(a.x[0]); else return log10((qd_real const &)(a)); }

floating abs(const floating &a) { if (floating::fast) return fabsl(a.x[0]); else return abs((qd_real const &)(a)); }
floating sqrt(const floating &a) { if (floating::fast) return sqrtl(a.x[0]); else return sqrt((qd_real const &)(a)); }
floating floor(const floating &a) { if (floating::fast) return floorl(a.x[0]); else return floor((qd_real const &)(a)); }
floating pow(const floating &a, const floating &b) { if (floating::fast) return powl(a.x[0], b.x[0]); else return pow((qd_real const &)(a), (qd_real const &)(b)); }
floating drem(const floating &a, const floating &b) { if (floating::fast) return fmodl(a.x[0], b.x[0]); else return drem((qd_real const &)(a), (qd_real const &)(b)); }

floating operator + (const floating &a, const floating &b) { if (floating::fast) return a.x[0] + b.x[0]; else return (qd_real const &)(a) + (qd_real const &)(b); }
floating operator - (const floating &a, const floating &b) { if (floating::fast) return a.x[0] - b.x[0]; else return (qd_real const &)(a) - (qd_real const &)(b); }
floating operator * (const floating &a, const floating &b) { if (floating::fast) return a.x[0] * b.x[0]; else return (qd_real const &)(a) * (qd_real const &)(b); }
floating operator / (const floating &a, const floating &b) { if (floating::fast) return a.x[0] / b.x[0]; else return (qd_real const &)(a) / (qd_real const &)(b); }
#endif
