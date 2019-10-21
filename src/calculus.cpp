/**
    Copyright 2019 Fornux Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "calculus.h"

#include <functional>

#include <qstring.h>
#include <qregexp.h>

#include "type.h"
#include "normed.h"
#include "vector.h"
#include "floating.h"
#include "fraction.h"
#include "parser_yacc.h"




using namespace std;
using namespace fdi;




typedef normed (binary_function<fdi::vector<normed>, fdi::vector<normed>, normed>::* bmmpv)(fdi::vector<normed> const &, fdi::vector<normed> const &) const;

typedef fdi::vector<normed> (binary_function< fdi::vector<normed>, fdi::vector<normed>, fdi::vector<normed> >::* bfmpv)(fdi::vector<normed> const &, fdi::vector<normed> const &) const;




/**
	Shared variables and utilities.
*/

extern int depth;

extern bool saved;

extern char graph;

extern QString line;

extern variable_t variable[128];

extern expression_t expression[128];


extern normed const & flush(normed const &);




normed fdi::angle(normed const & d, int t)
{
	switch (t)
	{
	case 0: return d * floating(floating::_pi) / 180.0L;
	case 1: return d;
	case 2: return d * floating(floating::_pi) / 200.0L;
	}

	return string("Internal Fault");
}

normed fdi::angle_inverse(normed const & d, int t)
{
	switch (t)
	{
	case 0: return d * 180.0L / floating(floating::_pi);
	case 1: return d;
	case 2: return d * 200.0L / floating(floating::_pi);
	}

	return string("Internal Fault");
}




/**
	Extended functors.

	Functors that are not defined in <functional>.
*/

floating ran::operator () ()
{
	static bool first = true;

	if (first)
	{
		timeb t;

		first = false;

		ftime(& t);
		srand(t.millitm);
	}

	return floating(rand() % 1000) / floating(1000.0);
}

floating fac::operator () (floating const & num)
{
	floating res = floating(1);

	if (num.isinf() || num.isnan())
	{
		return num;
	}
    else if (num < floating(0) || num != floor(num))
	{
		return floating::_nan;
	}
	else if (num < floating(2))
	{
		return res;
	}

	for (floating i = floating(2); i <= num; i += floating(1))
	{
		res *= i;

		if (res.isinf() || res.isnan())
		{
			return floating::_inf;
		}
	}

	return res;
}

floating gcd::operator () (floating const & num1, floating const & num2)
{
	floating remainder = abs(drem(num2, num1));

	if (remainder != floating(0.0))
	{
		return operator () (remainder, num1);
	}

	return num1;
}

floating pi::operator () ()
{
	return floating();
}

floating todegrees::operator () (floating const & deg, floating const & min, floating const & sec)
{
	return floating();
}




normed len::operator () (fdi::vector<normed> const & a) const
{
	normed r = floating(0.0);

	for (fdi::vector<normed>::size_type i = 0; i < a.size(); ++ i)
	{
		r = r + a[i] * a[i];
	}

	return sqrt(r);
}

normed dot::operator () (fdi::vector<normed> const & a, fdi::vector<normed> const & b) const
{
	normed r = floating(0.0);

	for (fdi::vector<normed>::size_type i = 0; i < a.size(); ++ i)
	{
		r = r + a[i] * b[i];
	}

	return r;
}

fdi::vector<normed> cross::operator () (fdi::vector<normed> const & a, fdi::vector<normed> const & b) const
{
	fdi::vector<normed> r(a.size());
	fdi::vector<normed>::size_type const s = a.size();

	for (fdi::vector<normed>::size_type i = 0; i < s; ++ i)
	{
		r[i] = floating(0.0);

		for (fdi::vector<normed>::size_type j = i + 1; j < i + s; ++ j)
		{
			for (fdi::vector<normed>::size_type k = j + 1; k < i + s; ++ k)
			{
				r[i] = r[i] + a[j % s] * b[k % s] - a[k % s] * b[j % s];
			}
		}
	}

	return r;
}




floating permutation::operator () (floating const & a, floating const & b)
{
	return fac()(a) / fac()(a - b);
}

floating combination::operator () (floating const & a, floating const & b)
{
	return fac()(a) / fac()(b) / fac()(a - b);
}

normed polar::operator () (normed const & a)
{
	if (type_t< fdi::vector<normed> > const * r = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& a)))
	{
		fdi::vector<normed> u(2);

		if (r->value.size() != 2)
		{
			return string("Invalid Type");
		}

		u[0] = sqrt(r->value[0].to_floating() * r->value[0].to_floating() + r->value[1].to_floating() * r->value[1].to_floating());
		u[1] = atan(r->value[1].to_floating() / r->value[0].to_floating());

		return u;
	}

	return string("Invalid Type");
}

normed rectangular::operator () (normed const & a)
{
	if (type_t< fdi::vector<normed> > const * r = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& a)))
	{
		fdi::vector<normed> u(2);

		if (r->value.size() != 2)
		{
			return string("Invalid Type");
		}

		u[0] = r->value[0].to_floating() * cos(r->value[1].to_floating());
		u[1] = r->value[0].to_floating() * sin(r->value[1].to_floating());

		return u;
	}

	return string("Invalid Type");
}

normed vectorsize::operator () (normed const & a)
{
	if (type_t< fdi::vector<normed> > const * r = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& a)))
	{
		if (r->value.size() > 0)
		{
			return floating(r->value.size());
		}
	}

	return string("Invalid Type");
}

normed sort::operator () (normed const & a)
{
	if (type_t< fdi::vector<normed> > const * p = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& a)))
	{
		fdi::vector<normed> u(p->value.size());

		std::copy(p->value.begin(), p->value.end(), u.begin());
		std::sort(u.begin(), u.end());

		return u;
	}

	return string("Invalid Type");
}

normed part::operator () (normed const & a, normed const & b)
{
	if (type_t<floating> const * p = dynamic_cast<type_t<floating> const *>(reinterpret_cast<type const *>(& a)))
	{
		if (type_t< fdi::vector<normed> > const * q = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& b)))
		{
			if (p->value != floor(p->value))
			{
				return string("Invalid Type");
			}

			if (p->value >= floating(0.0) && q->value.size() > 0 && q->value.size() > (real const &) (p->value))
			{
				return q->value[(int) (real const &) (p->value)];
			}

			return string("Out of Range");
		}
	}

	return string("Invalid Type");
}

normed vectormerge::operator () (normed const & a, normed const & b)
{
	if (type_t< fdi::vector<normed> > const * p = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& a)))
	{
		if (type_t< fdi::vector<normed> > const * q = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& b)))
		{
			fdi::vector<normed> u(p->value.size() + q->value.size());

			fdi::vector<normed>::iterator i = std::copy(p->value.begin(), p->value.end(), u.begin());
			fdi::vector<normed>::iterator j = std::copy(q->value.begin(), q->value.end(), i);

			return u;
		}
	}

	return string("Invalid Type");
}

normed colx::operator () (normed const & a)
{
	normed const b = transpose()(a);

	if (type_t< fdi::vector<normed> > const * p = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& b)))
	{
		if (p->value.size() != 2)
		{
			return string("Invalid Type");
		}

		if (type_t< fdi::vector<normed> > const * q = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& p->value[0])))
		{
			return q->value;
		}
	}

	return string("Invalid Type");
}

normed coly::operator () (normed const & a)
{
	normed const b = transpose()(a);

	if (type_t< fdi::vector<normed> > const * p = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& b)))
	{
		if (p->value.size() != 2)
		{
			return string("Invalid Type");
		}

		if (type_t< fdi::vector<normed> > const * q = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& p->value[1])))
		{
			return q->value;
		}
	}

	return string("Invalid Type");
}

normed transpose::operator () (normed const & a)
{
	if (type_t< fdi::vector<normed> > const * p = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& a)))
	{
		fdi::vector<normed> u;

		for (fdi::vector<normed>::size_type i = 0; i < p->value.size(); ++ i)
		{
			if (type_t< fdi::vector<normed> > const * q = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& p->value[i])))
			{
				if (! i)
				{
					u.resize(q->value.size());
				}
				else if (dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& p->value[0]))->value.size() != q->value.size())
				{
					return string("Invalid Type");
				}

				for (fdi::vector<normed>::size_type j = 0; j < q->value.size(); ++ j)
				{
					if (! i)
					{
						u[j] = fdi::vector<normed>(p->value.size());
					}

					reinterpret_cast<type_t< fdi::vector<normed> > *>(& u[j])->value[i] = q->value[j];
				}

				continue;
			}

			return string("Invalid Type");
		}

		return u;
	}

	return string("Invalid Type");
}

normed minormatrix::operator () (normed const & a, fdi::vector<normed>::size_type r, fdi::vector<normed>::size_type c)
{
	if (type_t< fdi::vector<normed> > const * p = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& a)))
	{
		fdi::vector<normed> result(p->value.size() - 1);

		for (fdi::vector<normed>::size_type i = 0, m = 0; i < p->value.size(); ++ i)
		{
			if (type_t< fdi::vector<normed> > const * q = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& p->value[i])))
			{
				if (i == r) continue;

				result[m] = fdi::vector<normed>(q->value.size() - 1);

				for (fdi::vector<normed>::size_type j = 0, n = 0; j < q->value.size(); ++ j)
				{
					if (j == c) continue;

					reinterpret_cast<type_t< fdi::vector<normed> > *>(& result[m])->value[n] = q->value[j];

					++ n;
				}

				++ m;

				continue;
			}

			return string("Invalid Type");
		}

		return result;
	}

	return string("Invalid Type");
}

normed det::operator () (normed const & a)
{
	if (type_t< fdi::vector<normed> > const * p = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& a)))
	{
		normed result = floating(0.0);

		for (fdi::vector<normed>::size_type i = 0; i < p->value.size(); ++ i)
		{
			if (type_t< fdi::vector<normed> > const * q = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& p->value[i])))
			{
				if (p->value.size() != q->value.size())
				{
					return string("Invalid Type");
				}
				else if (q->value.size() == 1)
				{
					return q->value[0];
				}
				else for (fdi::vector<normed>::size_type j = 0; j < q->value.size(); ++ j)
				{
					result = result + q->value[j] * operator () (minormatrix()(p->value, 0, j)) * (j % 2 ? -1 : 1);
				}

				break;
			}

			return string("Invalid Type");
		}

		return result;
	}

	return string("Invalid Type");
}

normed adjoint::operator () (normed const & a)
{
	if (type_t< fdi::vector<normed> > const * p = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& a)))
	{
		fdi::vector<normed> result(p->value.size());

		for (fdi::vector<normed>::size_type i = 0; i < p->value.size(); ++ i)
		{
			if (type_t< fdi::vector<normed> > const * q = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& p->value[i])))
			{
				result[i] = fdi::vector<normed>(q->value.size());

				for (fdi::vector<normed>::size_type j = 0; j < q->value.size(); ++ j)
				{
					reinterpret_cast<type_t< fdi::vector<normed> > *>(& result[i])->value[j] = det()(minormatrix()(a, i, j)) * ((i + j) % 2 ? -1 : 1);
				}

				continue;
			}

			return string("Invalid Type");
		}

		return transpose()(result);
	}

	return string("Invalid Type");
}

normed inverse::operator () (normed const & a)
{
	normed result = det()(a);

	if (result.to_floating() != floating(0.0))
	{
		return adjoint()(a) / result;
	}

	return string("Invalid Type");
}


// Sum x
normed sum::operator () (normed const & a)
{
	if (type_t< fdi::vector<normed> > const * r = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& a)))
	{
		normed t = floating(0.0);

		for (fdi::vector<normed>::size_type i = 0; i < r->value.size(); ++ i)
		{
			t = t + r->value[i];
		}

		return t;
	}

	return string("Invalid Type");
}

// Sum [xy]
normed sumxy::operator () (normed const & a)
{
	if (type_t< fdi::vector<normed> > const * r = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& a)))
	{
		normed t = floating(0.0);

		for (fdi::vector<normed>::size_type i = 0; i < r->value.size(); ++ i)
		{
			if (type_t< fdi::vector<normed> > const * s = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& r->value[i])))
			{
				if (s->value.size() != 2)
				{
					return string("Invalid Type");
				}

				t = t + s->value[0] * s->value[1];

				continue;
			}

			return string("Invalid Type");
		}

		return t;
	}

	return string("Invalid Type");
}

// Mul x
normed product::operator () (normed const & a)
{
	if (type_t< fdi::vector<normed> > const * r = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& a)))
	{
		normed t = floating(1.0);

		for (fdi::vector<normed>::size_type i = 0; i < r->value.size(); ++ i)
		{
			t = t * r->value[i];
		}

		return t;
	}

	return string("Invalid Type");
}

// Mathematica: mean
normed mean::operator () (normed const & a)
{
	if (type_t< fdi::vector<normed> > const * p = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& a)))
	{
		normed t = floating(0.0);

		for (fdi::vector<normed>::size_type i = 0; i < p->value.size(); ++ i)
		{
			t = t + p->value[i];
		}

		return t / floating(p->value.size());
	}

	return string("Invalid Type");
}

// Mathematica: median
normed median::operator () (normed const & a)
{
	normed const b = sort()(a);

	if (type_t< fdi::vector<normed> > const * p = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& b)))
	{
		fdi::vector<normed>::size_type i = p->value.size();

		if (i % 2)
		{
			return p->value[i / 2];
		}
		else
		{
			return (p->value[i / 2 - 1] + p->value[i / 2]) / 2;
		}
	}

	return string("Invalid Type");
}

// Mathematica: variance
normed variance::operator () (normed const & a)
{
	if (type_t< fdi::vector<normed> > const * r = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& a)))
	{
		if (r->value.size() > 0)
		{
			normed t = mean()(a);
			normed u = floating(0.0);

			for (fdi::vector<normed>::size_type i = 0; i < r->value.size(); ++ i)
			{
				u = u + (r->value[i] - t) * (r->value[i] - t);
			}

			return u / floating(r->value.size() - 1);
		}

		return string("Invalid Type");
	}

	return string("Invalid Type");
}

// Mathematica: variancemle
normed variancemle::operator () (normed const & a)
{
	if (type_t< fdi::vector<normed> > const * r = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& a)))
	{
		normed t = mean()(a);
		normed u = floating(0.0);

		for (fdi::vector<normed>::size_type i = 0; i < r->value.size(); ++ i)
		{
			u = u + (r->value[i] - t) * (r->value[i] - t);
		}

		return u / floating(r->value.size());
	}

	return string("Invalid Type");
}

// Mathematica: covariance
normed covariance::operator () (normed const & a)
{
	return normed();
}

// Mathematica: fit
normed alpha::operator () (normed const & a)
{
	if (type_t< fdi::vector<normed> > const * r = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& a)))
	{
		normed x = floating(0.0);
		normed y = floating(0.0);
		normed xi = floating(0.0);
		normed yi = floating(0.0);
		normed xi2 = floating(0.0);
		normed xiyi = floating(0.0);

		for (fdi::vector<normed>::size_type i = 0; i < r->value.size(); ++ i)
		{
			if (type_t< fdi::vector<normed> > const * s = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& r->value[i])))
			{
				if (s->value.size() != 2)
				{
					return string("Invalid Type");
				}

				xi = xi + s->value[0];
				yi = yi + s->value[1];

				xi2 = xi2 + s->value[0] * s->value[0];

				xiyi = xiyi + s->value[0] * s->value[1];

				continue;
			}

			return string("Invalid Type");
		}

		x = xi / floating(r->value.size());
		y = yi / floating(r->value.size());

		return (y * xi2 - x * xiyi) / (xi2 - x * x * floating(r->value.size()));
	}

	return string("Invalid Type");
}

// Mathematica: fit
normed beta::operator () (normed const & a)
{
	if (type_t< fdi::vector<normed> > const * r = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& a)))
	{
		normed x = floating(0.0);
		normed y = floating(0.0);
		normed xi = floating(0.0);
		normed yi = floating(0.0);
		normed xi2 = floating(0.0);
		normed xiyi = floating(0.0);

		for (fdi::vector<normed>::size_type i = 0; i < r->value.size(); ++ i)
		{
			if (type_t< fdi::vector<normed> > const * s = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& r->value[i])))
			{
				if (s->value.size() != 2)
				{
					return string("Invalid Type");
				}

				xi = xi + s->value[0];
				yi = yi + s->value[1];

				xi2 = xi2 + s->value[0] * s->value[0];

				xiyi = xiyi + s->value[0] * s->value[1];

				continue;
			}

			return string("Invalid Type");
		}

		x = xi / floating(r->value.size());
		y = yi / floating(r->value.size());

		return (xiyi - x * y * floating(r->value.size())) / (xi2 - x * x * floating(r->value.size()));
	}

	return string("Invalid Type");
}

// Mathematica: correlation
normed correlation::operator () (normed const & a)
{
	if (type_t< fdi::vector<normed> > const * r = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& a)))
	{
		normed xi = floating(0.0);
		normed yi = floating(0.0);
		normed xi2 = floating(0.0);
		normed yi2 = floating(0.0);
		normed xiyi = floating(0.0);

		for (fdi::vector<normed>::size_type i = 0; i < r->value.size(); ++ i)
		{
			if (type_t< fdi::vector<normed> > const * s = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& r->value[i])))
			{
				if (s->value.size() != 2)
				{
					return string("Invalid Type");
				}

				xi = xi + s->value[0];
				yi = yi + s->value[1];

				xi2 = xi2 + s->value[0] * s->value[0];
				yi2 = yi2 + s->value[1] * s->value[1];

				xiyi = xiyi + s->value[0] * s->value[1];

				continue;
			}

			return string("Invalid Type");
		}

		return (xiyi * floating(r->value.size()) - xi * yi) / sqrt((xi2 * floating(r->value.size()) - xi * xi) * (yi2 * floating(r->value.size()) - yi * yi));
	}

	return string("Invalid Type");
}




/**
	Evaluation routines.

	These routines return a type that can handle different mathematical
	normed values.
*/

normed fdi::evaluate(type * const p, int const & angle_t, bool const & stat, int depth)
{
    ::depth = depth;

	// recursive expression
	if (depth > 1000)
	{
		return string("Recursive Expression");
	}
	// commands [program execution]
	if (type_t< std::list< tuple<type_p, int> > > * q = dynamic_cast<type_t< std::list< tuple<type_p, int> > > *>(p))
	{
		for (std::list< tuple<type_p, int> >::iterator i = q->value.begin(); i != q->value.end(); ++ i)
		{
			flush(evaluate(i->first.get(), angle_t, stat, depth + 1));
		}

		return string("");
	}
/*
	// commands [exit request]
	if (type_t<void> * q = dynamic_cast<type_t<void> *>(p))
	{
		return string("Exit Request");
	}
	// commands [IF statement]
	if (type_t< tuple<type_p, type_p, type_p> > * q = dynamic_cast< type_t< tuple<type_p, type_p, type_p> > *>(p))
	{
		if (evaluate(q->value.first.get(), angle_t, stat, depth + 1).to_floating() != floating(0))
		{
			flush(evaluate(q->value.second.get(), angle_t, stat, depth + 1));
		}
		else if (q->value.third.get())
		{
			flush(evaluate(q->value.third.get(), angle_t, stat, depth + 1));
		}

		return string("");
	}
	// commands [FOR loop]
	if (type_t< tuple<type_p, type_p, type_p, type_p> > * q = dynamic_cast< type_t< tuple<type_p, type_p, type_p, type_p> > *>(p))
	{
		normed res;

		for
		(
			evaluate(q->value.first.get(), angle_t, stat, depth + 1);
			evaluate(q->value.second.get(), angle_t, stat, depth + 1).to_floating() != floating(0);
			evaluate(q->value.third.get(), angle_t, stat, depth + 1)
		)
		{
			flush(evaluate(q->value.fourth.get(), angle_t, stat, depth + 1));
		}

		return string("");
	}
*/
	// variables
	if (type_t<char> * q = dynamic_cast<type_t<char> *>(p))
	{
		return evaluate(variable[tolower(q->value)].get(), angle_t, stat, depth + 1);
	}
	if (type_t<integer> * q = dynamic_cast<type_t<integer> *>(p))
	{
		return q->value;
	}
	if (type_t<floating> * q = dynamic_cast<type_t<floating> *>(p))
	{
		return q->value;
	}
	if (type_t<fraction> * q = dynamic_cast<type_t<fraction> *>(p))
	{
		return q->value;
	}
	if (type_t< fdi::vector<type_p> > * q = dynamic_cast<type_t< fdi::vector<type_p> > *>(p))
	{
		normed res = fdi::vector<normed>(q->value.size());

		fdi::vector<normed> & v = reinterpret_cast<type_t<fdi::vector<normed> > *>(& res)->value;

		for (unsigned int i = 0; i < v.size(); ++ i)
		{
			v[i] = evaluate(q->value[i].get(), angle_t, stat, depth + 1);
		}

		return res;
	}
	// result() function
	if (type_t< tuple<normed, type_p> > * q = dynamic_cast<type_t< tuple<normed, type_p> > *>(p))
	{
		return q->value.first;
	}
	// assignment operator
	if (type_t< tuple<char, type_p> > * q = dynamic_cast<type_t< tuple<char, type_p> > *>(p))
	{
		normed res;
		variable_t tmp;

		// save old value if using the result() affectator
		if (type_t< tuple<normed, type_p> > * r = dynamic_cast<type_t< tuple<normed, type_p> > *>(q->value.second.get()))
		{
			r->value.first = evaluate(r->value.second.get(), angle_t, stat, depth + 1);
		}

		// backup
		(mutable_ptr<type> &) tmp = (mutable_ptr<type> &) (variable[tolower(q->value.first)]);

		// affect
		(mutable_ptr<type> &) (variable[tolower(q->value.first)]) = q->value.second;

		// evaluate
		res = evaluate(q->value.second.get(), angle_t, stat, depth + 1);

		// error handler
		if (type_t<string> const * r = dynamic_cast<type_t<string> const *>(reinterpret_cast<type const *>(& res)))
		{
			if (r->value != "")
			{
				// restore & exit
				(mutable_ptr<type> &) (variable[tolower(q->value.first)]) = (mutable_ptr<type> &) tmp;
				return res;
			}
		}

		// exit
		saved = false;

		switch (char g = tolower(q->value.first))
		{
		case 'v':
		case 'w':
		case 'y':
		case 'z':
			graph = g;
			break;
		}

        (QString &) (expression[tolower(q->value.first)]) = line.right(line.length() - line.indexOf(QRegExp("[^ ]"), line.indexOf('=') + 1));

		return res;
	}

	// functions [T = normed]
	if (type_t< tuple<ufmpn, type_p> > * q = dynamic_cast<type_t< tuple<ufmpn, type_p> > *>(p))
	{
		if
		(
			q->value.first == (ufmpn) & polar::operator ()
		)
		{
            normed temp = polar()(evaluate(q->value.second.get(), angle_t, stat, depth + 1));

            if (type_t< fdi::vector<normed> > * r = dynamic_cast<type_t< fdi::vector<normed> > *>(reinterpret_cast<type *>(& temp)))
            {
                r->value[1] = angle_inverse(r->value[1], angle_t);

                return r->value;
            }

            return temp;
		}

		if
		(
			q->value.first == (ufmpn) & rectangular::operator ()
		)
		{
            normed temp = evaluate(q->value.second.get(), angle_t, stat, depth + 1);

            if (type_t< fdi::vector<normed> > * r = dynamic_cast<type_t< fdi::vector<normed> > *>(reinterpret_cast<type *>(& temp)))
            {
                r->value[1] = angle(r->value[1], angle_t);

                return rectangular()(r->value);
            }

            return temp;
		}

		return (unary_function<normed, normed>().*q->value.first)(evaluate(q->value.second.get(), angle_t, stat, depth + 1));
	}
	if (type_t< tuple<bfmpn, type_p, type_p> > * q = dynamic_cast<type_t< tuple<bfmpn, type_p, type_p> > *>(p))
	{
		return (binary_function<normed, normed, normed>().*q->value.first)(evaluate(q->value.second.get(), angle_t, stat, depth + 1), evaluate(q->value.third.get(), angle_t, stat, depth + 1));
	}

	// functions [T = long]
	if (type_t<fmpl> * q = dynamic_cast<type_t<fmpl> *>(p))
	{
		return (function<integer>().*q->value)();
	}
	if (type_t< tuple<ufmpl, type_p> > * q = dynamic_cast<type_t< tuple<ufmpl, type_p> > *>(p))
	{
		return apply(q->value.first, evaluate(q->value.second.get(), angle_t, stat, depth + 1));
	}
	if (type_t< tuple<bfmpl, type_p, type_p> > * q = dynamic_cast<type_t< tuple<bfmpl, type_p, type_p> >* >(p))
	{
		return apply(q->value.first, evaluate(q->value.second.get(), angle_t, stat, depth + 1), evaluate(q->value.third.get(), angle_t, stat, depth + 1));
	}

	// functions [T = floating]
	if (type_t<fmpd> * q = dynamic_cast<type_t<fmpd> *>(p))
	{
		return (function<floating>().*q->value)();
	}
	if (type_t< tuple<ufmpd, type_p> > * q = dynamic_cast<type_t< tuple<ufmpd, type_p> > *>(p))
    {
		if
		(
            q->value.first == (ufmpd) & unary_cmath_sin::operator () ||
            q->value.first == (ufmpd) & unary_cmath_cos::operator () ||
            q->value.first == (ufmpd) & unary_cmath_tan::operator ()
		)
        {
			return apply(q->value.first, angle(evaluate(q->value.second.get(), angle_t, stat, depth + 1), angle_t));
		}

		if
		(
            q->value.first == (ufmpd) & unary_cmath_asin::operator () ||
            q->value.first == (ufmpd) & unary_cmath_acos::operator () ||
            q->value.first == (ufmpd) & unary_cmath_atan::operator ()
		)
		{
			return angle_inverse(apply(q->value.first, evaluate(q->value.second.get(), angle_t, stat, depth + 1)), angle_t);
		}

		return apply(q->value.first, evaluate(q->value.second.get(), angle_t, stat, depth + 1));
	}
	if (type_t< tuple<bfmpd, type_p, type_p> > * q = dynamic_cast<type_t< tuple<bfmpd, type_p, type_p> >* >(p))
	{
		if
		(
            q->value.first == (bfmpd) & binary_cmath_atan2::operator ()
		)
		{
			return angle_inverse(apply(q->value.first, evaluate(q->value.second.get(), angle_t, stat, depth + 1), evaluate(q->value.third.get(), angle_t, stat, depth + 1)), angle_t);
		}

		if (stat)
		{
			if
			(
				q->value.first == (bfmpd) & plus<floating>::operator () ||
				q->value.first == (bfmpd) & minus<floating>::operator () ||
				q->value.first == (bfmpd) & divides<floating>::operator () ||
				q->value.first == (bfmpd) & multiplies<floating>::operator ()
			)
			{
				normed a = evaluate(q->value.second.get(), angle_t, stat, depth + 1);
				normed b = evaluate(q->value.third.get(), angle_t, stat, depth + 1);

				return apply(q->value.first, a, a * b / 100);
			}
		}

		return apply(q->value.first, evaluate(q->value.second.get(), angle_t, stat, depth + 1), evaluate(q->value.third.get(), angle_t, stat, depth + 1));
	}

	// functions [T = fdi::vector]
	if (type_t< tuple<bmmpv, type_p, type_p> > * q = dynamic_cast<type_t< tuple<bmmpv, type_p, type_p> > *>(p))
	{
		typedef binary_function<fdi::vector<normed>, fdi::vector<normed>, normed> bf;
		typedef normed (bf::* bfmp)(fdi::vector<normed> const &, fdi::vector<normed> const &) const;

		normed a = evaluate(q->value.second.get(), angle_t, stat, depth + 1);

		if (type_t< fdi::vector<normed> > const * r = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& a)))
		{
			normed b = evaluate(q->value.third.get(), angle_t, stat, depth + 1);

			if (type_t< fdi::vector<normed> > const * s = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& b)))
			{
				if (r->value.size() != s->value.size()) return string("Invalid Type");

				return (bf().*q->value.first)(r->value, s->value);
			}
		}

		return string("Invalid Type");
	}
	if (type_t< tuple<bfmpv, type_p, type_p> > * q = dynamic_cast<type_t< tuple<bfmpv, type_p, type_p> > *>(p))
	{
		typedef binary_function< fdi::vector<normed>, fdi::vector<normed>, fdi::vector<normed> > bf;
		typedef fdi::vector<normed> (bf::* bfmp)(fdi::vector<normed> const &, fdi::vector<normed> const &) const;

		normed a = evaluate(q->value.second.get(), angle_t, stat, depth + 1);

		if (type_t< fdi::vector<normed> > const * r = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& a)))
		{
			normed b = evaluate(q->value.third.get(), angle_t, stat, depth + 1);

			if (type_t< fdi::vector<normed> > const * s = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& b)))
			{
				if (r->value.size() != s->value.size()) return string("Invalid Type");

				if (q->value.first == (bfmpv) & cross::operator () && r->value.size() != 3) return string("Invalid Type");

				return (bf().*q->value.first)(r->value, s->value);
			}
		}

		return string("Invalid Type");
	}

	return floating(0.0L);
}

template <typename T>
	normed fdi::apply(T (unary_function<T, T>::* o)(T const &) const, normed const & a)
	{
		typedef T (unary_function<T, T>::* ufmp)(T const &) const;

		// error
		if (dynamic_cast<type_t<string> const *>(reinterpret_cast<type const *>(& a)))
		{
			return a;
		}

		// type specializations
		if (type_t< fdi::vector<normed> > const * p = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& a)))
		{
#if EDITION != 0
			if (type_t< fdi::vector<normed> > const * q = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& p->value[0])))
			{
				// linear algebra
                if (o == (ufmp) & unary_cmath_abs::operator ()) return det()(p->value);
			}
			else
#endif
			{
				// vector algebra
                if (o == (ufmp) & unary_cmath_abs::operator ()) return len()(p->value);
			}

			normed r;

			r = fdi::vector<normed>(p->value.size());

			fdi::vector<normed> & v = reinterpret_cast<type_t< fdi::vector<normed> > *>(& r)->value;

			for (unsigned int i = 0; i < v.size(); ++ i)
			{
				v[i] = apply(o, p->value[i]);
			}

			return r;
		}
		if (type_t<integer> const * p = dynamic_cast<type_t<integer> const *>(reinterpret_cast<type const *>(& a)))
		{
			if (o == (ufmp) & negate<floating>::operator ()) return negate<integer>()(p->value);

			return (unary_function<T, T>().*o)(p->value);
		}
		if (type_t<fraction> const * p = dynamic_cast<type_t<fraction> const *>(reinterpret_cast<type const *>(& a)))
		{
			if (o == (ufmp) & negate<floating>::operator ()) return negate<fraction>()(p->value);

			return (unary_function<T, T>().*o)(p->value);
		}
		if (type_t<floating> const * p = dynamic_cast<type_t<floating> const *>(reinterpret_cast<type const *>(& a)))
        {
			return (unary_function<T, T>().*o)(p->value);
		}

		return floating(0.0L);
	}

template <typename T>
	normed fdi::apply(T (binary_function<T, T, T>::* o)(T const &, T const &) const, normed const & a, normed const & b)
	{
		typedef T (binary_function<T, T, T>::* bfmp)(T const &, T const &) const;

		// error
		if (dynamic_cast<type_t<string> const *>(reinterpret_cast<type const *>(& a)))
		{
			return a;
		}
		if (dynamic_cast<type_t<string> const *>(reinterpret_cast<type const *>(& b)))
		{
			return b;
		}

		// type specializations
		type_t< fdi::vector<normed> > const * p = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& a));
		type_t< fdi::vector<normed> > const * q = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& b));

		if (p || q)
		{
			typedef typename fdi::vector<normed>::size_type size_type;

			normed res;

			fdi::vector<normed> & v = reinterpret_cast<type_t< fdi::vector<normed> > *>(& res)->value;

			if (! p)
			{
				res = fdi::vector<normed>(q->value.size());

				for (size_type i = 0; i < v.size(); ++ i)
				{
					v[i] = apply(o, a, q->value[i]);
				}
			}
			else if (! q)
			{
				type_t<floating> const * number = dynamic_cast<type_t<floating> const *>(reinterpret_cast<type const *>(& b));

#if EDITION != 0
				if
				(
					number &&
                    o == (bfmp) & binary_cmath_pow::operator ()
				)
				{
					// linear algebra
					if (floor(number->value) != number->value || number->value < floating(-1)) return string("Invalid Type");

					if (number->value == floating(-1))
					{
						res = inverse()(a);
					}
					else if (number->value == floating(0))
					{
						res = a;

						for (size_type i = 0; i < p->value.size(); ++ i)
						{
							type_t< fdi::vector<normed> > const * q = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& p->value[i]));

							for (size_type j = 0; j < q->value.size(); ++ j)
							{
								reinterpret_cast<type_t< fdi::vector<normed> > *>(& reinterpret_cast<type_t< fdi::vector<normed> > *>(& res)->value[i])->value[j] = (i == j ? 1 : 0);
							}
						}
					}
					else
					{
						// ex.: [[1, 2], [3, 4]] ^ 2
						res = a;

						for (floating i = number->value; i > floating(1); i = i - floating(1))
						{
							res = apply((bfmp) & multiplies<floating>::operator (), res, a);
						}
					}
				}
				else
#endif
				{
					// vector algebra
					res = fdi::vector<normed>(p->value.size());

					for (size_type i = 0; i < v.size(); ++ i)
					{
						v[i] = apply(o, p->value[i], b);
					}
				}
			}
			else
			{
				if (p->value.size() == 0 || q->value.size() == 0) return string("Invalid Type");

				type_t< fdi::vector<normed> > const * vector1 = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& p->value[0]));
				type_t< fdi::vector<normed> > const * vector2 = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& q->value[0]));

#if EDITION != 0
				if
				(
					(vector1 && vector2) &&
					(
						o == (bfmp) & divides<floating>::operator () ||
						o == (bfmp) & multiplies<floating>::operator ()
					)
				)
				{
					// linear algebra
					size_type x = vector2->value.size();
					size_type y = p->value.size();
					size_type m = vector1->value.size();

					if (q->value.size() != m) return string("Invalid Type");

					res = fdi::vector<normed>(y, fdi::vector<normed>(x));

					// ex.: [[1, 2], [3, 4]] * [[5, 6], [7, 8]]
					for (size_type i = 0; i < y; ++ i)
					{
						vector1 = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& p->value[i]));

						if (! vector1 || vector1->value.size() != m) return string("Invalid Type");

						for (size_type j = 0; j < x; ++ j)
						{
							normed sum;

							for (size_type k = 0; k < m; ++ k)
							{
								vector2 = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& q->value[k]));

								if (! vector2 || vector2->value.size() != x) return string("Invalid Type");

								sum = apply((bfmpd) & plus<floating>::operator (), sum, apply(o, vector1->value[k], vector2->value[j]));
							}

							reinterpret_cast<type_t< fdi::vector<normed> > *>(& reinterpret_cast<type_t< fdi::vector<normed> > *>(& res)->value[i])->value[j] = sum;
						}
					}
				}
				else
#endif
				{
					// vector algebra
					res = fdi::vector<normed>(p->value.size());

					for (size_type i = 0; i < v.size(); ++ i)
					{
						v[i] = apply(o, p->value[i], q->value[i]);
					}
				}
			}

			return res;
		}
		if (type_t<integer> const * p = dynamic_cast<type_t<integer> const *>(reinterpret_cast<type const *>(& a)))
		{
			if (type_t<integer> const * q = dynamic_cast<type_t<integer> const *>(reinterpret_cast<type const *>(& b)))
			{
				if (o == (bfmp) & plus<floating>::operator ()) return plus<integer>()(p->value, q->value);
				if (o == (bfmp) & minus<floating>::operator ()) return minus<integer>()(p->value, q->value);
				if (o == (bfmp) & divides<floating>::operator ()) return divides<integer>()(p->value, q->value);
				if (o == (bfmp) & multiplies<floating>::operator ()) return multiplies<integer>()(p->value, q->value);

				return (binary_function<T, T, T>().*o)(p->value, q->value);
			}
			if (type_t<fraction> const * q = dynamic_cast<type_t<fraction> const *>(reinterpret_cast<type const *>(& b)))
			{
				if (o == (bfmp) & plus<floating>::operator ()) return plus<fraction>()(fraction(p->value), q->value);
				if (o == (bfmp) & minus<floating>::operator ()) return minus<fraction>()(fraction(p->value), q->value);
				if (o == (bfmp) & divides<floating>::operator ()) return divides<fraction>()(fraction(p->value), q->value);
				if (o == (bfmp) & multiplies<floating>::operator ()) return multiplies<fraction>()(fraction(p->value), q->value);

				return (binary_function<T, T, T>().*o)(p->value, q->value);
			}
			if (type_t<floating> const * q = dynamic_cast<type_t<floating> const *>(reinterpret_cast<type const *>(& b)))
			{
				return (binary_function<T, T, T>().*o)(p->value, q->value);
			}
		}
		if (type_t<fraction> const * p = dynamic_cast<type_t<fraction> const *>(reinterpret_cast<type const *>(& a)))
		{
			if (type_t<integer> const * q = dynamic_cast<type_t<integer> const *>(reinterpret_cast<type const *>(& b)))
			{
				if (o == (bfmp) & plus<floating>::operator ()) return plus<fraction>()(p->value, fraction(q->value));
				if (o == (bfmp) & minus<floating>::operator ()) return minus<fraction>()(p->value, fraction(q->value));
				if (o == (bfmp) & divides<floating>::operator ()) return divides<fraction>()(p->value, fraction(q->value));
				if (o == (bfmp) & multiplies<floating>::operator ()) return multiplies<fraction>()(p->value, fraction(q->value));

				return (binary_function<T, T, T>().*o)(p->value, q->value);
			}
			if (type_t<fraction> const * q = dynamic_cast<type_t<fraction> const *>(reinterpret_cast<type const *>(& b)))
			{
				if (o == (bfmp) & plus<floating>::operator ()) return plus<fraction>()(p->value, q->value);
				if (o == (bfmp) & minus<floating>::operator ()) return minus<fraction>()(p->value, q->value);
				if (o == (bfmp) & divides<floating>::operator ()) return divides<fraction>()(p->value, q->value);
				if (o == (bfmp) & multiplies<floating>::operator ()) return multiplies<fraction>()(p->value, q->value);

				return (binary_function<T, T, T>().*o)(p->value, q->value);
			}
			if (type_t<floating> const * q = dynamic_cast<type_t<floating> const *>(reinterpret_cast<type const *>(& b)))
			{
				if (floor(q->value) == q->value)
				{
					if (o == (bfmp) & plus<floating>::operator ()) return plus<fraction>()(p->value, q->value);
					if (o == (bfmp) & minus<floating>::operator ()) return minus<fraction>()(p->value, q->value);
					if (o == (bfmp) & divides<floating>::operator ()) return divides<fraction>()(p->value, q->value);
					if (o == (bfmp) & multiplies<floating>::operator ()) return multiplies<fraction>()(p->value, q->value);
				}

				return (binary_function<T, T, T>().*o)(p->value, q->value);
			}
		}
		if (type_t<floating> const * p = dynamic_cast<type_t<floating> const *>(reinterpret_cast<type const *>(& a)))
		{
			if (type_t<integer> const * q = dynamic_cast<type_t<integer> const *>(reinterpret_cast<type const *>(& b)))
			{
				return (binary_function<T, T, T>().*o)(p->value, q->value);
			}
			if (type_t<fraction> const * q = dynamic_cast<type_t<fraction> const *>(reinterpret_cast<type const *>(& b)))
			{
				if (floor(p->value) == p->value)
				{
					if (o == (bfmp) & plus<floating>::operator ()) return plus<fraction>()(p->value, q->value);
					if (o == (bfmp) & minus<floating>::operator ()) return minus<fraction>()(p->value, q->value);
					if (o == (bfmp) & divides<floating>::operator ()) return divides<fraction>()(p->value, q->value);
					if (o == (bfmp) & multiplies<floating>::operator ()) return multiplies<fraction>()(p->value, q->value);
				}

				return (binary_function<T, T, T>().*o)(p->value, q->value);
			}
			if (type_t<floating> const * q = dynamic_cast<type_t<floating> const *>(reinterpret_cast<type const *>(& b)))
			{
				return (binary_function<T, T, T>().*o)(p->value, q->value);
			}
		}

		return string("Internal Fault");
	}


template
	normed fdi::apply<real>(real (unary_function<real, real>::*)(real const &) const, normed const &);

template
	normed fdi::apply<normed>(normed (unary_function<normed, normed>::*)(normed const &) const, normed const &);

template
	normed fdi::apply<integer>(integer (unary_function<integer, integer>::*)(integer const &) const, normed const &);

template
	normed fdi::apply<floating>(floating (unary_function<floating, floating>::*)(floating const &) const, normed const &);

// template
// 	normed fdi::apply<fraction>(fraction (unary_function<fraction, fraction>::*)(fraction const &) const, normed const &);


template
	normed fdi::apply<real>(real (binary_function<real, real, real>::*)(real const &, real const &) const, normed const &, normed const &);

template
	normed fdi::apply<normed>(normed (binary_function<normed, normed, normed>::*)(normed const &, normed const &) const, normed const &, normed const &);

template
	normed fdi::apply<integer>(integer (binary_function<integer, integer, integer>::*)(integer const &, integer const &) const, normed const &, normed const &);

template
	normed fdi::apply<floating>(floating (binary_function<floating, floating, floating>::*)(floating const &, floating const &) const, normed const &, normed const &);

// template
// 	normed fdi::apply<fraction>(fraction (binary_function<fraction, fraction, fraction>::*)(fraction const &, fraction const &) const, normed const &, normed const &);
