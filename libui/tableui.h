/**
    Copyright 2019 Fornux Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef TABLE_UI
#define TABLE_UI


#include <list>
#include <QTableWidget>
#include <QTreeWidget>
//Added by qt3to4:
#include <QResizeEvent>


class QLineEdit;
class QTreeWidgetItem;


class TableUI;


class TableItemUI : public QTreeWidgetItem
{
	friend class TableUI;

public:
    TableItemUI(QTreeWidget *, QTreeWidgetItem *, QString, QString = QString::null);

    virtual void paintCell(QPainter *, const QPalette &, int, int, int);
};


/**
	QListBox with editable items.
*/

class TableUI : public QTreeWidget
{
	friend class TableItemUI;

	QLineEdit * pLineEdit;

public:
	enum RowHeader {eReadOnly, eReadWrite, eHeader} eRowHeader;

	TableUI(QWidget *, const char * = 0);

	void setRowHeader(RowHeader);

	QLineEdit * lineEdit();

protected:
	virtual void resizeEvent(QResizeEvent *);
};


inline void TableUI::setRowHeader(RowHeader aHeader)
{
	eRowHeader = aHeader;
}

inline QLineEdit * TableUI::lineEdit()
{
	return pLineEdit;
}

#endif
