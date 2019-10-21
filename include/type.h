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


#ifndef TYPE_H
#define TYPE_H


#include "mutable_ptr.h"


namespace fdi
{


struct type;


/**
	Run-time type information on complex typenames.
*/

struct type
{
	int counter;

	virtual bool is_typename() { return false; }
	virtual bool is_const() { return false; }
	virtual bool is_volatile() { return false; }
	virtual bool is_reference() { return false; }
	virtual bool is_array() { return false; }
	virtual bool is_pointer() { return false; }
	virtual bool is_member_pointer() { return false; }
	virtual bool is_function(int const = 0) { return false; }

	virtual int size() { return 0; }
	virtual type * ptr_t() { return 0; }
	virtual type * class_t() { return 0; }
	virtual type * return_t() { return 0; }
	virtual type * param_t(int const) { return 0; }

	virtual ~type() {}
};


/**
	Type information.
*/

template <typename _Type>
	struct info_t : type
	{
		virtual bool is_typename() { return true; }
	};

// Modifier
template <typename _Type>
	struct info_t<_Type const> : info_t<_Type>
	{
		virtual bool is_const() { return true; }
	};

template <typename _Type>
	struct info_t<_Type volatile> : info_t<_Type>
	{
		virtual bool is_volatile() { return true; }
	};

template <typename _Type>
	struct info_t<_Type const volatile> : info_t<_Type>
	{
		virtual bool is_const() { return true; }
		virtual bool is_volatile() { return true; }
	};

// Reference
template <typename _Type>
	struct info_t<_Type &> : info_t<_Type>
	{
		virtual bool is_reference() { return true; }
	};

// Array
template <typename _Type>
	struct info_t<_Type []> : type
	{
		virtual bool is_array() { return true; }

		virtual type * ptr_t() { static info_t<_Type> _type; return & _type; }
	};

template <typename _Type, int _Size>
	struct info_t<_Type [_Size]> : type
	{
		virtual bool is_array() { return true; }

		virtual int size() { return _Size; }
		virtual type * ptr_t() { static info_t<_Type> _type; return & _type; }
	};

// Pointer
template <typename _Type>
	struct info_t<_Type *> : type
	{
		virtual bool is_pointer() { return true; }

		virtual type * ptr_t() { static info_t<_Type> _type; return & _type; }
	};

template <typename _Type, class _Class>
	struct info_t<_Type _Class::*> : type
	{
		virtual bool is_pointer() { return true; }
		virtual bool is_member_pointer() { return true; }

		virtual type * ptr_t() { static info_t<_Type> _type; return & _type; }
		virtual type * class_t() { static info_t<_Class> _class; return & _class; }
	};

// Function
template <typename _Type>
	struct info_t<_Type ()> : type
	{
		virtual bool is_function(int const p) { return p > 0 ? false : true; }

		virtual type * return_t() { static info_t<_Type> _type; return & _type; }
	};

// Unary function
template <typename _Type, typename _P1>
	struct info_t<_Type (_P1)> : type
	{
		virtual bool is_function(int const p) { return p > 1 ? false : true; }

		virtual type * return_t() { static info_t<_Type> _type; return & _type; }
		virtual type * param_t(int const p)
		{
			static info_t<_P1> _p1;

			switch (p)
			{
			case 0: return & _p1;
			}

			return 0;
		}
	};

// Binary function
template <typename _Type, typename _P1, typename _P2>
	struct info_t<_Type (_P1, _P2)> : type
	{
		virtual bool is_function(int const p) { return p > 2 ? false : true; }

		virtual type * return_t() { static info_t<_Type> _type; return & _type; }
		virtual type * param_t(int const p)
		{
			static info_t<_P1> _p1;
			static info_t<_P2> _p2;

			switch (p)
			{
			case 0: return & _p1;
			case 1: return & _p2;
			}

			return 0;
		}
	};

// Tertiary function
template <typename _Type, typename _P1, typename _P2, typename _P3>
	struct info_t<_Type (_P1, _P2, _P3)> : type
	{
		virtual bool is_function(int const p) { return p > 3 ? false : true; }

		virtual type * return_t() { static info_t<_Type> _type; return & _type; }
		virtual type * param_t(int const p)
		{
			static info_t<_P1> _p1;
			static info_t<_P2> _p2;
			static info_t<_P3> _p3;

			switch (p)
			{
			case 0: return & _p1;
			case 1: return & _p2;
			case 2: return & _p3;
			}

			return 0;
		}
	};

// ...


/**
	Typenames classified.
*/

template <typename _Type>
	struct type_t : info_t<_Type>
	{
		_Type value;

		type_t() {}
		type_t(_Type const & t) : info_t<_Type>(), value(t) {}
		type_t(type_t const & t) : info_t<_Type>(), value(t.value) {}

		static void copy(_Type const & source, _Type & dest) { dest = source; }
	};

template <>
	struct type_t<void> : info_t<void>
	{
		type_t() {}
	};

template <typename _Type>
	struct type_t<_Type &> : info_t<_Type &>
	{
		_Type value;

		type_t() {}
		type_t(_Type const t) : info_t<_Type &>(), value(t) {}
		type_t(type_t const & t) : info_t<_Type &>(), value(t.value) {}

		static void copy(_Type const source, _Type dest) { dest = source; }
	};

template <typename _Type, int _Size>
	struct type_t<_Type [_Size]> : info_t<_Type [_Size]>
	{
		typedef _Type _t[_Size];

		_t value;

		type_t() {}
		type_t(_t const t) : info_t<_Type [_Size]>()
		{
			type_t<_t>::copy(t, value);
		}
		type_t(type_t const & t) : info_t<_Type [_Size]>()
		{
			type_t<_t>::copy(t.value, value);
		}

		static void copy(_t const source, _t & dest)
		{
			for (int i = 0; i < _Size; ++ i)
			{
				type_t<_Type>::copy(source[i], dest[i]);
			}
		}
	};


/**
	Reference count version of mutable_ptr<>.
*/

template <>
	struct mutable_ptr<type>
	{
		typedef type element_type;


		mutable_ptr() : m_ptr(0)
		{
		}

		mutable_ptr(element_type * a_p) : m_ptr(a_p)
		{
			if (m_ptr) 
				m_ptr->counter = 1;
		}

		mutable_ptr(mutable_ptr<type> const & a_p) : m_ptr(a_p.share())
		{
		}

		mutable_ptr & operator = (mutable_ptr<type> const & a_p)
		{
			if (& a_p != this)
			{
				m_ptr = a_p.share();
			}
			return * this;
		}

		~mutable_ptr()
		{
			if (m_ptr) 
				if (! -- m_ptr->counter) 
					delete m_ptr;
		}

		element_type & operator * () const
		{
			return * m_ptr;
		}

		element_type * operator -> () const
		{
			return m_ptr;
		}

		element_type * get() const
		{
			return m_ptr;
		}

		element_type * share() const
		{
			if (m_ptr) 
				++ m_ptr->counter;
			return 
				m_ptr;
		}


	private:
		mutable element_type * m_ptr;
	};


}


#endif

