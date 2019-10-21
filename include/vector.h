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


#ifndef VECTOR_H
#define VECTOR_H




#include <vector>
#include <iostream>
#include <functional>
#include "qd.h"




namespace fdi
{




/**
	Algebric vector.

	Vector that defines basic operations.
*/

template <typename T>
	class vector : public std::vector<T>
	{
	public:
		vector(int = 0);
		vector(vector const &);
		vector(int, vector const &);
		~vector();

		template <typename> friend vector operator - (vector const &);
		template <typename> friend vector operator + (vector const &, vector const &);
		template <typename> friend vector operator - (vector const &, vector const &);
		template <typename> friend vector operator * (vector const &, vector const &);
		template <typename> friend vector operator / (vector const &, vector const &);

		template <typename> friend std::ostream & operator << (std::ostream &, vector const &);
	};

template <typename T>
	inline vector<T>::vector(int a) : std::vector<T>(a)
	{
	}

template <typename T>
	inline vector<T>::vector(vector const & a) : std::vector<T>(a)
	{
	}

template <typename T>
	inline vector<T>::vector(int a, vector const & b) : std::vector<T>(a, b)
	{
	}

template <typename T>
	inline vector<T>::~vector()
	{
	}

template <typename T>
	inline vector<T> operator - (vector<T> const & a)
	{
		return a.apply(std::negate<T>());
	}

template <typename T>
	inline vector<T> operator + (vector<T> const & a, vector<T> const & b)
	{
		return a.apply(b, std::plus<T>());
	}

template <typename T>
	inline vector<T> operator - (vector<T> const & a, vector<T> const & b)
	{
		return a.apply(b, std::minus<T>());
	}

template <typename T>
	inline vector<T> operator * (vector<T> const & a, vector<T> const & b)
	{
		return a.apply(b, std::multiplies<T>());
	}

template <typename T>
	inline vector<T> operator / (vector<T> const & a, vector<T> const & b)
	{
		return a.apply(b, std::divides<T>());
	}

template <typename T>
	inline std::ostream & operator << (std::ostream & o, vector<T> const & a)
	{
		o << "[";

		for (typename vector<T>::size_type i = 0; i < a.size(); ++ i)
		{
            if (i)
                if (o.flags() & qd_real::showmagnitude)
                    o << "; ";
                else
                    o << ", ";

			o << a[i];
		}

		o << "]";

		return o;
	}




}




#endif
