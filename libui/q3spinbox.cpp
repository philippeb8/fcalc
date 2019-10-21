/**
    Copyright 2019 Fornux Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "q3spinbox.h"
#ifndef QT_NO_SPINBOX

#include "qcursor.h"
#include "qpushbutton.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "qlineedit.h"
#include "qvalidator.h"
#include "qpixmapcache.h"
#include "qapplication.h"
#include "qstyle.h"
#include <QKeyEvent>
#include <QStyleOptionSpinBox>
#if defined(QT_ACCESSIBILITY_SUPPORT)
#include "qaccessible.h"
#endif

class Q3SpinBoxPrivate
{
public:
    Q3SpinBoxPrivate() {}
    Q3SpinWidget* controls;
    uint selreq	: 1;
};

class Q3SpinBoxValidator : public QIntValidator
{
public:
    Q3SpinBoxValidator( Q3SpinBox *sb, const char *name )
    : QIntValidator( sb ), spinBox( sb ) { }

    virtual State validate( QString& str, int& pos ) const;

private:
    Q3SpinBox *spinBox;
};

QValidator::State Q3SpinBoxValidator::validate( QString& str, int& pos ) const
{
    QString pref = spinBox->prefix();
    QString suff = spinBox->suffix();
    QString suffStriped = suff.simplified();
    uint overhead = pref.length() + suff.length();
    State state = Invalid;

    ((QIntValidator *) this)->setRange( spinBox->minValue(),
					spinBox->maxValue() );
    if ( overhead == 0 ) {
	state = QIntValidator::validate( str, pos );
    } else {
    bool stripedVersion = false;
	if ( str.length() >= overhead && str.startsWith(pref)
	     && (str.endsWith(suff)
		 || (stripedVersion = str.endsWith(suffStriped))) ) {
	    if ( stripedVersion )
		overhead = pref.length() + suffStriped.length();
	    QString core = str.mid( pref.length(), str.length() - overhead );
	    int corePos = pos - pref.length();
	    state = QIntValidator::validate( core, corePos );
	    pos = corePos + pref.length();
	    str.replace( pref.length(), str.length() - overhead, core );
	} else {
	    state = QIntValidator::validate( str, pos );
	    if ( state == Invalid ) {
        // simplified(), cf. Q3SpinBox::interpretText()
        QString special = spinBox->specialValueText().simplified();
        QString candidate = str.simplified();

		if ( special.startsWith(candidate) ) {
		    if ( candidate.length() == special.length() ) {
			state = Acceptable;
		    } else {
			state = Intermediate;
		    }
		}
	    }
	}
    }
    return state;
}

/*!
    \class Q3SpinBox
    \brief The Q3SpinBox class provides a spin box widget (spin button).

    \ingroup basic
    \mainclass

    Q3SpinBox allows the user to choose a value either by clicking the
    up/down buttons to increase/decrease the value currently displayed
    or by typing the value directly into the spin box. If the value is
    entered directly into the spin box, Enter (or Return) must be
    pressed to apply the new value. The value is usually an integer.

    Every time the value changes Q3SpinBox emits the valueChanged()
    signal. The current value can be fetched with value() and set
    with setValue().

    The spin box keeps the value within a numeric range, and to
    multiples of the lineStep() size (see Q3RangeControl for details).
    Clicking the up/down buttons or using the keyboard accelerator's
    up and down arrows will increase or decrease the current value in
    steps of size lineStep(). The minimum and maximum value and the
    step size can be set using one of the constructors, and can be
    changed later with setMinValue(), setMaxValue() and setLineStep().

    Most spin boxes are directional, but Q3SpinBox can also operate as
    a circular spin box, i.e. if the range is 0-99 and the current
    value is 99, clicking "up" will give 0. Use setWrapping() if you
    want circular behavior.

    The displayed value can be prepended and appended with arbitrary
    strings indicating, for example, currency or the unit of
    measurement. See setPrefix() and setSuffix(). The text in the spin
    box is retrieved with text() (which includes any prefix() and
    suffix()), or with cleanText() (which has no prefix(), no suffix()
    and no leading or trailing whitespace). currentValueText() returns
    the spin box's current value as text.

    Normally the spin box displays up and down arrows in the buttons.
    You can use setButtonSymbols() to change the display to show
    <b>+</b> and <b>-</b> symbols if you prefer. In either case the up
    and down arrow keys work as expected.

    It is often desirable to give the user a special (often default)
    choice in addition to the range of numeric values. See
    setSpecialValueText() for how to do this with Q3SpinBox.

    The default \l QWidget::focusPolicy() is StrongFocus.

    If using prefix(), suffix() and specialValueText() don't provide
    enough control, you can ignore them and subclass Q3SpinBox instead.

    Q3SpinBox can easily be subclassed to allow the user to input
    things other than an integer value as long as the allowed input
    can be mapped to a range of integers. This can be done by
    overriding the virtual functions mapValueToText() and
    mapTextToValue(), and setting another suitable validator using
    setValidator().

    For example, these functions could be changed so that the user
    provided values from 0.0 to 10.0, or -1 to signify 'Auto', while
    the range of integers used inside the program would be -1 to 100:

    \code
	class MySpinBox : public Q3SpinBox
	{
	    Q_OBJECT
	public:
	    ...

	    QString mapValueToText( int value )
	    {
		if ( value == -1 ) // special case
		    return QString( "Auto" );

		return QString( "%1.%2" ) // 0.0 to 10.0
		    .arg( value / 10 ).arg( value % 10 );
	    }

	    int mapTextToValue( bool *ok )
	    {
		if ( text() == "Auto" ) // special case
		    return -1;

		return (int) ( 10 * text().toFloat() ); // 0 to 100
	    }
	};
    \endcode

    <img src=qspinbox-m.png> <img src=qspinbox-w.png>

    \sa QScrollBar QSlider
    \link guibooks.html#fowler GUI Design Handbook: Spin Box \endlink
*/


/*!
    Constructs a spin box with the default Q3RangeControl range and
    step values. It is called \a name and has parent \a parent.

    \sa minValue(), maxValue(), setRange(), lineStep(), setSteps()
*/

Q3SpinBox::Q3SpinBox( QWidget * parent , const char *name )
    : QWidget( parent ),
      Q3RangeControl()
{
    initSpinBox();
}


/*!
    Constructs a spin box that allows values from \a minValue to \a
    maxValue inclusive, with step amount \a step. The value is
    initially set to \a minValue.

    The spin box is called \a name and has parent \a parent.

    \sa minValue(), maxValue(), setRange(), lineStep(), setSteps()
*/

Q3SpinBox::Q3SpinBox( int minValue, int maxValue, int step, QWidget* parent,
		    const char* name )
    : QWidget( parent ),
      Q3RangeControl( minValue, maxValue, step, step, minValue )
{
    initSpinBox();
}

/*
  \internal Initialization.
*/

void Q3SpinBox::initSpinBox()
{
    d = new Q3SpinBoxPrivate;

    d->controls = new Q3SpinWidget( this, "controls" );
    connect( d->controls, SIGNAL( stepUpPressed() ), SLOT( stepUp() ) );
    connect( d->controls, SIGNAL( stepDownPressed() ), SLOT( stepDown() ) );

    wrap = false;
    edited = false;
    d->selreq = false;

    validate = new Q3SpinBoxValidator( this, "validator" );
    vi = new QLineEdit( this );
    d->controls->setEditWidget( vi );
    vi->setValidator( validate );
    vi->installEventFilter( this );
    vi->setFrame( false );
    setFocusProxy( vi );

    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) );
    /**setBackgroundMode( Qt::PaletteBackground, Qt::PaletteBase );*/

    updateDisplay();

    connect( vi, SIGNAL(textChanged(const QString&)), SLOT(textChanged()) );
}

/*!
    Destroys the spin box, freeing all memory and other resources.
*/

Q3SpinBox::~Q3SpinBox()
{
    delete d;
}


/*!
    \property Q3SpinBox::text
    \brief the spin box's text, including any prefix() and suffix()

    There is no default text.

    \sa value()
*/

QString Q3SpinBox::text() const
{
    return vi->text();
}



/*!
    \property Q3SpinBox::cleanText
    \brief the spin box's text with no prefix(), suffix() or leading
    or trailing whitespace.

    \sa text, prefix, suffix
*/

QString Q3SpinBox::cleanText() const
{
    QString s = QString(text()).simplified();
    if ( !prefix().isEmpty() ) {
    QString px = QString(prefix()).simplified();
	int len = px.length();
	if ( len && s.left(len) == px )  // Remove _only_ if it is the prefix
	    s.remove( (uint)0, len );
    }
    if ( !suffix().isEmpty() ) {
    QString sx = QString(suffix()).simplified();
	int len = sx.length();
	if ( len && s.right(len) == sx )  // Remove _only_ if it is the suffix
	    s.truncate( s.length() - len );
    }
    return s.simplified();
}


/*!
    \property Q3SpinBox::specialValueText
    \brief the special-value text

    If set, the spin box will display this text instead of a numeric
    value whenever the current value is equal to minVal(). Typical use
    is to indicate that this choice has a special (default) meaning.

    For example, if your spin box allows the user to choose the margin
    width in a print dialog and your application is able to
    automatically choose a good margin width, you can set up the spin
    box like this:
    \code
	Q3SpinBox marginBox( -1, 20, 1, parent, "marginBox" );
	marginBox->setSuffix( " mm" );
	marginBox->setSpecialValueText( "Auto" );
    \endcode
    The user will then be able to choose a margin width from 0-20
    millimeters or select "Auto" to leave it to the application to
    choose. Your code must then interpret the spin box value of -1 as
    the user requesting automatic margin width.

    All values are displayed with the prefix() and suffix() (if set),
    \e except for the special value, which only shows the special
    value text.

    To turn off the special-value text display, call this function
    with an empty string. The default is no special-value text, i.e.
    the numeric value is shown as usual.

    If no special-value text is set, specialValueText() returns
    QString::null.
*/

void Q3SpinBox::setSpecialValueText( const QString &text )
{
    specText = text;
    updateDisplay();
}


QString Q3SpinBox::specialValueText() const
{
    if ( specText.isEmpty() )
	return QString::null;
    else
	return specText;
}


/*!
    \property Q3SpinBox::prefix
    \brief the spin box's prefix

    The prefix is prepended to the start of the displayed value.
    Typical use is to display a unit of measurement or a currency
    symbol. For example:

    \code
	sb->setPrefix( "$" );
    \endcode

    To turn off the prefix display, set this property to an empty
    string. The default is no prefix. The prefix is not displayed for
    the minValue() if specialValueText() is not empty.

    If no prefix is set, prefix() returns QString::null.

    \sa suffix()
*/

void Q3SpinBox::setPrefix( const QString &text )
{
    pfix = text;
    updateDisplay();
}


QString Q3SpinBox::prefix() const
{
    if ( pfix.isEmpty() )
	return QString::null;
    else
	return pfix;
}


/*!
    \property Q3SpinBox::suffix
    \brief the suffix of the spin box

    The suffix is appended to the end of the displayed value. Typical
    use is to display a unit of measurement or a currency symbol. For
    example:

    \code
	sb->setSuffix( " km" );
    \endcode

    To turn off the suffix display, set this property to an empty
    string. The default is no suffix. The suffix is not displayed for
    the minValue() if specialValueText() is not empty.

    If no suffix is set, suffix() returns a QString::null.

    \sa prefix()
*/

void Q3SpinBox::setSuffix( const QString &text )
{
    sfix = text;
    updateDisplay();
}

QString Q3SpinBox::suffix() const
{
    if ( sfix.isEmpty() )
	return QString::null;
    else
	return sfix;
}


/*!
    \property Q3SpinBox::wrapping
    \brief whether it is possible to step the value from the highest
    value to the lowest value and vice versa

    By default, wrapping is turned off.

    If you have a range of 0..100 and wrapping is off when the user
    reaches 100 and presses the Up Arrow nothing will happen; but if
    wrapping is on the value will change from 100 to 0, then to 1,
    etc. When wrapping is on, navigating past the highest value takes
    you to the lowest and vice versa.

    \sa minValue, maxValue, setRange()
*/

void Q3SpinBox::setWrapping( bool on )
{
    wrap = on;
    updateDisplay();
}

bool Q3SpinBox::wrapping() const
{
    return wrap;
}

/*!
    \reimp
*/
QSize Q3SpinBox::sizeHint() const
{
    /**constPolish();*/
    QSize sz = vi->sizeHint();
    int h = sz.height();
    QFontMetrics fm( font() );
    int w = 35;
    int wx = fm.width( ' ' )*2;
    QString s;
    s = prefix() + ( (Q3SpinBox*)this )->mapValueToText( minValue() ) + suffix();
    w = qMax( w, fm.width( s ) + wx);
    s = prefix() + ( (Q3SpinBox*)this )->mapValueToText( maxValue() ) + suffix();
    w = qMax(w, fm.width( s ) + wx );
    if ( !specialValueText().isEmpty() ) {
	s = specialValueText();
    w = qMax( w, fm.width( s ) + wx );
    }
	QStyleOptionSpinBox opt;
    return style()->sizeFromContents(QStyle::CT_SpinBox, & opt,
				    QSize( w + d->controls->downRect().width(),
					   h + style()->pixelMetric( QStyle::PM_DefaultFrameWidth ) * 2).
				    expandedTo( QApplication::globalStrut() ));
}


/*!
    \reimp
*/
QSize Q3SpinBox::minimumSizeHint() const
{
    int w = vi->minimumSizeHint().width() + d->controls->downRect().width();
    int h = qMax( vi->minimumSizeHint().height(), d->controls->minimumSizeHint().height() );
    return QSize( w, h );
}

// Does the layout of the lineedit and the buttons

void Q3SpinBox::arrangeWidgets()
{
    d->controls->arrange();
}

/*!
    \property Q3SpinBox::value
    \brief the value of the spin box

    \sa Q3RangeControl::setValue()
*/

void Q3SpinBox::setValue( int value )
{
    edited = false; // we ignore anything entered and not yet interpreted
    Q3RangeControl::setValue( value );
    updateDisplay();
}

int Q3SpinBox::value() const
{
    Q3SpinBox * that = (Q3SpinBox *) this;
    if ( edited ) {
    that->edited = false;  // avoid recursion
	that->interpretText();
    }
    return Q3RangeControl::value();
}


/*!
    Increases the spin box's value by one lineStep(), wrapping as
    necessary if wrapping() is true. This is the same as clicking on
    the pointing-up button and can be used for keyboard accelerators,
    for example.

    \sa stepDown(), addLine(), lineStep(), setSteps(), setValue(), value()
*/

void Q3SpinBox::stepUp()
{
    if ( edited )
	interpretText();
    if ( wrapping() && ( value()+lineStep() > maxValue() ) )
	setValue( minValue() );
    else
	addLine();
}


/*!
    Decreases the spin box's value one lineStep(), wrapping as
    necessary if wrapping() is true. This is the same as clicking on
    the pointing-down button and can be used for keyboard
    accelerators, for example.

    \sa stepUp(), subtractLine(), lineStep(), setSteps(), setValue(), value()
*/

void Q3SpinBox::stepDown()
{
    if ( edited )
	interpretText();
    if ( wrapping() && ( value()-lineStep() < minValue() ) )
	setValue( maxValue() );
    else
	subtractLine();
}


/*!
    \fn void Q3SpinBox::valueChanged( int value )

    This signal is emitted every time the value of the spin box
    changes; the new value is passed in \a value. This signal will be
    emitted as a result of a call to setValue(), or because the user
    changed the value by using a keyboard accelerator or mouse click,
    etc.

    Note that the valueChanged() signal is emitted \e every time, not
    just for the "last" step; i.e. if the user clicks "up" three
    times, this signal is emitted three times.

    \sa value()
*/


/*!
    \fn void Q3SpinBox::valueChanged( const QString& valueText )

    \overload

    This signal is emitted whenever the valueChanged( int ) signal is
    emitted, i.e. every time the value of the spin box changes
    (whatever the cause, e.g. by setValue(), by a keyboard
    accelerator, by mouse clicks, etc.).

    The \a valueText parameter is the same string that is displayed in
    the edit field of the spin box.

    \sa value() prefix() suffix() specialValueText()
*/



/*!
    Intercepts and handles the events coming to the embedded QLineEdit
    that have special meaning for the Q3SpinBox. The object is passed
    as \a o and the event is passed as \a ev.
*/

bool Q3SpinBox::eventFilter( QObject* o, QEvent* ev )
{
    if (o != vi)
	return QWidget::eventFilter(o,ev);

    if ( ev->type() == QEvent::KeyPress ) {
	QKeyEvent* k = (QKeyEvent*)ev;

    bool retval = false; // workaround for MSVC++ optimization bug
    if( (k->key() == Qt::Key_Tab) /**|| (k->key() == Qt::Key_BackTab)*/ ){
        if ( k->modifiers() & Qt::ControlModifier )
        return false;
	    if ( edited )
		interpretText();
	    qApp->sendEvent( this, ev );
        retval = true;
	} if ( k->key() == Qt::Key_Up ) {
	    stepUp();
        retval = true;
	} else if ( k->key() == Qt::Key_Down ) {
	    stepDown();
        retval = true;
	} else if ( k->key() == Qt::Key_Enter || k->key() == Qt::Key_Return ) {
	    interpretText();
        return false;
	}
	if ( retval )
	    return retval;
    } else if ( ev->type() == QEvent::FocusOut || ev->type() == QEvent::Hide ) {
	if ( edited ) {
	    interpretText();
	}
    return false;
    }
    return false;
}

/*!
    \reimp
 */
void Q3SpinBox::setEnabled( bool enabled )
{
    QWidget::setEnabled( enabled );
    updateDisplay();
}

/*!
    \reimp
*/
void Q3SpinBox::leaveEvent( QEvent* )
{
}


/*!
    \reimp
*/
void Q3SpinBox::resizeEvent( QResizeEvent* )
{
    d->controls->resize( width(), height() );
}

/*!
    \reimp
*/
#ifndef QT_NO_WHEELEVENT
void Q3SpinBox::wheelEvent( QWheelEvent * e )
{
    e->accept();
    static float offset = 0;
    static Q3SpinBox* offset_owner = 0;
    if (offset_owner != this) {
	offset_owner = this;
	offset = 0;
    }
    offset += -e->delta()/120;
    if (qAbs(offset) < 1)
	return;
    int ioff = int(offset);
    int i;
    for (i=0; i<qAbs(ioff); i++)
	offset > 0 ? stepDown() : stepUp();
    offset -= ioff;
}
#endif

/*!
    This virtual function is called by Q3RangeControl whenever the
    value has changed. The Q3SpinBox reimplementation updates the
    display and emits the valueChanged() signals; if you need
    additional processing, either reimplement this or connect to one
    of the valueChanged() signals.
*/

void Q3SpinBox::valueChange()
{
    d->selreq = hasFocus();
    updateDisplay();
    d->selreq = false;
    emit valueChanged( value() );
    emit valueChanged( currentValueText() );
#if defined(QT_ACCESSIBILITY_SUPPORT)
    QAccessible::updateAccessibility( this, 0, QAccessible::ValueChanged );
#endif
}


/*!
    This virtual function is called by Q3RangeControl whenever the
    range has changed. It adjusts the default validator and updates
    the display; if you need additional processing, you can
    reimplement this function.
*/

void Q3SpinBox::rangeChange()
{
    updateDisplay();
}


/*!
    Sets the validator to \a v. The validator controls what keyboard
    input is accepted when the user is editing in the value field. The
    default is to use a suitable QIntValidator.

    Use setValidator(0) to turn off input validation (entered input
    will still be kept within the spin box's range).
*/

void Q3SpinBox::setValidator( const QValidator* v )
{
    if ( vi )
	vi->setValidator( v );
}


/*!
    Returns the validator that constrains editing for this spin box if
    there is any; otherwise returns 0.

    \sa setValidator() QValidator
*/

const QValidator * Q3SpinBox::validator() const
{
    return vi ? vi->validator() : 0;
}

/*!
    Updates the contents of the embedded QLineEdit to reflect the
    current value using mapValueToText(). Also enables/disables the
    up/down push buttons accordingly.

    \sa mapValueToText()
*/
void Q3SpinBox::updateDisplay()
{
    vi->setUpdatesEnabled( false );
    vi->setText( currentValueText() );
    if ( d->selreq && isVisible() && ( hasFocus() || vi->hasFocus() ) ) {
	selectAll();
    } else {
	if ( !suffix().isEmpty() && vi->text().endsWith(suffix()) )
	     vi->setCursorPosition( vi->text().length() - suffix().length() );
    }
    vi->setUpdatesEnabled( true );
    vi->repaint(); // immediate repaint needed for some reason
    edited = false;

    bool upEnabled = isEnabled() && ( wrapping() || value() < maxValue() );
    bool downEnabled = isEnabled() && ( wrapping() || value() > minValue() );

    d->controls->setUpEnabled( upEnabled );
    d->controls->setDownEnabled( downEnabled );
    vi->setEnabled( isEnabled() );
    repaint();
}


/*!
    Q3SpinBox calls this after the user has manually edited the
    contents of the spin box (i.e. by typing in the embedded
    QLineEdit, rather than using the up/down buttons/keys).

    The default implementation of this function interprets the new
    text using mapTextToValue(). If mapTextToValue() is successful, it
    changes the spin box's value; if not, the value is left unchanged.

    \sa editor()
*/

void Q3SpinBox::interpretText()
{
    bool ok = true;
    bool done = false;
    int newVal = 0;
    if ( !specialValueText().isEmpty() ) {
    QString s = text().simplified();
    QString t = specialValueText().simplified();
	if ( s == t ) {
	    newVal = minValue();
        done = true;
	}
    }
    if ( !done )
	newVal = mapTextToValue( &ok );
    if ( ok )
	setValue( newVal );
    updateDisplay(); // sometimes redundant
}


/*!
    Returns the geometry of the "up" button.
*/

QRect Q3SpinBox::upRect() const
{
    return d->controls->upRect();
}


/*!
    Returns the geometry of the "down" button.
*/

QRect Q3SpinBox::downRect() const
{
    return d->controls->downRect();
}


/*!
    Returns a pointer to the embedded QLineEdit.
*/

QLineEdit* Q3SpinBox::editor() const
{
    return vi;
}


/*!
    This slot is called whenever the user edits the spin box's text.
*/

void Q3SpinBox::textChanged()
{
    edited = true; // this flag is cleared in updateDisplay()
}


/*!
    This virtual function is used by the spin box whenever it needs to
    display value \a v. The default implementation returns a string
    containing \a v printed in the standard way. Reimplementations may
    return anything. (See the example in the detailed description.)

    Note that Qt does not call this function for specialValueText()
    and that neither prefix() nor suffix() are included in the return
    value.

    If you reimplement this, you may also need to reimplement
    mapTextToValue().

    \sa updateDisplay(), mapTextToValue()
*/

QString Q3SpinBox::mapValueToText( int v )
{
    QString s;
    s.setNum( v );
    return s;
}


/*!
    This virtual function is used by the spin box whenever it needs to
    interpret text entered by the user as a value. The text is
    available as text() and as cleanText(), and this function must
    parse it if possible. If \a ok is not 0: if it parses the text
    successfully, \a *ok is set to true; otherwise \a *ok is set to
    false.

    Subclasses that need to display spin box values in a non-numeric
    way need to reimplement this function.

    Note that Qt handles specialValueText() separately; this function
    is only concerned with the other values.

    The default implementation tries to interpret the text() as an
    integer in the standard way and returns the integer value.

    \sa interpretText(), mapValueToText()
*/

int Q3SpinBox::mapTextToValue( bool* ok )
{
    QString s = text();
    int newVal = s.toInt( ok );
    if ( !(*ok) && !( prefix().isEmpty() && suffix().isEmpty() ) ) {// Try removing any pre/suffix
	s = cleanText();
	newVal = s.toInt( ok );
    }
    return newVal;
}


/*!
    Returns the full text calculated from the current value, including
    any prefix and suffix. If there is special value text and the
    value is minValue() the specialValueText() is returned.
*/

QString Q3SpinBox::currentValueText()
{
    QString s;
    if ( (value() == minValue()) && !specialValueText().isEmpty() ) {
	s = specialValueText();
    } else {
	s = prefix();
	s.append( mapValueToText( value() ) );
	s.append( suffix() );
    }
    return s;
}

/*!
    \reimp
*/

void Q3SpinBox::styleChange( QStyle& old )
{
    arrangeWidgets();
    /**QWidget::styleChange( old );*/
}


/*!
    \enum Q3SpinBox::ButtonSymbols

    This enum type determines what the buttons in a spin box show.

    \value UpDownArrows the buttons show little arrows in the classic
    style.

    \value PlusMinus the buttons show <b>+</b> and <b>-</b> symbols.

    \sa Q3SpinBox::buttonSymbols
*/

/*!
    \property Q3SpinBox::buttonSymbols

    \brief the current button symbol mode

    The possible values can be either \c UpDownArrows or \c PlusMinus.
    The default is \c UpDownArrows.

    \sa ButtonSymbols
*/

void Q3SpinBox::setButtonSymbols( ButtonSymbols newSymbols )
{
    if ( buttonSymbols() == newSymbols )
	return;

    switch ( newSymbols ) {
    case UpDownArrows:
	d->controls->setButtonSymbols( Q3SpinWidget::UpDownArrows );
	break;
    case PlusMinus:
	d->controls->setButtonSymbols( Q3SpinWidget::PlusMinus );
	break;
    }
    //    repaint( false );
}

Q3SpinBox::ButtonSymbols Q3SpinBox::buttonSymbols() const
{
    switch( d->controls->buttonSymbols() ) {
    case Q3SpinWidget::UpDownArrows:
	return UpDownArrows;
    case Q3SpinWidget::PlusMinus:
	return PlusMinus;
    }
    return UpDownArrows;
}

/*!
    \property Q3SpinBox::minValue

    \brief the minimum value of the spin box

    When setting this property, \l Q3SpinBox::maxValue is adjusted, if
    necessary, to ensure that the range remains valid.

    \sa setRange() setSpecialValueText()
*/

int Q3SpinBox::minValue() const
{
    return Q3RangeControl::minValue();
}

void Q3SpinBox::setMinValue( int minVal )
{
    Q3RangeControl::setMinValue( minVal );
}

/*!
    \property Q3SpinBox::maxValue
    \brief the maximum value of the spin box

    When setting this property, \l Q3SpinBox::minValue is adjusted, if
    necessary, to ensure that the range remains valid.

    \sa setRange() setSpecialValueText()
*/

int Q3SpinBox::maxValue() const
{
    return Q3RangeControl::maxValue();
}

void Q3SpinBox::setMaxValue( int maxVal )
{
    Q3RangeControl::setMaxValue( maxVal );
}

/*!
    \property Q3SpinBox::lineStep
    \brief the line step

    When the user uses the arrows to change the spin box's value the
    value will be incremented/decremented by the amount of the line
    step.

    The setLineStep() function calls the virtual stepChange() function
    if the new line step is different from the previous setting.

    \sa Q3RangeControl::setSteps() setRange()
*/

int Q3SpinBox::lineStep() const
{
    return Q3RangeControl::lineStep();
}

void Q3SpinBox::setLineStep( int i )
{
    setSteps( i, pageStep() );
}

/*!
    Selects all the text in the spin box's editor.
*/

void Q3SpinBox::selectAll()
{
    int overhead = prefix().length() + suffix().length();
    if ( !overhead || currentValueText() == specialValueText() ) {
	vi->selectAll();
    } else {
	vi->setSelection( prefix().length(), vi->text().length() - overhead );
    }
}

#endif
