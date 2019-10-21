/**
    Fornux PowerCalc - Scientific calculator for your desktop.

    Copyright (C) 2012  Phil Bouchard <pbouchard8@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef CALCULUS_H
#define CALCULUS_H


#include <stdlib.h>
#include <sys/timeb.h>
#include <vector>
#include <functional>
#include <qstring.h>

#include "type.h"
#include "vector.h"
#include "floating.h"
#include "mutable_ptr.h"




/**
	Functor base class.

	<functional> extension accepting functors with 3 arguments.
*/

template <class _Arg1, class _Arg2, class _Arg3, class _Result>
	struct tertiary_function
	{
		typedef _Arg1 first_argument_type;
		typedef _Arg2 second_argument_type;
		typedef _Arg3 third_argument_type;
		typedef _Result result_type;
	};




namespace fdi
{




using namespace std;




class normed;

class fraction;




/**
	Variables.

	This structure holds cached variables.
*/

struct variable_t : mutable_ptr<type>
{
	variable_t() : mutable_ptr<type>(new type_t<floating>(0)) {}
};


/**
	Expressions.

	This structure caches input strings.
*/

struct expression_t : QString
{
	expression_t() : QString("0") {}
};




/**
	Direct functor.

	Functor with no arguments.
*/

template <class _Result>
	struct function
	{
		typedef _Result result_type;
	};




/**
	Functor typedefs.

	Functor typedefs simplifying a lot the syntax.
*/

typedef real (function<real>::* fmpr)() const;

typedef real (unary_function<real, real>::* ufmpr)(real const &) const;

typedef real (binary_function<real, real, real>::* bfmpr)(real const &, real const &) const;


typedef normed (function<normed>::* fmpn)() const;

typedef normed (unary_function<normed, normed>::* ufmpn)(normed const &) const;

typedef normed (binary_function<normed, normed, normed>::* bfmpn)(normed const &, normed const &) const;


typedef integer (function<integer>::* fmpl)() const;

typedef integer (unary_function<integer, integer>::* ufmpl)(integer const &) const;

typedef integer (binary_function<integer, integer, integer>::* bfmpl)(integer const &, integer const &) const;


typedef floating (function<floating>::* fmpd)() const;

typedef floating (unary_function<floating, floating>::* ufmpd)(floating const &) const;

typedef floating (binary_function<floating, floating, floating>::* bfmpd)(floating const &, floating const &) const;

typedef floating (tertiary_function<floating, floating, floating, floating>::* tfmpd)(floating const &, floating const &, floating const &) const;


typedef fraction (function<fraction>::* fmpf)() const;

typedef fraction (unary_function<fraction, fraction>::* ufmpf)(fraction const &) const;

typedef fraction (binary_function<fraction, fraction, fraction>::* bfmpf)(fraction const &, fraction const &) const;


typedef normed (binary_function<floating, floating, normed>::* bmmpd)(floating const &, floating const &) const;

typedef normed (binary_function<fdi::vector<normed>, fdi::vector<normed>, normed>::* bmmpv)(fdi::vector<normed> const &, fdi::vector<normed> const &) const;

typedef fdi::vector<normed> (binary_function< fdi::vector<normed>, fdi::vector<normed>, fdi::vector<normed> >::* bfmpv)(fdi::vector<normed> const &, fdi::vector<normed> const &) const;




/**
	Bitwise functors.

	Functors defining bitwise operations.
*/

template <class T>
	struct bitwise_not : public unary_function<T, T>
	{
		T operator () (T const & x) const { return - x - 1; }
	};

template <class T>
	struct bitwise_lsh : public binary_function<T, T, T>
	{
		T operator () (T const & x, T const & y) const { return x << y; }
	};

template <class T>
	struct bitwise_rsh : public binary_function<T, T, T>
	{
		T operator () (T const & x, T const & y) const { return x >> y; }
	};

template <class T>
	struct bitwise_mod : public binary_function<T, T, T>
	{
		T operator () (T const & x, T const & y) const { return x % y; }
	};

template <class T>
	struct bitwise_or : public binary_function<T, T, T>
	{
		T operator () (T const & x, T const & y) const { return x | y; }
	};

template <class T>
	struct bitwise_xor : public binary_function<T, T, T>
	{
		T operator () (T const & x, T const & y) const { return x ^ y; }
	};

template <class T>
	struct bitwise_and : public binary_function<T, T, T>
	{
		T operator () (T const & x, T const & y) const { return x & y; }
	};

template <class T>
	struct bitwise_nor : public binary_function<T, T, T>
	{
		T operator () (T const & x, T const & y) const { return bitwise_not<T>()(bitwise_or<T>()(x, y)); }
	};

template <class T>
	struct bitwise_xnor : public binary_function<T, T, T>
	{
		T operator () (T const & x, T const & y) const { return bitwise_not<T>()(bitwise_xor<T>()(x, y)); }
	};

template <class T>
	struct bitwise_nand : public binary_function<T, T, T>
	{
		T operator () (T const & x, T const & y) const { return bitwise_not<T>()(bitwise_and<T>()(x, y)); }
	};




/**
	Wrapper functors.

	Functors wrapping basic <cmath> functions.
*/

template <floating (* I)()>
	struct cmath : public function<floating>
	{
		floating operator () () const { return (* I)(); }
	};

template <floating (* I)(floating const &)>
	struct unary_cmath : public unary_function<floating, floating>
	{
		floating operator () (floating const & x) const { return (* I)(x); }
	};

template <floating (* I)(floating const &, floating const &)>
	struct binary_cmath : public binary_function<floating, floating, floating>
	{
		floating operator () (floating const & x, floating const & y) const { return (* I)(x, y); }
	};

    struct unary_cmath_abs : public unary_function<floating, floating>
    {
        floating operator () (floating const & x) const { return abs(x); }
    };

    struct unary_cmath_sqrt : public unary_function<floating, floating>
    {
        floating operator () (floating const & x) const { return sqrt(x); }
    };

    struct unary_cmath_sin : public unary_function<floating, floating>
    {
        floating operator () (floating const & x) const { return sin(x); }
    };

    struct unary_cmath_cos : public unary_function<floating, floating>
    {
        floating operator () (floating const & x) const { return cos(x); }
    };

    struct unary_cmath_tan : public unary_function<floating, floating>
    {
        floating operator () (floating const & x) const { return tan(x); }
    };

    struct unary_cmath_asin : public unary_function<floating, floating>
    {
        floating operator () (floating const & x) const { return asin(x); }
    };

    struct unary_cmath_acos : public unary_function<floating, floating>
    {
        floating operator () (floating const & x) const { return acos(x); }
    };

    struct unary_cmath_atan : public unary_function<floating, floating>
    {
        floating operator () (floating const & x) const { return atan(x); }
    };

    struct unary_cmath_sinh : public unary_function<floating, floating>
    {
        floating operator () (floating const & x) const { return sinh(x); }
    };

    struct unary_cmath_cosh : public unary_function<floating, floating>
    {
        floating operator () (floating const & x) const { return cosh(x); }
    };

    struct unary_cmath_tanh : public unary_function<floating, floating>
    {
        floating operator () (floating const & x) const { return tanh(x); }
    };

    struct unary_cmath_asinh : public unary_function<floating, floating>
    {
        floating operator () (floating const & x) const { return asinh(x); }
    };

    struct unary_cmath_acosh : public unary_function<floating, floating>
    {
        floating operator () (floating const & x) const { return acosh(x); }
    };

    struct unary_cmath_atanh : public unary_function<floating, floating>
    {
        floating operator () (floating const & x) const { return atanh(x); }
    };

    struct unary_cmath_exp : public unary_function<floating, floating>
    {
        floating operator () (floating const & x) const { return exp(x); }
    };

    struct unary_cmath_log : public unary_function<floating, floating>
    {
        floating operator () (floating const & x) const { return log(x); }
    };

    struct unary_cmath_log10 : public unary_function<floating, floating>
    {
        floating operator () (floating const & x) const { return log10(x); }
    };

    struct unary_cmath_floor : public unary_function<floating, floating>
    {
        floating operator () (floating const & x) const { return floor(x); }
    };

    struct binary_cmath_atan2 : public binary_function<floating, floating, floating>
    {
        floating operator () (floating const & x, floating const & y) const { return atan2(x, y); }
    };

    struct binary_cmath_pow : public binary_function<floating, floating, floating>
    {
        floating operator () (floating const & x, floating const & y) const { return pow(x, y); }
    };

    struct binary_cmath_drem : public binary_function<floating, floating, floating>
    {
        floating operator () (floating const & x, floating const & y) const { return drem(x, y); }
    };

    struct binary_cmath_add : public binary_function<floating, floating, floating>
    {
        floating operator () (floating const & x, floating const & y) const { return (x + y); }
    };
    struct binary_cmath_min : public binary_function<floating, floating, floating>
    {
        floating operator () (floating const & x, floating const & y) const { return (x - y); }
    };
    struct binary_cmath_mul : public binary_function<floating, floating, floating>
    {
        floating operator () (floating const & x, floating const & y) const { return (x * y); }
    };
    struct binary_cmath_div : public binary_function<floating, floating, floating>
    {
        floating operator () (floating const & x, floating const & y) const { return (x / y); }
    };




/**
	Extended functors.

	Functors that are not defined in <functional>.
*/

struct ran : public function<floating>
{
	floating operator () ();
};

struct fac : public unary_function<floating, floating>
{
	floating operator () (floating const & num);
};

struct gcd : public binary_function<floating, floating, floating>
{
	floating operator () (floating const & num1, floating const & num2);
};

// Permutation
struct permutation : public binary_function<floating, floating, floating>
{
	floating operator () (floating const &, floating const &);
};

// Combination
struct combination : public binary_function<floating, floating, floating>
{
	floating operator () (floating const &, floating const &);
};

// PI
struct pi : public function<floating>
{
	floating operator () ();
};

// Degrees, minutes, seconds
struct todegrees : public tertiary_function<floating, floating, floating, floating>
{
	floating operator () (floating const &, floating const &, floating const &);
};




struct len : public unary_function<fdi::vector<normed>,  normed>
{
	normed operator () (fdi::vector<normed> const &) const;
};

struct dot : public binary_function<fdi::vector<normed>, fdi::vector<normed>, normed>
{
	normed operator () (fdi::vector<normed> const &, fdi::vector<normed> const &) const;
};

struct cross : public binary_function< fdi::vector<normed>, fdi::vector<normed>, fdi::vector<normed> >
{
	fdi::vector<normed> operator () (fdi::vector<normed> const &, fdi::vector<normed> const &) const;
};




/**
	Normed routines.

	Miscellaneous advanced vector routines.
*/

// Polar
	struct polar : public unary_function<normed, normed>
	{
		normed operator () (normed const &);
	};

// Rectangular
	struct rectangular : public unary_function<normed, normed>
	{
		normed operator () (normed const &);
	};

// Vector size
	struct vectorsize : public unary_function<normed, normed>
	{
		normed operator () (normed const &);
	};

// Vector sort
	struct sort : public unary_function<normed, normed>
	{
		normed operator () (normed const &);
	};

// Element i'th of vector
	struct part : public binary_function<normed, normed, normed>
	{
		normed operator () (normed const &, normed const &);
	};

// X column of matrix
	struct colx : public unary_function<normed, normed>
	{
		normed operator () (normed const &);
	};

// Y column of matrix
	struct coly : public unary_function<normed, normed>
	{
		normed operator () (normed const &);
	};

// Vector merge
	struct vectormerge : public binary_function<normed, normed, normed>
	{
		normed operator () (normed const &, normed const &);
	};

// Matrix transpose
	struct transpose : public unary_function<normed, normed>
	{
		normed operator () (normed const &);
	};

// Matrix minormatrix
	struct minormatrix : public unary_function<normed, normed>
	{
		normed operator () (normed const &, fdi::vector<normed>::size_type, fdi::vector<normed>::size_type);
	};

// Matrix adjoint
	struct adjoint : public unary_function<normed, normed>
	{
		normed operator () (normed const &);
	};

// Matrix det
	struct det : public unary_function<normed, normed>
	{
		normed operator () (normed const &);
	};

// Matrix inverse
	struct inverse : public unary_function<normed, normed>
	{
		normed operator () (normed const &);
	};

// Sum x
	struct sum : public unary_function<normed, normed>
	{
		normed operator () (normed const &);
	};

// Sum [xy]
	struct sumxy : public unary_function<normed, normed>
	{
		normed operator () (normed const &);
	};

// Product x
	struct product : public unary_function<normed, normed>
	{
		normed operator () (normed const &);
	};

// Mathematica: mean
	struct mean : public unary_function<normed, normed>
	{
		normed operator () (normed const &);
	};

// Mathematica: median
	struct median : public unary_function<normed, normed>
	{
		normed operator () (normed const &);
	};

// Mathematica: variance
	struct variance : public unary_function<normed, normed>
	{
		normed operator () (normed const &);
	};

// Mathematica: variancemle
	struct variancemle : public unary_function<normed, normed>
	{
		normed operator () (normed const &);
	};

// Mathematica: covariance
	struct covariance : public unary_function<normed, normed>
	{
		normed operator () (normed const &);
	};

// Mathematica: fit
	struct alpha : public unary_function<normed, normed>
	{
		normed operator () (normed const &);
	};

// Mathematica: fit
	struct beta : public unary_function<normed, normed>
	{
		normed operator () (normed const &);
	};

// Mathematica: correlation
	struct correlation : public unary_function<normed, normed>
	{
		normed operator () (normed const &);
	};




/**
	Evaluation routines.

	Those routines returns a type that can handle different mathematical
	normed values.
*/

normed angle(normed const &, int);

normed angle_inverse(normed const &, int);

normed evaluate(type * const, int const &, bool const & = false, int = 0);

template <typename T>
	normed apply(T (unary_function<T, T>::*)(T const &) const, normed const &);

template <typename T>
	normed apply(T (binary_function<T, T, T>::*)(T const &, T const &) const, normed const &, normed const &);




}




/**
	User defined routines.
*/

struct function1st : public std::unary_function<fdi::normed, fdi::normed>
{
	fdi::normed operator () (fdi::normed const &);
};

struct function2nd : public std::binary_function<fdi::normed, fdi::normed, fdi::normed>
{
	fdi::normed operator () (fdi::normed const &, fdi::normed const &);
};




#endif

