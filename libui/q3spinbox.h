/**
    Copyright 2019 Fornux Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef Q3SPINBOX_H
#define Q3SPINBOX_H

#ifndef QT_H
#include "qwidget.h"
#include "q3rangecontrol.h"
#endif // QT_H

#ifndef QT_NO_SPINBOX

class QLineEdit;
class QValidator;
class Q3SpinBoxPrivate;

class /*Q_EXPORT*/ Q3SpinBox: public QWidget, public Q3RangeControl
{
    Q_OBJECT
    Q_ENUMS( ButtonSymbols )
    Q_PROPERTY( QString text READ text )
    Q_PROPERTY( QString prefix READ prefix WRITE setPrefix )
    Q_PROPERTY( QString suffix READ suffix WRITE setSuffix )
    Q_PROPERTY( QString cleanText READ cleanText )
    Q_PROPERTY( QString specialValueText READ specialValueText WRITE setSpecialValueText )
    Q_PROPERTY( bool wrapping READ wrapping WRITE setWrapping )
    Q_PROPERTY( ButtonSymbols buttonSymbols READ buttonSymbols WRITE setButtonSymbols )
    Q_PROPERTY( int maxValue READ maxValue WRITE setMaxValue )
    Q_PROPERTY( int minValue READ minValue WRITE setMinValue )
    Q_PROPERTY( int lineStep READ lineStep WRITE setLineStep )
    Q_PROPERTY( int value READ value WRITE setValue )

public:
    Q3SpinBox( QWidget* parent=0, const char* name=0 );
    Q3SpinBox( int minValue, int maxValue, int step = 1,
	      QWidget* parent=0, const char* name=0 );
    ~Q3SpinBox();

    QString		text() const;

    virtual QString	prefix() const;
    virtual QString	suffix() const;
    virtual QString	cleanText() const;

    virtual void	setSpecialValueText( const QString &text );
    QString		specialValueText() const;

    virtual void	setWrapping( bool on );
    bool		wrapping() const;

    enum ButtonSymbols { UpDownArrows, PlusMinus };
    virtual void	setButtonSymbols( ButtonSymbols );
    ButtonSymbols	buttonSymbols() const;

    virtual void	setValidator( const QValidator* v );
    const QValidator * validator() const;

    QSize		sizeHint() const;
    QSize		minimumSizeHint() const;

    int	 minValue() const;
    int	 maxValue() const;
    void setMinValue( int );
    void setMaxValue( int );
    int	 lineStep() const;
    void setLineStep( int );
    int  value() const;

    QRect		upRect() const;
    QRect		downRect() const;

public slots:
    virtual void	setValue( int value );
    virtual void	setPrefix( const QString &text );
    virtual void	setSuffix( const QString &text );
    virtual void	stepUp();
    virtual void	stepDown();
    virtual void 	setEnabled( bool enabled );
    virtual void 	selectAll();

signals:
    void		valueChanged( int value );
    void		valueChanged( const QString &valueText );

protected:
    virtual QString	mapValueToText( int value );
    virtual int		mapTextToValue( bool* ok );
    QString		currentValueText();

    virtual void	updateDisplay();
    virtual void	interpretText();

    QLineEdit*		editor() const;

    virtual void	valueChange();
    virtual void	rangeChange();

    bool		eventFilter( QObject* obj, QEvent* ev );
    void		resizeEvent( QResizeEvent* ev );
#ifndef QT_NO_WHEELEVENT
    void		wheelEvent( QWheelEvent * );
#endif
    void		leaveEvent( QEvent* );

    void		styleChange( QStyle& );

protected slots:
    void		textChanged();

private:
    void initSpinBox();
    Q3SpinBoxPrivate* d;
    QLineEdit* vi;
    QValidator* validate;
    QString pfix;
    QString sfix;
    QString specText;

    uint wrap		: 1;
    uint edited		: 1;

    void arrangeWidgets();

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    Q3SpinBox( const Q3SpinBox& );
    Q3SpinBox& operator=( const Q3SpinBox& );
#endif

};

#endif // QT_NO_SPINBOX

#endif // QSPINBOX_H
