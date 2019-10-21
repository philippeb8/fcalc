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


#ifndef TUPLE_H
#define TUPLE_H


namespace fdi
{


template <typename _T1, typename _T2 = void, typename _T3 = void, typename _T4 = void, typename _T5 = void, typename _T6 = void, typename _T7 = void, typename _T8 = void>
	class tuple;


template <typename _T1, typename _T2, typename _T3, typename _T4, typename _T5, typename _T6, typename _T7, typename _T8>
	class tuple
	{
	public:
		_T1 first;
		_T2 second;
		_T3 third;
		_T4 fourth;
		_T5 fifth;
		_T6 sixth;
		_T7 seventh;
		_T8 eighth;

		tuple(_T1 const & _t1 = _T1(), _T2 const & _t2 = _T2(), _T3 const & _t3 = _T3(), _T4 const & _t4 = _T4(), _T5 const & _t5 = _T5(), _T6 const & _t6 = _T6(), _T7 const & _t7 = _T7(), _T8 const & _t8 = _T8()): first(_t1), second(_t2), third(_t3), fourth(_t4), fifth(_t5), sixth(_t6), seventh(_t7), eighth(_t8)  {}
	};


template <typename _T1, typename _T2, typename _T3, typename _T4, typename _T5, typename _T6, typename _T7>
	class tuple<_T1, _T2, _T3, _T4, _T5, _T6, _T7, void>
	{
	public:
		_T1 first;
		_T2 second;
		_T3 third;
		_T4 fourth;
		_T5 fifth;
		_T6 sixth;
		_T7 seventh;

		tuple(_T1 const & _t1 = _T1(), _T2 const & _t2 = _T2(), _T3 const & _t3 = _T3(), _T4 const & _t4 = _T4(), _T5 const & _t5 = _T5(), _T6 const & _t6 = _T6(), _T7 const & _t7 = _T7()): first(_t1), second(_t2), third(_t3), fourth(_t4), fifth(_t5), sixth(_t6), seventh(_t7) {}
	};


template <typename _T1, typename _T2, typename _T3, typename _T4, typename _T5, typename _T6>
	class tuple<_T1, _T2, _T3, _T4, _T5, _T6, void, void>
	{
	public:
		_T1 first;
		_T2 second;
		_T3 third;
		_T4 fourth;
		_T5 fifth;
		_T6 sixth;

		tuple(_T1 const & _t1 = _T1(), _T2 const & _t2 = _T2(), _T3 const & _t3 = _T3(), _T4 const & _t4 = _T4(), _T5 const & _t5 = _T5(), _T6 const & _t6 = _T6()): first(_t1), second(_t2), third(_t3), fourth(_t4), fifth(_t5), sixth(_t6) {}
	};


template <typename _T1, typename _T2, typename _T3, typename _T4, typename _T5>
	class tuple<_T1, _T2, _T3, _T4, _T5, void, void, void>
	{
	public:
		_T1 first;
		_T2 second;
		_T3 third;
		_T4 fourth;
		_T5 fifth;

		tuple(_T1 const & _t1 = _T1(), _T2 const & _t2 = _T2(), _T3 const & _t3 = _T3(), _T4 const & _t4 = _T4(), _T5 const & _t5 = _T5()): first(_t1), second(_t2), third(_t3), fourth(_t4), fifth(_t5) {}
	};


template <typename _T1, typename _T2, typename _T3, typename _T4>
	class tuple<_T1, _T2, _T3, _T4, void, void, void, void>
	{
	public:
		_T1 first;
		_T2 second;
		_T3 third;
		_T4 fourth;

		tuple(_T1 const & _t1 = _T1(), _T2 const & _t2 = _T2(), _T3 const & _t3 = _T3(), _T4 const & _t4 = _T4()): first(_t1), second(_t2), third(_t3), fourth(_t4) {}
	};


template <typename _T1, typename _T2, typename _T3>
	class tuple<_T1, _T2, _T3, void, void, void, void, void>
	{
	public:
		_T1 first;
		_T2 second;
		_T3 third;

		tuple(_T1 const & _t1 = _T1(), _T2 const & _t2 = _T2(), _T3 const & _t3 = _T3()): first(_t1), second(_t2), third(_t3) {}
	};


template <typename _T1, typename _T2>
	class tuple<_T1, _T2, void, void, void, void, void, void>
	{
	public:
		_T1 first;
		_T2 second;

		tuple(_T1 const & _t1 = _T1(), _T2 const & _t2 = _T2()): first(_t1), second(_t2) {}
	};


template <typename _T1>
	class tuple<_T1, void, void, void, void, void, void, void>
	{
	public:
		_T1 first;

		tuple(_T1 const & _t1 = _T1()): first(_t1) {}
	};


}


#endif
