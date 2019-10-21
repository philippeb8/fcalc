/**
    Copyright 2019 Fornux Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef MATRIXUI_H
#define MATRIXUI_H

#include <QScrollArea>
#include <QtGui/qpixmap.h>
#include <QVector>
#include <QHeaderView>
#include <QLinkedList>
#include <QSharedData>
#include <QHash>
#include <QStringList>
#include <QTableView>
#include <QComboBox>
#include <QTableWidget>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class /*Q_COMPAT_EXPORT*/ MatrixUI : public QTableWidget
{
    Q_OBJECT

public:
    MatrixUI(QWidget* parent=0, const char* name=0);
};

#if 0
class MatrixHeader;
class QValidator;
class MatrixUI;
class QPaintEvent;
class QTimer;
class QResizeEvent;
class QCheckBox;
class Q3DragObject;
typedef QIcon QIconSet;
typedef QComboBox Q3ComboBox;
typedef QPalette QColorGroup;
typedef QScrollArea Q3ScrollView;
template <typename T> struct Q3MemArray : QVector<T> { Q3MemArray(const QVector<T> & v) : QVector<T>(v) {} };
template <typename T> struct Q3PtrVector : QVector<T *> { bool autoDelete; void setAutoDelete(bool b) { autoDelete = b; } };
template <typename T> struct Q3PtrListIterator : QLinkedListIterator<T *> { Q3PtrListIterator(const QLinkedList<T *> & list) : QLinkedListIterator<T *>(list) {} };

struct MatrixPrivate;
struct MatrixHeaderPrivate;


class /*Q_COMPAT_EXPORT*/ MatrixSelection
{
public:
    MatrixSelection();
    MatrixSelection(int start_row, int start_col, int end_row, int end_col);
    void init(int row, int col);
    void expandTo(int row, int col);
    bool operator==(const MatrixSelection &s) const;
    bool operator!=(const MatrixSelection &s) const { return !(operator==(s)); }

    int topRow() const { return tRow; }
    int bottomRow() const { return bRow; }
    int leftCol() const { return lCol; }
    int rightCol() const { return rCol; }
    int anchorRow() const { return aRow; }
    int anchorCol() const { return aCol; }
    int numRows() const;
    int numCols() const;

    bool isActive() const { return active; }
    bool isEmpty() const { return numRows() == 0; }

private:
    uint active : 1;
    uint inited : 1;
    int tRow, lCol, bRow, rCol;
    int aRow, aCol;
};

class /*Q_COMPAT_EXPORT*/ MatrixItem
{
    friend class MatrixUI;

public:
    enum EditType { Never, OnTyping, WhenCurrent, Always };

    MatrixItem(MatrixUI *table, EditType et);
    MatrixItem(MatrixUI *table, EditType et, const QString &text);
    MatrixItem(MatrixUI *table, EditType et, const QString &text,
                const QPixmap &p);
    virtual ~MatrixItem();

    virtual QPixmap pixmap() const;
    virtual QString text() const;
    virtual void setPixmap(const QPixmap &p);
    virtual void setText(const QString &t);
    MatrixUI *table() const { return t; }

    virtual int alignment() const;
    virtual void setWordWrap(bool b);
    bool wordWrap() const;

    EditType editType() const;
    virtual QWidget *createEditor() const;
    virtual void setContentFromEditor(QWidget *w);
    virtual void setReplaceable(bool);
    bool isReplaceable() const;

    virtual QString key() const;
    virtual QSize sizeHint() const;

    virtual void setSpan(int rs, int cs);
    int rowSpan() const;
    int colSpan() const;

    virtual void setRow(int r);
    virtual void setCol(int c);
    int row() const;
    int col() const;

    virtual void paint(QPainter *p, const QColorGroup &cg,
                        const QRect &cr, bool selected);

    void updateEditor(int oldRow, int oldCol);

    virtual void setEnabled(bool b);
    bool isEnabled() const;

    virtual int rtti() const;
    static int RTTI;

private:
    QString txt;
    QPixmap pix;
    MatrixUI *t;
    EditType edType;
    uint wordwrap : 1;
    uint tcha : 1;
    uint enabled : 1;
    int rw, cl;
    int rowspan, colspan;
};

class /*Q_COMPAT_EXPORT*/ ComboMatrixItem : public MatrixItem
{
public:
    ComboMatrixItem(MatrixUI *table, const QStringList &list, bool editable = false);
    ~ComboMatrixItem();
    virtual QWidget *createEditor() const;
    virtual void setContentFromEditor(QWidget *w);
    virtual void paint(QPainter *p, const QColorGroup &cg,
                        const QRect &cr, bool selected);
    virtual void setCurrentItem(int i);
    virtual void setCurrentItem(const QString &i);
    int currentItem() const;
    QString currentText() const;
    int count() const;
#if !defined(Q_NO_USING_KEYWORD)
    using MatrixItem::text;
#else
    inline QString text() const { return MatrixItem::text(); }
#endif
    QString text(int i) const;
    virtual void setEditable(bool b);
    bool isEditable() const;
    virtual void setStringList(const QStringList &l);

    int rtti() const;
    static int RTTI;

    QSize sizeHint() const;

private:
    Q3ComboBox *cb;
    QStringList entries;
    int current;
    bool edit;
    static Q3ComboBox *fakeCombo;
    static QWidget *fakeComboWidget;
    static int fakeRef;

};

class /*Q_COMPAT_EXPORT*/ CheckMatrixItem : public MatrixItem
{
public:
    CheckMatrixItem(MatrixUI *table, const QString &txt);
    void setText(const QString &t);
    virtual QWidget *createEditor() const;
    virtual void setContentFromEditor(QWidget *w);
    virtual void paint(QPainter *p, const QColorGroup &cg,
                        const QRect &cr, bool selected);
    virtual void setChecked(bool b);
    bool isChecked() const;

    int rtti() const;
    static int RTTI;

    QSize sizeHint() const;

private:
    QCheckBox *cb;
    bool checked;

};

class /*Q_COMPAT_EXPORT*/ MatrixUI : public QTableWidget
{
    Q_OBJECT
    Q_ENUMS(SelectionMode FocusStyle)
    Q_PROPERTY(int numRows READ numRows WRITE setNumRows)
    Q_PROPERTY(int numCols READ numCols WRITE setNumCols)
    Q_PROPERTY(bool showGrid READ showGrid WRITE setShowGrid)
    Q_PROPERTY(bool rowMovingEnabled READ rowMovingEnabled WRITE setRowMovingEnabled)
    Q_PROPERTY(bool columnMovingEnabled READ columnMovingEnabled WRITE setColumnMovingEnabled)
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)
    Q_PROPERTY(bool sorting READ sorting WRITE setSorting)
    Q_PROPERTY(SelectionMode selectionMode READ selectionMode WRITE setSelectionMode)
    Q_PROPERTY(FocusStyle focusStyle READ focusStyle WRITE setFocusStyle)
    Q_PROPERTY(int numSelections READ numSelections)

    friend class MatrixHeader;
    friend class ComboMatrixItem;
    friend class CheckMatrixItem;
    friend class MatrixItem;

public:
    MatrixUI(QWidget* parent=0, const char* name=0);
    MatrixUI(int numRows, int numCols,
            QWidget* parent=0, const char* name=0);
    ~MatrixUI();

    QHeaderView *horizontalHeader() const;
    QHeaderView *verticalHeader() const;

    enum SelectionMode { Single, Multi, SingleRow, MultiRow, NoSelection };
    virtual void setSelectionMode(SelectionMode mode);
    SelectionMode selectionMode() const;

    virtual void setItem(int row, int col, MatrixItem *item);
    virtual void setText(int row, int col, const QString &text);
    virtual void setPixmap(int row, int col, const QPixmap &pix);
    virtual MatrixItem *item(int row, int col) const;
    virtual QString text(int row, int col) const;
    virtual QPixmap pixmap(int row, int col) const;
    virtual void clearCell(int row, int col);

    virtual QRect cellGeometry(int row, int col) const;
    virtual int columnWidth(int col) const;
    virtual int rowHeight(int row) const;
    virtual int columnPos(int col) const;
    virtual int rowPos(int row) const;
    virtual int columnAt(int x) const;
    virtual int rowAt(int y) const;

    virtual int numRows() const;
    virtual int numCols() const;

    void updateCell(int row, int col);

    bool eventFilter(QObject * o, QEvent *);

    int currentRow() const { return curRow; }
    int currentColumn() const { return curCol; }
    void ensureCellVisible(int row, int col);

    bool isSelected(int row, int col) const;
    bool isRowSelected(int row, bool full = false) const;
    bool isColumnSelected(int col, bool full = false) const;
    int numSelections() const;
    MatrixSelection selection(int num) const;
    virtual int addSelection(const MatrixSelection &s);
    virtual void removeSelection(const MatrixSelection &s);
    virtual void removeSelection(int num);
    virtual int currentSelection() const;

    void selectCells(int start_row, int start_col, int end_row, int end_col);
    virtual void selectRow(int row);
    virtual void selectColumn(int col);

    bool showGrid() const;

    QVariant inputMethodQuery(Qt::InputMethodQuery query) const;

    bool columnMovingEnabled() const;
    bool rowMovingEnabled() const;

    virtual void sortColumn(int col, bool ascending = true,
                             bool wholeRows = false);
    bool sorting() const;

    virtual void takeItem(MatrixItem *i);

    virtual void setCellWidget(int row, int col, QWidget *e);
    virtual QWidget *cellWidget(int row, int col) const;
    virtual void clearCellWidget(int row, int col);

    virtual QRect cellRect(int row, int col) const;

    virtual void paintCell(QPainter *p, int row, int col,
                            const QRect &cr, bool selected);
    virtual void paintCell(QPainter *p, int row, int col,
                            const QRect &cr, bool selected, const QColorGroup &cg);
    virtual void paintFocus(QPainter *p, const QRect &r);
    QSize sizeHint() const;

    bool isReadOnly() const;
    bool isRowReadOnly(int row) const;
    bool isColumnReadOnly(int col) const;

    void setEnabled(bool b);

    void repaintSelections();

    enum FocusStyle { FollowStyle, SpreadSheet };
    virtual void setFocusStyle(FocusStyle fs);
    FocusStyle focusStyle() const;

    void updateHeaderStates();

    bool isRowHidden(int row) const;
    bool isColumnHidden(int col) const;
    bool isColumnStretchable(int col) const;
    bool isRowStretchable(int row) const;
    bool dragEnabled() const;

public Q_SLOTS:
    virtual void setNumRows(int r);
    virtual void setNumCols(int r);
    virtual void setShowGrid(bool b);
    virtual void hideRow(int row);
    virtual void hideColumn(int col);
    virtual void showRow(int row);
    virtual void showColumn(int col);

    virtual void setColumnWidth(int col, int w);
    virtual void setRowHeight(int row, int h);

    virtual void adjustColumn(int col);
    virtual void adjustRow(int row);

    virtual void setColumnStretchable(int col, bool stretch);
    virtual void setRowStretchable(int row, bool stretch);
    virtual void setSorting(bool b);
    virtual void swapRows(int row1, int row2, bool swapHeader = false);
    virtual void swapColumns(int col1, int col2, bool swapHeader = false);
    virtual void swapCells(int row1, int col1, int row2, int col2);

    virtual void setLeftMargin(int m);
    virtual void setTopMargin(int m);
    virtual void setCurrentCell(int row, int col);
    void clearSelection(bool repaint = true);
    virtual void setColumnMovingEnabled(bool b);
    virtual void setRowMovingEnabled(bool b);

    virtual void setReadOnly(bool b);
    virtual void setRowReadOnly(int row, bool ro);
    virtual void setColumnReadOnly(int col, bool ro);

    virtual void setDragEnabled(bool b);

    virtual void insertRows(int row, int count = 1);
    virtual void insertColumns(int col, int count = 1);
    virtual void removeRow(int row);
    virtual void removeRows(const Q3MemArray<int> &rows);
    virtual void removeColumn(int col);
    virtual void removeColumns(const Q3MemArray<int> &cols);

    virtual void editCell(int row, int col, bool replace = false);

    void setRowLabels(const QStringList &labels);
    void setColumnLabels(const QStringList &labels);

	//! altered
    void showHControls(bool = true);
    void showVControls(bool = true);

public:
//! altered
    enum EditMode { NotEditing, Editing, Replacing };
    void drawContents(QPainter *p, int cx, int cy, int cw, int ch);
    void contentsMousePressEvent(QMouseEvent*);
    void contentsMouseMoveEvent(QMouseEvent*);
    void contentsMouseDoubleClickEvent(QMouseEvent*);
    void contentsMouseReleaseEvent(QMouseEvent*);
    void contentsContextMenuEvent(QContextMenuEvent * e);
    void keyPressEvent(QKeyEvent*);
    void focusInEvent(QFocusEvent*);
    void focusOutEvent(QFocusEvent*);
    void viewportResizeEvent(QResizeEvent *);
    void showEvent(QShowEvent *e);
    void paintEvent(QPaintEvent *e);
    void setEditMode(EditMode mode, int row, int col);
#ifndef QT_NO_DRAGANDDROP
    virtual void contentsDragEnterEvent(QDragEnterEvent *e);
    virtual void contentsDragMoveEvent(QDragMoveEvent *e);
    virtual void contentsDragLeaveEvent(QDragLeaveEvent *e);
    virtual void contentsDropEvent(QDropEvent *e);
    virtual Q3DragObject *dragObject();
    virtual void startDrag();
#endif

    virtual void paintEmptyArea(QPainter *p, int cx, int cy, int cw, int ch);
    virtual void activateNextCell();
    virtual QWidget *createEditor(int row, int col, bool initFromCell) const;
    virtual void setCellContentFromEditor(int row, int col);
    virtual QWidget *beginEdit(int row, int col, bool replace);
    virtual void endEdit(int row, int col, bool accept, bool replace);

    virtual void resizeData(int len);
    virtual void insertWidget(int row, int col, QWidget *w);
    int indexOf(int row, int col) const;

    void windowActivationChange(bool);
    bool isEditing() const;
    EditMode editMode() const;
    int currEditRow() const;
    int currEditCol() const;

protected Q_SLOTS:
    virtual void columnWidthChanged(int col);
    virtual void rowHeightChanged(int row);
    virtual void columnIndexChanged(int section, int fromIndex, int toIndex);
    virtual void rowIndexChanged(int section, int fromIndex, int toIndex);
    virtual void columnClicked(int col);

Q_SIGNALS:
    void currentChanged(int row, int col);
    void clicked(int row, int col, int button, const QPoint &mousePos);
    void doubleClicked(int row, int col, int button, const QPoint &mousePos);
    void pressed(int row, int col, int button, const QPoint &mousePos);
    void selectionChanged();
    void valueChanged(int row, int col);
    void contextMenuRequested(int row, int col, const QPoint &pos);
#ifndef QT_NO_DRAGANDDROP
    void dropped(QDropEvent *e);
#endif

private Q_SLOTS:
//! altered
	void slotAddRow();
	void slotRemoveRow();
	void slotAddColumn();
	void slotRemoveColumn();
	void setHOffset(int);
	void setVOffset(int);

    void doAutoScroll();
    void doValueChanged();
    void updateGeometriesSlot();

private:
//! altered
    void adjustButtons();

    void contentsMousePressEventEx(QMouseEvent*);
    void drawContents(QPainter*);
    void updateGeometries();
    void repaintSelections(MatrixSelection *oldSelection,
                            MatrixSelection *newSelection,
                            bool updateVertical = true,
                            bool updateHorizontal = true);
    QRect rangeGeometry(int topRow, int leftCol,
                         int bottomRow, int rightCol, bool &optimize);
    void fixRow(int &row, int y);
    void fixCol(int &col, int x);

    void init(int numRows, int numCols);
    QSize tableSize() const;
    void repaintCell(int row, int col);
    void contentsToViewport2(int x, int y, int& vx, int& vy);
    QPoint contentsToViewport2(const QPoint &p);
    void viewportToContents2(int vx, int vy, int& x, int& y);
    QPoint viewportToContents2(const QPoint &p);

    void updateRowWidgets(int row);
    void updateColWidgets(int col);
    bool isSelected(int row, int col, bool includeCurrent) const;
    void setCurrentCell(int row, int col, bool updateSelections, bool ensureVisible = false);
    void fixCell(int &row, int &col, int key);
    void delayedUpdateGeometries();
    struct TableWidget
    {
	TableWidget(QWidget *w, int r, int c) : wid(w), row(r), col (c) {}
	QWidget *wid;
	int row, col;
    };
    void saveContents(Q3PtrVector<MatrixItem> &tmp,
               Q3PtrVector<TableWidget> &tmp2);
    void updateHeaderAndResizeContents(MatrixHeader *header,
					int num, int colRow,
					int width, bool &updateBefore);
    void restoreContents(Q3PtrVector<MatrixItem> &tmp,
              Q3PtrVector<TableWidget> &tmp2);
    void finishContentsResze(bool updateBefore);

private:
    Q3PtrVector<MatrixItem> contents;
    Q3PtrVector<QWidget> widgets;
    int curRow;
    int curCol;
    MatrixHeader *leftHeader, *topHeader;
    EditMode edMode;
    int editCol, editRow;
    QLinkedList<MatrixSelection *> selections;
    MatrixSelection *currentSel;
    QTimer *autoScrollTimer;
    int lastSortCol;
    bool sGrid : 1;
    bool mRows : 1;
    bool mCols : 1;
    bool asc : 1;
    bool doSort : 1;
    bool unused : 1;
    bool readOnly : 1;
    bool shouldClearSelection : 1;
    bool dEnabled : 1;
    bool context_menu : 1;
    bool drawActiveSelection : 1;
    bool was_visible : 1;
    SelectionMode selMode;
    int pressedRow, pressedCol;
    MatrixPrivate *d;
    QHash<int, int> roRows;
    QHash<int, int> roCols;
    int startDragRow;
    int startDragCol;
    QPoint dragStartPos;
    int oldCurrentRow, oldCurrentCol;
    FocusStyle focusStl;

    Q_DISABLE_COPY(MatrixUI)
};

class /*Q_COMPAT_EXPORT*/ MatrixHeader : public QHeaderView
{
    friend class MatrixUI;
    Q_OBJECT

public:
    enum SectionState {
        Normal,
        Bold,
        Selected
    };

    MatrixHeader(Qt::Orientation, int, MatrixUI *t, QWidget* parent=0, const char* name=0);
    ~MatrixHeader() {};
    void addLabel(const QString &s, int size);
    void setLabel(int section, const QString & s, int size = -1);
    void setLabel(int section, const QIcon & iconset, const QString & s,
                   int size = -1);

    void setLabels(const QStringList & labels);

    void removeLabel(int section);

    void setSectionState(int s, SectionState state);
    void setSectionStateToAll(SectionState state);
    SectionState sectionState(int s) const;

    int sectionSize(int section) const;
    int sectionPos(int section) const;
    int sectionAt(int section) const;

    void setSectionStretchable(int s, bool b);
    bool isSectionStretchable(int s) const;

    void updateCache();

signals:
    void sectionSizeChanged(int s);
    void sizeChange(int section, int oldSize, int newSize);

protected:
    void paintEvent(QPaintEvent *e);
    void paintSection(QPainter *p, int index, const QRect& fr);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent *e);

private slots:
    void doAutoScroll();
    void sectionWidthChanged(int col, int os, int ns);
    /**void indexChanged(int sec, int oldIdx, int newIdx);*/
    void updateStretches();
    void updateWidgetStretches();

private:
    void updateSelections();
    void saveStates();
    void setCaching(bool b);
    /**void swapSections(int oldIdx, int newIdx, bool swapTable = true);*/
    bool doSelection(QMouseEvent *e);
    void sectionLabelChanged(int section);
    void resizeArrays(int n);

private:
    QVector<int> states, oldStates;
    QVector<bool> stretchable;
    QVector<int> sectionSizes, sectionPoses;
    bool mousePressed;
    int pressPos, startPos, endPos;
    MatrixUI *table;
    QTimer *autoScrollTimer;
    QWidget *line1, *line2;
    bool caching;
    int resizedSection;
    bool isResizing;
    int numStretches;
    QTimer *stretchTimer, *widgetStretchTimer;
    MatrixHeaderPrivate *d;

    Q_DISABLE_COPY(MatrixHeader)
};
#endif

QT_END_NAMESPACE

QT_END_HEADER

#endif // Q3TABLE_H
