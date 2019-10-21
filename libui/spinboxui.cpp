/**
    Copyright 2019 Fornux Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "spinboxui.h"

#include <qtimer.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <limits>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <QDoubleValidator>

//#include "qd.h"

#include "portability.h"


using namespace std;


SpinBoxUI::SpinBoxUI(QWidget * p, const char * n) : Q3SpinBox(p, n), bRecursive(false)
{
	pTimer = new QTimer(this);

	connect(pTimer, SIGNAL(timeout()), SLOT(slotTimeout()));

	nValue = 0.0;
	nCounter = 0;

	nMinValue = - numeric_limits<real>::max();
	nMaxValue = numeric_limits<real>::max();

	setValidator(new QDoubleValidator(this));

	Q3SpinBox::setMinValue(-100);
	Q3SpinBox::setMaxValue(100);

	updateDisplay();
}


void SpinBoxUI::updateDisplay()
{
	if (! bRecursive)
	{
		// Stream the value out now
		ostringstream o;

		o << nValue;

		// Stop possible recursions
		bRecursive = true;

		// Enable / disable buttons
		if (nValue <= nMinValue)
		{
			Q3SpinBox::setValue(Q3SpinBox::minValue());
		}
		else if (nValue >= nMaxValue)
		{
			Q3SpinBox::setValue(Q3SpinBox::maxValue());
		}
		else
		{
			Q3SpinBox::setValue(0);
		}

		// Refresh value and buttons
		editor()->setUpdatesEnabled(false);
		Q3SpinBox::updateDisplay();

		editor()->setText(QString::fromStdString(o.str()));
		editor()->setUpdatesEnabled(true);
        editor()->repaint();

		emit valueChanged(nValue);

		// Unstop recursions
		bRecursive = false;
	}
}


void SpinBoxUI::setValue(real n)
{
	if (nValue < nMinValue)
	{
		nValue = nMinValue;
	}
	else if (nValue > nMaxValue)
	{
		nValue = nMaxValue;
	}
	else
	{
		nValue = n;
	}

	updateDisplay();
}

void SpinBoxUI::setMinimum(real v)
{
	nMinValue = v;

	if (nValue < nMinValue)
	{
		nValue = nMinValue;
		updateDisplay();
	}
}


void SpinBoxUI::setMaximum(real v)
{
	nMaxValue = v;

	if (nValue > nMaxValue)
	{
		nValue = nMaxValue;
		updateDisplay();
	}
}

void SpinBoxUI::interpretText()
{
	real v;

    istringstream o(text().toLatin1().data());

	o >> v;

	nValue = v;

	updateDisplay();
}

void SpinBoxUI::stepUp()
{
	pTimer->stop();

	interpretText();

	int const factor = ++ nCounter / 40;
    int const magnitude = nValue ? floor(log10(fabsl(nValue))) : 0;
    real const multiplicator = pow(10.0L, magnitude - 6 < 0 ? factor : factor + magnitude - 5) * (nCounter / 20 % 2 ? 5 : 1);

	if (multiplicator > nMaxValue - nValue)
	{
		nValue = nMaxValue;
	}
	else
	{
        nValue = (rint(nValue / multiplicator) + 1) * multiplicator;
	}

	updateDisplay();

    pTimer->setSingleShot(true);
    pTimer->start(250);
}

void SpinBoxUI::stepDown()
{
	pTimer->stop();

	interpretText();

	int const factor = ++ nCounter / 40;
    int const magnitude = nValue ? floor(log10(fabs(nValue))) : 0;
    real const multiplicator = pow(10.0L, magnitude - 6 < 0 ? factor : factor + magnitude - 5) * (nCounter / 20 % 2 ? 5 : 1);

	if (multiplicator > nValue - nMinValue)
	{
		nValue = nMinValue;
	}
	else
	{
        nValue = (rint(nValue / multiplicator) - 1) * multiplicator;
	}

	updateDisplay();

    pTimer->setSingleShot(true);
    pTimer->start(250);
}

void SpinBoxUI::slotTimeout()
{
	nCounter = 0;
}

