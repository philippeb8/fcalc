/**
    Copyright 2019 Fornux Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef SPINBOXUI_H
#define SPINBOXUI_H


#include <q3spinbox.h>



typedef long double real;


class QTimer;


class SpinBoxUI : public Q3SpinBox
{
	Q_OBJECT

	bool bRecursive;
	real nValue, nMinValue, nMaxValue;

	int nCounter;
	QTimer * pTimer;

public:
	SpinBoxUI(QWidget * p, const char * n = 0);

	real value() const;
	real minValue() const;
	real maxValue() const;

	void setMinimum(real);
	void setMaximum(real);

signals:
	void valueChanged(real);

protected:
	virtual void updateDisplay();

public slots:
	virtual void setValue(real);
	virtual void stepUp();
	virtual void stepDown();

protected slots:
	virtual void interpretText();
	void slotTimeout();
};


inline real SpinBoxUI::value() const
{
	return nValue;
}

inline real SpinBoxUI::minValue() const
{
	return nMinValue;
}

inline real SpinBoxUI::maxValue() const
{
	return nMaxValue;
}


#endif
