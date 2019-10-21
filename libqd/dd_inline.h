/*
 * include/dd_inline.h
 *
 * This work was supported by the Director, Office of Science, Division
 * of Mathematical, Information, and Computational Sciences of the
 * U.S. Department of Energy under contract number DE-AC03-76SF00098.
 *
 * Copyright (c) 2000-2001
 *
 * Contains small functions (suitable for inlining) in the double-double
 * arithmetic package.
 */
#ifndef _QD_DD_INLINE_H
#define _QD_DD_INLINE_H

#include <cmath>
#include "inline.h"

#ifndef QD_INLINE
#define inline
#endif



/********** Casts **********/
/*
inline dd_real::operator int () const {
  return int(hi);
}

inline dd_real::operator bool () const {
  return bool(hi);
}
*/
inline dd_real::operator const real & () const {
  return hi;
}

/********** Limits **********/
inline bool dd_real::isinf() const
{
  return hi != real(0.0) && hi == 2.0 * hi;
}

inline bool dd_real::isnan() const
{
  return hi != hi;
}

/*********** Additions ************/
/* -real = real + real */
inline dd_real dd_real::add(real a, real b) {
  real s, e;
  s = qd::two_sum(a, b, e);
  return dd_real(s, e);
}

/* real-real + real */
inline dd_real operator+(const dd_real &a, real b) {
  real s1, s2;
  s1 = qd::two_sum(a.hi, b, s2);
  s2 += a.lo;
  s1 = qd::quick_two_sum(s1, s2, s2);
  return dd_real(s1, s2);
}

/* real-real + real-real */
inline dd_real operator+(const dd_real &a, const dd_real &b) {
#ifndef QD_IEEE_ADD

  /* This is the less accurate version ... obeys Cray-style
     error bound. */
  real s, e;

  s = qd::two_sum(a.hi, b.hi, e);
  e += a.lo;
  e += b.lo;
  s = qd::quick_two_sum(s, e, e);
  return dd_real(s, e);
#else

  /* This one satisfies IEEE style error bound,
     due to K. Briggs and W. Kahan.                   */
  real s1, s2, t1, t2;

  s1 = qd::two_sum(a.hi, b.hi, s2);
  t1 = qd::two_sum(a.lo, b.lo, t2);
  s2 += t1;
  s1 = qd::quick_two_sum(s1, s2, s2);
  s2 += t2;
  s1 = qd::quick_two_sum(s1, s2, s2);
  return dd_real(s1, s2);
#endif
}

/* real + real-real */
inline dd_real operator+(real a, const dd_real &b) {
  return (b + a);
}


/*********** Self-Additions ************/
/* real-real += real */
inline dd_real &dd_real::operator+=(real a) {
  real s1, s2;
  s1 = qd::two_sum(hi, a, s2);
  s2 += lo;
  hi = qd::quick_two_sum(s1, s2, lo);
  return *this;
}

/* real-real += real-real */
inline dd_real &dd_real::operator+=(const dd_real &a) {
#ifndef QD_IEEE_ADD
  real s, e;
  s = qd::two_sum(hi, a.hi, e);
  e += lo;
  e += a.lo;
  hi = qd::quick_two_sum(s, e, lo);
  return *this;
#else
  real s1, s2, t1, t2;
  s1 = qd::two_sum(hi, a.hi, s2);
  t1 = qd::two_sum(lo, a.lo, t2);
  s2 += t1;
  s1 = qd::quick_two_sum(s1, s2, s2);
  s2 += t2;
  hi = qd::quick_two_sum(s1, s2, lo);
  return *this;
#endif
}

/*********** Subtractions ************/
/* real-real = real - real */
inline dd_real dd_real::sub(real a, real b) {
  real s, e;
  s = qd::two_diff(a, b, e);
  return dd_real(s, e);
}

/* real-real - real */
inline dd_real operator-(const dd_real &a, real b) {
  real s1, s2;
  s1 = qd::two_diff(a.hi, b, s2);
  s2 += a.lo;
  s1 = qd::quick_two_sum(s1, s2, s2);
  return dd_real(s1, s2);
}

/* real-real - real-real */
inline dd_real operator-(const dd_real &a, const dd_real &b) {
#ifndef QD_IEEE_ADD
  real s, e;
  s = qd::two_diff(a.hi, b.hi, e);
  e += a.lo;
  e -= b.lo;
  s = qd::quick_two_sum(s, e, e);
  return dd_real(s, e);
#else
  real s1, s2, t1, t2;
  s1 = qd::two_diff(a.hi, b.hi, s2);
  t1 = qd::two_diff(a.lo, b.lo, t2);
  s2 += t1;
  s1 = qd::quick_two_sum(s1, s2, s2);
  s2 += t2;
  s1 = qd::quick_two_sum(s1, s2, s2);
  return dd_real(s1, s2);
#endif
}

/* real - real-real */
inline dd_real operator-(real a, const dd_real &b) {
  real s1, s2;
  s1 = qd::two_diff(a, b.hi, s2);
  s2 -= b.lo;
  s1 = qd::quick_two_sum(s1, s2, s2);
  return dd_real(s1, s2);
}

/*********** Self-Subtractions ************/
/* real-real -= real */
inline dd_real &dd_real::operator-=(real a) {
  real s1, s2;
  s1 = qd::two_diff(hi, a, s2);
  s2 += lo;
  hi = qd::quick_two_sum(s1, s2, lo);
  return *this;
}

/* real-real -= real-real */
inline dd_real &dd_real::operator-=(const dd_real &a) {
#ifndef QD_IEEE_ADD
  real s, e;
  s = qd::two_diff(hi, a.hi, e);
  e += lo;
  e -= a.lo;
  hi = qd::quick_two_sum(s, e, lo);
  return *this;
#else
  real s1, s2, t1, t2;
  s1 = qd::two_diff(hi, a.hi, s2);
  t1 = qd::two_diff(lo, a.lo, t2);
  s2 += t1;
  s1 = qd::quick_two_sum(s1, s2, s2);
  s2 += t2;
  hi = qd::quick_two_sum(s1, s2, lo);
  return *this;
#endif
}

/*********** Unary Minus ***********/
inline dd_real dd_real::operator-() const {
  return dd_real(-hi, -lo);
}

/*********** Multiplications ************/
/* real-real = real * real */
inline dd_real dd_real::mul(real a, real b) {
  real p, e;
  p = qd::two_prod(a, b, e);
  return dd_real(p, e);
}

/* real-real * (2.0 ^ exp) */
inline dd_real ldexp(const dd_real &a, int exp) {
  return dd_real(ldexp(a.hi, exp), ldexp(a.lo, exp));
}

/* real-real * real,  where real is a power of 2. */
inline dd_real mul_pwr2(const dd_real &a, real b) {
  return dd_real(a.hi * b, a.lo * b);
}

/* real-real * real */
inline dd_real operator*(const dd_real &a, real b) {
  real p1, p2;

  p1 = qd::two_prod(a.hi, b, p2);
  p2 += (a.lo * b);
  p1 = qd::quick_two_sum(p1, p2, p2);
  return dd_real(p1, p2);
}

/* real-real * real-real */
inline dd_real operator*(const dd_real &a, const dd_real &b) {
  real p1, p2;

  p1 = qd::two_prod(a.hi, b.hi, p2);
  p2 += a.hi * b.lo;
  p2 += a.lo * b.hi;
  p1 = qd::quick_two_sum(p1, p2, p2);
  return dd_real(p1, p2);
}

/* real * real-real */
inline dd_real operator*(real a, const dd_real &b) {
  return (b * a);
}

/*********** Self-Multiplications ************/
/* real-real *= real */
inline dd_real &dd_real::operator*=(real a) {
  real p1, p2;
  p1 = qd::two_prod(hi, a, p2);
  p2 += lo * a;
  hi = qd::quick_two_sum(p1, p2, lo);
  return *this;
}

/* real-real *= real-real */
inline dd_real &dd_real::operator*=(const dd_real &a) {
  real p1, p2;
  p1 = qd::two_prod(hi, a.hi, p2);
  p2 += a.lo * hi;
  p2 += a.hi * lo;
  hi = qd::quick_two_sum(p1, p2, lo);
  return *this;
}

/*********** Divisions ************/
inline dd_real dd_real::div(real a, real b) {
  real q1, q2;
  real p1, p2;
  real s, e;

  q1 = a / b;

  /* Compute  a - q1 * b */
  p1 = qd::two_prod(q1, b, p2);
  s = qd::two_diff(a, p1, e);
  e -= p2;

  /* get next approximation */
  q2 = (s + e) / b;

  s = qd::quick_two_sum(q1, q2, e);

  return dd_real(s, e);
}

/* real-real / real */
inline dd_real operator/(const dd_real &a, real b) {

  real q1, q2;
  real p1, p2;
  real s, e;
  dd_real r;
  
  q1 = a.hi / b;   /* approximate quotient. */

  /* Compute  this - q1 * d */
  p1 = qd::two_prod(q1, b, p2);
  s = qd::two_diff(a.hi, p1, e);
  e += a.lo;
  e -= p2;
  
  /* get next approximation. */
  q2 = (s + e) / b;

  /* renormalize */
  r.hi = qd::quick_two_sum(q1, q2, r.lo);

  return r;
}

/* real-real / real-real */
inline dd_real operator/(const dd_real &a, const dd_real &b) {
  real q1, q2;
  dd_real r;

  q1 = a.hi / b.hi;  /* approximate quotient */

#ifdef QD_SLOPPY_DIV
  real s1, s2;

  /* compute  this - q1 * dd */
  r = b * q1;
  s1 = qd::two_diff(a.hi, r.hi, s2);
  s2 -= r.lo;
  s2 += a.lo;

  /* get next approximation */
  q2 = (s1 + s2) / b.hi;

  /* renormalize */
  r.hi = qd::quick_two_sum(q1, q2, r.lo);
  return r;
#else
  real q3;
  r = a - q1 * b;
  
  q2 = r.hi / b.hi;
  r -= (q2 * b);

  q3 = r.hi / b.hi;

  q1 = qd::quick_two_sum(q1, q2, q2);
  r = dd_real(q1, q2) + q3;
  return r;
#endif
}

/* real / real-real */
inline dd_real operator/(real a, const dd_real &b) {
  return dd_real(a) / b;
}

inline dd_real inv(const dd_real &a) {
  return real(1.0) / a;
}

/*********** Self-Divisions ************/
/* real-real /= real */
inline dd_real &dd_real::operator/=(real a) {
  *this = *this / a;
  return *this;
}

/* real-real /= real-real */
inline dd_real &dd_real::operator/=(const dd_real &a) {
  *this = *this / a;
  return *this;
}

/********** Remainder **********/
inline dd_real drem(const dd_real &a, const dd_real &b) {
  dd_real n = nint(a / b);
  return (a - n * b);
}

inline dd_real divrem(const dd_real &a, const dd_real &b, dd_real &r) {
  dd_real n = nint(a / b);
  r = a - n * b;
  return n;
}

/*********** Squaring **********/
inline dd_real sqr(const dd_real &a) {
  real p1, p2;
  real s1, s2;
  p1 = qd::two_sqr(a.hi, p2);
  p2 += real(2.0) * a.hi * a.lo;
  p2 += a.lo * a.lo;
  s1 = qd::quick_two_sum(p1, p2, s2);
  return dd_real(s1, s2);
}

inline dd_real dd_real::sqr(real a) {
  real p1, p2;
  p1 = qd::two_sqr(a, p2);
  return dd_real(p1, p2);
}


/********** Exponentiation **********/
inline dd_real dd_real::operator^(int n) {
  return npwr(*this, n);
}


/*********** Assignments ************/
/* real-real = real */
inline dd_real &dd_real::operator=(real a) {
  hi = a;
  lo = real(0.0);
  return *this;
}

/*********** Equality Comparisons ************/
/* real-real == real */
inline bool operator==(const dd_real &a, real b) {
  return (a.hi == b && a.lo == real(0.0));
}

/* real-real == real-real */
inline bool operator==(const dd_real &a, const dd_real &b) {
  return (a.hi == b.hi && a.lo == b.lo);
}

/* real == real-real */
inline bool operator==(real a, const dd_real &b) {
  return (a == b.hi && b.lo == real(0.0));
}

/*********** Greater-Than Comparisons ************/
/* real-real > real */
inline bool operator>(const dd_real &a, real b) {
  return (a.hi > b || (a.hi == b && a.lo > real(0.0)));
}

/* real-real > real-real */
inline bool operator>(const dd_real &a, const dd_real &b) {
  return (a.hi > b.hi || (a.hi == b.hi && a.lo > b.lo));
}

/* real > real-real */
inline bool operator>(real a, const dd_real &b) {
  return (a > b.hi || (a == b.hi && b.lo < real(0.0)));
}

/*********** Less-Than Comparisons ************/
/* real-real < real */
inline bool operator<(const dd_real &a, real b) {
  return (a.hi < b || (a.hi == b && a.lo < real(0.0)));
}

/* real-real < real-real */
inline bool operator<(const dd_real &a, const dd_real &b) {
  return (a.hi < b.hi || (a.hi == b.hi && a.lo < b.lo));
}

/* real < real-real */
inline bool operator<(real a, const dd_real &b) {
  return (a < b.hi || (a == b.hi && b.lo > real(0.0)));
}

/*********** Greater-Than-Or-Equal-To Comparisons ************/
/* real-real >= real */
inline bool operator>=(const dd_real &a, real b) {
  return (a.hi > b || (a.hi == b && a.lo >= real(0.0)));
}

/* real-real >= real-real */
inline bool operator>=(const dd_real &a, const dd_real &b) {
  return (a.hi > b.hi || (a.hi == b.hi && a.lo >= b.lo));
}

/* real >= real-real */
inline bool operator>=(real a, const dd_real &b) {
  return (b <= a);
}

/*********** Less-Than-Or-Equal-To Comparisons ************/
/* real-real <= real */
inline bool operator<=(const dd_real &a, real b) {
  return (a.hi < b || (a.hi == b && a.lo <= real(0.0)));
}

/* real-real <= real-real */
inline bool operator<=(const dd_real &a, const dd_real &b) {
  return (a.hi < b.hi || (a.hi == b.hi && a.lo <= b.lo));
}

/* real <= real-real */
inline bool operator<=(real a, const dd_real &b) {
  return (b >= a);
}

/*********** Not-Equal-To Comparisons ************/
/* real-real != real */
inline bool operator!=(const dd_real &a, real b) {
  return (a.hi != b || a.lo != real(0.0));
}

/* real-real != real-real */
inline bool operator!=(const dd_real &a, const dd_real &b) {
  return (a.hi != b.hi || a.lo != b.lo);
}

/* real != real-real */
inline bool operator!=(real a, const dd_real &b) {
  return (a != b.hi || b.lo != real(0.0));
}

/*********** Micellaneous ************/
/*  this == 0 */
inline bool dd_real::is_zero() const {
  return (hi == real(0.0));
}

/*  this == 1 */
inline bool dd_real::is_one() const {
  return (hi == real(1.0) && lo == real(0.0));
}

/*  this > 0 */
inline bool dd_real::is_positive() const {
  return (hi > real(0.0));
}

/* this < 0 */
inline bool dd_real::is_negative() const {
  return (hi < real(0.0));
}

/* Absolute value */
inline dd_real fabs(const dd_real &a) {
  return (a.hi < real(0.0)) ? -a : a;
}

inline dd_real abs(const dd_real &a) {
  return fabs(a);
}

/* Round to Nearest integer */
inline dd_real nint(const dd_real &a) {
  real hi = qd::nint(a.hi);
  real lo;

  if (hi == a.hi) {
    /* High word is an integer already.  Round the low word.*/
    lo = qd::nint(a.lo);
    
    /* Renormalize. This is needed if hi = some integer, lo = 1/2.*/
    hi = qd::quick_two_sum(hi, lo, lo);
  } else {
    /* High word is not an integer. */
    lo = real(0.0);
    if (fabsl(hi-a.hi) == real(0.5) && a.lo < real(0.0)) {
      /* There is a tie in the high word, consult the low word 
	 to break the tie. */
      hi -= real(1.0);      /* NOTE: This does not cause INEXACT. */
    }
  }

  return dd_real(hi, lo);
}

inline dd_real floor(const dd_real &a) {
  real hi = floorl(a.hi);
  real lo = real(0.0);

  if (hi == a.hi) {
    /* High word is integer already.  Round the low word. */
    lo = floorl(a.lo);
    hi = qd::quick_two_sum(hi, lo, lo);
  }

  return dd_real(hi, lo);
}

inline dd_real ceil(const dd_real &a) {
  real hi = ceill(a.hi);
  real lo = real(0.0);

  if (hi == a.hi) {
    /* High word is integer already.  Round the low word. */
    lo = ceill(a.lo);
    hi = qd::quick_two_sum(hi, lo, lo);
  }

  return dd_real(hi, lo);
}

inline dd_real aint(const dd_real &a) {
  return (a.hi >= real(0.0)) ? floor(a) : ceil(a);
}

/* Random number generator */
inline dd_real dd_real::rand() {
  return ddrand();
}

#endif /* _QD_DD_INLINE_H */
