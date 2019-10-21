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


#ifndef PORTABILITY_H
#define PORTABILITY_H

#include <cmath>

typedef long double real;

#ifdef _WIN32 
#include <time.h>

inline real floorl(real x) { return std::floor(x); }

inline real powl(real d, real x) { return std::pow(d, x); }

inline int isnan(real x) { return x != x; }

inline int isinf(real x) { return !isnan(x) && isnan(x - x); }

inline real rintl(real x) { return floorl(x + 0.5); }

inline void sleep(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}
#endif

inline real roundl(real x, int d) { return floor(x * pow(10L, d) + 0.5) / pow(10L, d); }

#endif

#ifdef _WIN32 
#undef max
#undef min
#endif
