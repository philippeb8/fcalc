/*
 * src/dd.cc
 *
 * This work was supported by the Director, Office of Science, Division
 * of Mathematical, Information, and Computational Sciences of the
 * U.S. Department of Energy under contract number DE-AC03-76SF00098.
 *
 * Copyright (c) 2000-2001
 *
 * Contains implementation of non-inlined functions of double-double
 * package.  Inlined functions are found in dd_inline.h (in include directory).
 */
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <iostream>
#include <limits>

//#include "config.h"
#include "dd.h"
//#include "bits.h"

#ifndef QD_INLINE
#include "dd_inline.h"
#endif

#ifdef _MSC_VER
#define STD_RAND  ::rand
#else
#define STD_RAND  std::rand
#endif

using std::cout;
using std::cerr;
using std::endl;
using std::ostream;
using std::istream;
using std::ios_base;

static const char *digits = "0123456789";

const dd_real dd_real::_nan = dd_real(std::numeric_limits<real>::quiet_NaN());
const dd_real dd_real::_inf = dd_real(std::numeric_limits<real>::infinity());
const dd_real dd_real::_2pi = dd_real(6.283185307179586232e+00,
                                      2.449293598294706414e-16);
const dd_real dd_real::_pi = dd_real(3.141592653589793116e+00,
                                     1.224646799147353207e-16);
const dd_real dd_real::_pi2 = dd_real(1.570796326794896558e+00,
                                      6.123233995736766036e-17);
const dd_real dd_real::_pi4 = dd_real(7.853981633974482790e-01,
                                      3.061616997868383018e-17);
const dd_real dd_real::_pi8 = dd_real(3.926990816987241395e-01,
                                      1.530808498934191509e-17);
const dd_real dd_real::_pi16 = dd_real(1.963495408493620697e-01,
                                       7.654042494670957545e-18);
const dd_real dd_real::_3pi4 = dd_real(2.356194490192344837e+00,
                                       9.1848509936051484375e-17);
const dd_real dd_real::_e = dd_real(2.718281828459045091e+00,
                                    1.445646891729250158e-16);
const dd_real dd_real::_log2 = dd_real(6.931471805599452862e-01,
                                       2.319046813846299558e-17);
const dd_real dd_real::_log10 = dd_real(2.302585092994045901e+00,
                                        -2.170756223382249351e-16);
const real dd_real::_eps = 1.23259516440783e-32;  /* = 2^-106 */

/* This routine is called whenever a fatal error occurs. */
void dd_real::abort() {
  
  exit(-1);
}

/* Computes the square root of the real-real number dd.
   NOTE: dd must be a non-negative number.                   */
dd_real sqrt(const dd_real &a) {
  /* Strategy:  Use Karp's trick:  if x is an approximation
     to sqrt(a), then

        sqrt(a) = a*x + [a - (a*x)^2] * x / 2   (approx)

     The approximation is accurate to twice the accuracy of x.
     Also, the multiplication (a*x) and [-]*x can be done with
     only half the precision.
  */

  if (a.is_zero())
    return real(0.0);

  if (a.is_negative()) {
    //cerr << "ERROR (dd_real::sqrt): Negative argument." << endl;
    //dd_real::abort();
    return dd_real::_nan;
  }

  real x = real(1.0) / sqrtl(a.hi);
  real ax = a.hi * x;
  return dd_real::add(ax, (a - dd_real::sqr(ax)).hi * (x * real(0.5)));
}

/* Computes the square root of a real in real-real precision. 
   NOTE: d must not be negative.                                   */
dd_real dd_real::sqrt(real d) {
  return ::sqrt(dd_real(d));
}

/* Computes the n-th root of the real-real number a.
   NOTE: n must be a positive integer.  
   NOTE: If n is even, then a must not be negative.       */
dd_real nroot(const dd_real &a, int n) {
  /* Strategy:  Use Newton iteration for the function

          f(x) = x^(-n) - a

     to find its root a^{-1/n}.  The iteration is thus

          x' = x + x * (1 - a * x^n) / n

     which converges quadratically.  We can then find 
    a^{1/n} by taking the reciprocal.
  */

  if (n <= 0) {
    //cerr << "ERROR (dd_real::nroot): N must be positive." << endl;
    //dd_real::abort();
    return dd_real::_nan;
  }

  if (n%2 == 0 && a.is_negative()) {
    //cerr << "ERROR (dd_real::nroot): Negative argument." << endl;
    //dd_real::abort();
    return dd_real::_nan;
  }

  if (n == 1) {
    return a;
  } 
  if (n == 2) {
    return sqrt(a);
  }

  if (a.is_zero())
    return real(0.0);

  /* Note  a^{-1/n} = exp(-log(a)/n) */
  dd_real r = fabs(a);
  dd_real x = expl(-logl(r.hi) / n);

  /* Perform Newton's iteration. */
  x += x * (real(1.0) - r * npwr(x, n)) / (real) n;
  if (a.hi < real(0.0))
    x = -x;
  return real(1.0)/x;
}

/* Computes the n-th power of a real-real number. 
   NOTE:  0^0 causes an error.                         */
dd_real npwr(const dd_real &a, int n) {
  
  if (n == 0) {
    if (a.is_zero()) {
      //cerr << "ERROR (dd_real::npwr): Invalid argument." << endl;
      //dd_real::abort();
      return dd_real::_nan;
    }
    return real(1.0);
  }

  dd_real r = a;
  dd_real s = real(1.0);
  int N = abs(n);

  if (N > 1) {
    /* Use binary exponentiation */
    while (N > 0) {
      if (N % 2 == 1) {
	s *= r;
      }
      N /= 2;
      if (N > 0)
	r = sqr(r);
    }
  } else {
    s = r;
  }

  /* Compute the reciprocal if n is negative. */
  if (n < 0)
    return (real(1.0) / s);

  return s;
}

/* Computes real-real ^ real-real. */
dd_real pow(const dd_real &a, const dd_real &n) {
  if (n == floor(n))
  {
  	return npwr(a, n);
  }

  return exp(n * log(a));
}

/* Exponential.  Computes exp(x) in real-real precision. */
dd_real exp(const dd_real &a) {
  /* Strategy:  We first reduce the size of x by noting that

          exp(kr + m) = exp(m) * exp(r)^k

     Thus by choosing m to be a multiple of log(2) closest
     to x, we can make |kr| <= log(2) / 2 = 0.3466.  Now
     we can set k = 64, so that |r| <= 0.000542.  Then

          exp(x) = exp(kr + s log 2) = (2^s) * [exp(r)]^64

     Then exp(r) is evaluated using the familiar Taylor series.
     Reducing the argument substantially speeds up the convergence.
  */

  const int k = 64;

  if (a.hi <= real(-11335.0))
    return real(0.0);

  if (a.hi >=  real(11335.0)) {
    //cerr << "ERROR (dd_real::exp): Argument too large." << endl;
    //dd_real::abort();
    return dd_real::_inf;
  }

  if (a.is_zero()) {
    return real(1.0);
  }

  if (a.is_one()) {
    return dd_real::_e;
  }

  const int z = (int) nint(a / dd_real::_log2).hi;
  dd_real r = (a - dd_real::_log2 * (real) z) / (real) k;
  dd_real s, t, f, p;
  real m;

  s = real(1.0) + r;
  p = sqr(r);
  m = real(2.0);
  f = real(2.0);
  t = p / f;
  do {
    s += t;
    p *= r;
    m += real(1.0);
    f *= m;
    t = p / f;
  } while (fabs(t) > real(1.0e-35));

  s += t;
  r = npwr(s, k);
  r *= powl(real(2.0), z);

  return r;
}

/* Logarithm.  Computes log(x) in real-real precision.
   This is a natural logarithm (i.e., base e).            */
dd_real log(const dd_real &a) {
  /* Strategy.  The Taylor series for log converges much more
     slowly than that of exp, due to the lack of the factorial
     term in the denominator.  Hence this routine instead tries
     to determine the root of the function

         f(x) = exp(x) - a

     using Newton iteration.  The iteration is given by

         x' = x - f(x)/f'(x)
            = x - (1 - a * exp(-x))
            = x + a * exp(-x) - 1.

     Only one iteration is needed, since Newton's iteration
     approximately reals the number of digits per iteration. */

  if (a.is_one()) {
    return real(0.0);
  }

  if (a.hi <= real(0.0)) {
    //cerr << "ERROR (dd_real::log): Non-positive argument." << endl;
    //dd_real::abort();
    return dd_real::_nan;
  }

  dd_real x = logl(a.hi);   /* Initial approximation */

  x = x + a * exp(-x) - real(1.0);
  return x;
}

dd_real log10(const dd_real &a) {
  return log(a) / dd_real::_log10;
}


/* Computes sin(a) and cos(a) using Taylor series.
   Assumes |a| <= pi/32.                           */
static void sincos_taylor(const dd_real &a,
			  dd_real &sin_a, dd_real &cos_a) {
  const real thresh = real(1.0e-35) * fabsl((real) a);
  dd_real t;  /* Term being added. */
  dd_real p;  /* Current power of a. */
  dd_real f;  /* Denominator. */
  dd_real s;  /* Current partial sum. */
  dd_real x;  /* = -sqr(a) */
  real m;

  if (a.is_zero()) {
    sin_a = real(0.0);
    cos_a = real(1.0);
    return;
  }

  x = -sqr(a);
  s = a;
  p = a;
  m = real(1.0);
  f = real(1.0);
  do {
    p *= x;
    m += real(2.0);
    f *= (m*(m-1));
    t = p / f;
    s += t;
  } while (fabs(t) > thresh);

  sin_a = s;
  cos_a = sqrt(real(1.0) - sqr(s));
}

/* Table of sin(k * pi/16) and cos(k * pi/16). */
const dd_real dd_real::sin_table [] = {
  dd_real(1.950903220161282758e-01, -7.991079068461731263e-18), 
  dd_real(3.826834323650897818e-01, -1.005077269646158761e-17), 
  dd_real(5.555702330196021776e-01,  4.709410940561676821e-17),
  dd_real(7.071067811865475727e-01, -4.833646656726456726e-17)
};

const dd_real dd_real::cos_table [] = {
  dd_real(9.807852804032304306e-01, 1.854693999782500573e-17),
  dd_real(9.238795325112867385e-01, 1.764504708433667706e-17),
  dd_real(8.314696123025452357e-01, 1.407385698472802389e-18),
  dd_real(7.071067811865475727e-01, -4.833646656726456726e-17)
};

dd_real sin(const dd_real &a) {  

  /* Strategy.  To compute sin(x), we choose integers a, b so that

       x = s + a * (pi/2) + b * (pi/16)

     and |s| <= pi/32.  Using the fact that 

       sin(pi/16) = 0.5 * sqrt(2 - sqrt(2 + sqrt(2)))

     we can compute sin(x) from sin(s), cos(s).  This greatly 
     increases the convergence of the sine Taylor series. */

  if (a.is_zero()) {
    return real(0.0);
  }

  /* First reduce modulo 2*pi so that |r| <= pi. */
  dd_real r = drem(a, dd_real::_2pi);

  /* Now reduce by modulo pi/2 and then by pi/16 so that
     we obtain numbers a, b, and t. */
  dd_real t;
  dd_real sin_t, cos_t;
  dd_real s, c;
  int j = (int) divrem(r, dd_real::_pi2, t);
  int abs_j = abs(j);
  int k = (int) divrem(t, dd_real::_pi16, t);
  int abs_k = abs(k);

  if (abs_j > 2) {
    //cerr << "ERROR (dd_real::sin): Cannot reduce modulo pi/2." << endl;
    //dd_real::abort();
    return dd_real::_nan;
  }

  if (abs_j > 4) {
    //cerr << "ERROR (dd_real::sin): Cannot reduce modulo pi/16." << endl;
    //dd_real::abort();
    return dd_real::_nan;
  }

  sincos_taylor(t, sin_t, cos_t);

  if (abs_k == 0) {
    s = sin_t;
    c = cos_t;
  } else {
    dd_real u = dd_real::cos_table[abs_k-1];
    dd_real v = dd_real::sin_table[abs_k-1];

    if (k > 0) {
      s = u * sin_t + v * cos_t;
      c = u * cos_t - v * sin_t;
    } else {
      s = u * sin_t - v * cos_t;
      c = u * cos_t + v * sin_t;
    }
  }

  if (abs_j == 0) {
    r = s;
  } else if (j == 1) {
    r = c;
  } else if (j == -1) {
    r = -c;
  } else {
    r = -s;
  }

  return r;
}

dd_real cos(const dd_real &a) {

  if (a.is_zero()) {
    return real(1.0);
  }

  /* First reduce modulo 2*pi so that |r| <= pi. */
  dd_real r = drem(a, dd_real::_2pi);

  /* Now reduce by modulo pi/2 and then by pi/16 so that
     we obtain numbers a, b, and t. */
  dd_real t;
  dd_real sin_t, cos_t;
  dd_real s, c;
  int j = (int) divrem(r, dd_real::_pi2, t);
  int abs_j = abs(j);
  int k = (int) divrem(t, dd_real::_pi16, t);
  int abs_k = abs(k);

  if (abs_j > 2) {
    //cerr << "ERROR (dd_real::cos): Cannot reduce modulo pi/2." << endl;
    //dd_real::abort();
    return dd_real::_nan;
  }

  if (abs_k > 4) {
    //cerr << "ERROR (dd_real::cos): Cannot reduce modulo pi/16." << endl;
    //dd_real::abort();
    return dd_real::_nan;
  }

  sincos_taylor(t, sin_t, cos_t);

  if (abs_k == 0) {
    s = sin_t;
    c = cos_t;
  } else {
    dd_real u = dd_real::cos_table[abs_k-1];
    dd_real v = dd_real::sin_table[abs_k-1];

    if (k > 0) {
      s = u * sin_t + v * cos_t;
      c = u * cos_t - v * sin_t;
    } else {
      s = u * sin_t - v * cos_t;
      c = u * cos_t + v * sin_t;
    }
  }

  if (abs_j == 0) {
    r = c;
  } else if (j == 1) {
    r = -s;
  } else if (j == -1) {
    r = s;
  } else {
    r = -c;
  }

  return r;
}

void sincos(const dd_real &a, dd_real &sin_a, dd_real &cos_a) {

  if (a.is_zero()) {
    sin_a = real(0.0);
    cos_a = real(1.0);
    return;
  }

  /* First reduce modulo 2*pi so that |r| <= pi. */
  dd_real r = drem(a, dd_real::_2pi);

  /* Now reduce by modulo pi/2 and then by pi/16 so that
     we obtain numbers a, b, and t. */
  dd_real t;
  dd_real sin_t, cos_t;
  dd_real s, c;
  int j = (int) divrem(r, dd_real::_pi2, t);
  int abs_j = abs(j);
  int k = (int) divrem(t, dd_real::_pi16, t);
  int abs_k = abs(k);

  if (abs_j > 2) {
    //cerr << "ERROR (dd_real::sincos): Cannot reduce modulo pi/2." << endl;
    //dd_real::abort();
    return;
  }

  if (abs_k > 4) {
    //cerr << "ERROR (dd_real::sincos): Cannot reduce modulo pi/16." << endl;
    //dd_real::abort();
    return;
  }

  sincos_taylor(t, sin_t, cos_t);

  if (abs_k == 0) {
    s = sin_t;
    c = cos_t;
  } else {
    dd_real u = dd_real::cos_table[abs_k-1];
    dd_real v = dd_real::sin_table[abs_k-1];

    if (k > 0) {
      s = u * sin_t + v * cos_t;
      c = u * cos_t - v * sin_t;
    } else {
      s = u * sin_t - v * cos_t;
      c = u * cos_t + v * sin_t;
    }
  }

  if (abs_j == 0) {
    sin_a = s;
    cos_a = c;
  } else if (j == 1) {
    sin_a = c;
    cos_a = -s;
  } else if (j == -1) {
    sin_a = -c;
    cos_a = s;
  } else {
    sin_a = -s;
    cos_a = -c;
  }
  
}

dd_real atan(const dd_real &a) {
  return atan2(a, dd_real(1.0));
}

dd_real atan2(const dd_real &y, const dd_real &x) {
  /* Strategy: Instead of using Taylor series to compute 
     arctan, we instead use Newton's iteration to solve
     the equation

        sin(z) = y/r    or    cos(z) = x/r

     where r = sqrt(x^2 + y^2).
     The iteration is given by

        z' = z + (y - sin(z)) / cos(z)          (for equation 1)
        z' = z - (x - cos(z)) / sin(z)          (for equation 2)

     Here, x and y are normalized so that x^2 + y^2 = 1.
     If |x| > |y|, then first iteration is used since the 
     denominator is larger.  Otherwise, the second is used.
  */

  if (x.is_zero()) {
    
    if (y.is_zero()) {
      /* Both x and y is zero. */
      //cerr << "ERROR (dd_real::atan2): Both arguments zero." << endl;
      //dd_real::abort();
      return dd_real::_nan;
    }

    return (y.is_positive()) ? dd_real::_pi2 : -dd_real::_pi2;
  } else if (y.is_zero()) {
    return (x.is_positive()) ? dd_real(0.0) : dd_real::_pi;
  }

  if (x == y) {
    return (y.is_positive()) ? dd_real::_pi4 : -dd_real::_3pi4;
  }

  if (x == -y) {
    return (y.is_positive()) ? dd_real::_3pi4 : -dd_real::_pi4;
  }

  dd_real r = sqrt(sqr(x) + sqr(y));
  dd_real xx = x / r;
  dd_real yy = y / r;

  /* Compute real precision approximation to atan. */
  dd_real z = atan2((real) y, (real) x);
  dd_real sin_z, cos_z;

  if (xx > yy) {
    /* Use Newton iteration 1.  z' = z + (y - sin(z)) / cos(z)  */
    sincos(z, sin_z, cos_z);
    z += (yy - sin_z) / cos_z;
  } else {
    /* Use Newton iteration 2.  z' = z - (x - cos(z)) / sin(z)  */
    sincos(z, sin_z, cos_z);
    z -= (xx - cos_z) / sin_z;
  }

  return z;
}

dd_real tan(const dd_real &a) {
  dd_real s, c;
  sincos(a, s, c);
  return s/c;
}

dd_real asin(const dd_real &a) {
  dd_real abs_a = fabs(a);

  if (abs_a > real(1.0)) {
    //cerr << "ERROR (dd_real::asin): Argument out of domain." << endl;
    //dd_real::abort();
    return dd_real::_nan;
  }

  if (abs_a.is_one()) {
    return (a.is_positive()) ? dd_real::_pi2 : -dd_real::_pi2;
  }

  return atan2(a, sqrt(real(1.0) - sqr(a)));
}

dd_real acos(const dd_real &a) {
  dd_real abs_a = fabs(a);

  if (abs_a > real(1.0)) {
    //cerr << "ERROR (dd_real::acos): Argument out of domain." << endl;
    //dd_real::abort();
    return dd_real::_nan;
  }

  if (abs_a.is_one()) {
    return (a.is_positive()) ? dd_real(0.0) : dd_real::_pi;
  }

  return atan2(sqrt(real(1.0) - sqr(a)), a);
}
 
dd_real sinh(const dd_real &a) {
  if (a.is_zero()) {
    return real(0.0);
  }

  if (fabs(a) > real(0.05)) {
    dd_real ea = exp(a);
    return mul_pwr2(ea - inv(ea), real(0.5));
  }

  /* since a is small, using the above formula gives
     a lot of cancellation.  So use Taylor series.   */
  dd_real s = a;
  dd_real t = a;
  dd_real r = sqr(t);
  real m = real(1.0);
  real thresh = fabsl(((real) a) * dd_real::_eps);

  do {
    m += real(2.0);
    t *= r;
    t /= (m-1) * m;

    s += t;
  } while (fabs(t) > thresh);

  return s;

}

dd_real cosh(const dd_real &a) {
  if (a.is_zero()) {
    return real(1.0);
  }

  dd_real ea = exp(a);
  return mul_pwr2(ea + inv(ea), real(0.5));
}

dd_real tanh(const dd_real &a) {
  if (a.is_zero()) {
    return real(0.0);
  }

  dd_real ea = exp(a);
  dd_real inv_ea = inv(ea);
  return (ea - inv_ea) / (ea + inv_ea);
}

void sincosh(const dd_real &a, dd_real &sinh_a, dd_real &cosh_a) {
  sinh_a = sinh(a);
  cosh_a = cosh(a);
}

dd_real asinh(const dd_real &a) {
  return log(a + sqrt(sqr(a) + real(1.0)));
}

dd_real acosh(const dd_real &a) {
  if (a < real(1.0)) {
    //cerr << "ERROR (dd_real::acosh): Argument out of domain." << endl;
    //dd_real::abort();
    return dd_real::_nan;
  }

  return log(a + sqrt(sqr(a) - real(1.0)));
}

dd_real atanh(const dd_real &a) {
  if (fabs(a) >= real(1.0)) {
    //cerr << "ERROR (dd_real::atanh): Argument out of domain." << endl;
    //dd_real::abort();
    return dd_real::_nan;
  }

  return mul_pwr2(log((real(1.0) + a) / (real(1.0) - a)), real(0.5));
}

dd_real ddrand() {
  static const real m_const = 4.6566128730773926e-10;  /* = 2^{-31} */
  real m = m_const;
  dd_real r = real(0.0);
  real d;

  /* Strategy:  Generate 31 bits at a time, using lrand48
     random number generator.  Shift the bits, and reapeat
     4 times. */

  for (int i = 0; i < 4; i++, m *= m_const) {
//    d = lrand48() * m;
    d = STD_RAND() * m;
    r += d;
  }

  return r;
}

/* polyeval(c, n, x)
   Evaluates the given n-th degree polynomial at x.
   The polynomial is given by the array of (n+1) coefficients. */
dd_real polyeval(const dd_real *c, int n, const dd_real &x) {
  /* Just use Horner's method of polynomial evaluation. */
  dd_real r = c[n];
  
  for (int i = n-1; i >= 0; i--) {
    r *= x;
    r += c[i];
  }

  return r;
}

/* polyroot(c, n, x0)
   Given an n-th degree polynomial, finds a root close to 
   the given guess x0.  Note that this uses simple Newton
   iteration scheme, and does not work for multiple roots.  */
dd_real polyroot(const dd_real *c, int n, const dd_real &x0, 
		 real thresh) {
  dd_real x = x0;
  dd_real f;
  dd_real *d = new dd_real[n];
  bool conv = false;
  int i;

  /* Compute the coefficients of the derivatives. */
  for (i = 0; i < n; i++) {
    d[i] = c[i+1] * (real) (i+1);
  }

  /* Newton iteration. */
  for (i = 0; i < 20; i++) {
    f = polyeval(c, n, x);

    if (abs(f) < thresh) {
      conv = true;
      break;
    }
    x -= (f / polyeval(d, n-1, x));
  }
  delete [] d;

  if (!conv) {
    //cerr << "ERROR (qd_real::polyroot): Failed to converge." << endl;
    //dd_real::abort();
    return dd_real::_nan;
  }

  return x;
}


/* Constructor.  Reads a real-real number from the string s
   and constructs a real-real number.                         */
/*
dd_real::dd_real(const char *s) {
  dd_real::read(s, *this);
}
*/

/* Outputs the real-real number dd. */
ostream &operator<<(ostream &s, const dd_real &a) {
  char str[40];

  a.write(str, s.precision(), s.flags());
  return s << str;
}

/* Reads in the real-real number a. */
istream &operator>>(istream &s, dd_real &a) {
  char str[255];
  s >> str;
  a.read(str, a);
  return s;
}

/* Writes the real-real number into the string s.
   The integer d specifies how many significant digits to write.
   The string s must be able to hold at least (d+8) characters.  */
void dd_real::write(char *s, int d, ios_base::fmtflags l) const {

  dd_real r = fabs(*this);
  dd_real p;
  int e;
  int i = 0, j = 0;

  if (hi == real(0.0)) {
    s[i ++] = digits[0];
    s[i ++] = '.';
    s[i ++] = '\0';
    return;
  }

  /* Test for infinity and nan. */
  if (isinf())
  {
    sprintf(s, "INF");
    return;
  }

  if (isnan())
  {
    sprintf(s, "NAN");
    return;
  }

  /* First determine the (approximate) exponent. */
  e = (int) floorl(log10l(fabsl(hi)));
  p = dd_real(real(10.0)) ^ e;

  /* Find dimensions. */
  bool m = l & dd_real::showmagnitude, n = l & std::ios_base::showpoint;
  int D = (n ? d : e + (d < 1 ? d : 1)) + 1;

  /* Create digit array. */
  int *a = new int[D];

  /* Fix it if we are off by one. */
  r /= p;
  if (r >= real(10.0)) {
    r /= real(10.0);
    e++;
  } else if (r < real(1.0)) {
    r *= real(10.0);
    e--;
  }

  if (r >= real(10.0) || r < real(1.0)) {
    //cerr << "ERROR (dd_real::to_str): can't compute exponent." << endl;
    delete [] a;
    //dd_real::abort();
    return;
  }

  /* Extract the digits */
  for (i = 0; i < D; i++) {
    a[i] = (int) r.hi;
    r = r - (real) a[i];
    r *= real(10.0);
  }

  /* Fix negative digits. */
  for (i = D-1; i > 0; i--) {
    if (a[i] < 0) {
      a[i-1]--;
      a[i] += 10;
    }
  }

  if (a[0] <= 0) {
    //cerr << "ERROR (dd_real::to_str): non-positive leading digit." << endl;
    delete [] a;
    //dd_real::abort();
    return;
  }

  /* Round */
  if (a[D - 1] >= 5 && n)
  {
    a[D - 2] ++;

    int i = D - 2;
    while (i > 0 && a[i] >= 10)
    {
      a[i] -= 10;
      a[-- i] ++;
    }

    if (a[0] == 10)
    {
      ++ e;
      a[0] = 1;
    }
  }

  /* Start fill in the string. */
  if (hi < real(real(0.0)))
  {
    s[i ++] = '-';
  }

  bool scientific = ! (e > -4 && e < D - 1);
  int t, f = scientific ? 0 : e;

  for (t = (f > 0 ? f : 0); j < D - 1; -- f, ++ j)
  {
    if (a[j] != 0)
    {
      while (t != f)
      {
        s[i ++] = digits[0];

        if (t -- % 3 == 0) if (t > -1 && m) s[i ++] = ','; else if (t == -1 && (scientific || n)) s[i ++] = '.';
      }

      s[i ++] = digits[a[j]];

      if (t -- % 3 == 0) if (t > -1 && m) s[i ++] = ','; else if (t == -1 && (scientific || n)) s[i ++] = '.';
    }
  }

  while (t >= 0)
  {
    s[i ++] = digits[0];

    if (t -- % 3 == 0) if (t > -1 && m) s[i ++] = ','; else if (t == -1 && (scientific || n)) s[i ++] = '.';
  }

  if (e > -4 && e < D - 1)
  {
    s[i ++] = '\0';
  }
  else
  {
    s[i ++] = 'e';
    sprintf(&s[i], "%d", e);
  }

  delete [] a;
}

/* Reads in a real-real number from the string s. */
int dd_real::read(const char *s, dd_real &a) {
  const char *p = s;
  char ch;
  int sign = 0;
  int point = -1;
  int nd = 0;
  int e = 0;
  bool done = false;
  dd_real r = real(0.0);
  int nread;
  
  /* Skip any leading spaces */
  while (*p == ' ')
    p++;

  while (!done && (ch = *p) != '\0') {
    if (ch >= '0' && ch <= '9') {
      int d = ch - '0';
      r *= real(10.0);
      r += (real) d;
      nd++;
    } else {

      switch (ch) {

      case '.':
	if (point >= 0)
	  return -1;
	point = nd;
	break;

      case '-':
      case '+':
	if (sign != 0 || nd > 0)
	  return -1;
	sign = (ch == '-') ? -1 : 1;
	break;

      case 'E':
      case 'e':
	nread = sscanf(p+1, "%d", &e);
	done = true;
	if (nread != 1)
	  return -1;
	break;

      default:
	return -1;
      }
    }

    p++;
  }

  if (point >= 0) {
    e -= (nd - point);
  }

  if (e != 0) {
    r *= (dd_real(10.0) ^ e);
  }

  a = (sign == -1) ? -r : r;
  return 0;
}

#ifdef QD_DEBUG
/* Debugging routines */
void dd_real::dump() const {
  printf("[ %26.19e  ", hi);
  print_real_info(hi);
  printf("\n  %26.19e  ", lo);
  print_real_info(lo);
  printf(" ]\n");
}

void dd_real::dump_bits() const {
  cout << "[ ";
  print_real_info(hi);
  cout << endl << "  ";
  print_real_info(lo);
  cout << " ]" << endl;
}

void dd_real::dump_components() const {
  printf("[ %.19e %.19e ]\n", hi, lo);
}

dd_real dd_real::debug_rand() { 

  if (STD_RAND() % 2 == 0)
    return ddrand();

  int expn = 0;
  dd_real a = real(0.0);
  real d;
  for (int i = 0; i < 2; i++) {
	d = ldexp(((real) STD_RAND()) / RAND_MAX, -expn);
    a += d;
    expn = expn + 54 + STD_RAND() % 200;
  }
  return a;
}
#endif
