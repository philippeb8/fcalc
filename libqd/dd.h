/*
 * include/dd.h
 *
 * This work was supported by the Director, Office of Science, Division
 * of Mathematical, Information, and Computational Sciences of the
 * U.S. Department of Energy under contract number DE-AC03-76SF00098.
 *
 * Copyright (c) 2000-2001
 *
 * Double-double precision (>= 106-bit significand) floating point
 * arithmetic package based on David Bailey's Fortran-90 double-double
 * package, with some changes. See
 *
 *   http://www.nersc.gov/~dhbailey/mpdist/mpdist.html
 *
 * for the original Fortran-90 version.
 *
 * Overall structure is similar to that of Keith Brigg's C++ double-double
 * package.  See
 *
 *   http://www-epidem.plansci.cam.ac.uk/~kbriggs/doubledouble.html
 *
 * for more details.  In particular, the fix for x86 computers is borrowed
 * from his code.
 *
 * Yozo Hida
 */

#ifndef _QD_DD_H
#define _QD_DD_H

#include <iostream>
//#include "qd_config.h"
#include "portability.h"


class dd_real {
protected:

  /* Computes  dd * d  where d is known to be a power of 2. */
  friend dd_real mul_pwr2(const dd_real &dd, real d);

  /* Constant / Tables used in sin / cos. */
  static const dd_real _pi16;
  static const dd_real sin_table[];
  static const dd_real cos_table[];

public:
  /** overriding fmtflags */
  static const std::ios_base::fmtflags showmagnitude = std::ios_base::fmtflags(0x4000);

  real hi, lo;

  /* Constructor(s) */
  dd_real(real hi, real lo) : hi(hi), lo(lo) {}
  dd_real() : hi(0.0), lo(0.0) {}
  dd_real(real h) : hi(h), lo(0.0) {}
  //dd_real(int h) : hi((real) h), lo(0.0) {}
  //dd_real (const char *s);
  //dd_real (const real *d) : hi(d[0]), lo(d[1]) {}

  /* Cast */
  //operator int () const;
  //operator bool () const;
  operator const real & () const;

  /* Limits */
  bool isinf() const;
  bool isnan() const;

  /* Function to be called is a fatal error occurs. */
  static void abort();

  /* Accessors */
  real _hi() const { return hi; }
  real _lo() const { return lo; }

  /* Some constants */
  static const dd_real _inf;
  static const dd_real _nan;
  static const dd_real _2pi;
  static const dd_real _pi;
  static const dd_real _3pi4;
  static const dd_real _pi2;
  static const dd_real _pi4;
  static const dd_real _pi8;
  static const dd_real _e;
  static const dd_real _log2;
  static const dd_real _log10;

  static const real _eps;   /* = 2^-106 */

  /* Addition */
  friend dd_real operator+(const dd_real &a, real b);
  friend dd_real operator+(real a, const dd_real &b);
  friend dd_real operator+(const dd_real &a, const dd_real &b);
  static dd_real add(real a, real b);

  /* Self-Addition */
  dd_real &operator+=(real a);
  dd_real &operator+=(const dd_real &a);

  /* Subtraction */
  friend dd_real operator-(const dd_real &a, real b);
  friend dd_real operator-(real a, const dd_real &b);
  friend dd_real operator-(const dd_real &a, const dd_real &b);
  static dd_real sub(real a, real b);

  /* Self-Subtraction */
  dd_real &operator-=(real a);
  dd_real &operator-=(const dd_real &a);

  /* Negation */
  dd_real operator-() const;

  /* Multiplication */
  friend dd_real operator*(const dd_real &a, real b);
  friend dd_real operator*(real a, const dd_real &b);
  friend dd_real operator*(const dd_real &a, const dd_real &b);
  static dd_real mul(real a, real b);

  /* Self-Multiplication */
  dd_real &operator*=(real a);
  dd_real &operator*=(const dd_real &a);

  /* Division */
  friend dd_real operator/(const dd_real &a, real b);
  friend dd_real operator/(real a, const dd_real &b);
  friend dd_real operator/(const dd_real &a, const dd_real &b);
  static dd_real div(real a, real b);
  friend dd_real inv(const dd_real &a);
  
  /* Self-Division */
  dd_real &operator/=(real a);
  dd_real &operator/=(const dd_real &a);

  /* Remainder */
  friend dd_real rem(const dd_real &a, const dd_real &b);
  friend dd_real drem(const dd_real &a, const dd_real &b);
  friend dd_real divrem(const dd_real &a, const dd_real &b, dd_real &r);

  /* Assignment */
  dd_real &operator=(real a);

  /* Square, Square Root, Power, N-th Root. */
  dd_real operator^(int n);
  static dd_real sqr(real d);
  friend dd_real sqr(const dd_real &a);
  static dd_real sqrt(real a);
  friend dd_real sqrt(const dd_real &a);
  friend dd_real npwr(const dd_real &a, int n);
  friend dd_real nroot(const dd_real &a, int n);
  friend dd_real pow(const dd_real &a, const dd_real &n);
  
  /* Comparisons */
  friend bool operator==(const dd_real &a, real b);
  friend bool operator==(real a, const dd_real &b);
  friend bool operator==(const dd_real &a, const dd_real &b);

  friend bool operator<=(const dd_real &a, real b);
  friend bool operator<=(real a, const dd_real &b);
  friend bool operator<=(const dd_real &a, const dd_real &b);

  friend bool operator>=(const dd_real &a, real b);
  friend bool operator>=(real a, const dd_real &b);
  friend bool operator>=(const dd_real &a, const dd_real &b);

  friend bool operator<(const dd_real &a, real b);
  friend bool operator<(real a, const dd_real &b);
  friend bool operator<(const dd_real &a, const dd_real &b);

  friend bool operator>(const dd_real &a, real b);
  friend bool operator>(real a, const dd_real &b);
  friend bool operator>(const dd_real &a, const dd_real &b);

  friend bool operator!=(const dd_real &a, real b);
  friend bool operator!=(real a, const dd_real &b);
  friend bool operator!=(const dd_real &a, const dd_real &b);

  /* Other Comparisons.  These are faster than 
     directly comparing to 0 or 1.             */
  bool is_zero() const;
  bool is_one() const;
  bool is_positive() const;
  bool is_negative() const;

  /* Rounding */
  friend dd_real nint(const dd_real &a);
  friend dd_real floor(const dd_real &a);
  friend dd_real ceil(const dd_real &a);
  friend dd_real aint(const dd_real &a);

  /* Random Number */
  friend dd_real ddrand(void);
  static dd_real rand(void);

  /* Polynomial Solver */
/*
  friend dd_real polyroot(const dd_real *c, int n,
			  const dd_real &x0, real thresh = 1.0e-30);
  friend dd_real polyeval(const dd_real *c, int n, const dd_real &x);
*/

  /* Exponential and Logarithms */
  friend dd_real exp(const dd_real &a);
  friend dd_real ldexp(const dd_real &a, int exp);
  friend dd_real log(const dd_real &a);
  friend dd_real log10(const dd_real &a);

  /* Trigonometric Functions. */
  friend dd_real sin(const dd_real &a);
  friend dd_real cos(const dd_real &a);
  friend dd_real tan(const dd_real &a);
  friend void sincos(const dd_real &a, dd_real &sin_a, dd_real &cos_a);

  /* Inverse Trigonometric Functions. */
  friend dd_real asin(const dd_real &a);
  friend dd_real acos(const dd_real &a);
  friend dd_real atan(const dd_real &a);
  friend dd_real atan2(const dd_real &y, const dd_real &x);

  /* Hyperbolic Functions. */
  friend dd_real sinh(const dd_real &a);
  friend dd_real cosh(const dd_real &a);
  friend dd_real tanh(const dd_real &a);
  friend void sincosh(const dd_real &a, 
		      dd_real &sinh_a, dd_real &cosh_a);

  /* Inverse Hyperbolic Functions */
  friend dd_real asinh(const dd_real &a);
  friend dd_real acosh(const dd_real &a);
  friend dd_real atanh(const dd_real &a);

  /* Other operations */
  friend dd_real fabs(const dd_real &a);
  friend dd_real abs(const dd_real &a);   /* same as fabs */

  /* Input/Output */
  void write(char *s, int d = 32, std::ios_base::fmtflags l = std::ios_base::floatfield) const; /* s must hold d+8 chars */
  int read(const char *s, dd_real &a);

  friend std::ostream& operator<<(std::ostream &s, const dd_real &a);
  friend std::istream& operator>>(std::istream &s, dd_real &a);

#ifdef QD_DEBUG
  /* Debugging Methods */
  void dump() const;
  void dump_bits() const;
  void dump_components() const;

  static dd_real debug_rand();
#endif

  friend class qd_real;
};

dd_real ddrand(void);

#ifdef QD_INLINE
#include <qd/dd_inline.h>
#endif

#endif /* _QD_DD_H */

