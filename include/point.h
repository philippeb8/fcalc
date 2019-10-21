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


#ifndef POINT_H
#define POINT_H


#include "portability.h"


template <int, typename = real>
	struct point;


template <typename T>
	struct point<2, T>
	{
		T x, y;

		point() {}
		point(point const & v) : x(v.x), y(v.y) {}
		point(T const & i, T const & j) : x(i), y(j) {}

		point operator / (T const & n) const { return point(x / n, y / n); }
		point operator + (point const & v) const { return point(x + v.x, y + v.y); }
		point operator - (point const & v) const { return point(x - v.x, y - v.y); }
		point & operator += (point const & v) { x += v.x, y += v.y; return * this; }
		point & operator -= (point const & v) { x -= v.x, y -= v.y; return * this; }

		point operator * (point const & v) const { return point(x * v.y, - y * v.x); }

		T dot(point const & v) const { return x * v.x + y * v.y; }

		T length() const { return sqrt(x * x + y * y); }
	};


template <typename T>
	struct point<3, T>
	{
		T x, y, z;

		point() {}
		point(point const & v) : x(v.x), y(v.y), z(v.z) {}
		point(T const & i, T const & j, T const & k) : x(i), y(j), z(k) {}

		point operator / (T const & n) const { return point(x / n, y / n, z / n); }
		point operator + (point const & v) const { return point(x + v.x, y + v.y, z + v.z); }
		point operator - (point const & v) const { return point(x - v.x, y - v.y, z - v.z); }
		point & operator += (point const & v) { x += v.x, y += v.y, z += v.z; return * this; }
		point & operator -= (point const & v) { x -= v.x, y -= v.y, z -= v.z; return * this; }

		point operator * (point const & v) const { return point((y * v.z - z * v.y), - (x * v.z - z * v.x), (x * v.y - y * v.x)); }

		T dot(point const & v) const { return x * v.x + y * v.y + z * v.z; }

		T length() const { return sqrt(x * x + y * y + z * z); }
	};

#endif

