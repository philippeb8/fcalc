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


#ifndef NORMED_H
#define NORMED_H


#include <string>
#include <algorithm>
#include <functional>

#include "type.h"
#include "vector.h"
#include "dispatch.h"
#include "floating.h"
#include "fraction.h"
#include "calculus.h"




namespace fdi
{




/**
	Normed value.

	Mathematical value that can be normalized.
*/

struct normed
{
	union
	{
		char c[sizeof(type_t<string>)];
		char i[sizeof(type_t<integer>)];
		char d[sizeof(type_t<floating>)];
		char f[sizeof(type_t<fraction>)];
		char v[sizeof(type_t< fdi::vector<normed> >)];
	};

	/// overriding fmtflags
        static const std::ios_base::fmtflags trim        = std::ios_base::fmtflags(1L << 16);
        static const std::ios_base::fmtflags bin         = std::ios_base::fmtflags(1L << 17);
        static const std::ios_base::fmtflags sexagesimal = std::ios_base::fmtflags(1L << 18);

	normed();
	~normed();

	normed(normed const &);

	normed(string const &);
	normed(integer const &);
	normed(floating const &);
	normed(fraction const &);
	normed(fdi::vector<normed> const &);

	normed & operator = (normed const &);

	type * to_type() const;
	floating to_floating() const;

	friend bool operator < (normed const &, normed const &);

	friend normed operator + (normed const &, normed const &);
	friend normed operator - (normed const &, normed const &);
	friend normed operator / (normed const &, normed const &);
	friend normed operator * (normed const &, normed const &);

	friend ostream & operator << (ostream &, normed const &);
};

inline normed::normed()
{
	new (this) type_t<integer>(0);
}

inline normed::~normed()
{
	if (type_t<string> const * q = dynamic_cast<type_t<string> const *>(reinterpret_cast<type const *>(this))) q->value.~string();
	else if (type_t<integer> const * q = dynamic_cast<type_t<integer> const *>(reinterpret_cast<type const *>(this))) q->value.~integer();
	else if (type_t<fraction> const * q = dynamic_cast<type_t<fraction> const *>(reinterpret_cast<type const *>(this))) q->value.~fraction();
	else if (type_t<floating> const * q = dynamic_cast<type_t<floating> const *>(reinterpret_cast<type const *>(this))) q->value.~floating();
	else if (type_t< fdi::vector<normed> > const * q = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(this))) q->value.~vector<normed>();
}

inline normed::normed(normed const & a)
{
	if (type_t<string> const * q = dynamic_cast<type_t<string> const *>(reinterpret_cast<type const *>(& a))) new (this) type_t<string>(q->value);
	else if (type_t<integer> const * q = dynamic_cast<type_t<integer> const *>(reinterpret_cast<type const *>(& a))) new (this) type_t<integer>(q->value);
	else if (type_t<fraction> const * q = dynamic_cast<type_t<fraction> const *>(reinterpret_cast<type const *>(& a))) new (this) type_t<fraction>(q->value);
	else if (type_t<floating> const * q = dynamic_cast<type_t<floating> const *>(reinterpret_cast<type const *>(& a))) new (this) type_t<floating>(q->value);
	else if (type_t< fdi::vector<normed> > const * q = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& a))) new (this) type_t< fdi::vector<normed> >(q->value);
}

inline normed::normed(string const & a)
{
	new (this) type_t<string>(a);
}

inline normed::normed(integer const & a)
{
	new (this) type_t<integer>(a);
}

inline normed::normed(floating const & a)
{
	new (this) type_t<floating>(a);
}

inline normed::normed(fraction const & a)
{
	new (this) type_t<fraction>(a);
}

inline normed::normed(fdi::vector<normed> const & a)
{
	new (this) type_t< fdi::vector<normed> >(a);
}

inline normed & normed::operator = (normed const & a)
{
	this->~normed();

	new (this) normed(a);

	return * this;
}

inline type * normed::to_type() const
{
	if (type_t<string> const * q = dynamic_cast<type_t<string> const *>(reinterpret_cast<type const *>(this))) return new type_t<string>(q->value);
	else if (type_t<integer> const * q = dynamic_cast<type_t<integer> const *>(reinterpret_cast<type const *>(this))) return new type_t<integer>(q->value);
	else if (type_t<fraction> const * q = dynamic_cast<type_t<fraction> const *>(reinterpret_cast<type const *>(this))) return new type_t<fraction>(q->value);
	else if (type_t<floating> const * q = dynamic_cast<type_t<floating> const *>(reinterpret_cast<type const *>(this))) return new type_t<floating>(q->value);
	else if (type_t< fdi::vector<normed> > const * q = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(this)))
	{
		typedef mutable_ptr<type> type_p;

		type_t< fdi::vector<type_p> > * r = new type_t< fdi::vector<type_p> >(q->value.size());

		for (unsigned int i = 0; i < q->value.size(); ++ i)
		{
			r->value[i] = q->value[i].to_type();
		}

		return r;
	}

	return 0;
}

inline floating normed::to_floating() const
{
	if (/*type_t<string> const * q = */dynamic_cast<type_t<string> const *>(reinterpret_cast<type const *>(this))) return floating(floating::_nan);
	else if (type_t<integer> const * q = dynamic_cast<type_t<integer> const *>(reinterpret_cast<type const *>(this))) return q->value;
	else if (type_t<fraction> const * q = dynamic_cast<type_t<fraction> const *>(reinterpret_cast<type const *>(this))) return q->value;
	else if (type_t<floating> const * q = dynamic_cast<type_t<floating> const *>(reinterpret_cast<type const *>(this))) return q->value;
	else if (/*type_t< fdi::vector<normed> > const * q = */dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(this))) return floating(floating::_nan);

	return floating(0);
}

inline normed sqrt(normed const & a)
{
	return apply((ufmpd) & unary_cmath<& ::sqrt>::operator (), a);
}

inline bool operator < (normed const & a, normed const & b)
{
	return a.to_floating() < b.to_floating();
}

inline normed operator + (normed const & a, normed const & b)
{
	return apply((bfmpd) & plus<floating>::operator (), a, b);
}

inline normed operator - (normed const & a, normed const & b)
{
	return apply((bfmpd) & minus<floating>::operator (), a, b);
}

inline normed operator / (normed const & a, normed const & b)
{
	return apply((bfmpd) & divides<floating>::operator (), a, b);
}

inline normed operator * (normed const & a, normed const & b)
{
	return apply((bfmpd) & multiplies<floating>::operator (), a, b);
}

inline ostream & operator << (ostream & a, normed const & b)
{
	std::ios_base::fmtflags f = a.flags();
	streamsize p = a.precision();

	if (type_t<string> const * q = dynamic_cast<type_t<string> const *>(reinterpret_cast<type const *>(& b)))
	{
		a << q->value;
	}
	else if (type_t<integer> const * q = dynamic_cast<type_t<integer> const *>(reinterpret_cast<type const *>(& b)))
	{
		if (a.flags() & normed::bin)
			a << dispatch<integer, string>().bin(q->value);
		else if (a.flags() & ios_base::oct)
			a << dispatch<integer, string>().oct(q->value);
		else if (a.flags() & ios_base::hex)
			a << dispatch<integer, string>().hex(q->value);
		else
		{
			a << q->value << '.';
		}
	}
	else if (type_t<fraction> const * q = dynamic_cast<type_t<fraction> const *>(reinterpret_cast<type const *>(& b)))
	{
		if (a.flags() & normed::bin)
			a << dispatch<integer, string>().bin(q->value);
		else if (a.flags() & ios_base::oct)
			a << dispatch<integer, string>().oct(q->value);
		else if (a.flags() & ios_base::hex)
			a << dispatch<integer, string>().hex(q->value);
		else
		{
			if (a.flags() & normed::trim) 
				a.precision(std::max<streamsize>(p / 2, 1));

			a.flags(f & ~ ios::showpoint);

			a << q->value;
		}
	}
	else if (type_t<floating> const * q = dynamic_cast<type_t<floating> const *>(reinterpret_cast<type const *>(& b)))
	{
		if (a.flags() & normed::bin)
			a << dispatch<integer, string>().bin(q->value);
		else if (a.flags() & ios_base::oct)
			a << dispatch<integer, string>().oct(q->value);
		else if (a.flags() & ios_base::hex)
			a << dispatch<integer, string>().hex(q->value);
		else if (a.flags() & normed::sexagesimal)
		{
			floating n[3];

			n[0] = q->value;
			n[1] = q->value * floating(60.0);
			n[2] = q->value * floating(3600.0);

			n[1] = n[1] - floor(n[1] / floating(60.0)) * floating(60.0);
			n[2] = n[2] - floor(n[2] / floating(60.0)) * floating(60.0);

			if ((real &)(n[2]) == 60.0L) 
				n[2] = floating(0.0), n[1] += floating(1.0);
			if ((real &)(n[1]) == 60.0L) 
				n[1] = floating(0.0), n[0] += floating(1.0);

			a.flags(f & ~ ios::showpoint);
			if (a.flags() & normed::trim) 
				a.precision(std::max<streamsize>(p / 3, 1));

			a << floor(n[0]) << "° ";
			a << floor(n[1]) << "'";

			if ((real const &)(floor(log10(q->value)) - floor(log10(n[2]))) < p)
			{
				a.flags(f | ios::showpoint);
				if (a.flags() & normed::trim) 
					a.precision(std::max<streamsize>(p / 3, 1)); else a.precision(std::max<streamsize>(p - 4, 1));

				a << ' ' << floating(n[2]) << '"';
			}
		}
		else
		{
			a << q->value;
		}
	}
	else if (type_t< fdi::vector<normed> > const * q = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& b)))
	{
		if (a.flags() & normed::bin)
			a << dispatch<integer, string>().bin(0);
		else if (a.flags() & ios_base::oct)
			a << dispatch<integer, string>().oct(0);
		else if (a.flags() & ios_base::hex)
			a << dispatch<integer, string>().hex(0);
		else
		{
			if (a.flags() & normed::trim) 
				a.precision(std::max<streamsize>(p / std::max<streamsize>(q->value.size(), 1U), 1U));

			a << q->value;
		}
	}

	a.flags(f);
	a.precision(p);

	return a;
}




}




#endif

