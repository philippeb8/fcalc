/**
    Copyright 2019 Fornux Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _QD_INLINE_H
#define _QD_INLINE_H

#define _QD_SPLITTER 134217729.0               /* = 2^27 + 1 */

namespace qd {

/*********** Basic Functions ************/
/* Computes fl(a+b) and err(a+b).  Assumes |a| >= |b|. */
inline real quick_two_sum(real a, real b, real &err) {
  real s = a + b;
  err = b - (s - a);
  return s;
}

/* Computes fl(a-b) and err(a-b).  Assumes |a| >= |b| */
inline real quick_two_diff(real a, real b, real &err) {
  real s = a - b;
  err = (a - s) - b;
  return s;
}

/* Computes fl(a+b) and err(a+b).  */
inline real two_sum(real a, real b, real &err) {
  real s = a + b;
  real bb = s - a;
  err = (a - (s - bb)) + (b - bb);
  return s;
}

/* Computes fl(a-b) and err(a-b).  */
inline real two_diff(real a, real b, real &err) {
  real s = a - b;
  real bb = s - a;
  err = (a - (s - bb)) - (b + bb);
  return s;
}

#ifndef QD_HAS_FMA
/* Computes high word and lo word of a */
inline void split(real a, real &hi, real &lo) {
  real temp;
  temp = _QD_SPLITTER * a;
  hi = temp - (temp - a);
  lo = a - hi;
}
#endif

/* Computes fl(a*b) and err(a*b). */
inline real two_prod(real a, real b, real &err) {
#ifdef QD_HAS_FMA
  real p = -a * b;
  err = a * b + p;
  return -p;
#else
  real a_hi, a_lo, b_hi, b_lo;
  real p = a * b;
  split(a, a_hi, a_lo);
  split(b, b_hi, b_lo);
  err = ((a_hi * b_hi - p) + a_hi * b_lo + a_lo * b_hi) + a_lo * b_lo;
  return p;
#endif
}

/* Computes fl(a*a) and err(a*a).  Faster than the above method. */
inline real two_sqr(real a, real &err) {
#ifdef QD_HAS_FMA
  real p = -a * a;
  err = a * a + p;
  return -p;
#else
  real hi, lo;
  real q = a * a;
  split(a, hi, lo);
  err = ((hi * hi - q) + 2.0 * hi * lo) + lo * lo;
  return q;
#endif
}

/* Computes the nearest integer to d. */
inline real nint(real d) {
  if (d == floorl(d))
    return d;
  return floorl(d + 0.5);
}

/* Computes the truncated integer. */
inline real aint(real d) {
  return (d >= 0.0) ? floorl(d) : ceill(d);
}

/* These are provided to give consistent 
   interface for real with real-real and quad-real. */
inline void sincosh(real t, real &sinh_t, real &cosh_t) {
  sinh_t = sinhl(t);
  cosh_t = coshl(t);
}

inline real sqr(real t) {
  return t * t;
}

}

#endif /* _QD_INLINE_H */
