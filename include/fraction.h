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


#ifndef FRACTION_H
#define FRACTION_H




#include <iostream>
#include "floating.h"
#include "calculus.h"




namespace fdi
{




/**
	Fractional class.

	Fractional object based on integers.
*/

class fraction
{
	floating n, d;

public:
        static const std::ios_base::fmtflags mixed = std::ios_base::fmtflags(1L << 21);

	fraction();
	fraction(fraction const &);
	fraction(floating, floating = 1);

	operator integer () const;
	operator floating () const;

	friend fraction operator - (fraction const &);
	friend fraction operator + (fraction const &, fraction const &);
	friend fraction operator - (fraction const &, fraction const &);
	friend fraction operator * (fraction const &, fraction const &);
	friend fraction operator / (fraction const &, fraction const &);

	friend std::ostream & operator << (std::ostream &, fraction const &);
};

inline fraction::fraction()
{
}

inline fraction::fraction(fraction const & a) : n(a.n), d(a.d)
{
}

inline fraction::fraction(floating a, floating b) : n(a), d(b)
{
	if (a)
	{
		floating f = fdi::gcd()(abs(a), abs(b));

		n /= f;
		d /= f;
	}
	else
	{
		n = 0;
		d = 1;
	}
}

inline fraction::operator integer () const
{
	return integer(n) / integer(d);
}

inline fraction::operator floating () const
{
	return floating(n) / floating(d);
}

inline fraction operator - (fraction const & a)
{
	return fraction(- a.n, a.d);
}

inline fraction operator + (fraction const & a, fraction const & b)
{
	return fraction(a.n * b.d + b.n * a.d, a.d * b.d);
}

inline fraction operator - (fraction const & a, fraction const & b)
{
	return fraction(a.n * b.d - b.n * a.d, a.d * b.d);
}

inline fraction operator * (fraction const & a, fraction const & b)
{
	return fraction(a.n * b.n, a.d * b.d);
}

inline fraction operator / (fraction const & a, fraction const & b)
{
    return b.n.is_positive() ? fraction(a.n * b.d, a.d * b.n) : fraction(- a.n * b.d, a.d * abs(b.n));
}

inline std::ostream & operator << (std::ostream & o, fraction const & n)
{
	if (o.flags() & fraction::mixed && n.n > n.d)
		return ((real &)(n.n) == 0.0L) ? o << 0.0L : ((real &)(n.d) == 1.0L) ? o << n.n : o << floor(n.n / n.d) << ':' << n.n - floor(n.n / n.d) * n.d << ':' << n.d;
	else
		return ((real &)(n.n) == 0.0L) ? o << 0.0L : ((real &)(n.d) == 1.0L) ? o << n.n : o << n.n << ':' << n.d;
}




}




#endif
