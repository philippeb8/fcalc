/**
    Copyright 2019 Fornux Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "worksheetui.h"

#include <iostream>
#include <qlineedit.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QFrame>
#include <QKeyEvent>
#include <QEvent>
#include <QScrollBar>


/**
	Main widget.
*/

WorksheetUI::WorksheetUI(QWidget * a_pcWidget, char const * a_pzName): QTextEdit(a_pcWidget)
{
	pLineEdit = new QLineEdit(viewport());
//	pLineEdit->setLineWidth(1);
//	pLineEdit->setFrameShape(QLineEdit::NoFrame);
///	pLineEdit->setFrameShape(QFrame::Box);
///	pLineEdit->setFrameShadow(QFrame::Plain);

	QFont font(pLineEdit->font());
    font.setBold(true);
    pLineEdit->setFont(font);

	setReadOnly(true);
	setTabStopWidth(tabStopWidth() / 4);

//	connect(this, SIGNAL(contentsMoving(int, int)), SLOT(update()));
    connect(horizontalScrollBar(), &QScrollBar::valueChanged, this, &WorksheetUI::updateGeometry);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &WorksheetUI::updateGeometry);
}

bool WorksheetUI::event(QEvent * e)
{
	// key press
	if (QKeyEvent * p = dynamic_cast<QKeyEvent *>(e))
	{
		return false;
	}
	
    return QTextEdit::event(e);
}

void WorksheetUI::adjust()
{
//	int const frame = pLineEdit->frameWidth();
//	pLineEdit->move(contentsToViewport(paragraphRect(paragraphs() - 1).topLeft() - QPoint(0, frame)));
//	pLineEdit->resize(paragraphRect(paragraphs() - 1).size() += QSize(0, frame * 2));
}

void WorksheetUI::insert(QString const &sText)
{
	pLineEdit->insert(sText);
}

void WorksheetUI::setText(QString const &sText)
{
//	setBold(true);
//	setItalic(false);
	append(pLineEdit->text());

//	setBold(false);
//	setItalic(true);
	append(QString("\t") + sText);
//	insertParagraph("", -1);

	pLineEdit->clear();
	adjust();
}

void WorksheetUI::show()
{
    QTextEdit::show();

	adjust();
}

void WorksheetUI::resizeEvent(QResizeEvent *pEvent)
{
    QTextEdit::resizeEvent(pEvent);

	adjust();
}
