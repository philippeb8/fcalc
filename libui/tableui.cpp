/**
    Copyright 2019 Fornux Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "tableui.h"

#include <QHeaderView>
#include <qlineedit.h>
#include <qpainter.h>
#include <qdrawutil.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QFrame>


TableItemUI::TableItemUI(QTreeWidget * parent, QTreeWidgetItem * after, QString s1, QString s2) : QTreeWidgetItem(parent, after)
{
    setText(0, s1);
    setText(1, s2);
}

void TableItemUI::paintCell(QPainter * painter, const QPalette & colorgroup, int column, int width, int align)
{
//    if (TableUI * const p = static_cast<TableUI *>(treeWidget()))
//	{
//		int const x = column ? p->columnWidth(column - 1) : 0;

//		if (p->eRowHeader == TableUI::eReadOnly)
//		{
//            QTreeWidgetItem::paintCell(painter, colorgroup, column, width, align);

//			painter->setPen(Qt::lightGray);
//			painter->drawLine(width - 1, 0, width - 1, height() - 1);
//			painter->drawLine(0, height() - 1, width - 1, height() - 1);

//			if (isSelected() && isVisible() && column != 0)
//			{
//				p->pLineEdit->hide();
//				p->pLineEdit->setText(text(column));
//				p->pLineEdit->selectAll();
//				p->pLineEdit->move(x, p->itemRect(this).y());
//				p->pLineEdit->resize(width, height());
//				p->pLineEdit->show();
//			}
//		}
//		else if (p->eRowHeader == TableUI::eHeader)
//		{
//			QColorGroup headercg = QColorGroup(colorgroup);
//			headercg.setColor(QColorGroup::Base, colorgroup.background());

//            QTreeWidgetItem::paintCell(painter, headercg, column, width, align);
//			qDrawWinPanel(painter, x, 0, width, height(), headercg, false, 0);
//		}
//	}
}


TableUI::TableUI(QWidget * p, const char * n) : QTreeWidget(p), eRowHeader(eHeader)
{
	pLineEdit = new QLineEdit(viewport());

	pLineEdit->hide();
	pLineEdit->setReadOnly(true);
	pLineEdit->setFocusProxy(this);
//	pLineEdit->setLineWidth(1);
//	pLineEdit->setFrameShape(QLineEdit::Box);
//	pLineEdit->setFrameShadow(QLineEdit::Plain);

//	setSorting(-1);
//	header()->setResizeEnabled(false);
//	header()->setMovingEnabled(false);

//    setSelectionMode(QTreeWidget::Single);

	setAllColumnsShowFocus(true);

//	setHScrollBarMode(Q3ScrollView::AlwaysOff);
}

void TableUI::resizeEvent(QResizeEvent * p)
{
    QTreeWidget::resizeEvent(p);

    if (eRowHeader == eReadOnly && columnCount())
	{
//		setColumnWidth(columns() - 1, visibleWidth() - (header()->headerWidth() - columnWidth(columns() - 1)));

        pLineEdit->resize(columnWidth(columnCount() - 1), pLineEdit->height());
	}
}

