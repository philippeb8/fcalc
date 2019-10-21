/**
    Copyright 2019 Fornux Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "qglobal.h"
#if defined(Q_CC_BOR)
// needed for qsort() because of a std namespace problem on Borland
#include "qplatformdefs.h"
#endif

#include "matrixui.h"


#include <qpainter.h>
#include <qlineedit.h>
#include <qcursor.h>
#include <qapplication.h>
#include <qtimer.h>
#include <qicon.h>
#include <QListView>
#include <qstyleoption.h>
#include <qcheckbox.h>
#include <QMimeData>
#include <qevent.h>
#include <QListView>
#include <qstyle.h>
#include <QDataWidgetMapper>
#include <qvalidator.h>
#include <QAbstractButton>
//! altered
#include <qtooltip.h>
#include <qtoolbutton.h>
#include <qbitarray.h>
#include <QComboBox>
#include <QScrollBar>

#define NO_LINE_WIDGET

#include <stdlib.h>
#include <limits.h>

QT_BEGIN_NAMESPACE

using namespace Qt;

MatrixUI::MatrixUI(QWidget *parent, const char *name)
    : QTableWidget(parent)
{
}

#if 0
class QHeaderViewData
{
public:
    QHeaderViewData(int n)
    {
        count = n;
        sizes.resize(n);
        positions.resize(n);
        labels.resize(n);
        nullStringLabels.resize(n);
        icons.resize(n);
        i2s.resize(n);
        s2i.resize(n);
        clicks.resize(n);
        resize.resize(n);
        int p =0;
        for (int i = 0; i < n; i ++) {
            sizes[i] = 88;
            i2s[i] = i;
            s2i[i] = i;
            positions[i] = p;
            p += sizes[i];
        }
        clicks_default = true;
        resize_default = true;
        clicks.fill(clicks_default);
        resize.fill(resize_default);
        move = true;
        sortSection = -1;
        sortDirection = true;
        positionsDirty = true;
        lastPos = 0;
        fullSize = -2;
        pos_dirty = false;
        is_a_table_header = false;
        focusIdx = 0;
    }
    ~QHeaderViewData()
    {
        for (int i = 0; i < icons.size(); ++i)
            delete icons.at(i);
    }


    QVector<int>        sizes;
    int height; // we abuse the heights as widths for vertical layout
    bool heightDirty;
    QVector<int>        positions; // sorted by index
    QVector<QString>        labels;
    QVector<QIcon *> icons;
    QVector<int>                i2s;
    QVector<int>                s2i;

    QBitArray           clicks;
    QBitArray           resize;
    QBitArray           nullStringLabels;
    uint move : 1;
    uint clicks_default : 1; // default value for new clicks bits
    uint resize_default : 1; // default value for new resize bits
    uint pos_dirty : 1;
    uint is_a_table_header : 1;
    bool sortDirection;
    bool positionsDirty;
    int sortSection;
    int count;
    int lastPos;
    int fullSize;
    int focusIdx;
    int pressDelta;

    int sectionAt(int pos) {
        // positions is sorted by index, not by section
        if (!count)
            return -1;
        int l = 0;
        int r = count - 1;
        int i = ((l+r+1) / 2);
        while (r - l) {
            if (positions[i] > pos)
                r = i -1;
            else
                l = i;
            i = ((l+r+1) / 2);
        }
        if (positions[i] <= pos && pos <= positions[i] + sizes[i2s[i]])
            return i2s[i];
        return -1;
    }
};

static QStyleOptionHeader getStyleOption(const QHeaderView *header, int section)
{
    QStyleOptionHeader opt;
    opt.init(header);
    opt.section = section;
    opt.textAlignment = Qt::AlignVCenter;
    opt.iconAlignment = Qt::AlignVCenter;
/**
    if (header->iconSet(section))
        opt.icon = *header->iconSet(section);
    opt.text = header->label(section);
*/
    if (header->orientation() == Qt::Horizontal)
        opt.state = QStyle::State_Horizontal;
    return opt;
}

bool qt_get_null_label_bit(QHeaderViewData *data, int section)
{
    return data->nullStringLabels.testBit(section);
}

void qt_set_null_label_bit(QHeaderViewData *data, int section, bool b)
{
    data->nullStringLabels.setBit(section, b);
}

static bool qt_update_cell_widget = true;
static bool qt_table_clipper_enabled = true;
#ifndef QT_INTERNAL_TABLE
/*Q_COMPAT_EXPORT*/
#endif
void qt_set_table_clipper_enabled(bool enabled)
{
    qt_table_clipper_enabled = enabled;
}

#ifdef _WS_QWS_
# define NO_LINE_WIDGET
#endif



struct MatrixPrivate
{
    MatrixPrivate() : hasRowSpan(false), hasColSpan(false),
                      inMenuMode(false), redirectMouseEvent(false)
    {
        //hiddenRows.setAutoDelete(true);
        //hiddenCols.setAutoDelete(true);
    }
    uint hasRowSpan : 1;
    uint hasColSpan : 1;
    uint inMenuMode : 1;
    uint redirectMouseEvent : 1;
    QHash<int, int> hiddenRows, hiddenCols;
    QTimer *geomTimer;
    int lastVisRow;
    int lastVisCol;

//! altered
    QLineEdit *e;
    QToolButton *b[4];
};

struct MatrixHeaderPrivate
{
#ifdef NO_LINE_WIDGET
    int oldLinePos;
#endif
};

static bool isRowSelection(MatrixUI::SelectionMode selMode)
{
    return selMode == MatrixUI::SingleRow || selMode == MatrixUI::MultiRow;
}

/*!
    \class MatrixSelection
    \brief The MatrixSelection class provides access to a selected area in a
    MatrixUI.

    \compat

    The selection is a rectangular set of cells in a MatrixUI. One of
    the rectangle's cells is called the anchor cell; this is the cell
    that was selected first. The init() function sets the anchor and
    the selection rectangle to exactly this cell; the expandTo()
    function expands the selection rectangle to include additional
    cells.

    There are various access functions to find out about the area:
    anchorRow() and anchorCol() return the anchor's position;
    leftCol(), rightCol(), topRow() and bottomRow() return the
    rectangle's four edges. All four are part of the selection.

    A newly created MatrixSelection is inactive -- isActive() returns
    false. You must use init() and expandTo() to activate it.

    \sa MatrixUI MatrixUI::addSelection() MatrixUI::selection()
    MatrixUI::selectCells() MatrixUI::selectRow() MatrixUI::selectColumn()
*/

/*!
    Creates an inactive selection. Use init() and expandTo() to
    activate it.
*/

MatrixSelection::MatrixSelection()
    : active(false), inited(false), tRow(-1), lCol(-1),
      bRow(-1), rCol(-1), aRow(-1), aCol(-1)
{
}

/*!
    Creates an active selection, starting at \a start_row and \a
    start_col, ending at \a end_row and \a end_col.
*/

MatrixSelection::MatrixSelection(int start_row, int start_col, int end_row, int end_col)
    : active(false), inited(false), tRow(-1), lCol(-1),
      bRow(-1), rCol(-1), aRow(-1), aCol(-1)
{
    init(start_row, start_col);
    expandTo(end_row, end_col);
}

/*!
    Sets the selection anchor to cell \a row, \a col and the selection
    to only contain this cell. The selection is not active until
    expandTo() is called.

    To extend the selection to include additional cells, call
    expandTo().

    \sa isActive()
*/

void MatrixSelection::init(int row, int col)
{
    aCol = lCol = rCol = col;
    aRow = tRow = bRow = row;
    active = false;
    inited = true;
}

/*!
    Expands the selection to include cell \a row, \a col. The new
    selection rectangle is the bounding rectangle of \a row, \a col
    and the previous selection rectangle. After calling this function
    the selection is active.

    If you haven't called init(), this function does nothing.

    \sa init() isActive()
*/

void MatrixSelection::expandTo(int row, int col)
{
    if (!inited)
        return;
    active = true;

    if (row < aRow) {
        tRow = row;
        bRow = aRow;
    } else {
        tRow = aRow;
        bRow = row;
    }

    if (col < aCol) {
        lCol = col;
        rCol = aCol;
    } else {
        lCol = aCol;
        rCol = col;
    }
}

/*!
    Returns true if \a s includes the same cells as the selection;
    otherwise returns false.
*/

bool MatrixSelection::operator==(const MatrixSelection &s) const
{
    return (s.active == active &&
             s.tRow == tRow && s.bRow == bRow &&
             s.lCol == lCol && s.rCol == rCol);
}

/*!
    \fn bool MatrixSelection::operator!=(const MatrixSelection &s) const

    Returns true if \a s does not include the same cells as the
    selection; otherwise returns false.
*/


/*!
    \fn int MatrixSelection::topRow() const

    Returns the top row of the selection.

    \sa bottomRow() leftCol() rightCol()
*/

/*!
    \fn int MatrixSelection::bottomRow() const

    Returns the bottom row of the selection.

    \sa topRow() leftCol() rightCol()
*/

/*!
    \fn int MatrixSelection::leftCol() const

    Returns the left column of the selection.

    \sa topRow() bottomRow() rightCol()
*/

/*!
    \fn int MatrixSelection::rightCol() const

    Returns the right column of the selection.

    \sa topRow() bottomRow() leftCol()
*/

/*!
    \fn int MatrixSelection::anchorRow() const

    Returns the anchor row of the selection.

    \sa anchorCol() expandTo()
*/

/*!
    \fn int MatrixSelection::anchorCol() const

    Returns the anchor column of the selection.

    \sa anchorRow() expandTo()
*/

/*!
    \fn int MatrixSelection::numRows() const

    Returns the number of rows in the selection.

    \sa numCols()
*/
int MatrixSelection::numRows() const
{
    return (tRow < 0) ? 0 : bRow - tRow + 1;
}

/*!
    Returns the number of columns in the selection.

    \sa numRows()
*/
int MatrixSelection::numCols() const
{
    return (lCol < 0) ? 0 : rCol - lCol + 1;
}

/*!
    \fn bool MatrixSelection::isActive() const

    Returns whether the selection is active or not. A selection is
    active after init() \e and expandTo() have been called.
*/

/*!
    \fn bool MatrixSelection::isEmpty() const

    Returns whether the selection is empty or not.

    \sa numRows(), numCols()
*/

/*!
    \class MatrixItem
    \brief The MatrixItem class provides the cell content for MatrixUI cells.

    \compat

    For many applications MatrixItems are ideal for presenting and
    editing the contents of MatrixUI cells. In situations where you need
    to create very large tables you may prefer an alternative approach
    to using MatrixItems: see the notes on large tables.

    A MatrixItem contains a cell's data, by default, a string and a
    pixmap. The table item also holds the cell's display size and how
    the data should be aligned. The table item specifies the cell's
    \l EditType and the editor used for in-place editing (by default a
    QLineEdit). If you want checkboxes use \l{CheckMatrixItem}, and if
    you want comboboxes use \l{ComboMatrixItem}. The \l EditType (set
    in the constructor) determines whether the cell's contents may be
    edited.

    If a pixmap is specified it is displayed to the left of any text.
    You can change the text or pixmap with setText() and setPixmap()
    respectively. For text you can use setWordWrap().

    When sorting table items the key() function is used; by default
    this returns the table item's text(). Reimplement key() to
    customize how your table items will sort.

    Table items are inserted into a table using MatrixUI::setItem(). If
    you insert an item into a cell that already contains a table item
    the original item will be deleted.

    Example:
    \snippet doc/src/snippets/code/src_qt3support_itemviews_q3table.cpp 0

    You can move a table item from one cell to another, in the same or
    a different table, using MatrixUI::takeItem() and MatrixUI::setItem()
    but see also MatrixUI::swapCells().

    Table items can be deleted with delete in the standard way; the
    table and cell will be updated accordingly.

    Note, that if you have a table item that is not currently in a table
    then anything you do to that item other than insert it into a table
    will result in undefined behaviour.

    Reimplement createEditor() and setContentFromEditor() if you want
    to use your own widget instead of a QLineEdit for editing cell
    contents. Reimplement paint() if you want to display custom
    content.

    It is important to ensure that your custom widget can accept the
    keyboard focus, so that the user can use the tab key to navigate the
    table as normal. Therefore, if the widget returned by createEditor()
    does not itself accept the keyboard focus, it is necessary to
    nominate a child widget to do so on its behalf. For example, a
    QHBox with two child QLineEdit widgets may use one of them to
    accept the keyboard focus:

    \snippet doc/src/snippets/code/src_qt3support_itemviews_q3table.cpp 1

    By default, table items may be replaced by new MatrixItems
    during the lifetime of a MatrixUI. Therefore, if you create your
    own subclass of MatrixItem, and you want to ensure that
    this does not happen, you must call setReplaceable(false)
    in the constructor of your subclass.

    \img qtableitems.png Table Items

    \sa CheckMatrixItem ComboMatrixItem

*/

/*!
    \fn MatrixUI *MatrixItem::table() const

    Returns the MatrixUI the table item belongs to.

    \sa MatrixUI::setItem() MatrixItem()
*/

/*!
    \enum MatrixItem::EditType

    \target wheneditable
    This enum is used to define whether a cell is editable or
    read-only (in conjunction with other settings), and how the cell
    should be displayed.

    \value Always
    The cell always \e looks editable.

    Using this EditType ensures that the editor created with
    createEditor() (by default a QLineEdit) is always visible. This
    has implications for the alignment of the content: the default
    editor aligns everything (even numbers) to the left whilst
    numerical values in the cell are by default aligned to the right.

    If a cell with the edit type \c Always looks misaligned you could
    reimplement createEditor() for these items.

    \value WhenCurrent
    The cell \e looks editable only when it has keyboard focus (see
    MatrixUI::setCurrentCell()).

    \value OnTyping
    The cell \e looks editable only when the user types in it or
    double-clicks it. It resembles the \c WhenCurrent functionality
    but is, perhaps, nicer.

    The \c OnTyping edit type is the default when MatrixItem objects
    are created by the convenience functions MatrixUI::setText() and
    MatrixUI::setPixmap().

    \value Never  The cell is not editable.

    The cell is actually editable only if MatrixUI::isRowReadOnly() is
    false for its row, MatrixUI::isColumnReadOnly() is false for its
    column, and MatrixUI::isReadOnly() is false.

    Q3ComboTableItems have an isEditable() property. This property is
    used to indicate whether the user may enter their own text or are
    restricted to choosing one of the choices in the list.
    Q3ComboTableItems may be interacted with only if they are editable
    in accordance with their EditType as described above.

*/

/*!
    Creates a table item that is a child of table \a table with no
    text. The item has the \l EditType \a et.

    The table item will use a QLineEdit for its editor, will not
    word-wrap and will occupy a single cell. Insert the table item
    into a table with MatrixUI::setItem().

    The table takes ownership of the table item, so a table item
    should not be inserted into more than one table at a time.
*/

/*!
    Creates a table item that is a child of table \a table with text
    \a text. The item has the \l EditType \a et.

    The table item will use a QLineEdit for its editor, will not
    word-wrap and will occupy a single cell. Insert the table item
    into a table with MatrixUI::setItem().

    The table takes ownership of the table item, so a table item
    should not be inserted into more than one table at a time.
*/

MatrixItem::MatrixItem(MatrixUI *table, EditType et, const QString &text)
    : txt(text), pix(), t(table), edType(et), wordwrap(false),
      tcha(true), rw(-1), cl(-1), rowspan(1), colspan(1)
{
    enabled = true;
}

/*!
    Creates a table item that is a child of table \a table with text
    \a text and pixmap \a p. The item has the \l EditType \a et.

    The table item will display the pixmap to the left of the text. It
    will use a QLineEdit for editing the text, will not word-wrap and
    will occupy a single cell. Insert the table item into a table with
    MatrixUI::setItem().

    The table takes ownership of the table item, so a table item
    should not be inserted in more than one table at a time.
*/

MatrixItem::MatrixItem(MatrixUI *table, EditType et,
                        const QString &text, const QPixmap &p)
    : txt(text), pix(p), t(table), edType(et), wordwrap(false),
      tcha(true), rw(-1), cl(-1), rowspan(1), colspan(1)
{
    enabled = true;
}

/*!
    The destructor deletes this item and frees all allocated
    resources.

    If the table item is in a table (i.e. was inserted with
    setItem()), it will be removed from the table and the cell it
    occupied.
*/

MatrixItem::~MatrixItem()
{
    if (table())
        table()->takeItem(this);
}

int MatrixItem::RTTI = 0;

/*!
    Returns the Run Time Type Identification value for this table item
    which for MatrixItems is 0.

    When you create subclasses based on MatrixItem make sure that each
    subclass returns a unique rtti() value. It is advisable to use
    values greater than 1000, preferably large random numbers, to
    allow for extensions to this class.

    \sa CheckMatrixItem::rtti() ComboMatrixItem::rtti()
*/

int MatrixItem::rtti() const
{
    return RTTI;
}

/*!
    Returns the table item's pixmap or a null pixmap if no pixmap has
    been set.

    \sa setPixmap() text()
*/

QPixmap MatrixItem::pixmap() const
{
    return pix;
}


/*!
    Returns the text of the table item or an empty string if there is
    no text.

    To ensure that the current value of the editor is returned,
    setContentFromEditor() is called:
    \list 1
    \i if the editMode() is \c Always, or
    \i if editMode() is \e not \c Always but the editor of the cell is
    active and the editor is not a QLineEdit.
    \endlist

    This means that text() returns the original text value of the item
    if the editor is a line edit, until the user commits an edit (e.g.
    by pressing Enter or Tab) in which case the new text is returned.
    For other editors (e.g. a combobox) setContentFromEditor() is
    always called so the currently display value is the one returned.

    \sa setText() pixmap()
*/

QString MatrixItem::text() const
{
    QWidget *w = table()->cellWidget(rw, cl);
    if (w && (edType == Always ||
                rtti() == ComboMatrixItem::RTTI ||
                rtti() == CheckMatrixItem::RTTI))
        ((MatrixItem*)this)->setContentFromEditor(w);
    return txt;
}

/*!
    Sets pixmap \a p to be this item's pixmap.

    Note that setPixmap() does not update the cell the table item
    belongs to. Use MatrixUI::updateCell() to repaint the cell's
    contents.

    For \l{ComboMatrixItem}s and \l{CheckMatrixItem}s this function
    has no visible effect.

    \sa MatrixUI::setPixmap() pixmap() setText()
*/

void MatrixItem::setPixmap(const QPixmap &p)
{
    pix = p;
}

/*!
    Changes the table item's text to \a str.

    Note that setText() does not update the cell the table item
    belongs to. Use MatrixUI::updateCell() to repaint the cell's
    contents.

    \sa MatrixUI::setText() text() setPixmap() MatrixUI::updateCell()
*/

void MatrixItem::setText(const QString &str)
{
    txt = str;
}

/*!
    This virtual function is used to paint the contents of an item
    using the painter \a p in the rectangular area \a cr using the
    color group \a cg.

    If \a selected is true the cell is displayed in a way that
    indicates that it is highlighted.

    You don't usually need to use this function but if you want to
    draw custom content in a cell you will need to reimplement it.

    The painter passed to this function is translated so that 0, 0
    is the top-left corner of the item that is being painted.

    Note that the painter is not clipped by default in order to get
    maximum efficiency. If you want clipping, use

    \snippet doc/src/snippets/code/src_qt3support_itemviews_q3table.cpp 2

*/

void MatrixItem::paint(QPainter *p, const QColorGroup &cg,
                        const QRect &cr, bool selected)
{
    p->fillRect(0, 0, cr.width(), cr.height(),
                 selected ? cg.brush(QColorGroup::Highlight)
                          : cg.brush(QColorGroup::Base));

    int w = cr.width();
    int h = cr.height();

    int x = 0;
    if (!pix.isNull()) {
        p->drawPixmap(0, (cr.height() - pix.height()) / 2, pix);
        x = pix.width() + 2;
    }

/**
    if (selected)
        p->setPen(cg.highlightedText());
    else
        p->setPen(cg.text());
*/
    p->drawText(x + 2, 0, w - x - 4, h,
                 wordwrap ? (alignment() | TextWordWrap) : alignment(), text());
}

/*!
This virtual function creates an editor which the user can
interact with to edit the cell's contents. The default
implementation creates a QLineEdit.

If the function returns 0, the cell is read-only.

The returned widget should preferably be invisible, ideally with
MatrixUI::viewport() as parent.

If you reimplement this function you'll almost certainly need to
reimplement setContentFromEditor(), and may need to reimplement
sizeHint().

\sa MatrixUI::createEditor() setContentFromEditor() MatrixUI::viewport() setReplaceable()
*/

QWidget *MatrixItem::createEditor() const
{
    QLineEdit *e = new QLineEdit("qt_tableeditor", table()->viewport());
    e->setFrame(false);
    e->setText(text());
    return e;
}

/*!
Whenever the content of a cell has been edited by the editor \a w,
MatrixUI calls this virtual function to copy the new values into the
MatrixItem.

If you reimplement createEditor() and return something that is not
a QLineEdit you will need to reimplement this function.

\sa MatrixUI::setCellContentFromEditor()
*/

void MatrixItem::setContentFromEditor(QWidget *w)
{
    QLineEdit *le = qobject_cast<QLineEdit*>(w);
    if (le) {
        QString input = le->text();
        if (le->validator())
            le->validator()->fixup(input);
        setText(input);
    }
}

/*!
    The alignment function returns how the text contents of the cell
    are aligned when drawn. The default implementation aligns numbers
    to the right and any other text to the left.

    \sa Qt::Alignment
*/

// ed: For consistency reasons a setAlignment() should be provided
// as well.

int MatrixItem::alignment() const
{
//! altered

/*
    bool num;
    bool ok1 = false, ok2 = false;
    (void)text().toInt(&ok1);
    if (!ok1)
        (void)text().toDouble(&ok2); // ### should be .-aligned
    num = ok1 || ok2;

    return (num ? AlignRight : AlignLeft) | AlignVCenter;
*/

    return table()->isColumnReadOnly(col()) ? AlignCenter : (AlignLeft | AlignVCenter);
}

/*!
    If \a b is true, the cell's text will be wrapped over multiple
    lines, when necessary, to fit the width of the cell; otherwise the
    text will be written as a single line.

    \sa wordWrap() MatrixUI::adjustColumn() MatrixUI::setColumnStretchable()
*/

void MatrixItem::setWordWrap(bool b)
{
    wordwrap = b;
}

/*!
    Returns true if word wrap is enabled for the cell; otherwise
    returns false.

    \sa setWordWrap()
*/

bool MatrixItem::wordWrap() const
{
    return wordwrap;
}

/*! \internal */

void MatrixItem::updateEditor(int oldRow, int oldCol)
{
    if (edType != Always)
        return;
    if (oldRow != -1 && oldCol != -1)
        table()->clearCellWidget(oldRow, oldCol);
    if (rw != -1 && cl != -1)
        table()->setCellWidget(rw, cl, createEditor());
}

/*!
    Returns the table item's edit type.

    This is set when the table item is constructed.

    \sa EditType MatrixItem()
*/

MatrixItem::EditType MatrixItem::editType() const
{
    return edType;
}

/*!
    If \a b is true it is acceptable to replace the contents of the
    cell with the contents of another MatrixItem. If \a b is false the
    contents of the cell may not be replaced by the contents of
    another table item. Table items that span more than one cell may
    not have their contents replaced by another table item.

    (This differs from \l EditType because EditType is concerned with
    whether the \e user is able to change the contents of a cell.)

    \sa isReplaceable()
*/

void MatrixItem::setReplaceable(bool b)
{
    tcha = b;
}

/*!
    This function returns whether the contents of the cell may be
    replaced with the contents of another table item. Regardless of
    this setting, table items that span more than one cell may not
    have their contents replaced by another table item.

    (This differs from \l EditType because EditType is concerned with
    whether the \e user is able to change the contents of a cell.)

    \sa setReplaceable() EditType
*/

bool MatrixItem::isReplaceable() const
{
    if (rowspan > 1 || colspan > 1)
        return false;
    return tcha;
}

/*!
    This virtual function returns the key that should be used for
    sorting. The default implementation returns the text() of the
    relevant item.

    \sa MatrixUI::setSorting()
*/

QString MatrixItem::key() const
{
    return text();
}

/*!
    This virtual function returns the size a cell needs to show its
    entire content.

    If you subclass MatrixItem you will often need to reimplement this
    function.
*/

QSize MatrixItem::sizeHint() const
{
    QSize strutSize = QApplication::globalStrut();
    if (edType == Always && table()->cellWidget(rw, cl))
        return table()->cellWidget(rw, cl)->sizeHint().expandedTo(strutSize);

    QSize s;
    int x = 0;
    if (!pix.isNull()) {
        s = pix.size();
        s.setWidth(s.width() + 2);
        x = pix.width() + 2;
    }

    QString t = text();
    if (!wordwrap && t.indexOf(QLatin1Char('\n')) == -1)
        return QSize(s.width() + table()->fontMetrics().width(text()) + 10,
                      qMax(s.height(), table()->fontMetrics().height())).expandedTo(strutSize);

    QRect r = table()->fontMetrics().boundingRect(x + 2, 0, table()->columnWidth(col()) - x - 4, 0,
                                                   wordwrap ? (alignment() | TextWordWrap) : alignment(),
                                                   text());
    r.setWidth(qMax(r.width() + 10, table()->columnWidth(col())));
    return QSize(r.width(), qMax(s.height(), r.height())).expandedTo(strutSize);
}

/*!
    Changes the extent of the MatrixItem so that it spans multiple
    cells covering \a rs rows and \a cs columns. The top left cell is
    the original cell.

    \warning This function only works if the item has already been
    inserted into the table using e.g. MatrixUI::setItem(). This
    function also checks to make sure if \a rs and \a cs are within
    the bounds of the table and returns without changing the span if
    they are not. In addition swapping, inserting or removing rows and
    columns that cross MatrixItems spanning more than one cell is not
    supported.

    \sa rowSpan() colSpan()
*/

void MatrixItem::setSpan(int rs, int cs)
{
    if (rs == rowspan && cs == colspan)
        return;

    if (!table()->d->hasRowSpan)
        table()->d->hasRowSpan = rs > 1;
    if (!table()->d->hasColSpan)
        table()->d->hasColSpan = cs > 1;
    // return if we are thinking too big...
    if (rw + rs > table()->numRows())
        return;

    if (cl + cs > table()->numCols())
        return;

    if (rw == -1 || cl == -1)
        return;

    int rrow = rw;
    int rcol = cl;
    if (rowspan > 1 || colspan > 1) {
        MatrixUI* t = table();
        t->takeItem(this);
        t->setItem(rrow, rcol, this);
    }

    rowspan = rs;
    colspan = cs;

    for (int r = 0; r < rowspan; ++r) {
        for (int c = 0; c < colspan; ++c) {
            if (r == 0 && c == 0)
                continue;
            qt_update_cell_widget = false;
            table()->setItem(r + rw, c + cl, this);
            qt_update_cell_widget = true;
            rw = rrow;
            cl = rcol;
        }
    }

    table()->updateCell(rw, cl);
    QWidget *w = table()->cellWidget(rw, cl);
    if (w)
        w->resize(table()->cellGeometry(rw, cl).size());
}

/*!
    Returns the row span of the table item, usually 1.

    \sa setSpan() colSpan()
*/

int MatrixItem::rowSpan() const
{
    return rowspan;
}

/*!
    Returns the column span of the table item, usually 1.

    \sa setSpan() rowSpan()
*/

int MatrixItem::colSpan() const
{
    return colspan;
}

/*!
    Sets row \a r as the table item's row. Usually you do not need to
    call this function.

    If the cell spans multiple rows, this function sets the top row
    and retains the height of the multi-cell table item.

    \sa row() setCol() rowSpan()
*/

void MatrixItem::setRow(int r)
{
    rw = r;
}

/*!
    Sets column \a c as the table item's column. Usually you will not
    need to call this function.

    If the cell spans multiple columns, this function sets the
    left-most column and retains the width of the multi-cell table
    item.

    \sa col() setRow() colSpan()
*/

void MatrixItem::setCol(int c)
{
    cl = c;
}

/*!
    Returns the row where the table item is located. If the cell spans
    multiple rows, this function returns the top-most row.

    \sa col() setRow()
*/

int MatrixItem::row() const
{
    return rw;
}

/*!
    Returns the column where the table item is located. If the cell
    spans multiple columns, this function returns the left-most
    column.

    \sa row() setCol()
*/

int MatrixItem::col() const
{
    return cl;
}

/*!
    If \a b is true, the table item is enabled; if \a b is false the
    table item is disabled.

    A disabled item doesn't respond to user interaction.

    \sa isEnabled()
*/

void MatrixItem::setEnabled(bool b)
{
    if (b == (bool)enabled)
        return;
    enabled = b;
    table()->updateCell(row(), col());
}

/*!
    Returns true if the table item is enabled; otherwise returns false.

    \sa setEnabled()
*/

bool MatrixItem::isEnabled() const
{
    return (bool)enabled;
}

/*!
    \class ComboMatrixItem
    \brief The ComboMatrixItem class provides a means of using
    comboboxes in Matrixs.

    \compat

    A ComboMatrixItem is a table item which looks and behaves like a
    combobox. The advantage of using Q3ComboTableItems rather than real
    comboboxes is that a ComboMatrixItem uses far less resources than
    real comboboxes in \l{MatrixUI}s. When the cell has the focus it
    displays a real combobox which the user can interact with. When
    the cell does not have the focus the cell \e looks like a
    combobox. Only text items (i.e. no pixmaps) may be used in
    Q3ComboTableItems.

    ComboMatrixItem items have the edit type \c WhenCurrent (see
    \l{EditType}). The ComboMatrixItem's list of items is provided by
    a QStringList passed to the constructor.

    The list of items may be changed using setStringList(). The
    current item can be set with setCurrentItem() and retrieved with
    currentItem(). The text of the current item can be obtained with
    currentText(), and the text of a particular item can be retrieved
    with text().

    If isEditable() is true the ComboMatrixItem will permit the user
    to either choose an existing list item, or create a new list item
    by entering their own text; otherwise the user may only choose one
    of the existing list items.

    To populate a table cell with a ComboMatrixItem use
    MatrixUI::setItem().

    Q3ComboTableItems may be deleted with MatrixUI::clearCell().

    Q3ComboTableItems can be distinguished from \l{MatrixItem}s and
    \l{CheckMatrixItem}s using their Run Time Type Identification
    number (see rtti()).

    \img qtableitems.png Table Items

    \sa CheckMatrixItem MatrixItem Q3ComboBox
*/

Q3ComboBox *ComboMatrixItem::fakeCombo = 0;
QWidget *ComboMatrixItem::fakeComboWidget = 0;
int ComboMatrixItem::fakeRef = 0;

/*!
    Creates a combo table item for the table \a table. The combobox's
    list of items is passed in the \a list argument. If \a editable is
    true the user may type in new list items; if \a editable is false
    the user may only select from the list of items provided.

    By default Q3ComboTableItems cannot be replaced by other table
    items since isReplaceable() returns false by default.

    \sa MatrixUI::clearCell() EditType
*/

ComboMatrixItem::ComboMatrixItem(MatrixUI *table, const QStringList &list, bool editable)
    : MatrixItem(table, WhenCurrent, QLatin1String("")), entries(list), current(0), edit(editable)
{
    setReplaceable(false);
    if (!ComboMatrixItem::fakeCombo) {
        ComboMatrixItem::fakeComboWidget = new QWidget(0, 0);
        ComboMatrixItem::fakeCombo = new Q3ComboBox(/**false, */ComboMatrixItem::fakeComboWidget/**, 0*/);
        ComboMatrixItem::fakeCombo->hide();
    }
    ++ComboMatrixItem::fakeRef;
    if (entries.count())
        setText(entries.at(current));
}

/*!
    ComboMatrixItem destructor.
*/
ComboMatrixItem::~ComboMatrixItem()
{
    if (--ComboMatrixItem::fakeRef <= 0) {
        delete ComboMatrixItem::fakeComboWidget;
        ComboMatrixItem::fakeComboWidget = 0;
        ComboMatrixItem::fakeCombo = 0;
    }
}

/*!
    Sets the list items of this ComboMatrixItem to the strings in the
    string list \a l.
*/

void ComboMatrixItem::setStringList(const QStringList &l)
{
    entries = l;
    current = 0;
    if (entries.count())
        setText(entries.at(current));
    if (table()->cellWidget(row(), col())) {
        cb->clear();
        cb->addItems(entries);
    }
    table()->updateCell(row(), col());
}

/*! \reimp */

QWidget *ComboMatrixItem::createEditor() const
{
    // create an editor - a combobox in our case
    ((ComboMatrixItem*)this)->cb = new Q3ComboBox(/**edit, */table()->viewport()/**, "qt_editor_cb"*/);
    cb->addItems(entries);
    cb->setCurrentIndex(current);
    QObject::connect(cb, SIGNAL(activated(int)), table(), SLOT(doValueChanged()));
    return cb;
}

/*! \reimp */

void ComboMatrixItem::setContentFromEditor(QWidget *w)
{
    Q3ComboBox *cb = qobject_cast<Q3ComboBox*>(w);
    if (cb) {
        entries.clear();
        for (int i = 0; i < cb->count(); ++i)
            entries << cb->itemText(i);
        current = cb->currentIndex();
        setText(cb->currentText());
    }
}

/*! \reimp */

void ComboMatrixItem::paint(QPainter *p, const QColorGroup &cg,
                           const QRect &cr, bool selected)
{
    fakeCombo->resize(cr.width(), cr.height());

    QPalette pal2(cg);
    if (selected) {
        pal2.setBrush(QPalette::Base, cg.QPalette::brush(QPalette::Highlight));
        pal2.setColor(QPalette::Text, cg.color(QPalette::Text));
    }

    QStyle::State flags = QStyle::State_None;
    if(isEnabled() && table()->isEnabled())
        flags |= QStyle::State_Enabled;
    // Since we still have the "fakeCombo" may as well use it in this case.
    QStyleOptionComboBox opt;
    opt.initFrom(table());
    opt.rect = fakeCombo->rect();
    opt.palette = pal2;
    opt.state &= ~QStyle::State_HasFocus;
    opt.state &= ~QStyle::State_MouseOver;
    opt.state |= flags;
    opt.subControls = QStyle::SC_All;
    opt.activeSubControls = QStyle::SC_None;
    opt.editable = fakeCombo->isEditable();
    table()->style()->drawComplexControl(QStyle::CC_ComboBox, &opt, p, fakeCombo);

    p->save();
    QRect textR = table()->style()->subControlRect(QStyle::CC_ComboBox, &opt,
                                                   QStyle::SC_ComboBoxEditField, fakeCombo);
    int align = alignment(); // alignment() changes entries
    p->drawText(textR, wordWrap() ? (align | Qt::TextWordWrap) : align, entries.value(current));
    p->restore();
}

/*!
    Sets the list item \a i to be the combo table item's current list
    item.

    \sa currentItem()
*/

void ComboMatrixItem::setCurrentItem(int i)
{
    QWidget *w = table()->cellWidget(row(), col());
    Q3ComboBox *cb = qobject_cast<Q3ComboBox*>(w);
    if (cb) {
        cb->setCurrentIndex(i);
        current = cb->currentIndex();
        setText(cb->currentText());
    } else {
        if (i < 0 || i >= entries.count())
            return;
        current = i;
        setText(entries.at(i));
        table()->updateCell(row(), col());
    }
}

/*!
    \overload

    Sets the list item whose text is \a s to be the combo table item's
    current list item. Does nothing if no list item has the text \a s.

    \sa currentItem()
*/

void ComboMatrixItem::setCurrentItem(const QString &s)
{
    int i = entries.indexOf(s);
    if (i != -1)
        setCurrentItem(i);
}

/*!
    Returns the index of the combo table item's current list item.

    \sa setCurrentItem()
*/

int ComboMatrixItem::currentItem() const
{
    QWidget *w = table()->cellWidget(row(), col());
    Q3ComboBox *cb = qobject_cast<Q3ComboBox*>(w);
    if (cb)
        return cb->currentIndex();
    return current;
}

/*!
    Returns the text of the combo table item's current list item.

    \sa currentItem() text()
*/

QString ComboMatrixItem::currentText() const
{
    QWidget *w = table()->cellWidget(row(), col());
    Q3ComboBox *cb = qobject_cast<Q3ComboBox*>(w);
    if (cb)
        return cb->currentText();
    return entries.value(current);
}

/*!
    Returns the total number of list items in the combo table item.
*/

int ComboMatrixItem::count() const
{
    QWidget *w = table()->cellWidget(row(), col());
    Q3ComboBox *cb = qobject_cast<Q3ComboBox*>(w);
    if (cb)
        return cb->count();
    return (int)entries.count();
}

/*!
    Returns the text of the combo's list item at index \a i.

    \sa currentText()
*/

QString ComboMatrixItem::text(int i) const
{
    QWidget *w = table()->cellWidget(row(), col());
    Q3ComboBox *cb = qobject_cast<Q3ComboBox*>(w);
    if (cb)
        return cb->itemText(i);
    return entries.value(i);
}

/*!
    If \a b is true the combo table item can be edited, i.e. the user
    may enter a new text item themselves. If \a b is false the user may
    may only choose one of the existing items.

    \sa isEditable()
*/

void ComboMatrixItem::setEditable(bool b)
{
    edit = b;
}

/*!
    Returns true if the user can add their own list items to the
    combobox's list of items; otherwise returns false.

    \sa setEditable()
*/

bool ComboMatrixItem::isEditable() const
{
    return edit;
}

int ComboMatrixItem::RTTI = 1;

/*!
    \fn int ComboMatrixItem::rtti() const

    Returns 1.

    Make your derived classes return their own values for rtti()to
    distinguish between different table item subclasses. You should
    use values greater than 1000, preferably a large random number, to
    allow for extensions to this class.


    \sa MatrixItem::rtti()
*/

int ComboMatrixItem::rtti() const
{
    return RTTI;
}

/*! \reimp */

QSize ComboMatrixItem::sizeHint() const
{
    fakeCombo->addItem(currentText());
    fakeCombo->setCurrentIndex(fakeCombo->count() - 1);
    QSize sh = fakeCombo->sizeHint();
    fakeCombo->removeItem(fakeCombo->count() - 1);
    return sh.expandedTo(QApplication::globalStrut());
}

/*!
    \fn QString ComboMatrixItem::text() const

    Returns the text of the table item or an empty string if there is
    no text.

    \sa MatrixItem::text()
*/

/*!
    \class CheckMatrixItem
    \brief The CheckMatrixItem class provides checkboxes in Matrixs.

    \compat

    A CheckMatrixItem is a table item which looks and behaves like a
    checkbox. The advantage of using Q3CheckTableItems rather than real
    checkboxes is that a CheckMatrixItem uses far less resources than
    a real checkbox would in a \l{MatrixUI}. When the cell has the focus
    it displays a real checkbox which the user can interact with. When
    the cell does not have the focus the cell \e looks like a
    checkbox. Pixmaps may not be used in Q3CheckTableItems.

    CheckMatrixItem items have the edit type \c WhenCurrent (see
    \l{EditType}).

    To change the checkbox's label use setText(). The checkbox can be
    checked and unchecked with setChecked() and its state retrieved
    using isChecked().

    To populate a table cell with a CheckMatrixItem use
    MatrixUI::setItem().

    Q3CheckTableItems can be distinguished from \l{MatrixItem}s and
    \l{ComboMatrixItem}s using their Run Time Type Identification
    (rtti) value.

    \img qtableitems.png Table Items

    \sa rtti() EditType ComboMatrixItem MatrixItem QCheckBox
*/

/*!
    Creates a CheckMatrixItem with an \l{EditType} of \c WhenCurrent
    as a child of \a table. The checkbox is initially unchecked and
    its label is set to the string \a txt.
*/

CheckMatrixItem::CheckMatrixItem(MatrixUI *table, const QString &txt)
    : MatrixItem(table, WhenCurrent, txt), checked(false)
{
}

/*! \reimp */

void CheckMatrixItem::setText(const QString &t)
{
    MatrixItem::setText(t);
    QWidget *w = table()->cellWidget(row(), col());
    QCheckBox *cb = qobject_cast<QCheckBox*>(w);
    if (cb)
        cb->setText(t);
}


/*! \reimp */

QWidget *CheckMatrixItem::createEditor() const
{
    // create an editor - a combobox in our case
    ((CheckMatrixItem*)this)->cb = new QCheckBox(table()->viewport()/**, "qt_editor_checkbox"*/);
    cb->setChecked(checked);
    cb->setText(text());
    /**cb->setBackgroundColor(table()->viewport()->backgroundColor());*/
    cb->setAutoFillBackground(true);
    QObject::connect(cb, SIGNAL(toggled(bool)), table(), SLOT(doValueChanged()));
    return cb;
}

/*! \reimp */

void CheckMatrixItem::setContentFromEditor(QWidget *w)
{
    QCheckBox *cb = qobject_cast<QCheckBox*>(w);
    if (cb)
        checked = cb->isChecked();
}

/*! \reimp */

void CheckMatrixItem::paint(QPainter *p, const QColorGroup &cg,
                                const QRect &cr, bool selected)
{
    QPalette pal = cg;

    p->fillRect(0, 0, cr.width(), cr.height(),
                 selected ? pal.brush(QPalette::Highlight)
                          : pal.brush(QPalette::Base));

    QSize sz = QSize(table()->style()->pixelMetric(QStyle::PM_IndicatorWidth),
                      table()->style()->pixelMetric(QStyle::PM_IndicatorHeight));
    QPalette pal2(pal);
    pal2.setBrush(QPalette::Window, pal.brush(QPalette::Base));
    QStyleOptionButton opt;
    opt.initFrom(table());
    opt.rect.setRect(0, (cr.height() - sz.height()) / 2, sz.width(), sz.height());
    opt.palette = pal2;
    opt.state &= ~QStyle::State_HasFocus;
    opt.state &= ~QStyle::State_MouseOver;
    if(isEnabled())
        opt.state |= QStyle::State_Enabled;
    if (checked)
        opt.state |= QStyle::State_On;
    else
        opt.state |= QStyle::State_Off;
    if (isEnabled() && table()->isEnabled())
        opt.state |= QStyle::State_Enabled;
    table()->style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &opt, p, table());
    if (selected)
        p->setPen(pal.highlightedText().color());
    else
        p->setPen(pal.text().color());
    opt.rect.setRect(0, 0, cr.width(), cr.height());
    QRect textRect = table()->style()->subElementRect(QStyle::SE_CheckBoxContents, &opt, table());
    p->drawText(textRect, wordWrap() ? (alignment() | Qt::TextWordWrap) : alignment(), text());
}

/*!
    If \a b is true the checkbox is checked; if \a b is false the
    checkbox is unchecked.

    \sa isChecked()
*/

void CheckMatrixItem::setChecked(bool b)
{
    checked = b;
    table()->updateCell(row(), col());
    QWidget *w = table()->cellWidget(row(), col());
    QCheckBox *cb = qobject_cast<QCheckBox*>(w);
    if (cb)
        cb->setChecked(b);
}

/*!
    Returns true if the checkbox table item is checked; otherwise
    returns false.

    \sa setChecked()
*/

bool CheckMatrixItem::isChecked() const
{
    // #### why was this next line here. It must not be here, as
    // #### people want to call isChecked() from within paintCell()
    // #### and end up in an infinite loop that way
    // table()->updateCell(row(), col());
    QWidget *w = table()->cellWidget(row(), col());
    QCheckBox *cb = qobject_cast<QCheckBox*>(w);
    if (cb)
        return cb->isChecked();
    return checked;
}

int CheckMatrixItem::RTTI = 2;

/*!
    \fn int CheckMatrixItem::rtti() const

    Returns 2.

    Make your derived classes return their own values for rtti()to
    distinguish between different table item subclasses. You should
    use values greater than 1000, preferably a large random number, to
    allow for extensions to this class.

    \sa MatrixItem::rtti()
*/

int CheckMatrixItem::rtti() const
{
    return RTTI;
}

/*! \reimp */

QSize CheckMatrixItem::sizeHint() const
{
    QSize sz = QSize(table()->style()->pixelMetric(QStyle::PM_IndicatorWidth),
                      table()->style()->pixelMetric(QStyle::PM_IndicatorHeight));
    sz.setWidth(sz.width() + 6);
    QSize sh(MatrixItem::sizeHint());
    return QSize(sh.width() + sz.width(), qMax(sh.height(), sz.height())).
        expandedTo(QApplication::globalStrut());
}

/*!
    \class MatrixUI
    \brief The MatrixUI class provides a flexible editable table widget.

    \compat

    MatrixUI is easy to use, although it does have a large API because
    of the comprehensive functionality that it provides. MatrixUI
    includes functions for manipulating \link #headers
    headers\endlink, \link #columnsrows rows and columns\endlink,
    \link #cells cells\endlink and \link #selections
    selections\endlink. MatrixUI also provides in-place editing and
    drag and drop, as well as a useful set of
    \link #signals signals\endlink. MatrixUI efficiently supports very
    large tables, for example, tables one million by one million cells
    are perfectly possible. MatrixUI is economical with memory, using
    none for unused cells.

    \snippet doc/src/snippets/code/src_qt3support_itemviews_q3table.cpp 3

    The first line constructs the table specifying its size in rows
    and columns. We then insert a pixmap and some text into the \e
    same \link #cells cell\endlink, with the pixmap appearing to the
    left of the text. MatrixUI cells can be populated with
    \l{MatrixItem}s, \l{ComboMatrixItem}s or by \l{CheckMatrixItem}s.
    By default a vertical header appears at the left of the table
    showing row numbers and a horizontal header appears at the top of
    the table showing column numbers. (The numbers displayed start at
    1, although row and column numbers within MatrixUI begin at 0.)

    If you want to use mouse tracking call setMouseTracking(true) on
    the \e viewport.

    \img qtableitems.png Table Items

    \target headers
    \section1 Headers

    MatrixUI supports a header column, e.g. to display row numbers, and
    a header row, e.g to display column titles. To set row or column
    labels use QHeaderView::setLabel() on the pointers returned by
    verticalHeader() and horizontalHeader() respectively. The vertical
    header is displayed within the table's left margin whose width is
    set with setcontentsMargins().left(). The horizontal header is displayed
    within the table's top margin whose height is set with
    setcontentsMargins().top(). The table's grid can be switched off with
    setShowGrid(). If you want to hide a horizontal header call
    hide(), and call setTopMargin(0) so that the area the header
    would have occupied is reduced to zero size.

    Header labels are indexed via their section numbers. Note that the
    default behavior of QHeaderView regarding section numbers is overridden
    for MatrixUI. See the explanation below in the Rows and Columns
    section in the discussion of moving columns and rows.

    \target columnsrows
    \section1 Rows and Columns

    Row and column sizes are set with setRowHeight() and
    setColumnWidth(). If you want a row high enough to show the
    tallest item in its entirety, use adjustRow(). Similarly, to make
    a column wide enough to show the widest item use adjustColumn().
    If you want the row height and column width to adjust
    automatically as the height and width of the table changes use
    setRowStretchable() and setColumnStretchable().

    Rows and columns can be hidden and shown with hideRow(),
    hideColumn(), showRow() and showColumn(). New rows and columns are
    inserted using insertRows() and insertColumns(). Additional rows
    and columns are added at the  bottom (rows) or right (columns) if
    you set setNumRows() or setNumCols() to be larger than numRows()
    or numCols(). Existing rows and columns are removed with
    removeRow() and removeColumn(). Multiple rows and columns can be
    removed with removeRows() and removeColumns().

    Rows and columns can be set to be movable using
    rowMovingEnabled() and columnMovingEnabled(). The user can drag
    them to reorder them holding down the Ctrl key and dragging the
    mouse. For performance reasons, the default behavior of QHeaderView
    section numbers is overridden by MatrixUI. Currently in MatrixUI, when
    a row or column is dragged and reordered, the section number is
    also changed to its new position. Therefore, there is no
    difference between the section and the index fields in QHeaderView.
    The MatrixUI QHeaderView classes do not provide a mechanism for indexing
    independently of the user interface ordering.

    The table can be sorted using sortColumn(). Users can sort a
    column by clicking its header if setSorting() is set to true. Rows
    can be swapped with swapRows(), columns with swapColumns() and
    cells with swapCells().

    For editable tables (see setReadOnly()) you can set the read-only
    property of individual rows and columns with setRowReadOnly() and
    setColumnReadOnly(). (Whether a cell is editable or read-only
    depends on these settings and the cell's MatrixItem.

    The row and column which have the focus are returned by
    currentRow() and currentColumn() respectively.

    Although many MatrixUI functions operate in terms of rows and
    columns the indexOf() function returns a single integer
    identifying a particular cell.

    \target cells
    \section1 Cells

    All of a MatrixUI's cells are empty when the table is constructed.

    There are two approaches to populating the table's cells. The
    first and simplest approach is to use MatrixItems or MatrixItem
    subclasses. The second approach doesn't use MatrixItems at all
    which is useful for very large sparse tables but requires you to
    reimplement a number of functions. We'll look at each approach in
    turn.

    To put a string in a cell use setText(). This function will create
    a new MatrixItem for the cell if one doesn't already exist, and
    displays the text in it. By default the table item's widget will
    be a QLineEdit. A pixmap may be put in a cell with setPixmap(),
    which also creates a table item if required. A cell may contain \e
    both a pixmap and text; the pixmap is displayed to the left of the
    text. Another approach is to construct a MatrixItem or MatrixItem
    subclass, set its properties, then insert it into a cell with
    setItem().

    If you want cells which contain comboboxes use the ComboMatrixItem
    class. Similarly if you require cells containing checkboxes use
    the CheckMatrixItem class. These table items look and behave just
    like the combobox or checkbox widgets but consume far less memory.

    MatrixUI takes ownership of its MatrixItems and will delete them
    when the table itself is destroyed. You can take ownership of a
    table item using takeItem() which you use to move a cell's
    contents from one cell to another, either within the same table,
    or from one table to another. (See also, swapCells()).

    In-place editing of the text in MatrixItems, and the values in
    Q3ComboTableItems and Q3CheckTableItems works automatically. Cells
    may be editable or read-only, see MatrixItem::EditType. If you
    want fine control over editing see beginEdit() and endEdit().

    The contents of a cell can be retrieved as a MatrixItem using
    item(), or as a string with text() or as a pixmap (if there is
    one) with pixmap(). A cell's bounding rectangle is given by
    cellGeometry(). Use updateCell() to repaint a cell, for example to
    clear away a cell's visual representation after it has been
    deleted with clearCell(). The table can be forced to scroll to
    show a particular cell with ensureCellVisible(). The isSelected()
    function indicates if a cell is selected.

    It is possible to use your own widget as a cell's widget using
    setCellWidget(), but subclassing MatrixItem might be a simpler
    approach. The cell's widget (if there is one) can be removed with
    clearCellWidget().

    \keyword notes on large tables
    \target bigtables
    \section2 Large tables

    For large, sparse, tables using MatrixItems or other widgets is
    inefficient. The solution is to \e draw the cell as it should
    appear and to create and destroy cell editors on demand.

    This approach requires that you reimplement various functions.
    Reimplement paintCell() to display your data, and createEditor()
    and setCellContentFromEditor() to support in-place editing. It
    is very important to reimplement resizeData() to have no
    functionality, to prevent MatrixUI from attempting to create a huge
    array. You will also need to reimplement item(), setItem(),
    takeItem(), clearCell(), and insertWidget(), cellWidget() and
    clearCellWidget(). In almost every circumstance (for sorting,
    removing and inserting columns and rows, etc.), you also need
    to reimplement swapRows(), swapCells() and swapColumns(), including
    header handling.

    If you represent active cells with a dictionary of MatrixItems and
    QWidgets, i.e. only store references to cells that are actually
    used, many of the functions can be implemented with a single line
    of code.

    For more information on cells see the MatrixItem documenation.

    \target selections
    \section1 Selections

    MatrixUI's support single selection, multi-selection (multiple
    cells) or no selection. The selection mode is set with
    setSelectionMode(). Use isSelected() to determine if a particular
    cell is selected, and isRowSelected() and isColumnSelected() to
    see if a row or column is selected.

    MatrixUI's support many simultaneous selections. You can
    programmatically select cells with addSelection(). The number of
    selections is given by numSelections(). The current selection is
    returned by currentSelection(). You can remove a selection with
    removeSelection() and remove all selections with
    clearSelection(). Selections are MatrixSelection objects.

    To easily add a new selection use selectCells(), selectRow() or
    selectColumn().

    Alternatively, use addSelection() to add new selections using
    MatrixSelection objects. The advantage of using MatrixSelection
    objects is that you can call MatrixSelection::expandTo() to resize
    the selection and can query and compare them.

    The number of selections is given by numSelections(). The current
    selection is returned by currentSelection(). You can remove a
    selection with removeSelection() and remove all selections with
    clearSelection().

    \target signals
    \section1 Signals

    When the user clicks a cell the currentChanged() signal is
    emitted. You can also connect to the lower level clicked(),
    doubleClicked() and pressed() signals. If the user changes the
    selection the selectionChanged() signal is emitted; similarly if
    the user changes a cell's value the valueChanged() signal is
    emitted. If the user right-clicks (or presses the appropriate
    platform-specific key sequence) the contextMenuRequested() signal
    is emitted. If the user drops a drag and drop object the dropped()
    signal is emitted with the drop event.
*/

/*!
    \fn void MatrixUI::currentChanged(int row, int col)

    This signal is emitted when the current cell has changed to \a
    row, \a col.
*/

/*!
    \fn void MatrixUI::valueChanged(int row, int col)

    This signal is emitted when the user changed the value in the cell
    at \a row, \a col.
*/

/*!
    \fn int MatrixUI::currentRow() const

    Returns the current row.

    \sa currentColumn()
*/

/*!
    \fn int MatrixUI::currentColumn() const

    Returns the current column.

    \sa currentRow()
*/

/*!
    \enum MatrixUI::EditMode

    \value NotEditing  No cell is currently being edited.

    \value Editing  A cell is currently being edited. The editor was
    initialised with the cell's contents.

    \value Replacing  A cell is currently being edited. The editor was
    not initialised with the cell's contents.
*/

/*!
    \enum MatrixUI::SelectionMode

    \value NoSelection No cell can be selected by the user.

    \value Single The user may only select a single range of cells.

    \value Multi The user may select multiple ranges of cells.

    \value SingleRow The user may select one row at once.

    \value MultiRow The user may select multiple rows.
*/

/*!
    \enum MatrixUI::FocusStyle

    Specifies how the current cell (focus cell) is drawn.

    \value FollowStyle The current cell is drawn according to the
    current style and the cell's background is also drawn selected, if
    the current cell is within a selection

    \value SpreadSheet The current cell is drawn as in a spreadsheet.
    This means, it is signified by a black rectangle around the cell,
    and the background of the current cell is always drawn with the
    widget's base color - even when selected.

*/

/*!
    \fn void MatrixUI::clicked(int row, int col, int button, const QPoint &mousePos)

    This signal is emitted when mouse button \a button is clicked. The
    cell where the event took place is at \a row, \a col, and the
    mouse's position is in \a mousePos.

    \sa Qt::MouseButton
*/

/*!
    \fn void MatrixUI::doubleClicked(int row, int col, int button, const QPoint &mousePos)

    This signal is emitted when mouse button \a button is
    double-clicked. The cell where the event took place is at \a row,
    \a col, and the mouse's position is in \a mousePos.

    \sa Qt::MouseButton
*/

/*!
    \fn void MatrixUI::pressed(int row, int col, int button, const QPoint &mousePos)

    This signal is emitted when mouse button \a button is pressed. The
    cell where the event took place is at \a row, \a col, and the
    mouse's position is in \a mousePos.

    \sa Qt::MouseButton
*/

/*!
    \fn void MatrixUI::selectionChanged()

    This signal is emitted whenever a selection changes.

    \sa MatrixSelection
*/

/*!
    \fn void MatrixUI::contextMenuRequested(int row, int col, const QPoint & pos)

    This signal is emitted when the user invokes a context menu with
    the right mouse button (or with a system-specific keypress). The
    cell where the event took place is at \a row, \a col. \a pos is
    the position where the context menu will appear in the global
    coordinate system. This signal is always emitted, even if the
    contents of the cell are disabled.
*/

/*!
    Creates an empty table object called \a name as a child of \a
    parent.

    Call setNumRows() and setNumCols() to set the table size before
    populating the table if you're using MatrixItems.
*/

MatrixUI::MatrixUI(QWidget *parent, const char *name)
    : QTableWidget(parent/**, name, WNoAutoErase | WStaticContents*/),
      leftHeader(0), topHeader(0),
      currentSel(0), lastSortCol(-1), sGrid(true), mRows(false), mCols(false),
      asc(true), doSort(true), readOnly(false)
{
    init(0, 0);
}

/*!
    Constructs an empty table called \a name with \a numRows rows and
    \a numCols columns. The table is a child of \a parent.

    If you're using \l{MatrixItem}s to populate the table's cells, you
    can create MatrixItem, ComboMatrixItem and CheckMatrixItem items
    and insert them into the table using setItem(). (See the notes on
    large tables for an alternative to using MatrixItems.)
*/

MatrixUI::MatrixUI(int numRows, int numCols, QWidget *parent, const char *name)
    : Q3ScrollView(parent/**, name, WNoAutoErase | WStaticContents*/),
      leftHeader(0), topHeader(0),
      currentSel(0), lastSortCol(-1), sGrid(true), mRows(false), mCols(false),
      asc(true), doSort(true), readOnly(false)
{
    init(numRows, numCols);
}

/*! \internal
*/

void MatrixUI::init(int rows, int cols)
{
#ifndef QT_NO_DRAGANDDROP
    /**setDragAutoScroll(false);*/
#endif
    d = new MatrixPrivate;
    d->geomTimer = new QTimer(this);
    d->lastVisCol = 0;
    d->lastVisRow = 0;
    connect(d->geomTimer, SIGNAL(timeout()), this, SLOT(updateGeometriesSlot()));
    shouldClearSelection = false;
    dEnabled = false;
/**
    roRows.setAutoDelete(true);
    roCols.setAutoDelete(true);
*/
    setSorting(false);

    unused = true; // It's unused, ain't it? :)

    selMode = Multi;
/**
    contents.setAutoDelete(true);
    widgets.setAutoDelete(true);
*/
    // Enable clipper and set background mode
    /**enableClipper(qt_table_clipper_enabled);*/

    viewport()->setFocusProxy(this);
    viewport()->setFocusPolicy(Qt::WheelFocus);
    setFocusPolicy(Qt::WheelFocus);

/**
    viewport()->setBackgroundMode(PaletteBase);
    setBackgroundMode(PaletteBackground, PaletteBase);
    setResizePolicy(Manual);
    selections.setAutoDelete(true);
*/

    // Create headers
    leftHeader = new MatrixHeader(Vertical, rows, this, this, "left table header");
/**
    leftHeader->setTracking(true);
    leftHeader->setMovingEnabled(true);
*/
    topHeader = new MatrixHeader(Horizontal, cols, this, this, "right table header");
/**
    topHeader->setTracking(true);
    topHeader->setMovingEnabled(true);
*/
	
//! altered
    d->e = new QLineEdit(viewport());
    d->b[0] = new QToolButton(topHeader);
    d->b[1] = new QToolButton(topHeader);
    d->b[2] = new QToolButton(leftHeader);
    d->b[3] = new QToolButton(leftHeader);

/**
    if (QApplication::reverseLayout())
        setMargins(0, fontMetrics().height() + 4, 30, 0);
    else
        setMargins(30, fontMetrics().height() + 4, 0, 0);
*/

    topHeader->setUpdatesEnabled(false);
    leftHeader->setUpdatesEnabled(false);
    // Initialize headers
    int i = 0;
    for (i = 0; i < numCols(); ++i)
        topHeader->resizeSection(i, qMax(100, QApplication::globalStrut().height()));
    for (i = 0; i < numRows(); ++i)
        leftHeader->resizeSection(i, qMax(20, QApplication::globalStrut().width()));
    topHeader->setUpdatesEnabled(true);
    leftHeader->setUpdatesEnabled(true);

    // Prepare for contents
    /**contents.setAutoDelete(false);*/

    // Connect header, table and scroll bars
    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)),
             topHeader, SLOT(setOffset(int)));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)),
             leftHeader, SLOT(setOffset(int)));
    connect(topHeader, SIGNAL(sectionSizeChanged(int)),
             this, SLOT(columnWidthChanged(int)));
    connect(topHeader, SIGNAL(indexChange(int,int,int)),
             this, SLOT(columnIndexChanged(int,int,int)));
    connect(topHeader, SIGNAL(sectionClicked(int)),
             this, SLOT(columnClicked(int)));
    connect(leftHeader, SIGNAL(sectionSizeChanged(int)),
             this, SLOT(rowHeightChanged(int)));
    connect(leftHeader, SIGNAL(indexChange(int,int,int)),
             this, SLOT(rowIndexChanged(int,int,int)));

    // Initialize variables
    autoScrollTimer = new QTimer(this);
    connect(autoScrollTimer, SIGNAL(timeout()),
             this, SLOT(doAutoScroll()));
    curRow = curCol = 0;
    topHeader->setSectionState(curCol, MatrixHeader::Bold);
    leftHeader->setSectionState(curRow, MatrixHeader::Bold);
    edMode = NotEditing;
    editRow = editCol = -1;

    drawActiveSelection = true;

    installEventFilter(this);

    focusStl = SpreadSheet;

    was_visible = false;

    // initial size
    resize(640, 480);
	
//! altered
    d->e->hide();
    d->e->setFrame(false);

	d->b[0]->resize(22, topHeader->height());
	d->b[1]->resize(22, topHeader->height());
	d->b[2]->resize(22, topHeader->height());
	d->b[3]->resize(22, topHeader->height());

	d->b[0]->setAutoRaise(true);
	d->b[1]->setAutoRaise(true);
	d->b[2]->setAutoRaise(true);
	d->b[3]->setAutoRaise(true);

	d->b[0]->setIcon(QIcon(":/images/del_col.png"));
	d->b[1]->setIcon(QIcon(":/images/add_col.png"));
	d->b[2]->setIcon(QIcon(":/images/del_row.png"));
	d->b[3]->setIcon(QIcon(":/images/add_row.png"));

    d->b[0]->setToolTip("Remove last column");
    d->b[1]->setToolTip("Add a new column");
    d->b[2]->setToolTip("Remove last row");
    d->b[3]->setToolTip("Add a new row");

	connect(d->b[0], SIGNAL(clicked()), SLOT(slotRemoveColumn()));
	connect(d->b[1], SIGNAL(clicked()), SLOT(slotAddColumn()));
	connect(d->b[2], SIGNAL(clicked()), SLOT(slotRemoveRow()));
	connect(d->b[3], SIGNAL(clicked()), SLOT(slotAddRow()));
}

/*!
    Releases all the resources used by the MatrixUI object,
    including all \l{MatrixItem}s and their widgets.
*/

MatrixUI::~MatrixUI()
{
    setUpdatesEnabled(false);
    /**contents.setAutoDelete(true);*/
    contents.clear();
    widgets.clear();

    delete d;
}

void MatrixUI::setReadOnly(bool b)
{
    readOnly = b;

    MatrixItem *i = item(curRow, curCol);
    if (readOnly && isEditing()) {
        endEdit(editRow, editCol, true, false);
    } else if (!readOnly && i && (i->editType() == MatrixItem::WhenCurrent
                                  || i->editType() == MatrixItem::Always)) {
        editCell(curRow, curCol);
    }
}

/*!
    If \a ro is true, row \a row is set to be read-only; otherwise the
    row is set to be editable.

    Whether a cell in this row is editable or read-only depends on the
    cell's EditType, and this setting.

    \sa isRowReadOnly() setColumnReadOnly() setReadOnly()
*/

void MatrixUI::setRowReadOnly(int row, bool ro)
{
    if (ro)
        roRows[row] = int(0);
    else
        roRows.remove(row);

    if (curRow == row) {
        MatrixItem *i = item(curRow, curCol);
        if (ro && isEditing()) {
            endEdit(editRow, editCol, true, false);
        } else if (!ro && i && (i->editType() == MatrixItem::WhenCurrent
                                      || i->editType() == MatrixItem::Always)) {
            editCell(curRow, curCol);
        }
    }
}

/*!
    If \a ro is true, column \a col is set to be read-only; otherwise
    the column is set to be editable.

    Whether a cell in this column is editable or read-only depends on
    the cell's EditType, and this setting.

    \sa isColumnReadOnly() setRowReadOnly() setReadOnly()

*/

void MatrixUI::setColumnReadOnly(int col, bool ro)
{
    if (ro)
        roCols[col] = int(0);
    else
        roCols.remove(col);

    if (curCol == col) {
        MatrixItem *i = item(curRow, curCol);
        if (ro && isEditing()) {
            endEdit(editRow, editCol, true, false);
        } else if (!ro && i && (i->editType() == MatrixItem::WhenCurrent
                                      || i->editType() == MatrixItem::Always)) {
            editCell(curRow, curCol);
        }
    }
}

/*!
    \property MatrixUI::readOnly
    \brief whether the table is read-only

    Whether a cell in the table is editable or read-only depends on
    the cell's \link MatrixItem::EditType EditType\endlink, and this setting.

    \sa QWidget::enabled setColumnReadOnly() setRowReadOnly()
*/

bool MatrixUI::isReadOnly() const
{
    return readOnly;
}

/*!
    Returns true if row \a row is read-only; otherwise returns false.

    Whether a cell in this row is editable or read-only depends on the
    cell's \link MatrixItem::EditType EditType\endlink, and this
    setting.

    \sa setRowReadOnly() isColumnReadOnly()
*/

bool MatrixUI::isRowReadOnly(int row) const
{
    return (roRows.find(row) != roRows.end());
}

/*!
    Returns true if column \a col is read-only; otherwise returns
    false.

    Whether a cell in this column is editable or read-only depends on
    the cell's EditType, and this setting.

    \sa setColumnReadOnly() isRowReadOnly()
*/

bool MatrixUI::isColumnReadOnly(int col) const
{
    return (roCols.find(col) != roCols.end());
}

void MatrixUI::setSelectionMode(SelectionMode mode)
{
    if (mode == selMode)
        return;
    selMode = mode;
    clearSelection();
    if (isRowSelection(selMode) && numRows() > 0 && numCols() > 0) {
        currentSel = new MatrixSelection();
        selections.append(currentSel);
        currentSel->init(curRow, 0);
        currentSel->expandTo(curRow, numCols() - 1);
        repaintSelections(0, currentSel);
    }
}

/*!
    \property MatrixUI::selectionMode
    \brief the current selection mode

    The default mode is \c Multi which allows the user to select
    multiple ranges of cells.
*/

MatrixUI::SelectionMode MatrixUI::selectionMode() const
{
    return selMode;
}

/*!
    \property MatrixUI::focusStyle
    \brief how the current (focus) cell is drawn

    The default style is \c SpreadSheet.

    \sa MatrixUI::FocusStyle
*/

void MatrixUI::setFocusStyle(FocusStyle fs)
{
    focusStl = fs;
    updateCell(curRow, curCol);
}

MatrixUI::FocusStyle MatrixUI::focusStyle() const
{
    return focusStl;
}

/*!
    This functions updates all the header states to be in sync with
    the current selections. This should be called after
    programmatically changing, adding or removing selections, so that
    the headers are updated.
*/

void MatrixUI::updateHeaderStates()
{
    horizontalHeader()->setUpdatesEnabled(false);
    verticalHeader()->setUpdatesEnabled(false);

    ((MatrixHeader*)verticalHeader())->setSectionStateToAll(MatrixHeader::Normal);
    ((MatrixHeader*)horizontalHeader())->setSectionStateToAll(MatrixHeader::Normal);

    Q3PtrListIterator<MatrixSelection> it(selections);
    const MatrixSelection *s;
    while (it.hasNext()) {
        /**++it;*/
        s = it.next();
        if (s->isActive()) {
            if (s->leftCol() == 0 &&
                 s->rightCol() == numCols() - 1) {
                for (int i = 0; i < s->bottomRow() - s->topRow() + 1; ++i)
                    leftHeader->setSectionState(s->topRow() + i, MatrixHeader::Selected);
            }
            if (s->topRow() == 0 &&
                 s->bottomRow() == numRows() - 1) {
                for (int i = 0; i < s->rightCol() - s->leftCol() + 1; ++i)
                    topHeader->setSectionState(s->leftCol() + i, MatrixHeader::Selected);
            }
        }
    }

    horizontalHeader()->setUpdatesEnabled(true);
    verticalHeader()->setUpdatesEnabled(true);
    horizontalHeader()->repaint(/**false*/);
    verticalHeader()->repaint(/**false*/);
}

/*!
    Returns the table's top QHeaderView.

    This header contains the column labels.

    To modify a column label use QHeaderView::setLabel().

    \sa verticalHeader() setcontentsMargins().top() QHeaderView
*/

QHeaderView *MatrixUI::horizontalHeader() const
{
    return (QHeaderView*)topHeader;
}

/*!
    Returns the table's vertical QHeaderView.

    This header contains the row labels.

    \sa horizontalHeader() setcontentsMargins().left() QHeaderView
*/

QHeaderView *MatrixUI::verticalHeader() const
{
    return (QHeaderView*)leftHeader;
}

void MatrixUI::setShowGrid(bool b)
{
    if (sGrid == b)
        return;
    sGrid = b;
    update();
}

/*!
    \property MatrixUI::showGrid
    \brief whether the table's grid is displayed

    The grid is shown by default.
*/

bool MatrixUI::showGrid() const
{
    return sGrid;
}

/*!
    \property MatrixUI::columnMovingEnabled
    \brief whether columns can be moved by the user

    The default is false. Columns are moved by dragging whilst holding
    down the Ctrl key.

    \sa rowMovingEnabled
*/

void MatrixUI::setColumnMovingEnabled(bool b)
{
    mCols = b;
}

bool MatrixUI::columnMovingEnabled() const
{
    return mCols;
}

/*!
    \property MatrixUI::rowMovingEnabled
    \brief whether rows can be moved by the user

    The default is false. Rows are moved by dragging whilst holding
    down the Ctrl key.


    \sa columnMovingEnabled
*/

void MatrixUI::setRowMovingEnabled(bool b)
{
    mRows = b;
}

bool MatrixUI::rowMovingEnabled() const
{
    return mRows;
}

/*!
    This is called when MatrixUI's internal array needs to be resized to
    \a len elements.

    If you don't use MatrixItems you should reimplement this as an
    empty method to avoid wasting memory. See the notes on large
    tables for further details.
*/

void MatrixUI::resizeData(int len)
{
    contents.resize(len);
    widgets.resize(len);
}

/*!
    Swaps the data in \a row1 and \a row2.

    This function is used to swap the positions of two rows. It is
    called when the user changes the order of rows (see
    setRowMovingEnabled()), and when rows are sorted.

    If you don't use \l{MatrixItem}s and want your users to be able to
    swap rows, e.g. for sorting, you will need to reimplement this
    function. (See the notes on large tables.)

    If \a swapHeader is true, the rows' header contents is also
    swapped.

    This function will not update the MatrixUI, you will have to do
    this manually, e.g. by calling updateContents().

    \sa swapColumns() swapCells()
*/

void MatrixUI::swapRows(int row1, int row2, bool swapHeader)
{
/**
    if (swapHeader)
        leftHeader->swapSections(row1, row2, false);
*/

    Q3PtrVector<MatrixItem> tmpContents;
    tmpContents.resize(numCols());
    Q3PtrVector<QWidget> tmpWidgets;
    tmpWidgets.resize(numCols());
    int i;
/**
    contents.setAutoDelete(false);
    widgets.setAutoDelete(false);
*/
    for (i = 0; i < numCols(); ++i) {
        MatrixItem *i1, *i2;
        i1 = item(row1, i);
        i2 = item(row2, i);
        if (i1 || i2) {
            tmpContents.insert(i, i1);
            contents.remove(indexOf(row1, i));
            contents.insert(indexOf(row1, i), i2);
            contents.remove(indexOf(row2, i));
            contents.insert(indexOf(row2, i), tmpContents[ i ]);
            if (contents[ indexOf(row1, i) ])
                contents[ indexOf(row1, i) ]->setRow(row1);
            if (contents[ indexOf(row2, i) ])
                contents[ indexOf(row2, i) ]->setRow(row2);
        }

        QWidget *w1, *w2;
	w1 = cellWidget(row1, i);
        w2 = cellWidget(row2, i);
        if (w1 || w2) {
            tmpWidgets.insert(i, w1);
            widgets.remove(indexOf(row1, i));
            widgets.insert(indexOf(row1, i), w2);
            widgets.remove(indexOf(row2, i));
            widgets.insert(indexOf(row2, i), tmpWidgets[ i ]);
        }
    }

    contents.setAutoDelete(false);
    widgets.setAutoDelete(true);

    updateRowWidgets(row1);
    updateRowWidgets(row2);
    if (curRow == row1)
        curRow = row2;
    else if (curRow == row2)
        curRow = row1;
    if (editRow == row1)
        editRow = row2;
    else if (editRow == row2)
        editRow = row1;
}

/*!
    Sets the left margin to be \a m pixels wide.

    The verticalHeader(), which displays row labels, occupies this
    margin.

    In an Arabic or Hebrew localization, the verticalHeader() will
    appear on the right side of the table, and this call will set the
    right margin.

    \sa contentsMargins().left() setcontentsMargins().top() verticalHeader()
*/

void MatrixUI::setLeftMargin(int m)
{
//! altered
	showVControls(false);
/**
    if (QApplication::reverseLayout())
        setMargins(contentsMargins().left(), contentsMargins().top(), m, bottomMargin());
    else
        setMargins(m, contentsMargins().top(), contentsMargins().right(), bottomMargin());
*/
    updateGeometries();
}

/*!
    Sets the top margin to be \a m pixels high.

    The horizontalHeader(), which displays column labels, occupies
    this margin.

    \sa contentsMargins().top() setcontentsMargins().left()
*/

void MatrixUI::setTopMargin(int m)
{
    /**setMargins(contentsMargins().left(), m, contentsMargins().right(), bottomMargin());*/
    updateGeometries();
}

/*!
    Swaps the data in \a col1 with \a col2.

    This function is used to swap the positions of two columns. It is
    called when the user changes the order of columns (see
    setColumnMovingEnabled(), and when columns are sorted.

    If you don't use \l{MatrixItem}s and want your users to be able to
    swap columns you will need to reimplement this function. (See the
    notes on large tables.)

    If \a swapHeader is true, the columns' header contents is also
    swapped.

    \sa swapCells()
*/

void MatrixUI::swapColumns(int col1, int col2, bool swapHeader)
{
/**
    if (swapHeader)
        topHeader->swapSections(col1, col2, false);
*/

    Q3PtrVector<MatrixItem> tmpContents;
    tmpContents.resize(numRows());
    Q3PtrVector<QWidget> tmpWidgets;
    tmpWidgets.resize(numRows());
    int i;

    contents.setAutoDelete(false);
    widgets.setAutoDelete(false);
    for (i = 0; i < numRows(); ++i) {
        MatrixItem *i1, *i2;
        i1 = item(i, col1);
        i2 = item(i, col2);
        if (i1 || i2) {
            tmpContents.insert(i, i1);
            contents.remove(indexOf(i, col1));
            contents.insert(indexOf(i, col1), i2);
            contents.remove(indexOf(i, col2));
            contents.insert(indexOf(i, col2), tmpContents[ i ]);
            if (contents[ indexOf(i, col1) ])
                contents[ indexOf(i, col1) ]->setCol(col1);
            if (contents[ indexOf(i, col2) ])
                contents[ indexOf(i, col2) ]->setCol(col2);
        }

        QWidget *w1, *w2;
        w1 = cellWidget(i, col1);
        w2 = cellWidget(i, col2);
        if (w1 || w2) {
            tmpWidgets.insert(i, w1);
            widgets.remove(indexOf(i, col1));
            widgets.insert(indexOf(i, col1), w2);
            widgets.remove(indexOf(i, col2));
            widgets.insert(indexOf(i, col2), tmpWidgets[ i ]);
        }
    }
    contents.setAutoDelete(false);
    widgets.setAutoDelete(true);

    columnWidthChanged(col1);
    columnWidthChanged(col2);
    if (curCol == col1)
        curCol = col2;
    else if (curCol == col2)
        curCol = col1;
    if (editCol == col1)
        editCol = col2;
    else if (editCol == col2)
        editCol = col1;
}

/*!
    Swaps the contents of the cell at \a row1, \a col1 with the
    contents of the cell at \a row2, \a col2.

    This function is also called when the table is sorted.

    If you don't use \l{MatrixItem}s and want your users to be able to
    swap cells, you will need to reimplement this function. (See the
    notes on large tables.)

    \sa swapColumns() swapRows()
*/

void MatrixUI::swapCells(int row1, int col1, int row2, int col2)
{
    contents.setAutoDelete(false);
    widgets.setAutoDelete(false);
    MatrixItem *i1, *i2;
    i1 = item(row1, col1);
    i2 = item(row2, col2);
    if (i1 || i2) {
        MatrixItem *tmp = i1;
        contents.remove(indexOf(row1, col1));
        contents.insert(indexOf(row1, col1), i2);
        contents.remove(indexOf(row2, col2));
        contents.insert(indexOf(row2, col2), tmp);
        if (contents[ indexOf(row1, col1) ]) {
            contents[ indexOf(row1, col1) ]->setRow(row1);
            contents[ indexOf(row1, col1) ]->setCol(col1);
        }
        if (contents[ indexOf(row2, col2) ]) {
            contents[ indexOf(row2, col2) ]->setRow(row2);
            contents[ indexOf(row2, col2) ]->setCol(col2);
        }
    }

    QWidget *w1, *w2;
    w1 = cellWidget(row1, col1);
    w2 = cellWidget(row2, col2);
    if (w1 || w2) {
        QWidget *tmp = w1;
        widgets.remove(indexOf(row1, col1));
        widgets.insert(indexOf(row1, col1), w2);
        widgets.remove(indexOf(row2, col2));
        widgets.insert(indexOf(row2, col2), tmp);
    }

    updateRowWidgets(row1);
    updateRowWidgets(row2);
    updateColWidgets(col1);
    updateColWidgets(col2);
    contents.setAutoDelete(false);
    widgets.setAutoDelete(true);
}

static bool is_child_of(QWidget *child, QWidget *parent)
{
    while (child) {
        if (child == parent)
            return true;
        child = child->parentWidget();
    }
    return false;
}

/*!
    Draws the table contents on the painter \a p. This function is
    optimized so that it only draws the cells inside the \a cw pixels
    wide and \a ch pixels high clipping rectangle at position \a cx,
    \a cy.

    Additionally, drawContents() highlights the current cell.
*/

void MatrixUI::drawContents(QPainter *p, int cx, int cy, int cw, int ch)
{
    int colfirst = columnAt(cx);
    int collast = columnAt(cx + cw);
    int rowfirst = rowAt(cy);
    int rowlast = rowAt(cy + ch);

    if (rowfirst == -1 || colfirst == -1) {
        paintEmptyArea(p, cx, cy, cw, ch);
        return;
    }

    drawActiveSelection = hasFocus() || viewport()->hasFocus() || d->inMenuMode
                        || is_child_of(qApp->focusWidget(), viewport())
                        || !style()->styleHint(QStyle::SH_ItemView_ChangeHighlightOnFocus, 0, this);
    if (rowlast == -1)
        rowlast = numRows() - 1;
    if (collast == -1)
        collast = numCols() - 1;

    bool currentInSelection = false;

    Q3PtrListIterator<MatrixSelection> it( selections );
    MatrixSelection *s;
    while (it.hasNext()) {
        /**++it;*/
        s = it.next();
        if (s->isActive() &&
             curRow >= s->topRow() &&
             curRow <= s->bottomRow() &&
             curCol >= s->leftCol() &&
             curCol <= s->rightCol()) {
            currentInSelection = s->topRow() != curRow || s->bottomRow() != curRow || s->leftCol() != curCol || s->rightCol() != curCol;
            break;
        }
    }

    // Go through the rows
    for (int r = rowfirst; r <= rowlast; ++r) {
        // get row position and height
        int rowp = rowPos(r);
        int rowh = rowHeight(r);

        // Go through the columns in row r
        // if we know from where to where, go through [colfirst, collast],
        // else go through all of them
        for (int c = colfirst; c <= collast; ++c) {
            // get position and width of column c
            int colp, colw;
            colp = columnPos(c);
            colw = columnWidth(c);
            int oldrp = rowp;
            int oldrh = rowh;

            MatrixItem *itm = item(r, c);
            if (itm &&
                 (itm->colSpan() > 1 || itm->rowSpan() > 1)) {
                bool goon = (r == itm->row() && c == itm->col())
                            || (r == rowfirst && c == itm->col())
                            || (r == itm->row() && c == colfirst);
                if (!goon)
                    continue;
                rowp = rowPos(itm->row());
                rowh = 0;
                int i;
                for (i = 0; i < itm->rowSpan(); ++i)
                    rowh += rowHeight(i + itm->row());
                colp = columnPos(itm->col());
                colw = 0;
                for (i = 0; i < itm->colSpan(); ++i)
                    colw += columnWidth(i + itm->col());
            }

            // Translate painter and draw the cell
            p->translate(colp, rowp);
            bool selected = isSelected(r, c);
            if (focusStl != FollowStyle && selected && !currentInSelection &&
                 r == curRow && c == curCol )
                selected = false;
            paintCell(p, r, c, QRect(colp, rowp, colw, rowh), selected);
            p->translate(-colp, -rowp);

            rowp = oldrp;
            rowh = oldrh;

            QWidget *w = cellWidget(r, c);
            QRect cg(cellGeometry(r, c));
            if (w && w->geometry() != QRect(contentsToViewport(cg.topLeft()), cg.size() - QSize(1, 1))) {
                moveChild(w, colp, rowp);
                w->resize(cg.size() - QSize(1, 1));
            }
        }
    }
    d->lastVisCol = collast;
    d->lastVisRow = rowlast;

    // draw indication of current cell
    QRect focusRect = cellGeometry(curRow, curCol);
    p->translate(focusRect.x(), focusRect.y());
    paintFocus(p, focusRect);
    p->translate(-focusRect.x(), -focusRect.y());

    // Paint empty rects
    paintEmptyArea(p, cx, cy, cw, ch);

    drawActiveSelection = true;
}

/*!
    \reimp

    (Implemented to get rid of a compiler warning.)
*/

void MatrixUI::drawContents(QPainter *)
{
}

/*!
    Returns the geometry of cell \a row, \a col in the cell's
    coordinate system. This is a convenience function useful in
    paintCell(). It is equivalent to QRect(QPoint(0,0), cellGeometry(
    row, col).size());

    \sa cellGeometry()
*/

QRect MatrixUI::cellRect(int row, int col) const
{
    return QRect(QPoint(0,0), cellGeometry(row, col).size());
}

/*!
    \overload

    Use the other paintCell() function. This function is only included
    for backwards compatibility.
*/

void MatrixUI::paintCell(QPainter* p, int row, int col,
                        const QRect &cr, bool selected)
{
    if (cr.width() == 0 || cr.height() == 0)
        return;
#if defined(Q_WS_WIN)
    const QColorGroup &cg = (!drawActiveSelection && style()->styleHint(QStyle::SH_ItemView_ChangeHighlightOnFocus) ? palette().inactive() : colorGroup());
#else
    const QColorGroup &cg = palette();
#endif

    MatrixItem *itm = item(row, col);
    QColorGroup cg2(cg);
/**
    if (itm && !itm->isEnabled())
        cg2 = palette().disabled();
*/

    paintCell(p, row, col, cr, selected, cg2);
}

/*!
    Paints the cell at \a row, \a col on the painter \a p. The painter
    has already been translated to the cell's origin. \a cr describes
    the cell coordinates in the content coordinate system.

    If \a selected is true the cell is highlighted.

    \a cg is the colorgroup which should be used to draw the cell
    content.

    If you want to draw custom cell content, for example right-aligned
    text, you must either reimplement paintCell(), or subclass
    MatrixItem and reimplement MatrixItem::paint() to do the custom
    drawing.

    If you're using a MatrixItem subclass, for example, to store a
    data structure, then reimplementing MatrixItem::paint() may be the
    best approach. For data you want to draw immediately, e.g. data
    retrieved from a database, it is probably best to reimplement
    paintCell(). Note that if you reimplement paintCell(), i.e. don't
    use \l{MatrixItem}s, you must reimplement other functions: see the
    notes on large tables.

    Note that the painter is not clipped by default in order to get
    maximum efficiency. If you want clipping, use code like this:

    \snippet doc/src/snippets/code/src_qt3support_itemviews_q3table.cpp 4
*/

void MatrixUI::paintCell(QPainter *p, int row, int col,
                        const QRect &cr, bool selected, const QColorGroup &cg)
{
    if (focusStl == SpreadSheet && selected &&
         row == curRow &&
         col == curCol && (hasFocus() || viewport()->hasFocus()))
        selected = false;

    QPalette pal = cg;
    int w = cr.width();
    int h = cr.height();
    int x2 = w - 1;
    int y2 = h - 1;


    MatrixItem *itm = item(row, col);
    if (itm) {
        p->save();
        itm->paint(p, pal, cr, selected);
        p->restore();
    } else {
        p->fillRect(0, 0, w, h, selected ? pal.brush(QPalette::Highlight) : pal.brush(QPalette::Base));
    }

    if (sGrid) {
        // Draw our lines
        QPen pen(p->pen());
        int gridColor =        style()->styleHint(QStyle::SH_Table_GridLineColor, 0, this);
        if (gridColor != -1) {
            if (palette() != pal)
                p->setPen(pal.mid().color());
            else
                p->setPen((QRgb)gridColor);
        } else {
            p->setPen(pal.mid().color());
        }
        p->drawLine(x2, 0, x2, y2);
        p->drawLine(0, y2, x2, y2);
        p->setPen(pen);
    }
}

/*!
    Draws the focus rectangle of the current cell (see currentRow(),
    currentColumn()).

    The painter \a p is already translated to the cell's origin, while
    \a cr specifies the cell's geometry in content coordinates.
*/

void MatrixUI::paintFocus(QPainter *p, const QRect &cr)
{
    if (!hasFocus() && !viewport()->hasFocus())
        return;
    QRect focusRect(0, 0, cr.width(), cr.height());
    if (focusStyle() == SpreadSheet) {
        p->setPen(QPen(Qt::black, 1));
        p->setBrush(Qt::NoBrush);
        p->drawRect(focusRect.x(), focusRect.y(), focusRect.width() - 1, focusRect.height() - 1);
        p->drawRect(focusRect.x() - 1, focusRect.y() - 1, focusRect.width() + 1, focusRect.height() + 1);
    } else {
        QStyleOptionFocusRect opt;
        opt.init(this);
        opt.rect = focusRect;
        opt.palette = palette();
        opt.state |= QStyle::State_KeyboardFocusChange;
        if (isSelected(curRow, curCol, false)) {
            opt.state |= QStyle::State_FocusAtBorder;
            opt.backgroundColor = palette().highlight().color();
        } else {
            opt.state |= QStyle::State_None;
            opt.backgroundColor = palette().base().color();
        }
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &opt, p, this);
    }
}

/*!
    This function fills the \a cw pixels wide and \a ch pixels high
    rectangle starting at position \a cx, \a cy with the background
    color using the painter \a p.

    paintEmptyArea() is invoked by drawContents() to erase or fill
    unused areas.
*/

void MatrixUI::paintEmptyArea(QPainter *p, int cx, int cy, int cw, int ch)
{
    // Regions work with shorts, so avoid an overflow and adjust the
    // table size to the visible size
    QSize ts(tableSize());
    ts.setWidth(qMin(ts.width(), viewport()->width()));
    ts.setHeight(qMin(ts.height(), viewport()->height()));

    // Region of the rect we should draw, calculated in viewport
    // coordinates, as a region can't handle bigger coordinates
    contentsToViewport2(cx, cy, cx, cy);
    QRegion reg(QRect(cx, cy, cw, ch));

    // Subtract the table from it
    reg = reg.subtracted(QRect(QPoint(0, 0), ts));

    // And draw the rectangles (transformed inc contents coordinates as needed)
    Q3MemArray<QRect> r = reg.rects();
    for (int i = 0; i < (int)r.count(); ++i)
        p->fillRect(QRect(viewportToContents2(r[i].topLeft()),r[i].size()), viewport()->backgroundBrush());
}

/*!
    Returns the MatrixItem representing the contents of the cell at \a
    row, \a col.

    If \a row or \a col are out of range or no content has been set
    for this cell, item() returns 0.

    If you don't use \l{MatrixItem}s you may need to reimplement this
    function: see the notes on large tables.

    \sa setItem()
*/

MatrixItem *MatrixUI::item(int row, int col) const
{
    if (row < 0 || col < 0 || row > numRows() - 1 ||
         col > numCols() - 1 || row * col >= (int)contents.size())
        return 0;

    return contents[ indexOf(row, col) ];        // contents array lookup
}

/*!
    Inserts the table item \a item into the table at row \a row,
    column \a col, and repaints the cell. If a table item already
    exists in this cell it is deleted and replaced with \a item. The
    table takes ownership of the table item.

    If you don't use \l{MatrixItem}s you may need to reimplement this
    function: see the notes on large tables.

    \sa item() takeItem()
*/

void MatrixUI::setItem(int row, int col, MatrixItem *item)
{
    if (!item)
        return;

    if ((int)contents.size() != numRows() * numCols())
        resizeData(numRows() * numCols());

    int orow = item->row();
    int ocol = item->col();
    clearCell(row, col);

    contents.insert(indexOf(row, col), item);
    item->setRow(row);
    item->setCol(col);
    item->t = this;
    updateCell(row, col);
    if (qt_update_cell_widget)
        item->updateEditor(orow, ocol);

    if (row == curRow && col == curCol && item->editType() == MatrixItem::WhenCurrent) {
        if (beginEdit(row, col, false))
            setEditMode(Editing, row, col);
    }
}

/*!
    Removes the MatrixItem at \a row, \a col.

    If you don't use \l{MatrixItem}s you may need to reimplement this
    function: see the notes on large tables.
*/

void MatrixUI::clearCell(int row, int col)
{
    if ((int)contents.size() != numRows() * numCols())
        resizeData(numRows() * numCols());
    clearCellWidget(row, col);
    contents.setAutoDelete(true);
    contents.remove(indexOf(row, col));
    contents.setAutoDelete(false);
}

/*!
    Sets the text in the cell at \a row, \a col to \a text.

    If the cell does not contain a table item a MatrixItem is created
    with an \link MatrixItem::EditType EditType\endlink of \c OnTyping,
    otherwise the existing table item's text (if any) is replaced with
    \a text.

    \sa text() setPixmap() setItem() MatrixItem::setText()
*/

void MatrixUI::setText(int row, int col, const QString &text)
{
    MatrixItem *itm = item(row, col);
    if (itm) {
        itm->setText(text);
        itm->updateEditor(row, col);
        updateCell(row, col);
    } else {
        MatrixItem *i = new MatrixItem(this, MatrixItem::OnTyping,
                                        text, QPixmap());
        setItem(row, col, i);
    }
}

/*!
    Sets the pixmap in the cell at \a row, \a col to \a pix.

    If the cell does not contain a table item a MatrixItem is created
    with an \link MatrixItem::EditType EditType\endlink of \c OnTyping,
    otherwise the existing table item's pixmap (if any) is replaced
    with \a pix.

    Note that \l{ComboMatrixItem}s and \l{CheckMatrixItem}s don't show
    pixmaps.

    \sa pixmap() setText() setItem() MatrixItem::setPixmap()
*/

void MatrixUI::setPixmap(int row, int col, const QPixmap &pix)
{
    MatrixItem *itm = item(row, col);
    if (itm) {
        itm->setPixmap(pix);
        updateCell(row, col);
    } else {
        MatrixItem *i = new MatrixItem(this, MatrixItem::OnTyping,
                                        QString(), pix);
        setItem(row, col, i);
    }
}

/*!
    Returns the text in the cell at \a row, \a col, or an empty string
    if the relevant item does not exist or has no text.

    \sa setText() setPixmap()
*/

QString MatrixUI::text(int row, int col) const
{
    MatrixItem *itm = item(row, col);
    if (itm)
        return itm->text();
    return QString();
}

/*!
    Returns the pixmap set for the cell at \a row, \a col, or a
    null-pixmap if the cell contains no pixmap.

    \sa setPixmap()
*/

QPixmap MatrixUI::pixmap(int row, int col) const
{
    MatrixItem *itm = item(row, col);
    if (itm)
        return itm->pixmap();
    return QPixmap();
}

/*!
    Moves the focus to the cell at \a row, \a col.

    \sa currentRow() currentColumn()
*/

void MatrixUI::setCurrentCell(int row, int col)
{
    setCurrentCell(row, col, true, true);
}

// need to use a define, as contentsMargins().left() is protected
#define VERTICALMARGIN \
(QApplication::reverseLayout() ? \
       contentsMargins().right() \
       : \
       contentsMargins().left() \
)

/*!
    \reimp
*/
QVariant MatrixUI::inputMethodQuery(Qt::InputMethodQuery query) const
{
    if (query == Qt::ImMicroFocus)
        return QRect(columnPos(curCol) + contentsMargins().left() - visibleRegion().boundingRect().x(), rowPos(curRow) + contentsMargins().top() - visibleRegion().boundingRect().y(),
                     columnWidth(curCol), rowHeight(curRow));
    return QWidget::inputMethodQuery(query);

}

/*! \internal */

void MatrixUI::setCurrentCell(int row, int col, bool updateSelections, bool ensureVisible)
{
    MatrixItem *oldItem = item(curRow, curCol);

    if (row > numRows() - 1)
        row = numRows() - 1;
    if (col > numCols() - 1)
        col = numCols() - 1;

    if (curRow == row && curCol == col)
        return;


    MatrixItem *itm = oldItem;
    if (itm && itm->editType() != MatrixItem::Always && itm->editType() != MatrixItem::Never)
        endEdit(curRow, curCol, true, false);
    int oldRow = curRow;
    int oldCol = curCol;
    curRow = row;
    curCol = col;
    repaintCell(oldRow, oldCol);
    repaintCell(curRow, curCol);
    if (ensureVisible)
        ensureCellVisible(curRow, curCol);
    emit currentChanged(row, col);

    if (oldCol != curCol) {
        if (!isColumnSelected(oldCol))
            topHeader->setSectionState(oldCol, MatrixHeader::Normal);
        else if (isRowSelection(selectionMode()))
            topHeader->setSectionState(oldCol, MatrixHeader::Selected);
        topHeader->setSectionState(curCol, isColumnSelected(curCol, true) ?
                                    MatrixHeader::Selected : MatrixHeader::Bold);
    }

    if (oldRow != curRow) {
        if (!isRowSelected(oldRow))
            leftHeader->setSectionState(oldRow, MatrixHeader::Normal);
        leftHeader->setSectionState(curRow, isRowSelected(curRow, true) ?
                                     MatrixHeader::Selected : MatrixHeader::Bold);
    }

    itm = item(curRow, curCol);


    if (cellWidget(oldRow, oldCol) &&
         cellWidget(oldRow, oldCol)->hasFocus())
        viewport()->setFocus();

    if (itm && itm->editType() == MatrixItem::WhenCurrent) {
        if (beginEdit(curRow, curCol, false))
            setEditMode(Editing, row, col);
    } else if (itm && itm->editType() == MatrixItem::Always) {
        if (cellWidget(itm->row(), itm->col()))
            cellWidget(itm->row(), itm->col())->setFocus();
    }

    if (updateSelections && isRowSelection(selectionMode()) &&
         !isSelected(curRow, curCol, false)) {
        if (selectionMode() == MatrixUI::SingleRow)
            clearSelection();
        currentSel = new MatrixSelection();
        selections.append(currentSel);
        currentSel->init(curRow, 0);
        currentSel->expandTo(curRow, numCols() - 1);
        repaintSelections(0, currentSel);
    }
}

/*!
    Scrolls the table until the cell at \a row, \a col becomes
    visible.
*/

void MatrixUI::ensureCellVisible(int row, int col)
{
    if (!updatesEnabled() || !viewport()->updatesEnabled())
        return;
    int cw = columnWidth(col);
    int rh = rowHeight(row);
    if (cw < viewport()->width())
        ensureVisible(columnPos(col) + cw / 2, rowPos(row) + rh / 2, cw / 2, rh / 2);
    else
        ensureVisible(columnPos(col), rowPos(row) + rh / 2, 0, rh / 2);
}

/*!
    Returns true if the cell at \a row, \a col is selected; otherwise
    returns false.

    \sa isRowSelected() isColumnSelected()
*/

bool MatrixUI::isSelected(int row, int col) const
{
    return isSelected(row, col, true);
}

/*! \internal */

bool MatrixUI::isSelected(int row, int col, bool includeCurrent) const
{
    Q3PtrListIterator<MatrixSelection> it(selections);
    MatrixSelection *s;
    while (it.hasNext()) {
        /**++it;*/
        s = it.next();
        if (s->isActive() &&
             row >= s->topRow() &&
             row <= s->bottomRow() &&
             col >= s->leftCol() &&
             col <= s->rightCol())
            return true;
        if (includeCurrent && row == currentRow() && col == currentColumn())
            return true;
    }
    return false;
}

/*!
    Returns true if row \a row is selected; otherwise returns false.

    If \a full is false (the default), 'row is selected' means that at
    least one cell in the row is selected. If \a full is true, then 'row
    is selected' means every cell in the row is selected.

    \sa isColumnSelected() isSelected()
*/

bool MatrixUI::isRowSelected(int row, bool full) const
{
    if (!full) {
        Q3PtrListIterator<MatrixSelection> it(selections);
        MatrixSelection *s;
        while (it.hasNext()) {
            /**++it;*/
            s = it.next();
            if (s->isActive() &&
                 row >= s->topRow() &&
                 row <= s->bottomRow())
            return true;
        if (row == currentRow())
            return true;
        }
    } else {
        Q3PtrListIterator<MatrixSelection> it(selections);
        MatrixSelection *s;
        while (it.hasNext()) {
            /**++it;*/
            s = it.next();
            if (s->isActive() &&
                 row >= s->topRow() &&
                 row <= s->bottomRow() &&
                 s->leftCol() == 0 &&
                 s->rightCol() == numCols() - 1)
                return true;
        }
    }
    return false;
}

/*!
    Returns true if column \a col is selected; otherwise returns false.

    If \a full is false (the default), 'column is selected' means that
    at least one cell in the column is selected. If \a full is true,
    then 'column is selected' means every cell in the column is
    selected.

    \sa isRowSelected() isSelected()
*/

bool MatrixUI::isColumnSelected(int col, bool full) const
{
    if (!full) {
        Q3PtrListIterator<MatrixSelection> it(selections);
        MatrixSelection *s;
        while (it.hasNext()) {
            /**++it;*/
            s = it.next();
            if (s->isActive() &&
                 col >= s->leftCol() &&
                 col <= s->rightCol())
            return true;
        if (col == currentColumn())
            return true;
        }
    } else {
        Q3PtrListIterator<MatrixSelection> it(selections);
        MatrixSelection *s;
        while (it.hasNext()) {
            /**++it;*/
            s = it.next();
            if (s->isActive() &&
                 col >= s->leftCol() &&
                 col <= s->rightCol() &&
                 s->topRow() == 0 &&
                 s->bottomRow() == numRows() - 1)
                return true;
        }
    }
    return false;
}

/*!
    \property MatrixUI::numSelections
    \brief The number of selections.

    \sa currentSelection()
*/

int MatrixUI::numSelections() const
{
    return selections.count();
}

/*!
    Returns selection number \a num, or an inactive MatrixSelection if \a
    num is out of range (see MatrixSelection::isActive()).
*/

MatrixSelection MatrixUI::selection(int num) const
{
    if (num < 0 || num >= (int)selections.count())
        return MatrixSelection();

    QLinkedList<MatrixSelection *>::const_iterator it = selections.begin();
    for (int i = 0; i < num; ++i)
        ++it;
    return **it;
}

/*!
    Adds a selection described by \a s to the table and returns its
    number or -1 if the selection is invalid.

    Remember to call MatrixSelection::init() and
    MatrixSelection::expandTo() to make the selection valid (see also
    MatrixSelection::isActive(), or use the
    MatrixSelection(int,int,int,int) constructor).

    \sa numSelections() removeSelection() clearSelection()
*/

int MatrixUI::addSelection(const MatrixSelection &s)
{
    if (!s.isActive())
        return -1;

    const int maxr = numRows()-1;
    const int maxc = numCols()-1;
    currentSel = new MatrixSelection(qMin(s.anchorRow(), maxr), qMin(s.anchorCol(), maxc),
                                    qMin(s.bottomRow(), maxr), qMin(s.rightCol(), maxc));

    selections.append(currentSel);

    repaintSelections(0, currentSel, true, true);

    emit selectionChanged();

    return selections.count() - 1;
}

/*!
    If the table has a selection, \a s, this selection is removed from
    the table.

    \sa addSelection() numSelections()
*/

void MatrixUI::removeSelection(const MatrixSelection &s)
{
    /**selections.setAutoDelete(false);*/
    for (QLinkedList<MatrixSelection *>::iterator sel = selections.begin(); sel != selections.end(); ++sel) {
        if (s == **sel) {
            selections.removeOne(*sel);
            repaintSelections(*sel, 0, true, true);
            if (*sel == currentSel)
                currentSel = 0;
            delete *sel;
        }
    }
    /**selections.setAutoDelete(true);*/
    emit selectionChanged();
}

/*!
    \overload

    Removes selection number \a num from the table.

    \sa numSelections() addSelection() clearSelection()
*/

void MatrixUI::removeSelection(int num)
{
    if (num < 0 || num >= (int)selections.count())
        return;

    QLinkedList<MatrixSelection *>::iterator it = selections.begin();
    for (int i = 0; i < num; ++i)
        ++it;
    MatrixSelection *s = *it;
    if (s == currentSel)
        currentSel = 0;
    selections.removeOne(s);
    repaint();
}

/*!
    Returns the number of the current selection or -1 if there is no
    current selection.

    \sa numSelections()
*/

int MatrixUI::currentSelection() const
{
    if (!currentSel)
        return -1;
    return ((MatrixUI*)this)->selections.contains(currentSel);
}

/*! Selects the range starting at \a start_row and \a start_col and
  ending at \a end_row and \a end_col.

  \sa MatrixSelection
*/

void MatrixUI::selectCells(int start_row, int start_col, int end_row, int end_col)
{
    const int maxr = numRows()-1;
    const int maxc = numCols()-1;

    start_row = qMin(maxr, qMax(0, start_row));
    start_col = qMin(maxc, qMax(0, start_col));
    end_row = qMin(maxr, end_row);
    end_col = qMin(maxc, end_col);
    MatrixSelection sel(start_row, start_col, end_row, end_col);
    addSelection(sel);
}

/*! Selects the row \a row.

  \sa MatrixSelection
*/

void MatrixUI::selectRow(int row)
{
    row = qMin(numRows()-1, row);
    if (row < 0)
        return;
    if (selectionMode() == SingleRow) {
        setCurrentCell(row, currentColumn());
    } else {
        MatrixSelection sel(row, 0, row, numCols() - 1);
        addSelection(sel);
    }
}

/*! Selects the column \a col.

  \sa MatrixSelection
*/

void MatrixUI::selectColumn(int col)
{
    col = qMin(numCols()-1, col);
    if (col < 0)
        return;
    MatrixSelection sel(0, col, numRows() - 1, col);
    addSelection(sel);
}

/*! \reimp
*/
void MatrixUI::contentsMousePressEvent(QMouseEvent* e)
{
    contentsMousePressEventEx(e);
}

void MatrixUI::contentsMousePressEventEx(QMouseEvent* e)
{
    shouldClearSelection = false;
    if (isEditing()) {
        if (!cellGeometry(editRow, editCol).contains(e->pos())) {
            endEdit(editRow, editCol, true, edMode != Editing);
        } else {
            e->ignore();
            return;
        }
    }

    d->redirectMouseEvent = false;

    int tmpRow = rowAt(e->pos().y());
    int tmpCol = columnAt(e->pos().x());
    pressedRow = tmpRow;
    pressedCol = tmpCol;
    fixRow(tmpRow, e->pos().y());
    fixCol(tmpCol, e->pos().x());
    startDragCol = -1;
    startDragRow = -1;

    if (isSelected(tmpRow, tmpCol)) {
        startDragCol = tmpCol;
        startDragRow = tmpRow;
        dragStartPos = e->pos();
    }

    MatrixItem *itm = item(pressedRow, pressedCol);
    if (itm && !itm->isEnabled()) {
        emit pressed(tmpRow, tmpCol, e->button(), e->pos());
        return;
    }

    if ((e->buttons() & ShiftModifier) == ShiftModifier) {
          int oldRow = curRow;
          int oldCol = curCol;
        setCurrentCell(tmpRow, tmpCol, selMode == SingleRow, true);
        if (selMode != NoSelection && selMode != SingleRow) {
            if (!currentSel) {
                currentSel = new MatrixSelection();
                selections.append(currentSel);
                if (!isRowSelection(selectionMode()))
                    currentSel->init(oldRow, oldCol);
                else
                    currentSel->init(oldRow, 0);
            }
            MatrixSelection oldSelection = *currentSel;
            if (!isRowSelection(selectionMode()))
                currentSel->expandTo(tmpRow, tmpCol);
            else
                currentSel->expandTo(tmpRow, numCols() - 1);
            repaintSelections(&oldSelection, currentSel);
            emit selectionChanged();
        }
    } else if ((e->buttons() & ControlModifier) == ControlModifier) {
        setCurrentCell(tmpRow, tmpCol, false, true);
        if (selMode != NoSelection) {
            if (selMode == Single || (selMode == SingleRow && !isSelected(tmpRow, tmpCol, false)))
                clearSelection();
            if (!(selMode == SingleRow && isSelected(tmpRow, tmpCol, false))) {
                currentSel = new MatrixSelection();
                selections.append(currentSel);
                if (!isRowSelection(selectionMode())) {
                    currentSel->init(tmpRow, tmpCol);
                    currentSel->expandTo(tmpRow, tmpCol);
                } else {
                    currentSel->init(tmpRow, 0);
                    currentSel->expandTo(tmpRow, numCols() - 1);
                    repaintSelections(0, currentSel);
                }
                emit selectionChanged();
            }
        }
    } else {
        setCurrentCell(tmpRow, tmpCol, false, true);
        MatrixItem *itm = item(tmpRow, tmpCol);
        if (itm && itm->editType() == MatrixItem::WhenCurrent) {
            QWidget *w = cellWidget(tmpRow, tmpCol);
            if (qobject_cast<Q3ComboBox*>(w) || qobject_cast<QAbstractButton*>(w)) {
                QMouseEvent ev(e->type(), w->mapFromGlobal(e->globalPos()),
                                e->globalPos(), e->button(), e->buttons(), KeyboardModifiers());
                QApplication::sendPostedEvents(w, 0);
                QApplication::sendEvent(w, &ev);
                d->redirectMouseEvent = true;
            }
        }
        if (isSelected(tmpRow, tmpCol, false)) {
            shouldClearSelection = true;
        } else {
            bool b = signalsBlocked();
            if (selMode != NoSelection)
                blockSignals(true);
            clearSelection();
            blockSignals(b);
            if (selMode != NoSelection) {
                currentSel = new MatrixSelection();
                selections.append(currentSel);
                if (!isRowSelection(selectionMode())) {
                    currentSel->init(tmpRow, tmpCol);
                    currentSel->expandTo(tmpRow, tmpCol);
                } else {
                    currentSel->init(tmpRow, 0);
                    currentSel->expandTo(tmpRow, numCols() - 1);
                    repaintSelections(0, currentSel);
                }
                emit selectionChanged();
            }
        }
    }

    emit pressed(tmpRow, tmpCol, e->button(), e->pos());
}

/*! \reimp
*/

void MatrixUI::contentsMouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() != LeftButton)
        return;
    if (!isRowSelection(selectionMode()))
        clearSelection();
    int tmpRow = rowAt(e->pos().y());
    int tmpCol = columnAt(e->pos().x());
    MatrixItem *itm = item(tmpRow, tmpCol);
    if (itm && !itm->isEnabled())
        return;
    if (tmpRow != -1 && tmpCol != -1) {
        if (beginEdit(tmpRow, tmpCol, false))
            setEditMode(Editing, tmpRow, tmpCol);
    }

    emit doubleClicked(tmpRow, tmpCol, e->button(), e->pos());
}

/*!
    Sets the current edit mode to \a mode, the current edit row to \a
    row and the current edit column to \a col.

    \sa EditMode
*/

void MatrixUI::setEditMode(EditMode mode, int row, int col)
{
    edMode = mode;
    editRow = row;
    editCol = col;
}


/*! \reimp
*/

void MatrixUI::contentsMouseMoveEvent(QMouseEvent *e)
{
    if ((e->buttons() & MouseButtonMask) == NoButton)
        return;
    int tmpRow = rowAt(e->pos().y());
    int tmpCol = columnAt(e->pos().x());
    fixRow(tmpRow, e->pos().y());
    fixCol(tmpCol, e->pos().x());

#ifndef QT_NO_DRAGANDDROP
    if (dragEnabled() && startDragRow != -1 && startDragCol != -1) {
        if (QPoint(dragStartPos - e->pos()).manhattanLength() > QApplication::startDragDistance())
            startDrag();
        return;
    }
#endif
    if (selectionMode() == MultiRow && (e->buttons() & ControlModifier) == ControlModifier)
        shouldClearSelection = false;

    if (shouldClearSelection) {
        clearSelection();
        if (selMode != NoSelection) {
            currentSel = new MatrixSelection();
            selections.append(currentSel);
            if (!isRowSelection(selectionMode()))
                currentSel->init(tmpRow, tmpCol);
            else
                currentSel->init(tmpRow, 0);
            emit selectionChanged();
        }
        shouldClearSelection = false;
    }

    QPoint pos = mapFromGlobal(e->globalPos());
    pos -= QPoint(leftHeader->width(), topHeader->height());
    autoScrollTimer->stop();
    doAutoScroll();
    if (pos.x() < 0 || pos.x() > viewport()->width() || pos.y() < 0 || pos.y() > viewport()->height())
        autoScrollTimer->start(100, true);
}

/*! \internal
 */

void MatrixUI::doValueChanged()
{
    emit valueChanged(editRow, editCol);
}

/*! \internal
*/

void MatrixUI::doAutoScroll()
{
    QPoint pos = QCursor::pos();
    pos = mapFromGlobal(pos);
    pos -= QPoint(leftHeader->width(), topHeader->height());

    int tmpRow = curRow;
    int tmpCol = curCol;
    if (pos.y() < 0)
        tmpRow--;
    else if (pos.y() > viewport()->height())
        tmpRow++;
    if (pos.x() < 0)
        tmpCol--;
    else if (pos.x() > viewport()->width())
        tmpCol++;

    pos += QPoint(visibleRegion().boundingRect().x(), visibleRegion().boundingRect().y());
    if (tmpRow == curRow)
        tmpRow = rowAt(pos.y());
    if (tmpCol == curCol)
        tmpCol = columnAt(pos.x());
    pos -= QPoint(visibleRegion().boundingRect().x(), visibleRegion().boundingRect().y());

    fixRow(tmpRow, pos.y());
    fixCol(tmpCol, pos.x());

    if (tmpRow < 0 || tmpRow > numRows() - 1)
        tmpRow = currentRow();
    if (tmpCol < 0 || tmpCol > numCols() - 1)
        tmpCol = currentColumn();

    ensureCellVisible(tmpRow, tmpCol);

    if (currentSel && selMode != NoSelection) {
        MatrixSelection oldSelection = *currentSel;
        bool useOld = true;
        if (selMode != SingleRow) {
            if (!isRowSelection(selectionMode())) {
                currentSel->expandTo(tmpRow, tmpCol);
            } else {
                currentSel->expandTo(tmpRow, numCols() - 1);
            }
        } else {
            bool currentInSelection = tmpRow == curRow && isSelected(tmpRow, tmpCol);
            if (!currentInSelection) {
                useOld = false;
                clearSelection();
                currentSel = new MatrixSelection();
                selections.append(currentSel);
                currentSel->init(tmpRow, 0);
                currentSel->expandTo(tmpRow, numCols() - 1);
                repaintSelections(0, currentSel);
            } else {
                currentSel->expandTo(tmpRow, numCols() - 1);
            }
        }
        setCurrentCell(tmpRow, tmpCol, false, true);
        repaintSelections(useOld ? &oldSelection : 0, currentSel);
        if (currentSel && oldSelection != *currentSel)
            emit selectionChanged();
    } else {
        setCurrentCell(tmpRow, tmpCol, false, true);
    }

    if (pos.x() < 0 || pos.x() > viewport()->width() || pos.y() < 0 || pos.y() > viewport()->height())
        autoScrollTimer->start(100, true);
}

/*! \reimp
*/

void MatrixUI::contentsMouseReleaseEvent(QMouseEvent *e)
{
    if (pressedRow == curRow && pressedCol == curCol)
        emit clicked(curRow, curCol, e->button(), e->pos());

    if (e->button() != LeftButton)
        return;
    if (shouldClearSelection) {
        int tmpRow = rowAt(e->pos().y());
        int tmpCol = columnAt(e->pos().x());
        fixRow(tmpRow, e->pos().y());
        fixCol(tmpCol, e->pos().x());
        clearSelection();
        if (selMode != NoSelection) {
            currentSel = new MatrixSelection();
            selections.append(currentSel);
            if (!isRowSelection(selectionMode())) {
                currentSel->init(tmpRow, tmpCol);
            } else {
                currentSel->init(tmpRow, 0);
                currentSel->expandTo(tmpRow, numCols() - 1);
                repaintSelections(0, currentSel);
            }
            emit selectionChanged();
        }
        shouldClearSelection = false;
    }
    autoScrollTimer->stop();

    if (d->redirectMouseEvent && pressedRow == curRow && pressedCol == curCol &&
         item(pressedRow, pressedCol) && item(pressedRow, pressedCol)->editType() ==
         MatrixItem::WhenCurrent) {
        QWidget *w = cellWidget(pressedRow, pressedCol);
        if (w) {
            QMouseEvent ev(e->type(), w->mapFromGlobal(e->globalPos()),
                            e->globalPos(), e->button(), e->buttons(), KeyboardModifiers());
            QApplication::sendPostedEvents(w, 0);
            bool old = w->testAttribute(Qt::WA_NoMousePropagation);
            w->setAttribute(Qt::WA_NoMousePropagation, true);
            QApplication::sendEvent(w, &ev);
            w->setAttribute(Qt::WA_NoMousePropagation, old);
        }
    }
}

/*!
  \reimp
*/

void MatrixUI::contentsContextMenuEvent(QContextMenuEvent *e)
{
    if (!receivers(SIGNAL(contextMenuRequested(int,int,QPoint)))) {
        e->ignore();
        return;
    }
    if (e->reason() == QContextMenuEvent::Keyboard) {
        QRect r = cellGeometry(curRow, curCol);
        emit contextMenuRequested(curRow, curCol, viewport()->mapToGlobal(contentsToViewport(r.center())));
    } else {
        int tmpRow = rowAt(e->pos().y());
        int tmpCol = columnAt(e->pos().x());
        emit contextMenuRequested(tmpRow, tmpCol, e->globalPos());
    }
}


/*! \reimp
*/

bool MatrixUI::eventFilter(QObject *o, QEvent *e)
{
    switch (e->type()) {
    case QEvent::KeyPress: {
        MatrixItem *itm = item(curRow, curCol);
        QWidget *editorWidget = cellWidget(editRow, editCol);

        if (isEditing() && editorWidget && o == editorWidget) {
            itm = item(editRow, editCol);
            QKeyEvent *ke = (QKeyEvent*)e;
            if (ke->key() == Key_Escape) {
                if (!itm || itm->editType() == MatrixItem::OnTyping)
                    endEdit(editRow, editCol, false, edMode != Editing);
                return true;
            }

            if ((ke->modifiers() == NoButton || ke->modifiers() == Keypad)
                && (ke->key() == Key_Return || ke->key() == Key_Enter)) {
                if (!itm || itm->editType() == MatrixItem::OnTyping)
                    endEdit(editRow, editCol, true, edMode != Editing);
                activateNextCell();
                return true;
            }

            if (ke->key() == Key_Tab || ke->key() == Key_BackTab) {
                if (ke->modifiers() & Qt::ControlModifier)
                    return false;
                if (!itm || itm->editType() == MatrixItem::OnTyping)
                    endEdit(editRow, editCol, true, edMode != Editing);
                if ((ke->key() == Key_Tab) && !(ke->modifiers() & ShiftModifier)) {
                    if (currentColumn() >= numCols() - 1)
                        return true;
                    int cc  = qMin(numCols() - 1, currentColumn() + 1);
                    while (cc < numCols()) {
                        MatrixItem *i = item(currentRow(), cc);
                        if (d->hiddenCols.find(cc) == d->hiddenCols.end() && !isColumnReadOnly(cc) && (!i || i->isEnabled()))
                            break;
                        ++cc;
                    }
                    setCurrentCell(currentRow(), cc);
                } else { // Key_BackTab
                    if (currentColumn() == 0)
                        return true;
                    int cc  = qMax(0, currentColumn() - 1);
                    while (cc >= 0) {
                        MatrixItem *i = item(currentRow(), cc);
                        if (d->hiddenCols.find(cc) == d->hiddenCols.end() && !isColumnReadOnly(cc) && (!i || i->isEnabled()))
                            break;
                        --cc;
                    }
                    setCurrentCell(currentRow(), cc);
                }
                itm = item(curRow, curCol);
                if (beginEdit(curRow, curCol, false))
                    setEditMode(Editing, curRow, curCol);
                return true;
            }

            if ((edMode == Replacing ||
                   (itm && itm->editType() == MatrixItem::WhenCurrent)) &&
                 (ke->key() == Key_Up || /**ke->key() == Key_Prior ||*/
                   ke->key() == Key_Home || ke->key() == Key_Down ||
                   /**ke->key() == Key_Next ||*/ ke->key() == Key_End ||
                   ke->key() == Key_Left || ke->key() == Key_Right)) {
                if (!itm || itm->editType() == MatrixItem::OnTyping) {
                    endEdit(editRow, editCol, true, edMode != Editing);
                }
                keyPressEvent(ke);
                return true;
            }
        } else {
            QObjectList l = viewport()->queryList("QWidget");
            if (l.contains(o)) {
                QKeyEvent *ke = (QKeyEvent*)e;
                if ((ke->modifiers() & ControlModifier) == ControlModifier ||
                     (ke->key() != Key_Left && ke->key() != Key_Right &&
                       ke->key() != Key_Up && ke->key() != Key_Down &&
                       /**ke->key() != Key_Prior && ke->key() != Key_Next &&*/
                       ke->key() != Key_Home && ke->key() != Key_End))
                    return false;
                keyPressEvent((QKeyEvent*)e);
                return true;
            }
        }

        } break;
    case QEvent::FocusOut: {
        QWidget *editorWidget = cellWidget(editRow, editCol);
        if (isEditing() && editorWidget && o == editorWidget && ((QFocusEvent*)e)->reason() != Qt::PopupFocusReason) {
            // if the editor is the parent of the new focus widget, do nothing
            QWidget *w = QApplication::focusWidget();
            while (w) {
                w = w->parentWidget();
                if (w == editorWidget)
                    break;
            }
            if (w)
                break;
            // otherwise, end editing
            MatrixItem *itm = item(editRow, editCol);
            if (!itm || itm->editType() == MatrixItem::OnTyping) {
                endEdit(editRow, editCol, true, edMode != Editing);
                return true;
            }
        }
        break;
    }
#ifndef QT_NO_WHEELEVENT
    case QEvent::Wheel:
        if (o == this || o == viewport()) {
            QWheelEvent* we = (QWheelEvent*)e;
            scrollBy(0, -we->delta());
            we->accept();
            return true;
        }
#endif
    default:
        break;
    }

    return Q3ScrollView::eventFilter(o, e);
}

void MatrixUI::fixCell(int &row, int &col, int key)
{
    if (rowHeight(row) > 0 && columnWidth(col) > 0)
        return;
    if (rowHeight(row) <= 0) {
        if (key == Key_Down ||
             /**key == Key_Next ||*/
             key == Key_End) {
            while (row < numRows() && rowHeight(row) <= 0)
                row++;
            if (rowHeight(row) <= 0)
                row = curRow;
        } else if (key == Key_Up ||
                    /**key == Key_Prior ||*/
                    key == Key_Home)
            while (row >= 0 && rowHeight(row) <= 0)
                row--;
            if (rowHeight(row) <= 0)
                row = curRow;
    } else if (columnWidth(col) <= 0) {
        if (key == Key_Left) {
            while (col >= 0 && columnWidth(col) <= 0)
                col--;
            if (columnWidth(col) <= 0)
                col = curCol;
        } else if (key == Key_Right) {
            while (col < numCols() && columnWidth(col) <= 0)
                col++;
            if (columnWidth(col) <= 0)
                col = curCol;
        }
    }
}

/*! \reimp
*/

void MatrixUI::keyPressEvent(QKeyEvent* e)
{
    if (isEditing() && item(editRow, editCol) &&
         item(editRow, editCol)->editType() == MatrixItem::OnTyping)
        return;

    int tmpRow = curRow;
    int tmpCol = curCol;
    int oldRow = tmpRow;
    int oldCol = tmpCol;

    bool navigationKey = false;
    int r;
    switch (e->key()) {
    case Key_Left:
        tmpCol = qMax(0, tmpCol - 1);
        navigationKey = true;
        break;
    case Key_Right:
        tmpCol = qMin(numCols() - 1, tmpCol + 1);
        navigationKey = true;
        break;
    case Key_Up:
        tmpRow = qMax(0, tmpRow - 1);
        navigationKey = true;
        break;
    case Key_Down:
        tmpRow = qMin(numRows() - 1, tmpRow + 1);
        navigationKey = true;
        break;
/**
    case Key_Prior:
        r = qMax(0, rowAt(rowPos(tmpRow) - viewport()->height()));
        if (r < tmpRow || tmpRow < 0)
            tmpRow = r;
        navigationKey = true;
        break;
    case Key_Next:
        r = qMin(numRows() - 1, rowAt(rowPos(tmpRow) + viewport()->height()));
        if (r > tmpRow)
            tmpRow = r;
        else
            tmpRow = numRows() - 1;
        navigationKey = true;
        break;
*/
    case Key_Home:
        tmpRow = 0;
        navigationKey = true;
        break;
    case Key_End:
        tmpRow = numRows() - 1;
        navigationKey = true;
        break;
    case Key_F2:
        if (beginEdit(tmpRow, tmpCol, false))
            setEditMode(Editing, tmpRow, tmpCol);
        break;
//! altered
    case Key_Enter: case Key_Return:
//		activateNextCell();
		QApplication::postEvent(this, new QKeyEvent(QEvent::KeyPress, Key_F2, 0, 0));
		return;
    case Key_Tab: /**case Key_BackTab:*/
        if ((e->key() == Key_Tab) && !(e->modifiers() & ShiftModifier)) {
            if (currentColumn() >= numCols() - 1)
                return;
            int cc  = qMin(numCols() - 1, currentColumn() + 1);
            while (cc < numCols()) {
                MatrixItem *i = item(currentRow(), cc);
                if (d->hiddenCols.find(cc) == d->hiddenCols.end() && !isColumnReadOnly(cc) && (!i || i->isEnabled()))
                    break;
                ++cc;
            }
            setCurrentCell(currentRow(), cc);
        } else { // Key_BackTab
            if (currentColumn() == 0)
                return;
            int cc  = qMax(0, currentColumn() - 1);
            while (cc >= 0) {
                MatrixItem *i = item(currentRow(), cc);
                if (d->hiddenCols.find(cc) == d->hiddenCols.end() && !isColumnReadOnly(cc) && (!i || i->isEnabled()))
                    break;
                --cc;
            }
            setCurrentCell(currentRow(), cc);
        }
        return;
    case Key_Escape:
        e->ignore();
        return;
    default: // ... or start in-place editing
        if (e->text()[ 0 ].isPrint()) {
            MatrixItem *itm = item(tmpRow, tmpCol);
            if (!itm || itm->editType() == MatrixItem::OnTyping) {
                QWidget *w = beginEdit(tmpRow, tmpCol,
                                        itm ? itm->isReplaceable() : true);
                if (w) {
                    setEditMode((!itm || (itm && itm->isReplaceable())
                                   ? Replacing : Editing), tmpRow, tmpCol);
                    QApplication::sendEvent(w, e);
                    return;
                }
            }
        }
        e->ignore();
        return;
    }

    if (navigationKey) {
        fixCell(tmpRow, tmpCol, e->key());
        if ((e->modifiers() & ShiftModifier) == ShiftModifier &&
             selMode != NoSelection && selMode != SingleRow) {
            bool justCreated = false;
            setCurrentCell(tmpRow, tmpCol, false, true);
            if (!currentSel) {
                justCreated = true;
                currentSel = new MatrixSelection();
                selections.append(currentSel);
                if (!isRowSelection(selectionMode()))
                    currentSel->init(oldRow, oldCol);
                else
                    currentSel->init(oldRow < 0 ? 0 : oldRow, 0);
            }
            MatrixSelection oldSelection = *currentSel;
            if (!isRowSelection(selectionMode()))
                currentSel->expandTo(tmpRow, tmpCol);
            else
                currentSel->expandTo(tmpRow, numCols() - 1);
            repaintSelections(justCreated ? 0 : &oldSelection, currentSel);
            emit selectionChanged();
        } else {
            setCurrentCell(tmpRow, tmpCol, false, true);
            if (!isRowSelection(selectionMode())) {
                clearSelection();
            } else {
                bool currentInSelection = tmpRow == oldRow && isSelected(tmpRow, tmpCol, false);
                if (!currentInSelection) {
                    bool hasOldSel = false;
                    MatrixSelection oldSelection;
                    if (selectionMode() == MultiRow) {
                        bool b = signalsBlocked();
                        blockSignals(true);
                        clearSelection();
                        blockSignals(b);
                    } else {
                        if (currentSel) {
                            oldSelection = *currentSel;
                            hasOldSel = true;
                            selections.removeOne(currentSel);
                            leftHeader->setSectionState(oldSelection.topRow(), MatrixHeader::Normal);
                        }
                    }
                    currentSel = new MatrixSelection();
                    selections.append(currentSel);
                    currentSel->init(tmpRow, 0);
                    currentSel->expandTo(tmpRow, numCols() - 1);
                    repaintSelections(hasOldSel ? &oldSelection : 0, currentSel, !hasOldSel);
                    emit selectionChanged();
                }
            }
        }
    } else {
        setCurrentCell(tmpRow, tmpCol, false, true);
    }
}

/*! \reimp
*/

void MatrixUI::focusInEvent(QFocusEvent*)
{
    d->inMenuMode = false;
    QWidget *editorWidget = cellWidget(editRow, editCol);
    updateCell(curRow, curCol);
    if (style()->styleHint(QStyle::SH_ItemView_ChangeHighlightOnFocus, 0, this))
        repaintSelections();
    if (isEditing() && editorWidget)
        editorWidget->setFocus();

}


/*! \reimp
*/

void MatrixUI::focusOutEvent(QFocusEvent *e)
{
    updateCell(curRow, curCol);
    if (style()->styleHint(QStyle::SH_ItemView_ChangeHighlightOnFocus, 0, this)) {
        d->inMenuMode =
            e->reason() == Qt::PopupFocusReason ||
            (qApp->focusWidget() && qApp->focusWidget()->inherits("QMenuBar"));
        if (!d->inMenuMode)
            repaintSelections();
    }
}

/*! \reimp
*/

QSize MatrixUI::sizeHint() const
{
    if (cachedSizeHint().isValid())
        return cachedSizeHint();

    constPolish();

    QSize s = tableSize();
    QSize sh;
    if (s.width() < 500 && s.height() < 500) {
        sh = QSize(tableSize().width() + VERTICALMARGIN + 5,
                    tableSize().height() + contentsMargins().top() + 5);
    } else {
            sh = Q3ScrollView::sizeHint();
            if (!topHeader->isHidden())
                sh.setHeight(sh.height() + topHeader->height());
            if (!leftHeader->isHidden())
                sh.setWidth(sh.width() + leftHeader->width());
    }
    setCachedSizeHint(sh);
    return sh;
}

/*! \reimp
*/

void MatrixUI::viewportResizeEvent(QResizeEvent *e)
{
    Q3ScrollView::viewportResizeEvent(e);
    updateGeometries();
}

/*! \reimp
*/

void MatrixUI::showEvent(QShowEvent *e)
{
//! altered
    int xc = d->b[0]->isVisible() ? d->b[0]->width() * 2 : 0;
	int yc = d->b[2]->isVisible() ? d->b[2]->height() * 2 : 0;

    Q3ScrollView::showEvent(e);
    QRect r(cellGeometry(numRows() - 1, numCols() - 1));
    resizeContents(r.right() + 1 + xc, r.bottom() + 1 + yc);
    updateGeometries();
}

/*! \reimp
*/

void MatrixUI::paintEvent(QPaintEvent *e)
{
    QRect topLeftCorner = QStyle::visualRect(layoutDirection(), rect(), QRect(frameWidth(), frameWidth(), VERTICALMARGIN, contentsMargins().top()));
    erase(topLeftCorner); // erase instead of widget on top
    Q3ScrollView::paintEvent(e);

#ifdef Q_OS_WINCE
    QPainter p(this);
    p.drawLine(topLeftCorner.bottomLeft(), topLeftCorner.bottomRight());
    p.drawLine(topLeftCorner.bottomRight(), topLeftCorner.topRight());
#endif
}

static bool inUpdateCell = false;

/*!
    Repaints the cell at \a row, \a col.
*/

void MatrixUI::updateCell(int row, int col)
{
    if (inUpdateCell || row < 0 || col < 0)
        return;
    inUpdateCell = true;
    QRect cg = cellGeometry(row, col);
    QRect r(contentsToViewport(QPoint(cg.x() - 2, cg.y() - 2)),
             QSize(cg.width() + 4, cg.height() + 4));
    viewport()->update(r);
    inUpdateCell = false;
}

void MatrixUI::repaintCell(int row, int col)
{
    if (row == -1 || col == -1)
        return;
    QRect cg = cellGeometry(row, col);
    QRect r(QPoint(cg.x() - 2, cg.y() - 2),
             QSize(cg.width() + 4, cg.height() + 4));
    repaintContents(r, false);
}

void MatrixUI::contentsToViewport2(int x, int y, int& vx, int& vy)
{
    const QPoint v = contentsToViewport2(QPoint(x, y));
    vx = v.x();
    vy = v.y();
}

QPoint MatrixUI::contentsToViewport2(const QPoint &p)
{
    return QPoint(p.x() - visibleRegion().boundingRect().x(),
                   p.y() - visibleRegion().boundingRect().y());
}

QPoint MatrixUI::viewportToContents2(const QPoint& vp)
{
    return QPoint(vp.x() + visibleRegion().boundingRect().x(),
                   vp.y() + visibleRegion().boundingRect().y());
}

void MatrixUI::viewportToContents2(int vx, int vy, int& x, int& y)
{
    const QPoint c = viewportToContents2(QPoint(vx, vy));
    x = c.x();
    y = c.y();
}

/*!
    This function should be called whenever the column width of \a col
    has been changed. It updates the geometry of any affected columns
    and repaints the table to reflect the changes it has made.
*/

void MatrixUI::columnWidthChanged(int col)
{
    int p = columnPos(col);
    if (d->hasColSpan)
        p = visibleRegion().boundingRect().x();
    updateContents(p, visibleRegion().boundingRect().y(), visibleRegion().boundingRect().width(), viewport()->height());
    QSize s(tableSize());
    int w = visibleRegion().boundingRect().width();
    resizeContents(s.width(), s.height());
    if (visibleRegion().boundingRect().width() < w)
        repaintContents(s.width(), visibleRegion().boundingRect().y(),
                         w - s.width() + 1, viewport()->height(), true);
    else
        repaintContents(w, visibleRegion().boundingRect().y(),
                         s.width() - w + 1, viewport()->height(), false);

    // update widgets that are affected by this change
    if (widgets.size()) {
        int last = isHidden() ? numCols() - 1 : d->lastVisCol;
        for (int c = col; c <= last; ++c)
            updateColWidgets(c);
    }
    delayedUpdateGeometries();
}

/*!
    This function should be called whenever the row height of \a row
    has been changed. It updates the geometry of any affected rows and
    repaints the table to reflect the changes it has made.
*/

void MatrixUI::rowHeightChanged(int row)
{
    int p = rowPos(row);
    if (d->hasRowSpan)
        p = visibleRegion().boundingRect().y();
    updateContents(visibleRegion().boundingRect().x(), p, viewport()->width(), viewport()->height());
    QSize s(tableSize());
    int h = viewport()->height();
    resizeContents(s.width(), s.height());
    if (viewport()->height() < h) {
        repaintContents(visibleRegion().boundingRect().x(), viewport()->height(),
                         viewport()->width(), h - s.height() + 1, true);
    } else {
        repaintContents(visibleRegion().boundingRect().x(), h,
                         viewport()->width(), s.height() - h + 1, false);
    }

    // update widgets that are affected by this change
    if (widgets.size()) {
        d->lastVisRow = rowAt(visibleRegion().boundingRect().y() + viewport()->height() + (s.height() - h + 1));
        int last = isHidden() ? numRows() - 1 : d->lastVisRow;
        for (int r = row; r <= last; ++r)
            updateRowWidgets(r);
    }
    delayedUpdateGeometries();
}

/*! \internal */

void MatrixUI::updateRowWidgets(int row)
{
    for (int i = 0; i < numCols(); ++i) {
        QWidget *w = cellWidget(row, i);
        if (!w)
            continue;
        moveChild(w, columnPos(i), rowPos(row));
        w->resize(columnWidth(i) - 1, rowHeight(row) - 1);
    }
}

/*! \internal */

void MatrixUI::updateColWidgets(int col)
{
    for (int i = 0; i < numRows(); ++i) {
        QWidget *w = cellWidget(i, col);
        if (!w)
            continue;
        moveChild(w, columnPos(col), rowPos(i));
        w->resize(columnWidth(col) - 1, rowHeight(i) - 1);
    }
}

/*!
    This function is called when column order is to be changed, i.e.
    when the user moved the column header \a section from \a fromIndex
    to \a toIndex.

    If you want to change the column order programmatically, call
    swapRows() or swapColumns();

    \sa QHeaderView::indexChange() rowIndexChanged()
*/

void MatrixUI::columnIndexChanged(int, int fromIndex, int toIndex)
{
    if (doSort && lastSortCol == fromIndex && topHeader)
        topHeader->setSortIndicator(toIndex, topHeader->sortIndicatorOrder());
    repaintContents(visibleRegion().boundingRect().x(), visibleRegion().boundingRect().y(),
                     viewport()->width(), viewport()->height(), false);
}

/*!
    This function is called when the order of the rows is to be
    changed, i.e. the user moved the row header section \a section
    from \a fromIndex to \a toIndex.

    If you want to change the order programmatically, call swapRows()
    or swapColumns();

    \sa QHeaderView::indexChange() columnIndexChanged()
*/

void MatrixUI::rowIndexChanged(int, int, int)
{
    repaintContents(visibleRegion().boundingRect().x(), visibleRegion().boundingRect().y(),
                     viewport()->width(), viewport()->height(), false);
}

/*!
    This function is called when the column \a col has been clicked.
    The default implementation sorts this column if sorting() is true.
*/

void MatrixUI::columnClicked(int col)
{
    if (!sorting())
        return;

    if (col == lastSortCol) {
        asc = !asc;
    } else {
        lastSortCol = col;
        asc = true;
    }
    sortColumn(lastSortCol, asc);
}

/*!
    \property MatrixUI::sorting
    \brief whether a click on the header of a column sorts that column

    \sa sortColumn()
*/

void MatrixUI::setSorting(bool b)
{
    doSort = b;
    if (topHeader)
         topHeader->setSortIndicator(b ? lastSortCol : -1, AscendingOrder);
}

bool MatrixUI::sorting() const
{
    return doSort;
}

static bool inUpdateGeometries = false;

void MatrixUI::delayedUpdateGeometries()
{
    d->geomTimer->setSingleShot(true);
    d->geomTimer->start(0);
}

void MatrixUI::updateGeometriesSlot()
{
    updateGeometries();
}

/*!
    This function updates the geometries of the left and top header.
    You do not normally need to call this function.
*/

void MatrixUI::updateGeometries()
{
    if (inUpdateGeometries)
        return;
    inUpdateGeometries = true;
    QSize ts = tableSize();
    if (topHeader->offset() &&
         ts.width() < topHeader->offset() + topHeader->width())
        horizontalScrollBar()->setValue(ts.width() - topHeader->width());
    if (leftHeader->offset() &&
         ts.height() < leftHeader->offset() + leftHeader->height())
        verticalScrollBar()->setValue(ts.height() - leftHeader->height());

    leftHeader->setGeometry(QStyle::visualRect(layoutDirection(), rect(), QRect(frameWidth(), contentsMargins().top() + frameWidth(),
                             VERTICALMARGIN, viewport()->height())));
    topHeader->setGeometry(QStyle::visualRect(layoutDirection(), rect(), QRect(VERTICALMARGIN + frameWidth(), frameWidth(),
                                                      viewport()->width(), contentsMargins().top())));
    horizontalScrollBar()->raise();
    verticalScrollBar()->raise();
    topHeader->updateStretches();
    leftHeader->updateStretches();
    inUpdateGeometries = false;

//! altered
	adjustButtons();}

/*!
    Returns the width of column \a col.

    \sa setColumnWidth() rowHeight()
*/

int MatrixUI::columnWidth(int col) const
{
    return topHeader->sectionSize(col);
}

/*!
    Returns the height of row \a row.

    \sa setRowHeight() columnWidth()
*/

int MatrixUI::rowHeight(int row) const
{
    return leftHeader->sectionSize(row);
}

/*!
    Returns the x-coordinate of the column \a col in content
    coordinates.

    \sa columnAt() rowPos()
*/

int MatrixUI::columnPos(int col) const
{
    return topHeader->sectionPos(col);
}

/*!
    Returns the y-coordinate of the row \a row in content coordinates.

    \sa rowAt() columnPos()
*/

int MatrixUI::rowPos(int row) const
{
    return leftHeader->sectionPos(row);
}

/*!
    Returns the number of the column at position \a x. \a x must be
    given in content coordinates.

    \sa columnPos() rowAt()
*/

int MatrixUI::columnAt(int x) const
{
    return topHeader->sectionAt(x);
}

/*!
    Returns the number of the row at position \a y. \a y must be given
    in content coordinates.

    \sa rowPos() columnAt()
*/

int MatrixUI::rowAt(int y) const
{
    return leftHeader->sectionAt(y);
}

/*!
    Returns the bounding rectangle of the cell at \a row, \a col in
    content coordinates.
*/

QRect MatrixUI::cellGeometry(int row, int col) const
{
    MatrixItem *itm = item(row, col);

    if (!itm || (itm->rowSpan() == 1 && itm->colSpan() == 1))
        return QRect(columnPos(col), rowPos(row),
                      columnWidth(col), rowHeight(row));

    while (row != itm->row())
        row--;
    while (col != itm->col())
        col--;

    QRect rect(columnPos(col), rowPos(row),
                columnWidth(col), rowHeight(row));

    for (int r = 1; r < itm->rowSpan(); ++r)
        rect.setHeight(rect.height() + rowHeight(r + row));

    for (int c = 1; c < itm->colSpan(); ++c)
        rect.setWidth(rect.width() + columnWidth(c + col));

    return rect;
}

/*!
    Returns the size of the table.

    This is the same as the coordinates of the bottom-right edge of
    the last table cell.
*/

QSize MatrixUI::tableSize() const
{
    return QSize(columnPos(numCols() - 1) + columnWidth(numCols() - 1),
                  rowPos(numRows() - 1) + rowHeight(numRows() - 1));
}

/*!
    \property MatrixUI::numRows
    \brief The number of rows in the table

    \sa numCols
*/

int MatrixUI::numRows() const
{
    return leftHeader->count();
}

/*!
    \property MatrixUI::numCols
    \brief The number of columns in the table

    \sa numRows
*/

int MatrixUI::numCols() const
{
    return topHeader->count();
}

void MatrixUI::saveContents(Q3PtrVector<MatrixItem> &tmp,
                           Q3PtrVector<MatrixUI::TableWidget> &tmp2)
{
    int nCols = numCols();
    if (editRow != -1 && editCol != -1)
        endEdit(editRow, editCol, false, edMode != Editing);
    tmp.resize(contents.size());
    tmp2.resize(widgets.size());
    int i;
    for (i = 0; i < (int)tmp.size(); ++i) {
        MatrixItem *item = contents[ i ];
        if (item && (item->row() * nCols) + item->col() == i)
            tmp.insert(i, item);
        else
            tmp.insert(i, 0);
    }
    for (i = 0; i < (int)tmp2.size(); ++i) {
        QWidget *w = widgets[ i ];
        if (w)
            tmp2.insert(i, new TableWidget(w, i / nCols, i % nCols));
        else
            tmp2.insert(i, 0);
    }
}

void MatrixUI::updateHeaderAndResizeContents(MatrixHeader *header,
                                            int num, int rowCol,
                                            int width, bool &updateBefore)
{
    updateBefore = rowCol < num;
    if (rowCol > num) {
        header->MatrixHeader::resizeArrays(rowCol);
        int old = num;
        clearSelection(false);
        int i = 0;
        for (i = old; i < rowCol; ++i)
            header->addLabel(QString(), width);
    } else {
        clearSelection(false);
        if (header == leftHeader) {
            while (numRows() > rowCol)
                header->removeLabel(numRows() - 1);
        } else {
            while (numCols() > rowCol)
                header->removeLabel(numCols() - 1);
        }
    }

    contents.setAutoDelete(false);
    contents.clear();
    contents.setAutoDelete(true);
    widgets.setAutoDelete(false);
    widgets.clear();
    widgets.setAutoDelete(true);
    resizeData(numRows() * numCols());

    // keep numStretches in sync
    int n = 0;
    for (uint i = 0; i < header->stretchable.size(); i++)
        n += (header->stretchable.at(i) & 1); // avoid cmp
     header->numStretches = n;
}

void MatrixUI::restoreContents(Q3PtrVector<MatrixItem> &tmp,
                              Q3PtrVector<MatrixUI::TableWidget> &tmp2)
{
    int i;
    int nCols = numCols();
    for (i = 0; i < (int)tmp.size(); ++i) {
        MatrixItem *it = tmp[ i ];
        if (it) {
            int idx = (it->row() * nCols) + it->col();
            if ((uint)idx < contents.size() &&
                 it->row() == idx /  nCols && it->col() == idx % nCols) {
                contents.insert(idx, it);
                if (it->rowSpan() > 1 || it->colSpan() > 1) {
                    int ridx, iidx;
                    for (int irow = 0; irow < it->rowSpan(); irow++) {
                        ridx = idx + irow * nCols;
                        for (int icol = 0; icol < it->colSpan(); icol++) {
                            iidx = ridx + icol;
                            if (idx != iidx && (uint)iidx < contents.size())
                                contents.insert(iidx, it);
                        }
                    }

                }
            } else {
                delete it;
            }
        }
    }
    for (i = 0; i < (int)tmp2.size(); ++i) {
        TableWidget *w = tmp2[ i ];
        if (w) {
            int idx = (w->row * nCols) + w->col;
            if ((uint)idx < widgets.size() &&
                 w->row == idx / nCols && w->col == idx % nCols)
                widgets.insert(idx, w->wid);
            else
                delete w->wid;
            delete w;
        }
    }
}

void MatrixUI::finishContentsResze(bool updateBefore)
{
    QRect r(cellGeometry(numRows() - 1, numCols() - 1));
    viewport()->resize(r.right() + 1, r.bottom() + 1);
    updateGeometries();
    if (updateBefore)
        viewport()->repaint(visibleRegion().boundingRect().x(), visibleRegion().boundingRect().y(),
                         viewport()->width(), viewport()->height()/**, true*/);
    else
        viewport()->repaint(visibleRegion().boundingRect().x(), visibleRegion().boundingRect().y(),
                         viewport()->width(), viewport()->height()/**, false*/);

    if (isRowSelection(selectionMode())) {
        int r = curRow;
        curRow = -1;
        setCurrentCell(r, curCol);
    }
}

//! altered
void MatrixUI::showHControls(bool b)
{
    d->b[0]->setVisible(b);
    d->b[1]->setVisible(b);
}

void MatrixUI::showVControls(bool b)
{
    d->b[2]->setVisible(b);
    d->b[3]->setVisible(b);
}
void MatrixUI::setNumRows(int r)
{
    if (r < 0)
        return;

    if (r < numRows()) {
        // Removed rows are no longer hidden, and should thus be removed from "hiddenRows"
        for (int rr = numRows()-1; rr >= r; --rr) {
            if (d->hiddenRows.find(rr) != d->hiddenRows.end())
                d->hiddenRows.remove(rr);
        }
    }

/**
    fontChange(font()); // invalidate the sizeHintCache
*/

    Q3PtrVector<MatrixItem> tmp;
    Q3PtrVector<TableWidget> tmp2;
    saveContents(tmp, tmp2);

    bool updatesEnabled = leftHeader->updatesEnabled();
    if (updatesEnabled)
        leftHeader->setUpdatesEnabled(false);

    bool updateBefore;
    updateHeaderAndResizeContents(leftHeader, numRows(), r, 20, updateBefore);

    int w = fontMetrics().width(QString::number(r) + QLatin1Char('W'));
    if (VERTICALMARGIN > 0 && w > VERTICALMARGIN)
        setLeftMargin(w);

    restoreContents(tmp, tmp2);

/**
    leftHeader->calculatePositions();
*/

    finishContentsResze(updateBefore);
    if (updatesEnabled) {
        leftHeader->setUpdatesEnabled(true);
        leftHeader->update();
    }
    leftHeader->updateCache();
    if (curRow >= numRows()) {
        curRow = numRows() - 1;
        if (curRow < 0)
            curCol = -1;
        else
            repaintCell(curRow, curCol);
    }

    if (curRow > numRows())
        curRow = numRows();
}

void MatrixUI::setNumCols(int c)
{
    if (c < 0)
        return;

    if (c < numCols()) {
        // Removed columns are no longer hidden, and should thus be removed from "hiddenCols"
        for (int cc = numCols()-1; cc >= c; --cc) {
            if (d->hiddenCols.find(cc) != d->hiddenCols.end())
                d->hiddenCols.remove(cc);
        }
    }

/**
    fontChange(font()); // invalidate the sizeHintCache
*/

    Q3PtrVector<MatrixItem> tmp;
    Q3PtrVector<TableWidget> tmp2;
    saveContents(tmp, tmp2);

    bool updatesEnabled = topHeader->updatesEnabled();
    if (updatesEnabled)
        topHeader->setUpdatesEnabled(false);

    bool updateBefore;
    updateHeaderAndResizeContents(topHeader, numCols(), c, 100, updateBefore);

    restoreContents(tmp, tmp2);

/**
    topHeader->calculatePositions();
*/

    finishContentsResze(updateBefore);
    if (updatesEnabled) {
        topHeader->setUpdatesEnabled(true);
        topHeader->update();
    }
    topHeader->updateCache();
    if (curCol >= numCols()) {
        curCol = numCols() - 1;
        if (curCol < 0)
            curRow = -1;
        else
            repaintCell(curRow, curCol);
    }
}

//! altered
void MatrixUI::slotAddRow()
{
	if (isEditing()) endEdit(editRow, editCol, true, false);

	setNumRows(numRows() + 1);
	
	for (int i = 0; i < numCols(); ++ i)
	{
		QString sText = horizontalHeader()->label(i) == "Frequency" ? "1" : "0";

		setText(numRows() - 1, i, sText);
	}
	
	emit valueChanged(numRows() - 1, 0);

	QApplication::postEvent(this, new QShowEvent());
}

void MatrixUI::slotRemoveRow()
{
	if (isEditing()) endEdit(editRow, editCol, true, false);

	setNumRows(numRows() - 1);

	emit valueChanged(numRows() - 1, 0);

	QApplication::postEvent(this, new QShowEvent());
}

void MatrixUI::slotAddColumn()
{
	if (isEditing()) endEdit(editRow, editCol, true, false);

	setNumCols(numCols() + 1);

	for (int i = 0; i < numRows(); ++ i)
	{
		QString sText = "0";

		setText(i, numCols() - 1, sText);
	}

	emit valueChanged(0, numCols() - 1);

	QApplication::postEvent(this, new QShowEvent());
}

void MatrixUI::slotRemoveColumn()
{
	if (isEditing()) endEdit(editRow, editCol, true, false);

	setNumCols(numCols() - 1);

	emit valueChanged(0, numCols() - 1);

	QApplication::postEvent(this, new QShowEvent());
}

void MatrixUI::setHOffset(int n)
{
	int w = topHeader->width();

	// weird detour
	topHeader->setUpdatesEnabled(false);
    topHeader->resize(std::min(topHeader->width(), w), topHeader->height());
	topHeader->setOffset(n);
//	topHeader->resize(w + xc, topHeader->height());
	topHeader->setUpdatesEnabled(true);
}

void MatrixUI::setVOffset(int n)
{
	int h = leftHeader->height();

	// weird detour
	leftHeader->setUpdatesEnabled(false);
    leftHeader->resize(leftHeader->width(), std::min(leftHeader->width(), h));
	leftHeader->setOffset(n);
	leftHeader->resize(leftHeader->width(), h);
	leftHeader->setUpdatesEnabled(true);
}

/*! Sets the section labels of the verticalHeader() to \a labels */

void MatrixUI::setRowLabels(const QStringList &labels)
{
    leftHeader->setLabels(labels);
}

/*! Sets the section labels of the horizontalHeader() to \a labels */

void MatrixUI::setColumnLabels(const QStringList &labels)
{
   topHeader->setLabels(labels);
}

/*!
    This function returns the widget which should be used as an editor
    for the contents of the cell at \a row, \a col.

    If \a initFromCell is true, the editor is used to edit the current
    contents of the cell (so the editor widget should be initialized
    with this content). If \a initFromCell is false, the content of
    the cell is replaced with the new content which the user entered
    into the widget created by this function.

    The default functionality is as follows: if \a initFromCell is
    true or the cell has a MatrixItem and the table item's
    MatrixItem::isReplaceable() is false then the cell is asked to
    create an appropriate editor (using MatrixItem::createEditor()).
    Otherwise a QLineEdit is used as the editor.

    If you want to create your own editor for certain cells, implement
    a custom MatrixItem subclass and reimplement
    MatrixItem::createEditor().

    If you are not using \l{MatrixItem}s and you don't want to use a
    QLineEdit as the default editor, subclass MatrixUI and reimplement
    this function with code like this:
    \snippet doc/src/snippets/code/src_qt3support_itemviews_q3table.cpp 5
    Ownership of the editor widget is transferred to the caller.

    If you reimplement this function return 0 for read-only cells. You
    will need to reimplement setCellContentFromEditor() to retrieve
    the data the user entered.

    \sa MatrixItem::createEditor()
*/

QWidget *MatrixUI::createEditor(int row, int col, bool initFromCell) const
{
    if (isReadOnly() || isRowReadOnly(row) || isColumnReadOnly(col))
        return 0;

    QWidget *e = 0;

    // the current item in the cell should be edited if possible
    MatrixItem *i = item(row, col);
    if (initFromCell || (i && !i->isReplaceable())) {
        if (i) {
            if (i->editType() == MatrixItem::Never)
                return 0;

            e = i->createEditor();
            if (!e)
                return 0;
        }
    }

    // no contents in the cell yet, so open the default editor
    if (!e) {
        e = new QLineEdit(viewport()/**, "qt_lineeditor"*/);
        ((QLineEdit*)e)->setFrame(false);
    }

    return e;
}

/*!
    This function is called to start in-place editing of the cell at
    \a row, \a col. Editing is achieved by creating an editor
    (createEditor() is called) and setting the cell's editor with
    setCellWidget() to the newly created editor. (After editing is
    complete endEdit() will be called to replace the cell's content
    with the editor's content.) If \a replace is true the editor will
    start empty; otherwise it will be initialized with the cell's
    content (if any), i.e. the user will be modifying the original
    cell content.

    \sa endEdit()
*/

QWidget *MatrixUI::beginEdit(int row, int col, bool replace)
{
    if (isReadOnly() || isRowReadOnly(row) || isColumnReadOnly(col))
        return 0;
    if ( row < 0 || row >= numRows() || col < 0 || col >= numCols() )
        return 0;
    MatrixItem *itm = item(row, col);
    if (itm && !itm->isEnabled())
        return 0;
    if (cellWidget(row, col))
        return 0;
    ensureCellVisible(row, col);
    QLineEdit *e = static_cast<QLineEdit *>(createEditor(row, col, !replace));
    if (!e)
        return 0;
    setCellWidget(row, col, e);
    e->setActiveWindow();
    e->setFocus();
    updateCell(row, col);
    e->show();

//! altered
    bool ok1 = false, ok2 = false;
    double num = e->text().toInt(& ok1);
    if (! ok1) num = e->text().toDouble(& ok2);
    if ((ok1 || ok2) && num == 0.0) e->selectAll();

    return e;
}

/*!
    This function is called when in-place editing of the cell at \a
    row, \a col is requested to stop.

    If the cell is not being edited or \a accept is false the function
    returns and the cell's contents are left unchanged.

    If \a accept is true the content of the editor must be transferred
    to the relevant cell. If \a replace is true the current content of
    this cell should be replaced by the content of the editor (this
    means removing the current MatrixItem of the cell and creating a
    new one for the cell). Otherwise (if possible) the content of the
    editor should just be set to the existing MatrixItem of this cell.

    setCellContentFromEditor() is called to replace the contents of
    the cell with the contents of the cell's editor.

    Finally clearCellWidget() is called to remove the editor widget.

    \sa setCellContentFromEditor(), beginEdit()
*/

void MatrixUI::endEdit(int row, int col, bool accept, bool replace)
{
    QWidget *editor = cellWidget(row, col);
    if (!editor)
        return;

    if (!accept) {
        if (row == editRow && col == editCol)
            setEditMode(NotEditing, -1, -1);
        clearCellWidget(row, col);
        updateCell(row, col);
        viewport()->setFocus();
        updateCell(row, col);
        return;
    }

    MatrixItem *i = item(row, col);
    QString oldContent;
    if (i)
        oldContent = i->text();

    if (!i || replace) {
        setCellContentFromEditor(row, col);
        i = item(row, col);
    } else {
        i->setContentFromEditor(editor);
    }

    if (row == editRow && col == editCol)
        setEditMode(NotEditing, -1, -1);

    viewport()->setFocus();
    updateCell(row, col);

    if (!i || (oldContent != i->text()))
        emit valueChanged(row, col);

    clearCellWidget(row, col);
}

/*!
    This function is called to replace the contents of the cell at \a
    row, \a col with the contents of the cell's editor.

    If there already exists a MatrixItem for the cell,
    it calls MatrixItem::setContentFromEditor() on this MatrixItem.

    If, for example, you want to create different \l{MatrixItem}s
    depending on the contents of the editor, you might reimplement
    this function.

    If you want to work without \l{MatrixItem}s, you will need to
    reimplement this function to save the data the user entered into
    your data structure. (See the notes on large tables.)

    \sa MatrixItem::setContentFromEditor() createEditor()
*/

void MatrixUI::setCellContentFromEditor(int row, int col)
{
    QWidget *editor = cellWidget(row, col);
    if (!editor)
        return;

    MatrixItem *i = item(row, col);
    if (i) {
        i->setContentFromEditor(editor);
    } else {
        QLineEdit *le = qobject_cast<QLineEdit*>(editor);
        if (le)
            setText(row, col, le->text());
    }
}

/*!
    Returns true if the \l EditMode is \c Editing or \c Replacing;
    otherwise (i.e. the \l EditMode is \c NotEditing) returns false.

    \sa MatrixUI::EditMode
*/

bool MatrixUI::isEditing() const
{
    return edMode != NotEditing;
}

/*!
    Returns the current edit mode

    \sa MatrixUI::EditMode
*/

MatrixUI::EditMode MatrixUI::editMode() const
{
    return edMode;
}

/*!
    Returns the current edited row
*/

int MatrixUI::currEditRow() const
{
    return editRow;
}

/*!
    Returns the current edited column
*/

int MatrixUI::currEditCol() const
{
    return editCol;
}

/*!
    Returns a single integer which identifies a particular \a row and \a
    col by mapping the 2D table to a 1D array.

    This is useful, for example, if you have a sparse table and want to
    use a Q3IntDict to map integers to the cells that are used.
*/

int MatrixUI::indexOf(int row, int col) const
{
    return (row * numCols()) + col;
}

/*! \internal
*/

void MatrixUI::repaintSelections(MatrixSelection *oldSelection,
                                MatrixSelection *newSelection,
                                bool updateVertical, bool updateHorizontal)
{
    if (!oldSelection && !newSelection)
        return;
    if (oldSelection && newSelection && *oldSelection == *newSelection)
        return;
    if (oldSelection && !oldSelection->isActive())
         oldSelection = 0;

    bool optimizeOld = false;
    bool optimizeNew = false;

    QRect old;
    if (oldSelection)
        old = rangeGeometry(oldSelection->topRow(),
                             oldSelection->leftCol(),
                             oldSelection->bottomRow(),
                             oldSelection->rightCol(),
                             optimizeOld);
    else
        old = QRect(0, 0, 0, 0);

    QRect cur;
    if (newSelection)
        cur = rangeGeometry(newSelection->topRow(),
                             newSelection->leftCol(),
                             newSelection->bottomRow(),
                             newSelection->rightCol(),
                             optimizeNew);
    else
        cur = QRect(0, 0, 0, 0);
    int i;

    if (!optimizeOld || !optimizeNew ||
         old.width() > SHRT_MAX || old.height() > SHRT_MAX ||
         cur.width() > SHRT_MAX || cur.height() > SHRT_MAX) {
        QRect rr = cur.united(old);
        viewport()->repaint(rr);
    } else {
        old = QRect(contentsToViewport2(old.topLeft()), old.size());
        cur = QRect(contentsToViewport2(cur.topLeft()), cur.size());
        QRegion r1(old);
        QRegion r2(cur);
        QRegion r3 = r1.subtracted(r2);
        QRegion r4 = r2.subtracted(r1);

        for (i = 0; i < (int)r3.rects().count(); ++i) {
            QRect r(r3.rects()[ i ]);
            r = QRect(viewportToContents2(r.topLeft()), r.size());
            viewport()->repaint(r);
        }
        for (i = 0; i < (int)r4.rects().count(); ++i) {
            QRect r(r4.rects()[ i ]);
            r = QRect(viewportToContents2(r.topLeft()), r.size());
            viewport()->repaint(r);
        }
    }

    int top, left, bottom, right;
    {
        int oldTopRow = oldSelection ? oldSelection->topRow() : numRows() - 1;
        int newTopRow = newSelection ? newSelection->topRow() : numRows() - 1;
        top = qMin(oldTopRow, newTopRow);
    }

    {
        int oldLeftCol = oldSelection ? oldSelection->leftCol() : numCols() - 1;
        int newLeftCol = newSelection ? newSelection->leftCol() : numCols() - 1;
        left = qMin(oldLeftCol, newLeftCol);
    }

    {
        int oldBottomRow = oldSelection ? oldSelection->bottomRow() : 0;
        int newBottomRow = newSelection ? newSelection->bottomRow() : 0;
        bottom = qMax(oldBottomRow, newBottomRow);
    }

    {
        int oldRightCol = oldSelection ? oldSelection->rightCol() : 0;
        int newRightCol = newSelection ? newSelection->rightCol() : 0;
        right = qMax(oldRightCol, newRightCol);
    }

    if (updateHorizontal && numCols() > 0 && left >= 0 && !isRowSelection(selectionMode())) {
        register int *s = &topHeader->states.data()[left];
        for (i = left; i <= right; ++i) {
            if (!isColumnSelected(i))
                *s = MatrixHeader::Normal;
            else if (isColumnSelected(i, true))
                *s = MatrixHeader::Selected;
            else
                *s = MatrixHeader::Bold;
            ++s;
        }
        topHeader->repaint();
    }

    if (updateVertical && numRows() > 0 && top >= 0) {
        register int *s = &leftHeader->states.data()[top];
        for (i = top; i <= bottom; ++i) {
            if (!isRowSelected(i))
                *s = MatrixHeader::Normal;
            else if (isRowSelected(i, true))
                *s = MatrixHeader::Selected;
            else
                *s = MatrixHeader::Bold;
            ++s;
        }
        leftHeader->repaint();
    }
}

/*!
    Repaints all selections
*/

void MatrixUI::repaintSelections()
{
    if (selections.isEmpty())
        return;

    QRect r;
    for (QLinkedList<MatrixSelection *>::iterator s = selections.begin(); s != selections.end(); ++s) {
        bool b;
        r = r.united(rangeGeometry((*s)->topRow(),
                                    (*s)->leftCol(),
                                    (*s)->bottomRow(),
                                    (*s)->rightCol(), b));
    }

    repaint(r);
}

/*!
    Clears all selections and repaints the appropriate regions if \a
    repaint is true.

    \sa removeSelection()
*/

void MatrixUI::clearSelection(bool repaint)
{
    if (selections.isEmpty())
        return;
    bool needRepaint = !selections.isEmpty();

    QRect r;
    for (QLinkedList<MatrixSelection *>::iterator s = selections.begin(); s != selections.end(); ++s) {
        bool b;
        r = r.united(rangeGeometry((*s)->topRow(),
                                   (*s)->leftCol(),
                                   (*s)->bottomRow(),
                                   (*s)->rightCol(), b));
    }

    currentSel = 0;
    selections.clear();

    if (needRepaint && repaint)
        viewport()->repaint(r);

    leftHeader->setSectionStateToAll(MatrixHeader::Normal);
    leftHeader->repaint();
    if (!isRowSelection(selectionMode())) {
        topHeader->setSectionStateToAll(MatrixHeader::Normal);
        topHeader->repaint();
    }
    topHeader->setSectionState(curCol, MatrixHeader::Bold);
    leftHeader->setSectionState(curRow, MatrixHeader::Bold);
    emit selectionChanged();
}

/*! \internal
*/

QRect MatrixUI::rangeGeometry(int topRow, int leftCol,
                             int bottomRow, int rightCol, bool &optimize)
{
    topRow = qMax(topRow, rowAt(visibleRegion().boundingRect().y()));
    leftCol = qMax(leftCol, columnAt(visibleRegion().boundingRect().x()));
    int ra = rowAt(visibleRegion().boundingRect().y() + viewport()->height());
    if (ra != -1)
        bottomRow = qMin(bottomRow, ra);
    int ca = columnAt(visibleRegion().boundingRect().x() + viewport()->width());
    if (ca != -1)
        rightCol = qMin(rightCol, ca);
    optimize = true;
    QRect rect;
    for (int r = topRow; r <= bottomRow; ++r) {
        for (int c = leftCol; c <= rightCol; ++c) {
            rect = rect.united(cellGeometry(r, c));
            MatrixItem *i = item(r, c);
            if (i && (i->rowSpan() > 1 || i->colSpan() > 1))
                optimize = false;
        }
    }
    return rect;
}

/*!
    This function is called to activate the next cell if in-place
    editing was finished by pressing the Enter key.

    The default behaviour is to move from top to bottom, i.e. move to
    the cell beneath the cell being edited. Reimplement this function
    if you want different behaviour, e.g. moving from left to right.
*/

void MatrixUI::activateNextCell()
{
    int firstRow = 0;
    while (d->hiddenRows.find(firstRow) != d->hiddenRows.end())
        firstRow++;
    int firstCol = 0;
    while (d->hiddenCols.find(firstCol) != d->hiddenCols.end())
        firstCol++;
    int nextRow = curRow;
    int nextCol = curCol;
    while (d->hiddenRows.find(++nextRow) != d->hiddenRows.end()) {}
    if (nextRow >= numRows()) {
        nextRow = firstRow;
        while (d->hiddenCols.find(++nextCol) != d->hiddenCols.end()) {}
        if (nextCol >= numCols())
            nextCol = firstCol;
    }

//! altered
	clearSelection();

    if (!currentSel || !currentSel->isActive() ||
         (currentSel->leftCol() == currentSel->rightCol() &&
           currentSel->topRow() == currentSel->bottomRow())) {
        clearSelection();
        setCurrentCell(nextRow, nextCol);
    } else {
        if (curRow < currentSel->bottomRow())
            setCurrentCell(nextRow, curCol);
        else if (curCol < currentSel->rightCol())
            setCurrentCell(currentSel->topRow(), nextCol);
        else
            setCurrentCell(currentSel->topRow(), currentSel->leftCol());
    }

}

/*! \internal
*/

void MatrixUI::fixRow(int &row, int y)
{
    if (row == -1) {
        if (y < 0)
            row = 0;
        else
            row = numRows() - 1;
    }
}

/*! \internal
*/

void MatrixUI::fixCol(int &col, int x)
{
    if (col == -1) {
        if (x < 0)
            col = 0;
        else
            col = numCols() - 1;
    }
}

struct SortableTableItem
{
    MatrixItem *item;
};

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

#ifdef Q_OS_WINCE
static int _cdecl cmpTableItems(const void *n1, const void *n2)
#else
static int cmpTableItems(const void *n1, const void *n2)
#endif
{
    if (!n1 || !n2)
        return 0;

    SortableTableItem *i1 = (SortableTableItem *)n1;
    SortableTableItem *i2 = (SortableTableItem *)n2;

    return i1->item->key().localeAwareCompare(i2->item->key());
}

#if defined(Q_C_CALLBACKS)
}
#endif

/*!
    Sorts column \a col. If \a ascending is true the sort is in
    ascending order, otherwise the sort is in descending order.

    If \a wholeRows is true, entire rows are sorted using swapRows();
    otherwise only cells in the column are sorted using swapCells().

    Note that if you are not using MatrixItems you will need to
    reimplement swapRows() and swapCells(). (See the notes on large
    tables.)

    \sa swapRows()
*/

void MatrixUI::sortColumn(int col, bool ascending, bool wholeRows)
{
    int filledRows = 0, i;
    for (i = 0; i < numRows(); ++i) {
        MatrixItem *itm = item(i, col);
        if (itm)
            filledRows++;
    }

    if (!filledRows)
        return;

    SortableTableItem *items = new SortableTableItem[ filledRows ];
    int j = 0;
    for (i = 0; i < numRows(); ++i) {
        MatrixItem *itm = item(i, col);
        if (!itm)
            continue;
        items[ j++ ].item = itm;
    }

    qsort(items, filledRows, sizeof(SortableTableItem), cmpTableItems);

    bool updatesWereEnabled = updatesEnabled();
    if (updatesWereEnabled)
        setUpdatesEnabled(false);
    for (i = 0; i < numRows(); ++i) {
        if (i < filledRows) {
            if (ascending) {
                if (items[ i ].item->row() == i)
                    continue;
                if (wholeRows)
                    swapRows(items[ i ].item->row(), i);
                else
                    swapCells(items[ i ].item->row(), col, i, col);
            } else {
                if (items[ i ].item->row() == filledRows - i - 1)
                    continue;
                if (wholeRows)
                    swapRows(items[ i ].item->row(), filledRows - i - 1);
                else
                    swapCells(items[ i ].item->row(), col,
                               filledRows - i - 1, col);
            }
        }
    }
    if (updatesWereEnabled)
        setUpdatesEnabled(true);
    if (topHeader)
         topHeader->setSortIndicator(col, ascending ? Qt::AscendingOrder : Qt::DescendingOrder);

    if (!wholeRows)
        repaint(columnPos(col), visibleRegion().boundingRect().y(),
                         columnWidth(col), viewport()->height());
    else
        repaint(visibleRegion().boundingRect().x(), visibleRegion().boundingRect().y(),
                         viewport()->width(), viewport()->height());

    delete [] items;
}

/*!
    Hides row \a row.

    \sa showRow() hideColumn()
*/

void MatrixUI::hideRow(int row)
{
    if (d->hiddenRows.find(row) != d->hiddenRows.end())
        return;
    d->hiddenRows[row] = int(leftHeader->sectionSize(row));
    leftHeader->resizeSection(row, 0);
    leftHeader->setSectionResizeMode(QHeaderView::Fixed);
    if (isRowStretchable(row))
        leftHeader->numStretches--;
    rowHeightChanged(row);
    if (curRow == row) {
        int r = curRow;
        int c = curCol;
        int k = (r >= numRows() - 1 ? Key_Up : Key_Down);
        fixCell(r, c, k);
        if (numRows() > 0)
            setCurrentCell(r, c);
    }
}

/*!
    Hides column \a col.

    \sa showColumn() hideRow()
*/

void MatrixUI::hideColumn(int col)
{
    if (!numCols() || d->hiddenCols.find(col) != d->hiddenCols.end())
        return;
    d->hiddenCols[col] = int(topHeader->sectionSize(col));
    topHeader->resizeSection(col, 0);
    topHeader->setSectionResizeMode(QHeaderView::Fixed);
    if (isColumnStretchable(col))
        topHeader->numStretches--;
    columnWidthChanged(col);
    if (curCol == col) {
        int r = curRow;
        int c = curCol;
        int k = (c >= numCols() - 1 ? Key_Left : Key_Right);
        fixCell(r, c, k);
        if (numCols() > 0)
            setCurrentCell(r, c);
    }
}

/*!
    Shows row \a row.

    \sa hideRow() showColumn()
*/

void MatrixUI::showRow(int row)
{
    QHash<int, int>::iterator h = d->hiddenRows.find(row);
    if (h != d->hiddenRows.end()) {
        int rh = *h;
        d->hiddenRows.remove(row);
        setRowHeight(row, rh);
        if (isRowStretchable(row))
            leftHeader->numStretches++;
    } else if (rowHeight(row) == 0) {
        setRowHeight(row, 20);
    }
    leftHeader->setSectionResizeMode(QHeaderView::Interactive);
}

/*!
    Shows column \a col.

    \sa hideColumn() showRow()
*/

void MatrixUI::showColumn(int col)
{
    QHash<int, int>::iterator w = d->hiddenCols.find(col);
    if (w != d->hiddenCols.end()) {
        int cw = *w;
        d->hiddenCols.remove(col);
        setColumnWidth(col, cw);
        if (isColumnStretchable(col))
            topHeader->numStretches++;
    } else if (columnWidth(col) == 0) {
        setColumnWidth(col, 20);
    }
    topHeader->setSectionResizeMode(QHeaderView::Interactive);
}

/*!
    Returns true if row \a row is hidden; otherwise returns
    false.

    \sa hideRow(), isColumnHidden()
*/
bool MatrixUI::isRowHidden(int row) const
{
    return d->hiddenRows.find(row) != d->hiddenRows.end();
}

/*!
    Returns true if column \a col is hidden; otherwise returns
    false.

    \sa hideColumn(), isRowHidden()
*/
bool MatrixUI::isColumnHidden(int col) const
{
    return d->hiddenCols.find(col) != d->hiddenCols.end();
}

/*!
    Resizes column \a col to be \a w pixels wide.

    \sa columnWidth() setRowHeight()
*/

void MatrixUI::setColumnWidth(int col, int w)
{
    QHash<int, int>::iterator ow = d->hiddenCols.find(col);
    if (ow != d->hiddenCols.end()) {
        d->hiddenCols[col] = int(w);
    } else {
        topHeader->resizeSection(col, w);
        columnWidthChanged(col);
    }
}

/*!
    Resizes row \a row to be \a h pixels high.

    \sa rowHeight() setColumnWidth()
*/

void MatrixUI::setRowHeight(int row, int h)
{
    QHash<int, int>::iterator oh = d->hiddenRows.find(row);
    if (oh != d->hiddenCols.end()) {
        d->hiddenRows[row] = int(h);
    } else {
        leftHeader->resizeSection(row, h);
        rowHeightChanged(row);
    }
}

//! altered
void MatrixUI::adjustButtons()
{
    int xh = topHeader->width() - topHeader->offset();
	int yh = 0;
	int xv = (leftHeader->width() - d->b[2]->width()) / 2;
    int yv = leftHeader->width() - leftHeader->offset();

	d->b[0]->move(xh, yh);
	d->b[1]->move(xh + d->b[0]->width(), yh);
	d->b[2]->move(xv, yv);
	d->b[3]->move(xv, yv + d->b[2]->height());
}

/*!
    Resizes column \a col so that the column width is wide enough to
    display the widest item the column contains.

    \sa adjustRow()
*/

void MatrixUI::adjustColumn(int col)
{
    int w;
    if ( currentColumn() == col ) {
        QFont f = font();
        f.setBold(true);
        w = topHeader->sectionSizeHint( col );
    } else {
        w = topHeader->sectionSizeHint( col );
    }
    if (topHeader->iconSet(col))
        w += topHeader->iconSet(col)->pixmap().width();
    w = qMax(w, 20);
    for (int i = 0; i < numRows(); ++i) {
        MatrixItem *itm = item(i, col);
        if (!itm) {
            QWidget *widget = cellWidget(i, col);
            if (widget)
                w = qMax(w, widget->sizeHint().width());
        } else {
            if (itm->colSpan() > 1)
                w = qMax(w, itm->sizeHint().width() / itm->colSpan());
            else
                w = qMax(w, itm->sizeHint().width());
        }
    }
    w = qMax(w, QApplication::globalStrut().width());
    setColumnWidth(col, w);
}

/*!
    Resizes row \a row so that the row height is tall enough to
    display the tallest item the row contains.

    \sa adjustColumn()
*/

void MatrixUI::adjustRow(int row)
{
    int h = 20;
    h = qMax(h, leftHeader->sectionSizeHint(row));
    if (leftHeader->iconSet(row))
        h = qMax(h, leftHeader->iconSet(row)->pixmap().height());
    for (int i = 0; i < numCols(); ++i) {
        MatrixItem *itm = item(row, i);
        if (!itm) {
            QWidget *widget = cellWidget(row, i);
            if (widget)
                h = qMax(h, widget->sizeHint().height());
        } else {
            if (itm->rowSpan() > 1)
                h = qMax(h, itm->sizeHint().height() / itm->rowSpan());
            else
                h = qMax(h, itm->sizeHint().height());
        }
    }
    h = qMax(h, QApplication::globalStrut().height());
    setRowHeight(row, h);
}

/*!
    If \a stretch is true, column \a col is set to be stretchable;
    otherwise column \a col is set to be unstretchable.

    If the table widget's width decreases or increases stretchable
    columns will grow narrower or wider to fit the space available as
    completely as possible. The user cannot manually resize stretchable
    columns.

    \sa isColumnStretchable() setRowStretchable() adjustColumn()
*/

void MatrixUI::setColumnStretchable(int col, bool stretch)
{
    topHeader->setSectionStretchable(col, stretch);

    if (stretch && d->hiddenCols.find(col) != d->hiddenCols.end())
        topHeader->numStretches--;
}

/*!
    If \a stretch is true, row \a row is set to be stretchable;
    otherwise row \a row is set to be unstretchable.

    If the table widget's height decreases or increases stretchable
    rows will grow shorter or taller to fit the space available as
    completely as possible. The user cannot manually resize
    stretchable rows.

    \sa isRowStretchable() setColumnStretchable()
*/

void MatrixUI::setRowStretchable(int row, bool stretch)
{
    leftHeader->setSectionStretchable(row, stretch);

    if (stretch && d->hiddenRows.find(row) != d->hiddenRows.end())
        leftHeader->numStretches--;
}

/*!
    Returns true if column \a col is stretchable; otherwise returns
    false.

    \sa setColumnStretchable() isRowStretchable()
*/

bool MatrixUI::isColumnStretchable(int col) const
{
    return topHeader->isSectionStretchable(col);
}

/*!
    Returns true if row \a row is stretchable; otherwise returns
    false.

    \sa setRowStretchable() isColumnStretchable()
*/

bool MatrixUI::isRowStretchable(int row) const
{
    return leftHeader->isSectionStretchable(row);
}

/*!
    Takes the table item \a i out of the table. This function does \e
    not delete the table item. You must either delete the table item
    yourself or put it into a table (using setItem()) which will then
    take ownership of it.

    Use this function if you want to move an item from one cell in a
    table to another, or to move an item from one table to another,
    reinserting the item with setItem().

    If you want to exchange two cells use swapCells().
*/

void MatrixUI::takeItem(MatrixItem *i)
{
    if (!i)
        return;
    if (i->row() != -1 && i->col() != -1) {
        QRect rect = cellGeometry(i->row(), i->col());
        contents.setAutoDelete(false);
        int bottom = i->row() + i->rowSpan();
        if (bottom > numRows())
            bottom = numRows();
        int right = i->col() + i->colSpan();
        if (right > numCols())
            right = numCols();
        for (int r = i->row(); r < bottom; ++r) {
            for (int c = i->col(); c < right; ++c)
                contents.remove(indexOf(r, c));
        }
        contents.setAutoDelete(true);
        viewport()->repaint(rect);
        int orow = i->row();
        int ocol = i->col();
        i->setRow(-1);
        i->setCol(-1);
        i->updateEditor(orow, ocol);
    }
    i->t = 0;
}

/*!
    Sets the widget \a e to the cell at \a row, \a col and takes care of
    placing and resizing the widget when the cell geometry changes.

    By default widgets are inserted into a vector with numRows() *
    numCols() elements. In very large tables you will probably want to
    store the widgets in a data structure that consumes less memory (see
    the notes on large tables). To support the use of your own data
    structure this function calls insertWidget() to add the widget to
    the internal data structure. To use your own data structure
    reimplement insertWidget(), cellWidget() and clearCellWidget().

    Cell widgets are created dynamically with the \c new operator. The
    cell widgets are destroyed automatically once the table is
    destroyed; the table takes ownership of the widget when using
    setCellWidget.

*/

void MatrixUI::setCellWidget(int row, int col, QWidget *e)
{
    if (!e || row >= numRows() || col >= numCols())
        return;

    QWidget *w = cellWidget(row, col);
    if (w && row == editRow && col == editCol)
        endEdit(editRow, editCol, false, edMode != Editing);

    e->installEventFilter(this);
    clearCellWidget(row, col);
    if (e->parent() != viewport())
        e->reparent(viewport(), QPoint(0,0));
    MatrixItem *itm = item(row, col);
    if (itm && itm->row() >= 0 && itm->col() >= 0) { // get the correct row and col if the item is spanning
        row = itm->row();
        col = itm->col();
    }
    insertWidget(row, col, e);
    QRect cr = cellGeometry(row, col);
    e->resize(cr.size());
    moveChild(e, cr.x(), cr.y());
    e->show();
}

/*!
    Inserts widget \a w at \a row, \a col into the internal
    data structure. See the documentation of setCellWidget() for
    further details.

    If you don't use \l{MatrixItem}s you may need to reimplement this
    function: see the notes on large tables.
*/

void MatrixUI::insertWidget(int row, int col, QWidget *w)
{
    if (row < 0 || col < 0 || row > numRows() - 1 || col > numCols() - 1)
        return;

    if ((int)widgets.size() != numRows() * numCols())
        widgets.resize(numRows() * numCols());

    widgets.insert(indexOf(row, col), w);
}

/*!
    Returns the widget that has been set for the cell at \a row, \a
    col, or 0 if no widget has been set.

    If you don't use \l{MatrixItem}s you may need to reimplement this
    function: see the notes on large tables.

    \sa clearCellWidget() setCellWidget()
*/

QWidget *MatrixUI::cellWidget(int row, int col) const
{
    if (row < 0 || col < 0 || row > numRows() - 1 || col > numCols() - 1)
        return 0;

    if ((int)widgets.size() != numRows() * numCols())
        ((MatrixUI*)this)->widgets.resize(numRows() * numCols());

    return widgets[ indexOf(row, col) ];
}

/*!
    Removes the widget (if there is one) set for the cell at \a row,
    \a col.

    If you don't use \l{MatrixItem}s you may need to reimplement this
    function: see the notes on large tables.

    This function deletes the widget at \a row, \a col. Note that the
    widget is not deleted immediately; instead QObject::deleteLater()
    is called on the widget to avoid problems with timing issues.

    \sa cellWidget() setCellWidget()
*/

void MatrixUI::clearCellWidget(int row, int col)
{
    if (row < 0 || col < 0 || row > numRows() - 1 || col > numCols() - 1)
        return;

    if ((int)widgets.size() != numRows() * numCols())
        widgets.resize(numRows() * numCols());

    QWidget *w = cellWidget(row, col);
    if (w) {
        w->removeEventFilter(this);
        w->hide();
        w->deleteLater();
    }
    widgets.setAutoDelete(false);
    widgets.remove(indexOf(row, col));
    widgets.setAutoDelete(true);
}

/*!
    \fn void MatrixUI::dropped (QDropEvent * e)

    This signal is emitted when a drop event occurred on the table.

    \a e contains information about the drop.
*/

/*!
    If \a b is true, the table starts a drag (see dragObject()) when
    the user presses and moves the mouse on a selected cell.
*/

void MatrixUI::setDragEnabled(bool b)
{
    dEnabled = b;
}

/*!
    If this function returns true, the table supports dragging.

    \sa setDragEnabled()
*/

bool MatrixUI::dragEnabled() const
{
    return dEnabled;
}

/*!
    Inserts \a count empty rows at row \a row. Also clears the selection(s).

    \sa insertColumns() removeRow()
*/

void MatrixUI::insertRows(int row, int count)
{
    // special case, so a call like insertRow(currentRow(), 1) also
    // works, when we have 0 rows and currentRow() is -1
    if (row == -1 && curRow == -1)
        row = 0;
    if (row < 0 || count <= 0)
        return;

    if (curRow >= row && curRow < row + count)
        curRow = row + count;

    --row;
    if (row >= numRows())
        return;

    bool updatesWereEnabled = updatesEnabled();
    if (updatesWereEnabled)
        setUpdatesEnabled(false);
    bool leftHeaderUpdatesEnabled = leftHeader->updatesEnabled();
    if (leftHeaderUpdatesEnabled)
        leftHeader->setUpdatesEnabled(false);
    int oldLeftMargin = contentsMargins().left();

    setNumRows(numRows() + count);

    for (int i = numRows() - count - 1; i > row; --i)
        leftHeader->swapSections(i, i + count);

    if (leftHeaderUpdatesEnabled)
        leftHeader->setUpdatesEnabled(leftHeaderUpdatesEnabled);

    if (updatesWereEnabled)
        setUpdatesEnabled(true);

    int cr = qMax(0, currentRow());
    int cc = qMax(0, currentColumn());
    if (curRow > row)
        curRow -= count; // this is where curRow was
    setCurrentCell(cr, cc, true, false); // without ensureCellVisible

    // Repaint the header
    if (leftHeaderUpdatesEnabled) {
        int y = rowPos(row) - visibleRegion().boundingRect().y();
        if (contentsMargins().left() != oldLeftMargin || d->hasRowSpan)
            y = 0; // full repaint
        QRect rect(0, y, leftHeader->width(), viewport()->height());
        leftHeader->update(rect);
    }

    if (updatesWereEnabled) {
        int p = rowPos(row);
        if (d->hasRowSpan)
            p = visibleRegion().boundingRect().y();
        viewport()->update(visibleRegion().boundingRect().x(), p, viewport()->width(), viewport()->height() + 1);
    }
}

/*!
    Inserts \a count empty columns at column \a col.  Also clears the selection(s).

    \sa insertRows() removeColumn()
*/

void MatrixUI::insertColumns(int col, int count)
{
    // see comment in insertRows()
    if (col == -1 && curCol == -1)
        col = 0;
    if (col < 0 || count <= 0)
        return;

    if (curCol >= col && curCol < col + count)
        curCol = col + count;

    --col;
    if (col >= numCols())
        return;

    bool updatesWereEnabled = updatesEnabled();
    if (updatesWereEnabled)
        setUpdatesEnabled(false);
    bool topHeaderUpdatesEnabled = topHeader->updatesEnabled();
    if (topHeaderUpdatesEnabled)
        topHeader->setUpdatesEnabled(false);
    int oldTopMargin = contentsMargins().top();

    setNumCols(numCols() + count);

    for (int i = numCols() - count - 1; i > col; --i)
        topHeader->swapSections(i, i + count);

    if (topHeaderUpdatesEnabled)
        topHeader->setUpdatesEnabled(true);
    if (updatesWereEnabled)
        setUpdatesEnabled(true);

    int cr = qMax(0, currentRow());
    int cc = qMax(0, currentColumn());
    if (curCol > col)
        curCol -= count; // this is where curCol was
    setCurrentCell(cr, cc, true, false); // without ensureCellVisible

    // Repaint the header
    if (topHeaderUpdatesEnabled) {
        int x = columnPos(col) - visibleRegion().boundingRect().x();
        if (contentsMargins().top() != oldTopMargin || d->hasColSpan)
            x = 0; // full repaint
        QRect rect(x, 0, visibleRegion().boundingRect().width(), topHeader->height());
        topHeader->update(rect);
    }

    if (updatesWereEnabled) {
        int p = columnPos(col);
        if (d->hasColSpan)
            p = visibleRegion().boundingRect().x();
        update(p, visibleRegion().boundingRect().y(), visibleRegion().boundingRect().width() + 1, viewport()->height());
    }
}

/*!
    Removes row \a row, and deletes all its cells including any table
    items and widgets the cells may contain. Also clears the selection(s).

    \sa hideRow() insertRows() removeColumn() removeRows()
*/

void MatrixUI::removeRow(int row)
{
    if (row < 0 || row >= numRows())
        return;
    if (row < numRows() - 1) {
        if (d->hiddenRows.find(row) != d->hiddenRows.end())
            d->hiddenRows.remove(row);

        for (int i = row; i < numRows() - 1; ++i)
            ((MatrixHeader*)verticalHeader())->swapSections(i, i + 1);
    }
    setNumRows(numRows() - 1);
}

/*!
    Removes the rows listed in the array \a rows, and deletes all their
    cells including any table items and widgets the cells may contain.

    The array passed in must only contain valid rows (in the range
    from 0 to numRows() - 1) with no duplicates, and must be sorted in
    ascending order. Also clears the selection(s).

    \sa removeRow() insertRows() removeColumns()
*/

void MatrixUI::removeRows(const Q3MemArray<int> &rows)
{
    if (rows.count() == 0)
        return;
    int i;
    for (i = 0; i < (int)rows.count() - 1; ++i) {
        for (int j = rows[i] - i; j < rows[i + 1] - i - 1; j++) {
            ((MatrixHeader*)verticalHeader())->swapSections(j, j + i + 1);
        }
    }

    for (int j = rows[i] - i; j < numRows() - (int)rows.size(); j++)
        ((MatrixHeader*)verticalHeader())->swapSections(j, j + rows.count());

    setNumRows(numRows() - rows.count());
}

/*!
    Removes column \a col, and deletes all its cells including any
    table items and widgets the cells may contain. Also clears the
    selection(s).

    \sa removeColumns() hideColumn() insertColumns() removeRow()
*/

void MatrixUI::removeColumn(int col)
{
    if (col < 0 || col >= numCols())
        return;
    if (col < numCols() - 1) {
        if (d->hiddenCols.find(col) != d->hiddenCols.end())
            d->hiddenCols.remove(col);

        for (int i = col; i < numCols() - 1; ++i)
            ((MatrixHeader*)horizontalHeader())->swapSections(i, i + 1);
    }
    setNumCols(numCols() - 1);
}

/*!
    Removes the columns listed in the array \a cols, and deletes all
    their cells including any table items and widgets the cells may
    contain.

    The array passed in must only contain valid columns (in the range
    from 0 to numCols() - 1) with no duplicates, and must be sorted in
    ascending order. Also clears the selection(s).

   \sa removeColumn() insertColumns() removeRows()
*/

void MatrixUI::removeColumns(const Q3MemArray<int> &cols)
{
    if (cols.count() == 0)
        return;
    int i;
    for (i = 0; i < (int)cols.count() - 1; ++i) {
        for (int j = cols[i] - i; j < cols[i + 1] - i - 1; j++) {
            ((MatrixHeader*)horizontalHeader())->swapSections(j, j + i + 1);
        }
    }

    for (int j = cols[i] - i; j < numCols() - (int)cols.size(); j++)
        ((MatrixHeader*)horizontalHeader())->swapSections(j, j + cols.count());

    setNumCols(numCols() - cols.count());
}

/*!
    Starts editing the cell at \a row, \a col.

    If \a replace is true the content of this cell will be replaced by
    the content of the editor when editing is finished, i.e. the user
    will be entering new data; otherwise the current content of the
    cell (if any) will be modified in the editor.

    \sa beginEdit()
*/

void MatrixUI::editCell(int row, int col, bool replace)
{
    if (row < 0 || col < 0 || row > numRows() - 1 || col > numCols() - 1)
        return;

    if (beginEdit(row, col, replace)) {
        edMode = Editing;
        editRow = row;
        editCol = col;
    }
}

#ifndef QT_NO_DRAGANDDROP

/*!
    This event handler is called whenever a MatrixUI object receives a
    \l QDragEnterEvent \a e, i.e. when the user pressed the mouse
    button to drag something.

    The focus is moved to the cell where the QDragEnterEvent occurred.
*/

void MatrixUI::contentsDragEnterEvent(QDragEnterEvent *e)
{
    oldCurrentRow = curRow;
    oldCurrentCol = curCol;
    int tmpRow = rowAt(e->pos().y());
    int tmpCol = columnAt(e->pos().x());
    fixRow(tmpRow, e->pos().y());
    fixCol(tmpCol, e->pos().x());
    if (e->source() != (QObject*)cellWidget(currentRow(), currentColumn()))
        setCurrentCell(tmpRow, tmpCol, false, true);
    e->accept();
}

/*!
    This event handler is called whenever a MatrixUI object receives a
    \l QDragMoveEvent \a e, i.e. when the user actually drags the
    mouse.

    The focus is moved to the cell where the QDragMoveEvent occurred.
*/

void MatrixUI::contentsDragMoveEvent(QDragMoveEvent *e)
{
    int tmpRow = rowAt(e->pos().y());
    int tmpCol = columnAt(e->pos().x());
    fixRow(tmpRow, e->pos().y());
    fixCol(tmpCol, e->pos().x());
    if (e->source() != (QObject*)cellWidget(currentRow(), currentColumn()))
        setCurrentCell(tmpRow, tmpCol, false, true);
    e->accept();
}

/*!
    This event handler is called when a drag activity leaves \e this
    MatrixUI object with event \a e.
*/

void MatrixUI::contentsDragLeaveEvent(QDragLeaveEvent *)
{
    setCurrentCell(oldCurrentRow, oldCurrentCol, false, true);
}

/*!
    This event handler is called when the user ends a drag and drop by
    dropping something onto \e this MatrixUI and thus triggers the drop
    event, \a e.
*/

void MatrixUI::contentsDropEvent(QDropEvent *e)
{
    setCurrentCell(oldCurrentRow, oldCurrentCol, false, true);
    emit dropped(e);
}

/*!
    If the user presses the mouse on a selected cell, starts moving
    (i.e. dragging), and dragEnabled() is true, this function is
    called to obtain a drag object. A drag using this object begins
    immediately unless dragObject() returns 0.

    By default this function returns 0. You might reimplement it and
    create a Q3DragObject depending on the selected items.

    \sa dropped()
*/

Q3DragObject *MatrixUI::dragObject()
{
    return 0;
}

/*!
    Starts a drag.

    Usually you don't need to call or reimplement this function yourself.

    \sa dragObject()
*/

void MatrixUI::startDrag()
{
    if (startDragRow == -1 || startDragCol == -1)
        return;

    startDragRow = startDragCol = -1;

    Q3DragObject *drag = dragObject();
    if (!drag)
        return;

    /**drag->drag();*/
}

#endif

/*! \internal */
void MatrixUI::windowActivationChange(bool oldActive)
{
    if (oldActive && autoScrollTimer)
        autoScrollTimer->stop();

    if (!isVisible())
        return;
/**
    if (palette().active() != palette().inactive())
        viewport()->update();
*/
}

/*!
    \internal
*/
void MatrixUI::setEnabled(bool b)
{
    if (!b) {
        // editor will lose focus, causing a crash deep in setEnabled(),
        // so we'll end the edit early.
        endEdit(editRow, editCol, true, edMode != Editing);
    }
    Q3ScrollView::setEnabled(b);
}


/*
    \class MatrixHeader
    \brief The MatrixHeader class allows for creation and manipulation
    of table headers.

    \compat

   MatrixUI uses this subclass of QHeaderView for its headers. MatrixUI has a
   horizontalHeader() for displaying column labels, and a
   verticalHeader() for displaying row labels.

*/

/*
    \enum MatrixHeader::SectionState

    This enum type denotes the state of the header's text

    \value Normal the default
    \value Bold
    \value Selected  typically represented by showing the section "sunken"
    or "pressed in"
*/

/*!
    Creates a new table header called \a name with \a i sections. It
    is a child of widget \a parent and attached to table \a t.
*/

MatrixHeader::MatrixHeader(Qt::Orientation o, int i, MatrixUI *t,
                            QWidget *parent, const char *name)
    : QHeaderView(o, parent), mousePressed(false), startPos(-1),
      table(t), caching(false), resizedSection(-1),
      numStretches(0)
{
    /**setIsATableHeader(true);*/
    d = 0;
    states.resize(i);
    stretchable.resize(i);
    states.fill(Normal, -1);
    stretchable.fill(false, -1);
    autoScrollTimer = new QTimer(this);
    connect(autoScrollTimer, SIGNAL(timeout()),
             this, SLOT(doAutoScroll()));
#ifndef NO_LINE_WIDGET
    line1 = new QWidget(table->viewport());
    line1->hide();
    line1->setBackgroundMode(PaletteText);
    table->addChild(line1);
    line2 = new QWidget(table->viewport());
    line2->hide();
    line2->setBackgroundMode(PaletteText);
    table->addChild(line2);
#else
    d = new MatrixHeaderPrivate;
    d->oldLinePos = -1; //outside, in contents coords
#endif
    connect(this, SIGNAL(sizeChange(int,int,int)),
             this, SLOT(sectionWidthChanged(int,int,int)));
    connect(this, SIGNAL(indexChange(int,int,int)),
             this, SLOT(indexChanged(int,int,int)));

    stretchTimer = new QTimer(this);
    widgetStretchTimer = new QTimer(this);
    connect(stretchTimer, SIGNAL(timeout()),
             this, SLOT(updateStretches()));
    connect(widgetStretchTimer, SIGNAL(timeout()),
             this, SLOT(updateWidgetStretches()));
    startPos = -1;
}

/*!
    Adds a new section, \a size pixels wide (or high for vertical
    headers) with the label \a s. If \a size is negative the section's
    size is calculated based on the width (or height) of the label's
    text.
*/

void MatrixHeader::addLabel(const QString &s , int size)
{
    QHeaderView::addLabel(s, size);
    if (count() > (int)states.size()) {
        int s = states.size();
        states.resize(count());
        stretchable.resize(count());
        for (; s < count(); ++s) {
            states[ s ] = Normal;
            stretchable[ s ] = false;
        }
    }
}

void MatrixHeader::removeLabel(int section)
{
    QHeaderView::removeLabel(section);
    if (section == (int)states.size() - 1) {
        states.resize(states.size() - 1);
        stretchable.resize(stretchable.size() - 1);
    }
}

void MatrixHeader::resizeArrays(int n)
{
    int old = states.size();
    states.resize(n);
    stretchable.resize(n);
    if (n > old) {
        for (int i = old; i < n; ++i) {
            stretchable[ i ] = false;
            states[ i ] = Normal;
        }
    }
}

void MatrixHeader::setLabel(int section, const QString & s, int size)
{
    QHeaderView::setLabel(section, s, size);
    sectionLabelChanged(section);
}

void MatrixHeader::setLabel(int section, const QIconSet & iconset,
                             const QString & s, int size)
{
    QHeaderView::setLabel(section, iconset, s, size);
    sectionLabelChanged(section);
}

/*!
    Sets the SectionState of section \a s to \a astate.

    \sa sectionState()
*/

void MatrixHeader::setSectionState(int s, SectionState astate)
{
    if (s < 0 || s >= (int)states.count())
        return;
    if (states.data()[ s ] == astate)
        return;
    if (isRowSelection(table->selectionMode()) && orientation() == Horizontal)
        return;

    states.data()[ s ] = astate;
    if (updatesEnabled()) {
        if (orientation() == Horizontal)
            repaint(sectionPos(s) - offset(), 0, sectionSize(s), height());
        else
            repaint(0, sectionPos(s) - offset(), width(), sectionSize(s));
    }
}

void MatrixHeader::setSectionStateToAll(SectionState state)
{
    if (isRowSelection(table->selectionMode()) && orientation() == Horizontal)
        return;

    register int *d = (int *) states.data();
    int n = count();

    while (n >= 4) {
        d[0] = state;
        d[1] = state;
        d[2] = state;
        d[3] = state;
        d += 4;
        n -= 4;
    }

    if (n > 0) {
        d[0] = state;
        if (n > 1) {
            d[1] = state;
            if (n > 2) {
                d[2] = state;
            }
        }
    }
}

/*!
    Returns the SectionState of section \a s.

    \sa setSectionState()
*/

MatrixHeader::SectionState MatrixHeader::sectionState(int s) const
{
    return (s < 0 || s >= (int)states.count() ? Normal : (MatrixHeader::SectionState)states[s]);
}

/*! \reimp
*/

void MatrixHeader::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setPen(palette().buttonText());
    int pos = orientation() == Horizontal
                     ? e->rect().left()
                     : e->rect().top();
    int id = mapToIndex(sectionAt(pos + offset()));
    if (id < 0) {
        if (pos > 0)
            return;
        else
            id = 0;
    }

    QRegion reg = e->region();
    for (int i = id; i < count(); i++) {
        QRect r = sRect(i);
        reg -= r;
        p.save();
        if (!(orientation() == Horizontal && isRowSelection(table->selectionMode())) &&
             (sectionState(i) == Bold || sectionState(i) == Selected)) {
            QFont f(font());
            f.setBold(true);
            p.setFont(f);
        }
        paintSection(&p, i, r);
        p.restore();
        if ((orientation() == Horizontal && r. right() >= e->rect().right())
            || (orientation() == Vertical && r. bottom() >= e->rect().bottom()))
            return;
    }
    p.end();
    if (!reg.isEmpty())
        erase(reg);
}

/*!
    \reimp

    Paints the header section with index \a index into the rectangular
    region \a fr on the painter \a p.
*/

void MatrixHeader::paintSection(QPainter *p, int index, const QRect& fr)
{
    int section = mapToSection(index);
    if (section < 0 || cellSize(section) <= 0)
        return;

   if (sectionState(index) != Selected ||
         (orientation() == Horizontal && isRowSelection(table->selectionMode()))) {
        QHeaderView::paintSection(p, fr, index);
   } else {
       QStyleOptionHeader opt;
       opt.palette = palette();
       opt.rect = fr;
       opt.state = QStyle::State_Off | (orient == Qt::Horizontal ? QStyle::State_Horizontal
                                                                 : QStyle::State_None);
       if (isEnabled())
           opt.state |= QStyle::State_Enabled;
       if (isClickEnabled()) {
           if (sectionState(index) == Selected) {
               opt.state |= QStyle::State_Sunken;
               if (!mousePressed)
                   opt.state |= QStyle::State_On;
           }
       }
       if (!(opt.state & QStyle::State_Sunken))
           opt.state |= QStyle::State_Raised;
       style()->drawControl(QStyle::CE_HeaderSection, &opt, p, this);
       paintSectionLabel(p, index, fr);
   }
}

static int real_pos(const QPoint &p, Qt::Orientation o)
{
    if (o == Qt::Horizontal)
        return p.x();
    return p.y();
}

/*! \reimp
*/

void MatrixHeader::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != LeftButton)
        return;
    QHeaderView::mousePressEvent(e);
    mousePressed = true;
    pressPos = real_pos(e->pos(), orientation());
    if (!table->currentSel || (e->buttons() & ShiftModifier) != ShiftModifier)
        startPos = -1;
    setCaching(true);
    resizedSection = -1;
#ifdef QT_NO_CURSOR
    isResizing = false;
#else
    isResizing = cursor().shape() != ArrowCursor;
    if (!isResizing && sectionAt(pressPos) != -1)
        doSelection(e);
#endif
}

/*! \reimp
*/

void MatrixHeader::mouseMoveEvent(QMouseEvent *e)
{
    if ((e->buttons() & MouseButtonMask) != LeftButton // Using LeftButton simulates old behavior.
#ifndef QT_NO_CURSOR
         || cursor().shape() != ArrowCursor
#endif
         || ((e->buttons() & ControlModifier) == ControlModifier &&
              (orientation() == Horizontal
             ? table->columnMovingEnabled() : table->rowMovingEnabled()))) {
        QHeaderView::mouseMoveEvent(e);
        return;
    }

    if (!doSelection(e))
        QHeaderView::mouseMoveEvent(e);
}

bool MatrixHeader::doSelection(QMouseEvent *e)
{
    int p = real_pos(e->pos(), orientation()) + offset();

    if (isRowSelection(table->selectionMode())) {
        if (orientation() == Horizontal)
            return true;
        if (table->selectionMode() == MatrixUI::SingleRow) {
            int secAt = sectionAt(p);
            if (secAt == -1)
                return true;
            table->setCurrentCell(secAt, table->currentColumn());
            return true;
        }
    }

    if (startPos == -1) {
         int secAt = sectionAt(p);
        if (((e->buttons() & ControlModifier) != ControlModifier && (e->buttons() & ShiftModifier) != ShiftModifier)
            || table->selectionMode() == MatrixUI::Single
            || table->selectionMode() == MatrixUI::SingleRow) {
            startPos = p;
            bool b = table->signalsBlocked();
            table->blockSignals(true);
            table->clearSelection();
            table->blockSignals(b);
        }
        saveStates();

        if (table->selectionMode() != MatrixUI::NoSelection) {
            startPos = p;
            MatrixSelection *oldSelection = table->currentSel;

            if (orientation() == Vertical) {
                if (!table->isRowSelected(secAt, true)) {
                    table->currentSel = new MatrixSelection();
                    table->selections.append(table->currentSel);
                    table->currentSel->init(secAt, 0);
                    table->currentSel->expandTo(secAt, table->numCols() - 1);
                    emit table->selectionChanged();
                }
                table->setCurrentCell(secAt, 0);
            } else { // orientation == Horizontal
                if (!table->isColumnSelected(secAt, true)) {
                    table->currentSel = new MatrixSelection();
                    table->selections.append(table->currentSel);
                    table->currentSel->init(0, secAt);
                    table->currentSel->expandTo(table->numRows() - 1, secAt);
                    emit table->selectionChanged();
                }
                table->setCurrentCell(0, secAt);
            }

            if ((orientation() == Horizontal && table->isColumnSelected(secAt))
                || (orientation() == Vertical && table->isRowSelected(secAt))) {
                setSectionState(secAt, Selected);
            }

             table->repaintSelections(oldSelection, table->currentSel,
                                       orientation() == Horizontal,
                                       orientation() == Vertical);
            if (sectionAt(p) != -1)
                 endPos = p;

             return true;
        }
    }

    if (sectionAt(p) != -1)
        endPos = p;
    if (startPos != -1) {
        updateSelections();
        p -= offset();
        if (orientation() == Horizontal && (p < 0 || p > width())) {
            doAutoScroll();
            autoScrollTimer->setSingleShot(true);
            autoScrollTimer->start(100);
        } else if (orientation() == Vertical && (p < 0 || p > height())) {
            doAutoScroll();
            autoScrollTimer->setSingleShot(true);
            autoScrollTimer->start(100);
        }
        return true;
    }
    return table->selectionMode() == MatrixUI::NoSelection;
}

static inline bool mayOverwriteMargin(int before, int after)
{
    /*
      0 is the only user value that we always respect. We also never
      shrink a margin, in case the user wanted it that way.
    */
    return before != 0 && before < after;
}

void MatrixHeader::sectionLabelChanged(int section)
{
    emit sectionSizeChanged(section);

    // this does not really belong here
    if (orientation() == Horizontal) {
        int h = sizeHint().height();
        if (h != height() && mayOverwriteMargin(table->contentsMargins().top(), h))
            table->setTopMargin(h);
    } else {
        int w = sizeHint().width();
        if (w != width() && mayOverwriteMargin((/**QApplication::reverseLayout() ? table->contentsMargins().right() :*/ table->contentsMargins().left()), w))
            table->setLeftMargin(w);
    }
}

/*! \reimp */
void MatrixHeader::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() != LeftButton)
        return;
    autoScrollTimer->stop();
    mousePressed = false;
    setCaching(false);
    QHeaderView::mouseReleaseEvent(e);
#ifndef NO_LINE_WIDGET
    line1->hide();
    line2->hide();
#else
    if (d->oldLinePos >= 0)
        if (orientation() == Horizontal)
            table->updateContents(d->oldLinePos, table->visibleRegion().boundingRect().y(),
                                   1, table->viewport()->height());
        else
            table->updateContents( table->visibleRegion().boundingRect().x(), d->oldLinePos,
                                    table->viewport()->width(), 1);
    d->oldLinePos = -1;
#endif
    if (resizedSection != -1) {
        emit sectionSizeChanged(resizedSection);
        updateStretches();
    }

    //Make sure all newly selected sections are painted one last time
    QRect selectedRects;
    for (int i = 0; i < count(); i++) {
        if(sectionState(i) == Selected)
            selectedRects |= sRect(i);
    }
    if(!selectedRects.isNull())
        repaint(selectedRects);
}

/*! \reimp
*/

void MatrixHeader::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() != LeftButton)
        return;
    if (isResizing) {
        int p = real_pos(e->pos(), orientation()) + offset();
        int section = sectionAt(p);
        if (section == -1)
            return;
        section--;
        if (p >= sectionPos(count() - 1) + sectionSize(count() - 1))
            ++section;
        while (sectionSize(section) == 0)
            section--;
        if (section < 0)
            return;
        int oldSize = sectionSize(section);
        if (orientation() == Horizontal) {
            table->adjustColumn(section);
            int newSize = sectionSize(section);
            if (oldSize != newSize)
                emit sizeChange(section, oldSize, newSize);
            for (int i = 0; i < table->numCols(); ++i) {
                if (table->isColumnSelected(i) && sectionSize(i) != 0)
                    table->adjustColumn(i);
            }
        } else {
            table->adjustRow(section);
            int newSize = sectionSize(section);
            if (oldSize != newSize)
                emit sizeChange(section, oldSize, newSize);
            for (int i = 0; i < table->numRows(); ++i) {
                if (table->isRowSelected(i)  && sectionSize(i) != 0)
                    table->adjustRow(i);
            }
        }
    }
}

/*! \reimp
*/

void MatrixHeader::resizeEvent(QResizeEvent *e)
{
    stretchTimer->stop();
    widgetStretchTimer->stop();
    QHeaderView::resizeEvent(e);
    if (numStretches == 0)
        return;
    stretchTimer->setSingleShot(true);
    stretchTimer->start(0);
}

void MatrixHeader::updateStretches()
{
    if (numStretches == 0)
        return;

    int dim = orientation() == Horizontal ? width() : height();
    if (sectionPos(count() - 1) + sectionSize(count() - 1) == dim)
        return;
    int i;
    int pd = dim - (sectionPos(count() - 1)
                     + sectionSize(count() - 1));
    bool block = signalsBlocked();
    blockSignals(true);
    for (i = 0; i < (int)stretchable.count(); ++i) {
        if (!stretchable[i] ||
             (stretchable[i] && table->d->hiddenCols[i]))
            continue;
        pd += sectionSize(i);
    }
    pd /= numStretches;
    for (i = 0; i < (int)stretchable.count(); ++i) {
        if (!stretchable[i] ||
             (stretchable[i] && table->d->hiddenCols[i]))
            continue;
        if (i == (int)stretchable.count() - 1 &&
             sectionPos(i) + pd < dim)
            pd = dim - sectionPos(i);
        resizeSection(i, qMax(20, pd));
    }
    blockSignals(block);
    table->viewport()->repaint();
    widgetStretchTimer->setSingleShot(true);
    widgetStretchTimer->start(100);
}

void MatrixHeader::updateWidgetStretches()
{
    QSize s = table->tableSize();
    table->viewport()->resize(s.width(), s.height());
    for (int i = 0; i < table->numCols(); ++i)
        table->updateColWidgets(i);
}

void MatrixHeader::updateSelections()
{
    if (table->selectionMode() == MatrixUI::NoSelection ||
         (isRowSelection(table->selectionMode()) && orientation() != Vertical ))
        return;
    int a = sectionAt(startPos);
    int b = sectionAt(endPos);
    int start = qMin(a, b);
    int end = qMax(a, b);
    register int *s = states.data();
    for (int i = 0; i < count(); ++i) {
        if (i < start || i > end)
            *s = oldStates.data()[ i ];
        else
            *s = Selected;
        ++s;
    }
    repaint();

    if (table->currentSel) {
        MatrixSelection oldSelection = *table->currentSel;
        if (orientation() == Vertical)
            table->currentSel->expandTo(b, table->horizontalHeader()->count() - 1);
        else
            table->currentSel->expandTo(table->verticalHeader()->count() - 1, b);
        table->repaintSelections(&oldSelection, table->currentSel,
                                  orientation() == Horizontal,
                                  orientation() == Vertical);
    }
    emit table->selectionChanged();
}

void MatrixHeader::saveStates()
{
    oldStates.resize(count());
    register int *s = states.data();
    register int *s2 = oldStates.data();
    for (int i = 0; i < count(); ++i) {
        *s2 = *s;
        ++s2;
        ++s;
    }
}

void MatrixHeader::doAutoScroll()
{
    QPoint pos = mapFromGlobal(QCursor::pos());
    int p = real_pos(pos, orientation()) + offset();
    if (sectionAt(p) != -1)
        endPos = p;
    if (orientation() == Horizontal)
        table->ensureVisible(endPos, table->visibleRegion().boundingRect().y());
    else
        table->ensureVisible(table->visibleRegion().boundingRect().x(), endPos);
    updateSelections();
    autoScrollTimer->setSingleShot(true);
    autoScrollTimer->start(100);
}

void MatrixHeader::sectionWidthChanged(int col, int, int)
{
    resizedSection = col;
    if (orientation() == Horizontal) {
#ifndef NO_LINE_WIDGET
        table->moveChild(line1, QHeaderView::sectionPosition(col) - 1,
                          table->visibleRegion().boundingRect().y());
        line1->resize(1, table->viewport()->height());
        line1->show();
        line1->raise();
        table->moveChild(line2,
                          QHeaderView::sectionPosition(col) + QHeaderView::sectionSize(col) - 1,
                          table->visibleRegion().boundingRect().y());
        line2->resize(1, table->viewport()->height());
        line2->show();
        line2->raise();
#else
        QPainter p(table->viewport());
        int lx = QHeaderView::sectionPosition(col) + QHeaderView::sectionSize(col) - 1;
        int ly = table->visibleRegion().boundingRect().y();

        if (lx != d->oldLinePos) {
            QPoint pt = table->contentsToViewport(QPoint(lx, ly));
            p.drawLine(pt.x(), pt.y()+1,
                        pt.x(), pt.y()+ table->viewport()->height());
            if (d->oldLinePos >= 0)
                table->repaintContents(d->oldLinePos, table->visibleRegion().boundingRect().y(),
                                       1, table->viewport()->height());

            d->oldLinePos = lx;
        }
#endif
    } else {
#ifndef NO_LINE_WIDGET
        table->moveChild(line1, table->visibleRegion().boundingRect().x(),
                          QHeaderView::sectionPosition(col) - 1);
        line1->resize(table->viewport()->width(), 1);
        line1->show();
        line1->raise();
        table->moveChild(line2, table->visibleRegion().boundingRect().x(),
                          QHeaderView::sectionPosition(col) + QHeaderView::sectionSize(col) - 1);
        line2->resize(table->viewport()->width(), 1);
        line2->show();
        line2->raise();

#else
        QPainter p(table->viewport());
        int lx = table->visibleRegion().boundingRect().x();
        int ly = QHeaderView::sectionPosition(col) + QHeaderView::sectionSize(col) - 1;

        if (ly != d->oldLinePos) {
            QPoint pt = table->contentsToViewport(QPoint(lx, ly));
            p.drawLine(pt.x()+1, pt.y(),
                        pt.x() + table->viewport()->width(), pt.y());
            if (d->oldLinePos >= 0)
                table->repaintContents( table->visibleRegion().boundingRect().x(), d->oldLinePos,
                                        table->viewport()->width(), 1);
            d->oldLinePos = ly;
        }

#endif
    }
}

/*!
    \reimp

    Returns the size of section \a section in pixels or -1 if \a
    section is out of range.
*/

int MatrixHeader::sectionSize(int section) const
{
    if (count() <= 0 || section < 0 || section >= count())
        return -1;
    if (caching && section < (int)sectionSizes.count())
         return sectionSizes[ section ];
    return QHeaderView::sectionSize(section);
}

/*!
    \reimp

    Returns the start position of section \a section in pixels or -1
    if \a section is out of range.

    \sa sectionAt()
*/

int MatrixHeader::sectionPos(int section) const
{
    if (count() <= 0 || section < 0 || section >= count())
        return -1;
    if (caching && section < (int)sectionPoses.count())
        return sectionPoses[ section ];
    return QHeaderView::sectionPosition(section);
}

/*!
    \reimp

    Returns the number of the section at index position \a pos or -1
    if there is no section at the position given.

    \sa sectionPos()
*/

int MatrixHeader::sectionAt(int pos) const
{
    if (!caching || sectionSizes.count() <= 0 || sectionPoses.count() <= 0)
        return QHeaderView::sectionAt(pos);
    if (count() <= 0 || pos > sectionPoses[ count() - 1 ] + sectionSizes[ count() - 1 ])
        return -1;
    int l = 0;
    int r = count() - 1;
    int i = ((l+r+1) / 2);
    while (r - l) {
        if (sectionPoses[i] > pos)
            r = i -1;
        else
            l = i;
        i = ((l+r+1) / 2);
    }
    if (sectionPoses[i] <= pos &&
         pos <= sectionPoses[i] + sectionSizes[ mapToSection(i) ])
        return mapToSection(i);
    return -1;
}

void MatrixHeader::updateCache()
{
    sectionPoses.resize(count());
    sectionSizes.resize(count());
    if (!caching)
        return;
    for (int i = 0; i < count(); ++i) {
        sectionSizes[ i ] = QHeaderView::sectionSize(i);
        sectionPoses[ i ] = QHeaderView::sectionPosition(i);
    }
}

void MatrixHeader::setCaching(bool b)
{
    if (caching == b)
        return;
    caching = b;
    sectionPoses.resize(count());
    sectionSizes.resize(count());
    if (b) {
        for (int i = 0; i < count(); ++i) {
            sectionSizes[ i ] = QHeaderView::sectionSize(i);
            sectionPoses[ i ] = QHeaderView::sectionPosition(i);
        }
    }
}

/*!
    If \a b is true, section \a s is stretchable; otherwise the
    section is not stretchable.

    \sa isSectionStretchable()
*/

void MatrixHeader::setSectionStretchable(int s, bool b)
{
    if (stretchable[ s ] == b)
        return;
    stretchable[ s ] = b;
    if (b)
        numStretches++;
    else
        numStretches--;
}

/*!
    Returns true if section \a s is stretcheable; otherwise returns
    false.

    \sa setSectionStretchable()
*/

bool MatrixHeader::isSectionStretchable(int s) const
{
    return stretchable[ s ];
}

void MatrixHeader::swapSections(int oldIdx, int newIdx, bool swapTable)
{
    extern bool qt_qheader_label_return_null_strings; // qheader.cpp
    qt_qheader_label_return_null_strings = true;

    QIconSet oldIconSet, newIconSet;
/**
    if (w(oldIdx))
        oldIconSet = *iconSet(oldIdx);
    if (iconSet(newIdx))
        newIconSet = *iconSet(newIdx);
*/
    QString oldLabel = label(oldIdx);
    QString newLabel = label(newIdx);
    bool sectionsHasContent = !(oldIconSet.isNull() && newIconSet.isNull()
                            && oldLabel.isNull() && newLabel.isNull());
    if (sectionsHasContent) {
        QHeaderViewData *data = QHeaderView::d;
        bool oldNullLabel = qt_get_null_label_bit(data, oldIdx);
        bool newNullLabel = qt_get_null_label_bit(data, newIdx);
        setLabel(oldIdx, newIconSet, newLabel);
        setLabel(newIdx, oldIconSet, oldLabel);
        qt_set_null_label_bit(data, oldIdx, newNullLabel);
        qt_set_null_label_bit(data, newIdx, oldNullLabel);
    }

    qt_qheader_label_return_null_strings = false;

    int w1 = sectionSize(oldIdx);
    int w2 = sectionSize(newIdx);
    if (w1 != w2) {
        resizeSection(oldIdx, w2);
        resizeSection(newIdx, w1);
    }

    if (!swapTable)
        return;
    if (orientation() == Horizontal)
        table->swapColumns(oldIdx, newIdx);
    else
        table->swapRows(oldIdx, newIdx);
}

void MatrixHeader::indexChanged(int sec, int oldIdx, int newIdx)
{
    newIdx = mapToIndex(sec);
    if (oldIdx > newIdx)
        moveSection(sec, oldIdx + 1);
    else
        moveSection(sec, oldIdx);

    if (oldIdx < newIdx) {
        while (oldIdx < newIdx) {
            swapSections(oldIdx, oldIdx + 1);
            oldIdx++;
        }
    } else {
        while (oldIdx > newIdx) {
            swapSections(oldIdx - 1, oldIdx);
            oldIdx--;
        }
    }

    table->repaint(table->visibleRegion().boundingRect().x(), table->visibleRegion().boundingRect().y(),
                            table->viewport()->width(), table->viewport()->height());
}

void MatrixHeader::setLabels(const QStringList & labels)
{
    int i = 0;
    const int c = qMin(count(), (int)labels.count());
    bool updates = updatesEnabled();
    if (updates)
        setUpdatesEnabled(false);
    for (QStringList::ConstIterator it = labels.begin(); i < c; ++i, ++it) {
        if (i == c - 1) {
            if (updates)
                setUpdatesEnabled(true);
            setLabel(i, *it);
        } else {
            QHeaderView::setLabel(i, *it);
            emit sectionSizeChanged(i);
        }
    }
}
#endif

QT_END_NAMESPACE
