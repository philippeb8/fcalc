/*
 * include/qd.h
 *
 * This work was supported by the Director, Office of Science, Division
 * of Mathematical, Information, and Computational Sciences of the
 * U.S. Department of Energy under contract number DE-AC03-76SF00098.
 *
 * Copyright (c) 2000-2001
 *
 * Quad-double precision (>= 212-bit significand) floating point arithmetic
 * package, written in ANSI C++, taking full advantage of operator overloading.
 * Uses similar techniques as that of David Bailey's double-double package 
 * and that of Jonathan Shewchuk's adaptive precision floating point 
 * arithmetic package.  See
 *
 *   http://www.nersc.gov/~dhbailey/mpdist/mpdist.html
 *   http://www.cs.cmu.edu/~quake/robust.html
 *
 * for more details.
 *
 * Yozo Hida
 */
#ifndef _QD_QD_H
#define _QD_QD_H

#include <iostream>
//#include "qd_config.h"
#include "dd.h"
#include "portability.h"


class qd_real {
protected:

  /* Eliminates any zeros in the middle component(s). */
  void zero_elim();
  void zero_elim(real &e);

  void renorm();
  void renorm(real &e);

  void quick_accum(real d, real &e);
  void quick_prod_accum(real a, real b, real &e);

  /* Computes  qd * d  where d is known to be a power of 2.
     This can be done component wise.                      */
  friend qd_real mul_pwr2(const qd_real &qd, real d);

  /* Sin / Cos Tables */
  static const qd_real _pi1024;  /* Used in sin / cos routine. */
  static const qd_real sin_table[];
  static const qd_real cos_table[];

public:
  /** overriding fmtflags */
  static const std::ios_base::fmtflags showmagnitude = std::ios_base::fmtflags(0x4000);
  static const std::ios_base::fmtflags imperialsystem = std::ios_base::fmtflags(0x8000);

  real x[4];    /* The Components. */

  /* Protected Constructor. */
  qd_real(real x0, real x1, real x2, real x3);
  //qd_real(const real *xx);


  /* Useful constants. */
  static const qd_real _inf;
  static const qd_real _nan;
  static const qd_real _2pi;
  static const qd_real _pi;
  static const qd_real _3pi4;
  static const qd_real _pi2;
  static const qd_real _pi4;
  static const qd_real _pi8;
  static const qd_real _e;
  static const qd_real _log2;
  static const qd_real _log10;

  static const real _eps;      /* = 2^-212. */

  /* Constructors */
  qd_real();
  //qd_real(const char *s);
  //qd_real(const dd_real &dd);
  qd_real(const real &d);
  qd_real(const qd_real &dd);
  //qd_real(int i);

  /* Cast */
  //operator int () const;
  //operator bool () const;
  //operator dd_real () const;
  operator const real & () const;

  /* Limits */
  bool isinf() const;
  bool isnan() const;

  /* Member Access */
  real operator[](int i) const;

  static void abort();

  /* Addition */
  friend qd_real operator+(const qd_real &a, const qd_real &b);
  friend qd_real operator+(const dd_real &a, const qd_real &b);
  friend qd_real operator+(const qd_real &a, const dd_real &b);
  friend qd_real operator+(const qd_real &a, real b);
  friend qd_real operator+(real a, const qd_real &b);

  /* Self-Addition */
  qd_real &operator+=(real a);
  qd_real &operator+=(const dd_real &a);
  qd_real &operator+=(const qd_real &a);

  /* Subtraction */
  friend qd_real operator-(const qd_real &a, const qd_real &b);
  friend qd_real operator-(const dd_real &a, const qd_real &b);
  friend qd_real operator-(const qd_real &a, const dd_real &b);
  friend qd_real operator-(const qd_real &a, real b);
  friend qd_real operator-(real a, const qd_real &b);

  /* Self-Subtraction */
  qd_real &operator-=(real a);
  qd_real &operator-=(const dd_real &a);
  qd_real &operator-=(const qd_real &a);

  /* Multiplication */
  friend qd_real operator*(const qd_real &a, const qd_real &b);
  friend qd_real operator*(const dd_real &a, const qd_real &b);
  friend qd_real operator*(const qd_real &a, const dd_real &b);
  friend qd_real operator*(const qd_real &a, real b);
  friend qd_real operator*(real a, const qd_real &b);

  /* Self-Multiplication */
  qd_real &operator*=(real a);
  qd_real &operator*=(const dd_real &a);
  qd_real &operator*=(const qd_real &a);

  /* Division */
  friend qd_real operator/(const qd_real &a, const qd_real &b);
  friend qd_real operator/(const dd_real &a, const qd_real &b);
  friend qd_real operator/(const qd_real &a, const dd_real &b);
  friend qd_real operator/(const qd_real &a, real b);
  friend qd_real operator/(real a, const qd_real &b);

  /* Self-Division */
  qd_real &operator/=(real a);
  qd_real &operator/=(const dd_real &a);
  qd_real &operator/=(const qd_real &a);

  /* Square, Square Root, Power, N-th Root. */
  friend qd_real sqr(const qd_real &a);
  friend qd_real sqrt(const qd_real &a);
  friend qd_real npwr(const qd_real &a, int n);
  qd_real operator^(int n) const;
  friend qd_real nroot(const qd_real &a, int n);
  friend qd_real pow(const qd_real &a, const qd_real &n);

  /* Unary Minus */
  qd_real operator-() const;

  /* Remainder */
  friend qd_real rem(const qd_real &a, const qd_real &b);
  friend qd_real drem(const qd_real &a, const qd_real &b);
  friend qd_real divrem(const qd_real &a, const qd_real &b, qd_real &r);

  /* Equality Comparison */
  friend bool operator==(const qd_real &a, const qd_real &b);
  friend bool operator==(const qd_real &a, const dd_real &b);
  friend bool operator==(const dd_real &a, const qd_real &b);
  friend bool operator==(real a, const qd_real &b);
  friend bool operator==(const qd_real &a, real b);

  /* Less-Than Comparison */
  friend bool operator<(const qd_real &a, const qd_real &b);
  friend bool operator<(const qd_real &a, const dd_real &b);
  friend bool operator<(const dd_real &a, const qd_real &b);
  friend bool operator<(real a, const qd_real &b);
  friend bool operator<(const qd_real &a, real b);

  /* Greater-Than Comparison */
  friend bool operator>(const qd_real &a, const qd_real &b);
  friend bool operator>(const qd_real &a, const dd_real &b);
  friend bool operator>(const dd_real &a, const qd_real &b);
  friend bool operator>(real a, const qd_real &b);
  friend bool operator>(const qd_real &a, real b);

  /* Less-Than-Or-Equal-To Comparison */
  friend bool operator<=(const qd_real &a, const qd_real &b);
  friend bool operator<=(const qd_real &a, const dd_real &b);
  friend bool operator<=(const dd_real &a, const qd_real &b);
  friend bool operator<=(real a, const qd_real &b);
  friend bool operator<=(const qd_real &a, real b);

  /* Greater-Than-Or-Equal-To Comparison */
  friend bool operator>=(const qd_real &a, const qd_real &b);
  friend bool operator>=(const qd_real &a, const dd_real &b);
  friend bool operator>=(const dd_real &a, const qd_real &b);
  friend bool operator>=(real a, const qd_real &b);
  friend bool operator>=(const qd_real &a, real b);

  /* Not-Equal-To Comparison */
  friend bool operator!=(const qd_real &a, const qd_real &b);
  friend bool operator!=(const qd_real &a, const dd_real &b);
  friend bool operator!=(const dd_real &a, const qd_real &b);
  friend bool operator!=(real a, const qd_real &b);
  friend bool operator!=(const qd_real &a, real b);

  /* Other Comparisons.  These are faster than directly
     comparing to 0 or 1.                               */
  bool is_zero() const;
  bool is_one() const;
  bool is_positive() const;
  bool is_negative() const;

  /* Micellaneous algebraic operations */
  friend qd_real fabs(const qd_real &a);
  friend qd_real abs(const qd_real &a);    /* same as fabs */

  /* Computes  a * 2^n. */
  friend qd_real ldexp(const qd_real &a, int n);

  /* Rounding */
  friend qd_real nint(const qd_real &a);
  friend qd_real quick_nint(const qd_real &a);
  friend qd_real floor(const qd_real &a);
  friend qd_real ceil(const qd_real &a);
  friend qd_real aint(const qd_real &a);

  /* Trigonometric Functions */
  friend qd_real sin(const qd_real &a);
  friend qd_real cos(const qd_real &a);
  friend qd_real tan(const qd_real &a);
  friend void sincos(const qd_real &a, qd_real &s, qd_real &c);

  /* Inverse Trigonometric Functions */
  friend qd_real asin(const qd_real &a);
  friend qd_real acos(const qd_real &a);
  friend qd_real atan(const qd_real &a);
  friend qd_real atan2(const qd_real &y, const qd_real &x);

  /* Exponential / Logarithm */
  friend qd_real exp(const qd_real &a);
  friend qd_real log(const qd_real &a);
  friend qd_real log10(const qd_real &a);

  /* Hyperbolic Functions */
  friend qd_real sinh(const qd_real &a);
  friend qd_real cosh(const qd_real &a);
  friend qd_real tanh(const qd_real &a);
  friend void sincosh(const qd_real &a, qd_real &sin_qd, qd_real &cos_qd);

  /* Inverse Hyperbolic Functions */
  friend qd_real asinh(const qd_real &a);
  friend qd_real acosh(const qd_real &a);
  friend qd_real atanh(const qd_real &a);

  /* Random Number */
  static qd_real rand(void);
  friend qd_real qdrand(void);

  /* Min / Max */
  friend qd_real max(const qd_real &a, const qd_real &b);
  friend qd_real max(const qd_real &a, const qd_real &b, const qd_real &c);
  friend qd_real min(const qd_real &a, const qd_real &b);
  friend qd_real min(const qd_real &a, const qd_real &b, const qd_real &c);

  /* Polynomial Evaluator / Solver */
/*
  friend qd_real polyroot(const qd_real *c, int n,
			  const qd_real &x0, real thresh = 1.0e-62);
  friend qd_real polyeval(const qd_real *c, int n, const qd_real &x);
*/

  /* Input / Output */
  friend std::ostream &operator<<(std::ostream &s, const qd_real &a);
  friend std::istream &operator>>(std::istream &s, qd_real &a);

  void write(char *s, int d = 64, std::ios_base::fmtflags l = std::ios_base::floatfield) const;  /* Note: s must hold d+8 chars. */
  static int read(const char *s, qd_real &a);

#ifdef QD_DEBUG
  /* Debugging methods */
  void dump_components() const;
  void dump_bits() const;
  void dump() const;

  static qd_real debug_rand();
#endif

};

qd_real qdrand(void);

#ifdef QD_INLINE
#include <qd/qd_inline.h>
#endif

#endif /* _QD_QD_H */




