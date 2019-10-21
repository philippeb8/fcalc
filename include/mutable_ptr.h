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


#ifndef MUTABLE_PTR_H
#define MUTABLE_PTR_H


namespace fdi
{


/**
	Flexible version of STL auto_ptr<>.

	This class doesn't use reference counts, thus is the fastest one.  Be
	carefull because the compiler won't warn you about implicit invalid
	pointers it may generate.  It is beneficial when you want to perform
	simple operations on your containers of this type like to copy, to add,
	to remove, etc.
*/

template <typename T>
	struct mutable_ptr
	{
        typedef T elementType;


        mutable_ptr(elementType * a_p = 0) : m_ptr(a_p)
		{
		}

        mutable_ptr(mutable_ptr<T> const & a_p) : m_ptr(a_p.release())
		{
		}

        template <typename U>
            mutable_ptr(mutable_ptr<U> const & a_p) : m_ptr(a_p.release())
			{
			}

        mutable_ptr & operator = (mutable_ptr<T> const & a_p)
		{
			if (& a_p != this)
			{
				reset(a_p.release());
			}
			return * this;
		}

        template <typename U>
            mutable_ptr & operator = (mutable_ptr<U> const & a_p)
			{
				if (a_p.get() != this->get())
				{
					reset(a_p.release());
				}
				return * this;
			}

		~mutable_ptr()
		{
			delete m_ptr;
		}

        elementType & operator * () const
		{
			return * m_ptr;
		}

        elementType * operator -> () const
		{
			return m_ptr;
		}

        elementType * get() const
		{
			return m_ptr;
		}

        elementType * release() const
		{
            elementType * tmp = m_ptr;
			m_ptr = 0;
			return tmp;
		}

        void reset(elementType * a_p = 0)
		{
			delete m_ptr;
			m_ptr = a_p;
		}


	private:
        mutable elementType * m_ptr;
	};


}


#endif
