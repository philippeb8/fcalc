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


#ifndef __DISPATCH_H
#define __DISPATCH_H


#include "calculus.h"
#include <algorithm>
#include <string>
#include <sstream>
#include <iomanip>


namespace fdi
{


using namespace std;


template <class _Type1, class _Type2>
	struct dispatch : public unary_function<_Type1, _Type2>
	{
		_Type2 operator()(_Type1 const & t) const
		{
			return result_type(t);
		}
	};

template <class _Type>
	struct dispatch<_Type, _Type> : public unary_function<_Type, _Type>
	{
		_Type & operator()(_Type & t) const
		{
			return t;
		}

		_Type const & operator()(_Type const & t) const
		{
			return t;
		}
	};


// Typenames:
template <class _Type>
	struct dispatch<char *, _Type> : public unary_function<char *, _Type>
	{
		_Type operator()(char * const & t) const
		{
			_Type result;
			istringstream u(t);
			u >> result;
			return result;
		}
	};


// Class string:
template <class _Type>
	struct dispatch<string, _Type> : public unary_function<string, _Type>
	{
		_Type operator()(string const & t) const
		{
			_Type result;
			istringstream u(t);
			u >> result;
			return result;
		}
		_Type operator()(string const & t, bool & b) const
		{
			_Type result;
			istringstream u(t);
			u >> result;
			b = ! u.fail();
			return result;
		}
	};

template <>
	struct dispatch<string, char const *> : public unary_function<string, char const *>
	{
		char const * operator()(string const & t) const
		{
			return t.c_str();
		}
	};

template <class _Type>
	struct dispatch<_Type, string> : public unary_function<_Type, string>
	{
		string operator()(_Type const & t, int p = 24) const
		{
			ostringstream result;
			result << setprecision(p) << t;
			return result.str();
		}
	};

template <>
	struct dispatch<char *, string> : public unary_function<char *, string>
	{
		string operator()(char * const & t) const
		{
			return string(t);
		}
	};

template <>
	struct dispatch<char *, integer> : public unary_function<char *, integer>
	{
		integer operator()(char * t) const
		{
			integer result;
			istringstream u(t);
			return u >> result, result;
		}

		integer bin(char * t) const
		{
			integer result = 0;

			for (unsigned int i = 0; i < string(t).size() && i < sizeof(integer) * 8; ++ i)
			{
				result <<= 1;
				result += t[i] - '0';
			}

			return result;
		}

		integer oct(char * t) const
		{
			integer result = 0;

			for (unsigned int i = 0; i < string(t).size() && i < sizeof(integer) * 8 / 3 + 1; ++ i)
			{
				result <<= 3;
				result += t[i] - '0';
			}

			return result;
		}

		integer dec(char * t) const
		{
			integer result;
			istringstream(t) >> std::dec >> result;
			return result;
		}

		integer hex(char * t) const
		{
			integer result = 0;

			for (unsigned int i = 0; i < string(t).size() && i < sizeof(integer) * 8 / 4; ++ i)
			{
				int n;
				string s = string("") + t[i];

				istringstream(s) >> std::hex >> n;

				result <<= 4;
				result += n;
			}

			return result;
		}
	};

template <>
	struct dispatch<integer, string> : public unary_function<integer, string>
	{
		string operator()(integer const & t) const
		{
			ostringstream result;
			result << t;
			return result.str();
		}

		string bin(integer const & t) const
		{
			ostringstream result;

			int i = sizeof(integer) * 8;

			for (; i > 1; -- i)
			{
				if (1ULL << (i - 1) & t) break;
			}

			for (; i > 0; -- i)
			{
				result << (1ULL << (i - 1) & t ? '1' : '0');
			}

			result;

			return result.str();
		}

		string oct(integer const & t) const
		{
			ostringstream result;
			result << std::oct << t;
			return result.str();
		}

		string dec(integer const & t) const
		{
			ostringstream result;
			result << std::dec << t;
			return result.str();
		}

		string hex(integer const & t) const
		{
			ostringstream result;
			result << std::uppercase << std::hex << t;
			return result.str();
		}
	};


}


#endif
