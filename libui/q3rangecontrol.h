/**
    Copyright 2019 Fornux Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef QRANGECONTROL_H
#define QRANGECONTROL_H

#ifndef QT_H
#include "qglobal.h"
#include "qframe.h"
#endif // QT_H

#ifndef QT_NO_RANGECONTROL


class Q3RangeControlPrivate;


class /*Q_EXPORT*/ Q3RangeControl
{
public:
    Q3RangeControl();
    Q3RangeControl( int minValue, int maxValue,
		   int lineStep, int pageStep, int value );
    virtual ~Q3RangeControl();
    int		value()		const;
    void	setValue( int );
    void	addPage();
    void	subtractPage();
    void	addLine();
    void	subtractLine();

    int		minValue()	const;
    int		maxValue()	const;
    void	setRange( int minValue, int maxValue );
    void	setMinValue( int minVal );
    void	setMaxValue( int minVal );

    int		lineStep()	const;
    int		pageStep()	const;
    void	setSteps( int line, int page );

    int		bound( int ) const;

protected:
    int		positionFromValue( int val, int space ) const;
    int		valueFromPosition( int pos, int space ) const;
    void	directSetValue( int val );
    int		prevValue()	const;

    virtual void valueChange();
    virtual void rangeChange();
    virtual void stepChange();

private:
    int		minVal, maxVal;
    int		line, page;
    int		val, prevVal;

    Q3RangeControlPrivate * d;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    Q3RangeControl( const Q3RangeControl & );
    Q3RangeControl &operator=( const Q3RangeControl & );
#endif
};


inline int Q3RangeControl::value() const
{ return val; }

inline int Q3RangeControl::prevValue() const
{ return prevVal; }

inline int Q3RangeControl::minValue() const
{ return minVal; }

inline int Q3RangeControl::maxValue() const
{ return maxVal; }

inline int Q3RangeControl::lineStep() const
{ return line; }

inline int Q3RangeControl::pageStep() const
{ return page; }


#endif // QT_NO_RANGECONTROL

#ifndef QT_NO_SPINWIDGET

class Q3SpinWidgetPrivate;
class /*Q_EXPORT*/ Q3SpinWidget : public QWidget
{
    Q_OBJECT
public:
    Q3SpinWidget( QWidget* parent=0, const char* name=0 ) : QWidget(parent) {}
    ~Q3SpinWidget() {}

    void 	setEditWidget( QWidget * widget ) {}
    QWidget * 	editWidget() {}

    QRect upRect() const {}
    QRect downRect() const {}

    void setUpEnabled( bool on ) {}
    void setDownEnabled( bool on ) {}

    bool isUpEnabled() const {}
    bool isDownEnabled() const {}

    enum ButtonSymbols { UpDownArrows, PlusMinus };
    virtual void	setButtonSymbols( ButtonSymbols bs ) {}
    ButtonSymbols	buttonSymbols() const {}

    void arrange() {}

signals:
    void stepUpPressed();
    void stepDownPressed();

public slots:
    void stepUp() {}
    void stepDown() {}
    
protected:
    void mousePressEvent( QMouseEvent *e ) {}
    void resizeEvent( QResizeEvent* ev ) {}
    void mouseReleaseEvent( QMouseEvent *e ) {}
    void mouseMoveEvent( QMouseEvent *e ) {}
#ifndef QT_NO_WHEELEVENT
    void wheelEvent( QWheelEvent * ) {}
#endif
    void styleChange( QStyle& ) {}
    void paintEvent( QPaintEvent * ) {}
    void enableChanged( bool old ) {}
    void windowActivationChange( bool ) {}

private slots:
    void timerDone() {}
    void timerDoneEx() {}

private:
    Q3SpinWidgetPrivate * d;

    void updateDisplay() {}

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    Q3SpinWidget( const Q3SpinWidget& );
    Q3SpinWidget& operator=( const Q3SpinWidget& );
#endif
};

#endif // QT_NO_SPINWIDGET

#endif // QRANGECONTROL_H
