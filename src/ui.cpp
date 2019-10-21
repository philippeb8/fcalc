/**
    Copyright 2019 Fornux Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "ui.h"
#include "app.h"
#include "graphui.h"
#include "tableui.h"
#include "matrixui.h"
#include "spinboxui.h"
#include "worksheetui.h"
#include "ui_rangeui.h"
#include "ui_titleui.h"
#include "ui_aboutui.h"
#include "ui_setupui.h"
#include "ui_optionui.h"
#include "ui_graphicui.h"
#include "ui_variableui.h"
///#include "ui_registerui.h"
#include "dispatch.h"
#include "parser_yacc.h"
#include "parser_postfix_yacc.h"
#include "portability.h"

#include <math.h>
#include <stdlib.h>
///#include <unistd.h>

#include <list>
#include <vector>
#include <string>
#include <limits>
#include <fstream>
#include <sstream>
#include <iterator>
#include <typeinfo>

#include <qfont.h>
#include <qlabel.h>
#include <qevent.h>
#include <QFrame>
#include <qimage.h>
#include <qtabbar.h>
#include <qcursor.h>
#include <qbitmap.h>
#include <qaction.h>
#include <qlayout.h>
#include <qregexp.h>
#include <qobject.h>
#include <qmenubar.h>
#include <qpainter.h>
#include <qspinbox.h>
#include <QProcess>
#include <QPrinter>
#include <QTextEdit>
#include <qsettings.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qclipboard.h>
#include <qstatusbar.h>
#include <qvalidator.h>
#include <QMenu>
#include <qtabwidget.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qmessagebox.h>
#include <QFileDialog>
#include <QPushButton>
#include <QStackedWidget>
#include <qapplication.h>
#include <qsplashscreen.h>
#include <QPaintDevice>
//Added by qt3to4:
#include <QTimerEvent>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QPixmap>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QTextStream>
#include <QKeyEvent>
///#include <QCustomEvent>
#include <QActionGroup>
#include <QAbstractButton>
#include <QDesktopWidget>
#include <QAbstractButton>
#include <QScrollBar>
#include <QPrintDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QPrinter>
#include <QTimer>
#include <QDebug>

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#include <winbase.h>
#include <htmlhelp.h>
#include <qwindowsxpstyle.h>

typedef wchar_t const * CString;

#undef min
#undef max

#define BUFSIZE 512
#endif


struct RangeUI : QDialog, Ui_RangeUI { RangeUI(QWidget * parent) : QDialog(parent) { setupUi(this); setGeometry(QApplication::desktop()->availableGeometry()); } };
struct AboutUI : QDialog, Ui_AboutUI { AboutUI(QWidget * parent) : QDialog(parent) { setupUi(this); setGeometry(QApplication::desktop()->availableGeometry()); } };
struct SetupUI : QDialog, Ui_SetupUI { SetupUI(QWidget * parent) : QDialog(parent) { setupUi(this); setGeometry(QApplication::desktop()->availableGeometry()); } };
struct OptionUI : QDialog, Ui_OptionUI { OptionUI(QWidget * parent) : QDialog(parent) { setupUi(this); setGeometry(QApplication::desktop()->availableGeometry()); } };
struct GraphicUI : QDialog, Ui_GraphicUI { GraphicUI(QWidget * parent) : QDialog(parent) { setupUi(this); setGeometry(QApplication::desktop()->availableGeometry()); } };


extern "C" int convert_xls_csv(char const *, char const *);


using namespace std;


#define VERSION "6.0"


#define CAPTION "Fornux Calculator " VERSION
#define ABOUT   "Fornux Calculator " VERSION


MainUI * MainUI::pMainUI = 0;

char MainUI::pRegistrationKey[192] = {0};




/**
	Global variables.
*/

int depth;

bool saved;

char graph;

QString line;

variable_t variable[128];

expression_t expression[128];

std::list<variable_t> stack[6];




/**
	Graphic plotting routines.
*/

real plot(real x)
{
	((type_t<floating> *) (variable[(int) ('x')].get()))->value = x;

    return evaluate(variable[(int) ('y')].get(), MainUI::pMainUI->comboBox3->currentIndex()).to_floating();
}

real plot(real x, real y)
{
	((type_t<floating> *) (variable[(int) ('x')].get()))->value = x;
	((type_t<floating> *) (variable[(int) ('y')].get()))->value = y;

    return evaluate(variable[(int) ('z')].get(), MainUI::pMainUI->comboBox3->currentIndex()).to_floating();
}




/**
	Convertion routines.
*/

inline int checkedId(QButtonGroup * p)
{
    return p->id(p->checkedButton());
}

inline string to_string(QString const & a)
{
	string s;
	s.resize(a.length());

	for (unsigned int i = 0; i < a.length(); ++ i)
	{
        switch (a.utf16()[i])
		{
		default: s[i] = a[i].toLatin1(); break;
		case 0x03C0: s[i] = 128; break; // pi
		case 0x0435: s[i] = 129; break; // e
		case 0x2215: s[i] = 130; break; // frac
		case 0x221A: s[i] = 131; break; // sqrt
		case 0x00D7: s[i] = 132; break; // cross
		case 0x2219: s[i] = 133; break; // scalar
		case 0x00B0: s[i] = 134; break; // degree
		case 0x03B1: s[i] = 135; break; // alpha
		case 0x03B2: s[i] = 136; break; // beta
		case 0x03B3: s[i] = 137; break; // gamma
		}
	}

	return s;
}




/**
	Printing callback.

	This routine is called each time the external evaluate() routine prints
	its results.  This is also used to synchronize threads and to update
	the UI.
*/

normed const & flush(normed const & res)
{
	if (MainUI * const p = MainUI::pMainUI)
	{
		if (type_t<string> const * q = dynamic_cast<type_t<string> const *>(reinterpret_cast<type const *>(& res)))
		{
			if (q->value != "")
			{
                p->currentLineEdit()->setText(QString("ERROR: ") + QString::fromStdString(q->value));
			}
		}
		else
		{
            p->writeResult(res, p->currentLineEdit(), p->option.precision);
		}
	}

	return res;
}


/**
	Error handler.

	This will show the error message according to the current tab.
*/

void MainUI::error(string const & e)
{
	switch (pTabBar->currentIndex())
	{
/**
	case 1:
	case 3:
	case 4:
        QMessageBox::critical(this, "Console", QString("ERROR: ") + QString::fromStdString(e));
		break;

	case 2:
    {
        QPalette p(palette());
        p.setColor(QPalette::Background, Qt::red);
        worksheetUI2_1->setAutoFillBackground(true);
        worksheetUI2_1->setPalette(p);
		worksheetUI2_1->setText(QString("ERROR: ") + QString::fromStdString(e));
    }
        break;
*/

	default:
        writeResult(string("ERROR: ") + e, currentLineEdit(), option.precision);
		break;
	}
}


/**
	Execution kernel.

	This is the main function that parses the command line.
*/

bool MainUI::execute(QString const & line)
{
	CalculatorParser parser;
	CalculatorParserRPN parser_postfix;

	istringstream s(to_string(line));
	ostringstream e;

	switch (option.rpn)
	{
	case false:
		parser.switch_streams(& s, & e);

        switch (comboBox2->currentIndex())
		{
		case 0: parser.state(1); break;
		case 1: parser.state(0); break;
		case 2: parser.state(2); break;
		case 3: parser.state(3); break;
		}

		// parse expression
		if (! parser.parserparse())
		{
			::line = line;

			if (type * p = parser.config.value.get())
			{
                normed const result = evaluate(p, comboBox3->currentIndex(), stat);

				if (type_t<string> const * q = dynamic_cast<type_t<string> const *>(reinterpret_cast<type const *>(& result)))
				{
					if (q->value != "")
					{
						error(q->value);

						return false;
					}
				}

				return true;
			}
		}
		break;

	case true:
		parser_postfix.switch_streams(& s, & e);

        switch (comboBox2->currentIndex())
		{
			case 0: parser_postfix.state(1); break;
			case 1: parser_postfix.state(0); break;
			case 2: parser_postfix.state(2); break;
			case 3: parser_postfix.state(3); break;
		}

		// parse expression
		if (! parser_postfix.parser_postfixparse())
		{
				::line = line;

				if (type * p = parser_postfix.config.value.get())
				{
                    normed const result = evaluate(p, comboBox3->currentIndex(), stat);

					if (type_t<string> const * q = dynamic_cast<type_t<string> const *>(reinterpret_cast<type const *>(& result)))
					{
						if (q->value != "")
						{
							error(q->value);

							return false;
						}
					}

					return true;
				}
		}
		break;
	}

	if (e.str().size() != 0)
	{
		error(e.str());

		return false;
	}

	return true;
}

bool MainUI::event(QEvent * e)
{
	// refresh graphics
	if (QTimerEvent * p = dynamic_cast<QTimerEvent *>(e))
	{
		static QWidget *pDesktop = QApplication::desktop()->screen(0);

		if (p->timerId() == ee.nResizeId)
		{
			killTimer(ee.nResizeId);

			// refresh graphic
			slotRefresh(false);
		}

		return true;
	}

	// open
	else if (dynamic_cast<QCustomEvent *>(e))
	{	
		if (option.reload || option.commandline)
		{
			slotOpen(false);
		}
		else
		{
			slotRefresh(false);
		}

		return true;
	}

	// close
	else if (dynamic_cast<QCloseEvent *>(e))
	{
		if
		(
			! saved
		)
		switch
		(
			QMessageBox::warning
			(
				this,
				"Warning",
                QString::fromStdString(string("Save changes to ") + (filename.size() ? filename.substr(filename.find_last_of('/') + 1) : "default.fcv") + "?"),
				QMessageBox::Yes | QMessageBox::Default,
				QMessageBox::No,
				QMessageBox::Cancel | QMessageBox::Escape
			)
		)
		{
		case QMessageBox::Yes:
			slotSave();

			if (! saved) 
				return true;
			break;

		case QMessageBox::Cancel:
			return true;
		};

		// save settings
		save();
	}

	// key press
	else if (QKeyEvent * p = dynamic_cast<QKeyEvent *>(e))
	{
		if (e->type() == QEvent::KeyPress)
		{
		}
		else if (e->type() == QEvent::ShortcutOverride)
		{
            if (toolsTrace_ModeAction->isChecked())
			{
				point<2> t(0, 0);

				switch (p->key())
				{
				case Qt::Key_Escape:
                    toolsTrace_ModeAction->setEnabled(false);
					slotTrace();
					break;

				case Qt::Key_Left:
					t.x = -1;
					break;

				case Qt::Key_Right:
					t.x = 1;
					break;

				case Qt::Key_Up:
					t.y = 1;
					break;

				case Qt::Key_Down:
					t.y = -1;
					break;
				}

				switch (p->key())
				{
				case Qt::Key_Left:
				case Qt::Key_Right:
				case Qt::Key_Up:
				case Qt::Key_Down:
                    if (p->modifiers() & Qt::ControlModifier)
					{
						t.x *= graphUI1_1->precisionx * 2;
						t.y *= graphUI1_1->precisiony * 2;
					}

					if (graphUI1_1->trace((int) t.x, (int) t.y))
					{
						writeCoordinates();
					}

					break;
				}
			}
			else
			{
                if (p->modifiers() & Qt::AltModifier || p->modifiers() & Qt::ShiftModifier)
				{
					return true;
				}

                if (p->modifiers() & Qt::ControlModifier)
				{
					switch (p->key())
					{
					case Qt::Key_A:
					case Qt::Key_C:
					case Qt::Key_X:
						return true;
					}
				}

				switch (p->key())
				{
				case Qt::Key_F5:
				case Qt::Key_F7:
				case Qt::Key_F8:
					return true;

				case Qt::Key_Control:
                    return QMainWindow::event(e);
				}

				if (option.accelshift)
				{
					switch (p->key())
					{
					case Qt::Key_BracketLeft:
					case Qt::Key_Bar:
					case Qt::Key_Less:
					case Qt::Key_Greater:
					case Qt::Key_Colon:
					case Qt::Key_AsciiCircum:
                        buttonGroup4->buttons().at(0)->setDown(true);
						slotToggleClicked(0);
                        return QMainWindow::event(e);
					}
				}
/*
				if (option.accelalpha)
				{
					switch (p->key())
					{
					case Qt::Key_Space:
					case Qt::Key_A ... Qt::Key_Z:
						buttonGroup4->setButton(1);
						slotToggleClicked(1);
                        return QMainWindow::event(e);
					}
				}
*/

				// disable special key processing inside spreadsheets
				if
				(
					pTabBar->currentIndex() == 1 ||
					pTabBar->currentIndex() == 3 ||
					pTabBar->currentIndex() == 4
				)
				{
                    return QMainWindow::event(e);
				}
				else switch (p->key())
				{
				case Qt::Key_Up:
					pushButton11_1->animateClick();
					return true;

				case Qt::Key_Down:
					pushButton11_2->animateClick();
					return true;
				}


                if (currentLineEdit()->alignment() == Qt::AlignRight)
				{
					setReadOnly(false);
				}

                if (currentLineEdit()->text().length() > 0)
				{
					switch (p->key())
					{
					case Qt::Key_Enter:
					case Qt::Key_Return:
                        if (viewScientific_CalculatorAction->isEnabled())
						{
							pushButton1_40->animateClick();
						}
						else
						{
							pushButton4_24->animateClick();
						}
						return true;
					}
				}
			}
		}
	}

    return QMainWindow::event(e);
}

#ifdef _WIN32
/**
	Read registry.

	This will read any key stored in the Windows registry.
*/


CString GetRegKey(HKEY m_Folder, CString str_Path, CString str_Name)
{
	CString str_KeyOut = TEXT("");

	HKEY hKey1;
	TCHAR szData[BUFSIZE];
	DWORD dwBufLen = BUFSIZE;

    // button Registry Data
	if (! RegOpenKeyEx(m_Folder, str_Path, 0, KEY_QUERY_VALUE, & hKey1))
	{
		if (! RegQueryValueEx(hKey1, str_Name, 0, 0, (LPBYTE) szData, & dwBufLen))
		{
			// turn TCHAR into CString
			str_KeyOut = szData;
		}

		RegCloseKey(hKey1);
	}

	return str_KeyOut;
}

bool SetRegKey(HKEY m_Folder, CString str_Path, CString str_Name, CString str_Value)
{
	HKEY hKey1;
	DWORD dwDisp;

	if (RegCreateKeyEx(m_Folder, str_Path, 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, & hKey1, & dwDisp))
	{
		return false;
	}

	// set the name of the message file.
	if (RegSetValueEx(hKey1,              	// subkey handle
			str_Name,        				// value name
			0,                         		// must be zero
			REG_SZ,             			// value type
			(LPBYTE) str_Value,        		// pointer to value data
			lstrlen(str_Value)*sizeof(*str_Value)+1)) 			// length of value data
	{
		RegCloseKey(hKey1);
		return false;
	}

	RegCloseKey(hKey1);
	return true;
}

#endif

bool MainUI::eventFilter(QObject * pObject, QEvent * pEvent)
{
    if (pObject == graphUI1_1)
	{
		if (pEvent->type() == QEvent::Resize)
		{
			static int count = 0;

			if (count > 1)
			{
				GraphUI::fast = true;
				killTimer(ee.nResizeId);

				// refresh graphic
				slotRefresh(false);

				GraphUI::fast = false;
				ee.nResizeId = startTimer(2000);
			}
			else
			{
				++ count;
			}
		}

		// quick zoom
		else if (QWheelEvent * p = dynamic_cast<QWheelEvent *>(pEvent))
		{
			if (p->delta() > 0)
				slotZoomIn();
			else
				slotZoomOut();

			p->accept();
		}

		// quick translation
		else if (QMouseEvent * p = dynamic_cast<QMouseEvent *>(pEvent))
		{
			int dx = p->globalPos().x() - nMousePos.x();
			int dy = p->globalPos().y() - nMousePos.y();

			nMousePos = p->globalPos();

			switch (p->type())
			{
			case QEvent::MouseMove:
				// adjust range
				switch (graphUI1_1->option.stat)
				{
				case GraphUI::eScientific:
					switch (graphUI1_1->option.mode)
					{
					case GraphUI::e2d:
						graphUI1_1->option.min.x -= graphUI1_1->fx.stp.x * dx;
						graphUI1_1->option.max.x -= graphUI1_1->fx.stp.x * dx;
						graphUI1_1->option.min.y += graphUI1_1->fx.stp.y * dy;
						graphUI1_1->option.max.y += graphUI1_1->fx.stp.y * dy;
						break;

					case GraphUI::e3d:
						graphUI1_1->option.min.x -= graphUI1_1->fxy.stp.x * dx;
						graphUI1_1->option.max.x -= graphUI1_1->fxy.stp.x * dx;
						graphUI1_1->option.min.y += graphUI1_1->fxy.stp.y * dy;
						graphUI1_1->option.max.y += graphUI1_1->fxy.stp.y * dy;
						break;

					default:
						break;
					}
					break;

				default:
					break;
				}

				// refresh graphic
				QApplication::postEvent(graphUI1_1, new QResizeEvent(graphUI1_1->size(), graphUI1_1->size()));
				break;

			default:
				break;
			}

			p->accept();
		}
	}

    else if (pEvent->type() == QEvent::RequestSoftwareInputPanel)
    {
        return true;
    }

    else if (pObject == currentLineEdit())
	{
        qDebug() << pEvent->type() << endl;
/**
        if (QKeyEvent * p = dynamic_cast<QKeyEvent *>(pEvent))
		{
			if (option.brackets)
			{
				p->ignore();
			}
		}
*/


/**
        else if (pEvent->type() == QEvent::FocusOut)
        {
            return true;
        }
*/
    }

	else if (pObject == frame11 || pObject == frame12)
	{
		if (pEvent->type() == QEvent::Paint)
		{
			if (frame11->width() <= 1 || frame12->width() <= 1)
			{
				return true;
			}
		}
	}

#ifdef _WIN32
	else if (QLabel * p = dynamic_cast<QLabel *>(pObject))
	{
		if (pEvent->type() == QEvent::MouseButtonPress)
		{
            int i = p->text().button("\\\"");
            QString str = p->text().mid(i + 2, p->text().button("\\\"", i + 2) - i - 2);

            int j = str.button(":");
			QString protocol = str.mid(0, j);

			CString str_Browser = TEXT("");

			if (protocol == "mailto")
			{
				str_Browser = GetRegKey(HKEY_CLASSES_ROOT, TEXT("mailto\\shell\\open\\command"), TEXT(""));
			}
			else if (protocol == "http")
			{
				str_Browser = GetRegKey(HKEY_CLASSES_ROOT, TEXT("htmlfile\\shell\\opennew\\command"), TEXT(""));
			}

			char ccommand[BUFSIZE];

			wcstombs(ccommand, str_Browser, BUFSIZE);

			QString scommand = QString(ccommand);

			Q3Process * process = new Q3Process(this);

			scommand.replace("%1", str);
			scommand.replace("%ProgramFiles%", getenv("PROGRAMFILES"));

			process->addArgument(scommand);

			if (process->start())
			{
				return true;
			}

			QMessageBox::critical
			(
				this,
				"Error",
				QString("Unexpected error starting '") + scommand + "'.\n\n",
				"Ok"
			);
		}
	}
#endif

    return QMainWindow::eventFilter(pObject, pEvent);
}


/**
	Main widget.
*/

MainUI::MainUI(QWidget * a_pcWidget, char const * a_pzName):
    QMainWindow(a_pcWidget),
	mode(eNothing),
	nMaximumSize(maximumSize())
{
	// main
    pMainUI = this;

	ee.nResizeId = 0;
	ee.nFlashId = 0;
	ee.nTranslateId = 0;

	// update accelerators
    ///pushButton1_13->setAccel(Qt::Key_Comma);
    ///helpHelp_TopicsAction->setAccel(Qt::Key_F1);


	// build up actions
    setupUi(this);

    viewBaseNActionGroup = new QActionGroup(this);
	viewBaseNActionGroup->addAction(viewHexadecimalAction);
	viewBaseNActionGroup->addAction(viewDecimalAction);
	viewBaseNActionGroup->addAction(viewOctalAction);
	viewBaseNActionGroup->addAction(viewBinaryAction);

	viewTrigoActionGroup = new QActionGroup(this);
	viewTrigoActionGroup->addAction(viewDegreesAction);
	viewTrigoActionGroup->addAction(viewRadiansAction);
	viewTrigoActionGroup->addAction(viewGradsAction);

	toolsZoomActionGroup = new QActionGroup(this);
	toolsZoomActionGroup->addAction(toolsZoom25Action);
	toolsZoomActionGroup->addAction(toolsZoom50Action);
	toolsZoomActionGroup->addAction(toolsZoom100Action);
	toolsZoomActionGroup->addAction(toolsZoom200Action);
	toolsZoomActionGroup->addAction(toolsZoom400Action);

    //! hard coded
    filePrintAction->setVisible(false);
    fileImportAction->setVisible(false);
    fileImportSingle_VariablesAction->setVisible(false);
    fileImportPaired_VariablesAction->setVisible(false);
    fileSend_ToAction->setVisible(false);
    fileSend_ToMicrosoft_PaintAction->setVisible(false);
    filePage_SetupAction->setVisible(false);
    settingsGraphicAction->setVisible(false);
    settingsOptionsAction->setVisible(false);
    helpRegisterAction->setVisible(false);
    viewStatus_BarAction->setVisible(false);
    toolsZoomAction->setVisible(false);

	// build up tabbar
	pTabBar = new QTabBar(centralWidget());
    pTabBar->addTab("Graphic");
    pTabBar->addTab("Variables");
/**
    pTabBar->addTab("Worksheet");
    pTabBar->addTab("Vectors");
    pTabBar->addTab("Matrices");
    pTabBar->addTab("Spreadsheet");
*/
    pTabBar->setDocumentMode(true);

    //lineEdit1_1->setReadOnly(true);

    _2->insertWidget(0, pTabBar);

    table4_1->setColumnCount(2);
    table5_1->setColumnCount(2);
    table6_1->setColumnCount(2);

    table4_1->setRowCount(26);
    table5_1->setRowCount(26);
    table6_1->setRowCount(26);

    table4_1->verticalHeader()->hide();
    table5_1->verticalHeader()->hide();
    table6_1->verticalHeader()->hide();

    table4_1->horizontalHeader()->hide();
    table5_1->horizontalHeader()->hide();
    table6_1->horizontalHeader()->hide();

    table4_1->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    table5_1->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    table6_1->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    table4_1->setCurrentCell(0, 0);
    table5_1->setCurrentCell(0, 0);
    table6_1->setCurrentCell(0, 0);

    table4_1->setSelectionMode(QAbstractItemView::SingleSelection);
    table5_1->setSelectionMode(QAbstractItemView::SingleSelection);
    table6_1->setSelectionMode(QAbstractItemView::SingleSelection);

	// initialize variables
    for (int i = 0; i < 26; ++ i)
	{
        table4_1->setCellWidget(i, 0, new QLineEdit(QChar('A' + (char) i)));
        table5_1->setCellWidget(i, 0, new QLineEdit(QChar('A' + (char) i)));
        table6_1->setCellWidget(i, 0, new QLineEdit(QChar('A' + (char) i)));

        table4_1->setRowHeight(i, lineEdit1_1->height());
        table5_1->setRowHeight(i, lineEdit1_1->height());
        table6_1->setRowHeight(i, lineEdit1_1->height());

        table4_1->cellWidget(i, 0)->setFont(lineEdit1_1->font());
        table5_1->cellWidget(i, 0)->setFont(lineEdit1_1->font());
        table6_1->cellWidget(i, 0)->setFont(lineEdit1_1->font());

        static_cast<QLineEdit *>(table4_1->cellWidget(i, 0))->setReadOnly(true);
        static_cast<QLineEdit *>(table5_1->cellWidget(i, 0))->setReadOnly(true);
        static_cast<QLineEdit *>(table6_1->cellWidget(i, 0))->setReadOnly(true);
    }

/**
	table4_1->setLeftMargin(0);
	table5_1->setLeftMargin(0);
	table6_1->setLeftMargin(0);

	table4_1->setColumnReadOnly(0, true);
	table5_1->setColumnReadOnly(0, true);
	table6_1->setColumnReadOnly(0, true);

	table5_2->setTopMargin(table5_1->topMargin());
	table6_2->setTopMargin(table6_1->topMargin());
	table7_1->setTopMargin(table5_1->topMargin());
	table7_2->setTopMargin(table6_1->topMargin());
*/

	// initialize history
	statement.resize(pTabBar->count());

/**
    statement[5].pLineEdit = lineEdit7_1;

    statement[0].pLineEdit->installEventFilter(this);
    statement[1].pLineEdit->installEventFilter(this);
	statement[2].pLineEdit->installEventFilter(this);
    statement[3].pLineEdit->installEventFilter(this);
    statement[4].pLineEdit->installEventFilter(this);
	statement[5].pLineEdit->installEventFilter(this);
*/
    //lineEdit1_1->installEventFilter(this);

    for (int i = 0; i < pTabBar->count(); ++ i)
	{
        statement[i].l.push_back(fdi::tuple<QString, int, int>(QString(""), comboBox2->currentIndex(), comboBox3->currentIndex()));
		statement[i].i = -- statement[i].l.end();
	}

	// initialize printer
/*
	pPrinter = new QPrinter();
	pPrinter->setPageSize(QPrinter::Letter);
	pPrinter->setPaperSource(QPrinter::Auto);
*/
	// initialize options
	ee.k[0] = 0;
	ee.k[1] = 0;
	zoom = 0;
	stat = false;
	saved = true;
	graph = 'y';
	filename = "default.fcv";

    option.precision = numeric_limits<floating::type>::digits10;
	option.trim = true;
	option.exponent = false;
	option.magnitude = true;
	option.imperial = true;
	option.basen = 1;
	option.trigo = 0;
	option.width = minimumWidth();
	option.height = minimumHeight();
	option.mixed = true;
	option.sexagesimal = false;
	option.status = true;
	option.scientific = true;
	option.brackets = true;
	option.commandline = false;
	option.accelshift = true;
	option.accelalpha = true;
    option.rpn = false;
	option.color = std::vector<int>(1, 0);
	option.stat_color = std::vector<int>(1, 8);

	graphUI1_1->option.min.x = -100;
	graphUI1_1->option.max.x = 100;
	graphUI1_1->option.scl.x = 25;
	graphUI1_1->option.min.y = -100;
	graphUI1_1->option.max.y = 100;
	graphUI1_1->option.scl.y = 25;
	graphUI1_1->option.min.z = -100;
	graphUI1_1->option.max.z = 100;
	graphUI1_1->option.scl.z = 25;

	// filter out events
	frame11->installEventFilter(this);
	frame12->installEventFilter(this);
	graphUI1_1->installEventFilter(this);

	// tune up widgets
    worksheetUI2_1->setFrameShape(QFrame::NoFrame);

/**
	table4_1->horizontalHeader()->setResizeEnabled(false);
	table4_1->verticalHeader()->setResizeEnabled(false);
	table5_1->horizontalHeader()->setResizeEnabled(false);
	table5_1->verticalHeader()->setResizeEnabled(false);
	table5_2->horizontalHeader()->setResizeEnabled(false);
	table5_2->verticalHeader()->setResizeEnabled(false);
	table6_1->horizontalHeader()->setResizeEnabled(false);
	table6_1->verticalHeader()->setResizeEnabled(false);
	table6_2->horizontalHeader()->setResizeEnabled(false);
	table6_2->verticalHeader()->setResizeEnabled(false);
	table7_1->horizontalHeader()->setResizeEnabled(false);
	table7_1->verticalHeader()->setResizeEnabled(false);

	table4_1->showHControls(false);
	table5_1->showHControls(false);
	table5_2->showHControls(false);
	table6_1->showHControls(false);
	table7_1->showHControls(false);
	table7_1->showVControls(false);
	table7_2->showHControls(false);
	table7_2->showVControls(false);
*/

	table5_1->setMaximumWidth(table5_1->horizontalHeader()->sectionSize(0) + table5_1->verticalScrollBar()->width());
	table6_1->setMaximumWidth(table6_1->horizontalHeader()->sectionSize(0) + table6_1->verticalScrollBar()->width());

	table5_1->selectRow('v' - 'a');
	table6_1->selectRow('w' - 'a');

    setWindowTitle(CAPTION);

	// color
	QPalette p[5] = {pushButton1_1->palette(), pushButton1_1->palette(), pushButton1_1->palette(), pushButton1_1->palette(), pushButton1_1->palette()};

    p[0].setColor( QPalette::Normal, QPalette::ButtonText, QColor( 0, 0, 255 ) );
    p[1].setColor( QPalette::Normal, QPalette::ButtonText, QColor( 255, 0, 255 ) );
    p[2].setColor( QPalette::Normal, QPalette::ButtonText, QColor( 170, 0, 0 ) );
    p[3].setColor( QPalette::Normal, QPalette::ButtonText, QColor( 0, 85, 0 ) );
    p[4].setColor( QPalette::Normal, QPalette::ButtonText, QColor( 255, 0, 0 ) );

    p[0].setColor( QPalette::Inactive, QPalette::ButtonText, QColor( 0, 0, 255 ) );
    p[1].setColor( QPalette::Inactive, QPalette::ButtonText, QColor( 255, 0, 255 ) );
    p[2].setColor( QPalette::Inactive, QPalette::ButtonText, QColor( 170, 0, 0 ) );
    p[3].setColor( QPalette::Inactive, QPalette::ButtonText, QColor( 0, 85, 0 ) );
    p[4].setColor( QPalette::Inactive, QPalette::ButtonText, QColor( 255, 0, 0 ) );

    buttonGroup1->setExclusive(false);
    buttonGroup4->setExclusive(false);

    buttonGroup4->setId(pushButton1, 1);
    buttonGroup4->setId(pushButton2, 2);

    buttonGroup9->setId(pushButton3, 1);
    buttonGroup9->setId(pushButton4, 2);

    buttonGroup5->setId(pushButton1_1, 1);
    buttonGroup5->setId(pushButton1_2, 11);
    buttonGroup5->setId(pushButton1_3, 21);
    buttonGroup5->setId(pushButton1_4, 31);
    buttonGroup5->setId(pushButton1_5, 2);
    buttonGroup5->setId(pushButton1_6, 12);
    buttonGroup5->setId(pushButton1_7, 22);
    buttonGroup5->setId(pushButton1_8, 32);
    buttonGroup5->setId(pushButton1_9, 3);
    buttonGroup5->setId(pushButton1_10, 13);
    buttonGroup5->setId(pushButton1_11, 23);
    buttonGroup5->setId(pushButton1_12, 33);
    buttonGroup5->setId(pushButton1_13, 4);
    buttonGroup5->setId(pushButton1_14, 14);
    buttonGroup5->setId(pushButton1_15, 24);
    buttonGroup5->setId(pushButton1_16, 34);
    buttonGroup5->setId(pushButton1_17, 5);
    buttonGroup5->setId(pushButton1_18, 15);
    buttonGroup5->setId(pushButton1_19, 25);
    buttonGroup5->setId(pushButton1_20, 35);
    buttonGroup5->setId(pushButton1_21, 6);
    buttonGroup5->setId(pushButton1_22, 16);
    buttonGroup5->setId(pushButton1_23, 26);
    buttonGroup5->setId(pushButton1_24, 36);
    buttonGroup5->setId(pushButton1_25, 7);
    buttonGroup5->setId(pushButton1_26, 17);
    buttonGroup5->setId(pushButton1_27, 27);
    buttonGroup5->setId(pushButton1_28, 37);
    buttonGroup5->setId(pushButton1_29, 8);
    buttonGroup5->setId(pushButton1_30, 18);
    buttonGroup5->setId(pushButton1_31, 28);
    buttonGroup5->setId(pushButton1_32, 38);
    buttonGroup5->setId(pushButton1_33, 9);
    buttonGroup5->setId(pushButton1_34, 19);
    buttonGroup5->setId(pushButton1_35, 29);
    buttonGroup5->setId(pushButton1_36, 39);
    buttonGroup5->setId(pushButton1_37, 10);
    buttonGroup5->setId(pushButton1_38, 20);
    buttonGroup5->setId(pushButton1_39, 30);
    buttonGroup5->setId(pushButton1_40, 40);

    buttonGroup6->setId(pushButton2_1, 1);
    buttonGroup6->setId(pushButton2_2, 11);
    buttonGroup6->setId(pushButton2_3, 21);
    buttonGroup6->setId(pushButton2_4, 31);
    buttonGroup6->setId(pushButton2_5, 2);
    buttonGroup6->setId(pushButton2_6, 12);
    buttonGroup6->setId(pushButton2_7, 22);
    buttonGroup6->setId(pushButton2_8, 32);
    buttonGroup6->setId(pushButton2_9, 3);
    buttonGroup6->setId(pushButton2_10, 13);
    buttonGroup6->setId(pushButton2_11, 23);
    buttonGroup6->setId(pushButton2_12, 33);
    buttonGroup6->setId(pushButton2_13, 4);
    buttonGroup6->setId(pushButton2_14, 14);
    buttonGroup6->setId(pushButton2_15, 24);
    buttonGroup6->setId(pushButton2_16, 34);
    buttonGroup6->setId(pushButton2_33, 9);
    buttonGroup6->setId(pushButton2_34, 19);
    buttonGroup6->setId(pushButton2_35, 29);
    buttonGroup6->setId(pushButton2_36, 39);
    buttonGroup6->setId(pushButton2_37, 10);
    buttonGroup6->setId(pushButton2_38, 20);
    buttonGroup6->setId(pushButton2_39, 30);
    buttonGroup6->setId(pushButton2_40, 40);

    buttonGroup7->setId(pushButton3_5, 2);
    buttonGroup7->setId(pushButton3_6, 12);
    buttonGroup7->setId(pushButton3_7, 22);
    buttonGroup7->setId(pushButton3_9, 3);
    buttonGroup7->setId(pushButton3_10, 13);
    buttonGroup7->setId(pushButton3_11, 23);
    buttonGroup7->setId(pushButton3_13, 4);
    buttonGroup7->setId(pushButton3_14, 14);
    buttonGroup7->setId(pushButton3_15, 24);
    buttonGroup7->setId(pushButton3_17, 5);
    buttonGroup7->setId(pushButton3_18, 15);
    buttonGroup7->setId(pushButton3_19, 25);
    buttonGroup7->setId(pushButton3_21, 6);
    buttonGroup7->setId(pushButton3_22, 16);
    buttonGroup7->setId(pushButton3_23, 26);
    buttonGroup7->setId(pushButton3_25, 7);
    buttonGroup7->setId(pushButton3_26, 17);
    buttonGroup7->setId(pushButton3_27, 27);
    buttonGroup7->setId(pushButton3_29, 8);
    buttonGroup7->setId(pushButton3_30, 18);
    buttonGroup7->setId(pushButton3_31, 28);
    buttonGroup7->setId(pushButton3_33, 9);
    buttonGroup7->setId(pushButton3_34, 19);
    buttonGroup7->setId(pushButton3_35, 29);
    buttonGroup7->setId(pushButton3_36, 10);
    buttonGroup7->setId(pushButton3_37, 20);
    buttonGroup7->setId(pushButton3_38, 30);

    buttonGroup10->setId(pushButton10_1, 0);
    buttonGroup10->setId(pushButton10_2, 1);
    buttonGroup10->setId(pushButton10_5, 2);
    buttonGroup10->setId(pushButton10_6, 3);
    buttonGroup11->setId(pushButton11_1, 4);
    buttonGroup11->setId(pushButton11_2, 5);

    buttonGroup6_1->setId(pushButton2_41, 1);
    buttonGroup6_1->setId(pushButton2_42, 5);
    buttonGroup6_1->setId(pushButton2_43, 9);
    buttonGroup6_1->setId(pushButton2_44, 2);
    buttonGroup6_1->setId(pushButton2_45, 6);
    buttonGroup6_1->setId(pushButton2_46, 10);
    buttonGroup6_1->setId(pushButton2_47, 3);
    buttonGroup6_1->setId(pushButton2_48, 7);
    buttonGroup6_1->setId(pushButton2_49, 11);
    buttonGroup6_1->setId(pushButton2_50, 4);
    buttonGroup6_1->setId(pushButton2_51, 8);
    buttonGroup6_1->setId(pushButton2_52, 12);

    buttonGroup6_2->setId(pushButton2_53, 1);
    buttonGroup6_2->setId(pushButton2_54, 5);
    buttonGroup6_2->setId(pushButton2_55, 9);
    buttonGroup6_2->setId(pushButton2_56, 2);
    buttonGroup6_2->setId(pushButton2_57, 6);
    buttonGroup6_2->setId(pushButton2_58, 10);
    buttonGroup6_2->setId(pushButton2_59, 3);
    buttonGroup6_2->setId(pushButton2_60, 7);
    buttonGroup6_2->setId(pushButton2_61, 11);
    buttonGroup6_2->setId(pushButton2_62, 4);
    buttonGroup6_2->setId(pushButton2_63, 8);
    buttonGroup6_2->setId(pushButton2_64, 12);

    buttonGroup6_3->setId(pushButton5, 0);
    buttonGroup6_3->setId(pushButton6, 1);

    pushButton1->setPalette(p[4]);
	pushButton2->setPalette(p[4]);
	pushButton3->setPalette(p[4]);
	pushButton4->setPalette(p[4]);
	pushButton5->setPalette(p[4]);
	pushButton6->setPalette(p[4]);

	pushButton1_1->setPalette(p[0]);
	pushButton1_2->setPalette(p[0]);
	pushButton1_3->setPalette(p[0]);
	pushButton1_4->setPalette(p[0]);
	pushButton1_5->setPalette(p[1]);
	pushButton1_6->setPalette(p[1]);
	pushButton1_7->setPalette(p[1]);
	pushButton1_8->setPalette(p[1]);
	pushButton1_9->setPalette(p[1]);
	pushButton1_10->setPalette(p[1]);
	pushButton1_11->setPalette(p[1]);
	pushButton1_12->setPalette(p[1]);
	pushButton1_13->setPalette(p[1]);
	pushButton1_14->setPalette(p[1]);
	pushButton1_15->setPalette(p[1]);
	pushButton1_16->setPalette(p[1]);
	pushButton1_17->setPalette(p[0]);
	pushButton1_18->setPalette(p[0]);
	pushButton1_19->setPalette(p[0]);
	pushButton1_20->setPalette(p[0]);
	pushButton1_21->setPalette(p[0]);
	pushButton1_22->setPalette(p[0]);
	pushButton1_23->setPalette(p[0]);
	pushButton1_24->setPalette(p[0]);
	pushButton1_25->setPalette(p[0]);
	pushButton1_26->setPalette(p[0]);
	pushButton1_27->setPalette(p[0]);
	pushButton1_28->setPalette(p[0]);
	pushButton1_29->setPalette(p[2]);
	pushButton1_30->setPalette(p[2]);
	pushButton1_31->setPalette(p[2]);
	pushButton1_32->setPalette(p[2]);
	pushButton1_33->setPalette(p[3]);
	pushButton1_34->setPalette(p[3]);
	pushButton1_35->setPalette(p[3]);
	pushButton1_37->setPalette(p[3]);
	pushButton1_38->setPalette(p[3]);
	pushButton1_39->setPalette(p[3]);
	pushButton1_36->setPalette(p[0]);
	pushButton1_40->setPalette(p[4]);

	pushButton2_1->setPalette(p[0]);
	pushButton2_2->setPalette(p[0]);
	pushButton2_3->setPalette(p[0]);
	pushButton2_4->setPalette(p[0]);
	pushButton2_5->setPalette(p[1]);
	pushButton2_6->setPalette(p[1]);
	pushButton2_7->setPalette(p[1]);
	pushButton2_8->setPalette(p[1]);
	pushButton2_9->setPalette(p[1]);
	pushButton2_10->setPalette(p[1]);
	pushButton2_11->setPalette(p[1]);
	pushButton2_12->setPalette(p[1]);
	pushButton2_13->setPalette(p[1]);
	pushButton2_14->setPalette(p[1]);
	pushButton2_15->setPalette(p[1]);
	pushButton2_16->setPalette(p[1]);
	pushButton2_33->setPalette(p[3]);
	pushButton2_34->setPalette(p[3]);
	pushButton2_35->setPalette(p[3]);
	pushButton2_36->setPalette(p[3]);
	pushButton2_37->setPalette(p[3]);
	pushButton2_38->setPalette(p[3]);
	pushButton2_39->setPalette(p[3]);
	pushButton2_40->setPalette(p[3]);
	pushButton2_41->setPalette(p[2]);
	pushButton2_42->setPalette(p[2]);
	pushButton2_43->setPalette(p[2]);
	pushButton2_44->setPalette(p[2]);
	pushButton2_45->setPalette(p[2]);
	pushButton2_46->setPalette(p[2]);
	pushButton2_47->setPalette(p[2]);
	pushButton2_48->setPalette(p[2]);
	pushButton2_49->setPalette(p[2]);
	pushButton2_50->setPalette(p[2]);
	pushButton2_51->setPalette(p[2]);
	pushButton2_52->setPalette(p[2]);
	pushButton2_53->setPalette(p[2]);
	pushButton2_54->setPalette(p[2]);
	pushButton2_55->setPalette(p[2]);
	pushButton2_56->setPalette(p[2]);
	pushButton2_57->setPalette(p[2]);
	pushButton2_58->setPalette(p[2]);
	pushButton2_59->setPalette(p[2]);
	pushButton2_60->setPalette(p[2]);
	pushButton2_61->setPalette(p[2]);
	pushButton2_62->setPalette(p[2]);
	pushButton2_63->setPalette(p[2]);
	pushButton2_64->setPalette(p[2]);

	// signals / slots
    connect(pTabBar, SIGNAL(currentChanged(int)), SLOT(slotWidgetStack(int)));

    connect(comboBox2, SIGNAL(currentIndexChanged(int)), SLOT(slotBaseNClicked(int)));
    connect(comboBox3, SIGNAL(currentIndexChanged(int)), SLOT(slotTrigoClicked(int)));
    connect(buttonGroup4, SIGNAL(buttonClicked(int)), SLOT(slotToggleClicked(int)));
    connect(buttonGroup5, SIGNAL(buttonClicked(int)), SLOT(slotKeypadClicked(int)));
    connect(buttonGroup6, SIGNAL(buttonClicked(int)), SLOT(slotKeypadClicked(int)));
    connect(buttonGroup7, SIGNAL(buttonClicked(int)), SLOT(slotKeypadClicked(int)));
    connect(buttonGroup6_1, SIGNAL(buttonClicked(int)), SLOT(slotKeypadClicked(int)));
    connect(buttonGroup6_2, SIGNAL(buttonClicked(int)), SLOT(slotKeypadClicked(int)));
    connect(buttonGroup6_3, SIGNAL(buttonClicked(int)), widgetStack6_1, SLOT(setCurrentIndex(int)));

    connect(buttonGroup5, SIGNAL(buttonClicked(int)), SLOT(group5clicked(int)));
    connect(buttonGroup6, SIGNAL(buttonClicked(int)), SLOT(group6clicked(int)));
    connect(buttonGroup7, SIGNAL(buttonClicked(int)), SLOT(group7clicked(int)));
    connect(buttonGroup8, SIGNAL(buttonClicked(int)), SLOT(group8clicked(int)));
    connect(buttonGroup6_1, SIGNAL(buttonClicked(int)), SLOT(group6_1clicked(int)));
    connect(buttonGroup6_2, SIGNAL(buttonClicked(int)), SLOT(group6_2clicked(int)));
	connect(pushButton3, SIGNAL(clicked()), SLOT(button3clicked()));
	connect(pushButton4, SIGNAL(clicked()), SLOT(button4clicked()));
    connect(buttonGroup10, SIGNAL(buttonClicked(int)), SLOT(group10clicked(int)));
    connect(buttonGroup11, SIGNAL(buttonClicked(int)), SLOT(group10clicked(int)));

    connect(fileNewAction, SIGNAL(triggered()), SLOT(slotNew()));
    connect(fileOpenAction, SIGNAL(triggered()), SLOT(slotOpen()));
    connect(fileSaveAction, SIGNAL(triggered()), SLOT(slotSave()));
    connect(filePage_SetupAction, SIGNAL(triggered()), SLOT(slotSetup()));
    connect(filePrintAction, SIGNAL(triggered()), SLOT(slotPrint()));
    connect(fileSave_AsAction, SIGNAL(triggered()), SLOT(slotSaveAs()));
    connect(fileImportSingle_VariablesAction, SIGNAL(triggered()), SLOT(slotImportSingleVariables()));
    connect(fileImportPaired_VariablesAction, SIGNAL(triggered()), SLOT(slotImportPairedVariables()));

    connect(fileSend_ToMicrosoft_PaintAction, SIGNAL(triggered()), SLOT(slotPaint()));
    connect(fileExitAction, SIGNAL(triggered()), SLOT(slotExit()));
    connect(editCutAction, SIGNAL(triggered()), SLOT(slotCut()));
    connect(editCopyAction, SIGNAL(triggered()), SLOT(slotCopy()));
    connect(editPasteAction, SIGNAL(triggered()), SLOT(slotPaste()));
    connect(editClearAction, SIGNAL(triggered()), SLOT(slotClearText()));
    connect(editSelect_AllAction, SIGNAL(triggered()), SLOT(slotSelectAll()));

    //connect(viewPrevious_ExpressionAction, SIGNAL(triggered()), SLOT(slotPreviousExpression()));
    //connect(viewNext_ExpressionAction, SIGNAL(triggered()), SLOT(slotNextExpression()));
    connect(viewFirst_ExpressionAction, SIGNAL(triggered()), SLOT(slotFirstExpression()));
    connect(viewLast_ExpressionAction, SIGNAL(triggered()), SLOT(slotLastExpression()));
    connect(viewBaseNActionGroup, SIGNAL(triggered(QAction *)), SLOT(slotBaseN(QAction *)));
    connect(viewTrigoActionGroup, SIGNAL(triggered(QAction *)), SLOT(slotTrigo(QAction *)));
    connect(viewMixed_FractionAction, SIGNAL(triggered()), SLOT(slotMixed()));
    connect(viewSexagesimal_NotationAction, SIGNAL(triggered()), SLOT(slotSexagesimal()));
    connect(viewStatus_BarAction, SIGNAL(triggered()), SLOT(slotStatusBar()));
    //connect(viewScientific_CalculatorAction, SIGNAL(triggered()), SLOT(slotScientific()));

    connect(toolsReverse_Polish_NotationAction, SIGNAL(triggered()), SLOT(slotRPN()));
    connect(toolsPlotYAction, SIGNAL(triggered()), SLOT(slotPlotY()));
    connect(toolsPlotZAction, SIGNAL(triggered()), SLOT(slotPlotZ()));
    connect(toolsBar_ChartAction, SIGNAL(triggered()), SLOT(slotBarChart()));
    connect(toolsScatter_PlotAction, SIGNAL(triggered()), SLOT(slotScatterPlot()));
    connect(toolsClearAction, SIGNAL(triggered()), SLOT(slotClear()));
    connect(toolsZoomActionGroup, SIGNAL(triggered(QAction *)), SLOT(slotZoom(QAction *)));
    connect(toolsZoom_InAction, SIGNAL(triggered()), SLOT(slotZoomIn()));
    connect(toolsZoom_OutAction, SIGNAL(triggered()), SLOT(slotZoomOut()));
    connect(toolsTrace_ModeAction, SIGNAL(triggered()), SLOT(slotTrace()));

    connect(settingsRangeAction, SIGNAL(triggered()), SLOT(slotRange()));
    connect(settingsGraphicAction, SIGNAL(triggered()), SLOT(slotGraphic()));
    connect(settingsOptionsAction, SIGNAL(triggered()), SLOT(slotOptions()));

    connect(helpRegisterAction, SIGNAL(triggered()), SLOT(slotRegister()));
    connect(helpHelp_TopicsAction, SIGNAL(triggered()), SLOT(slotHelp()));
    connect(helpAbout_Fornux_CalculatorAction, SIGNAL(triggered()), SLOT(slotAbout()));

	connect(EditMenu, SIGNAL(aboutToShow()), SLOT(slotSelectionChanged()));

    connect(toolsInsertsortAction, SIGNAL(triggered()), SLOT(slotInsertSortClicked()));
    connect(toolsInsertpartAction, SIGNAL(triggered()), SLOT(slotInsertPartClicked()));
    connect(toolsInsertmergeAction, SIGNAL(triggered()), SLOT(slotInsertMergeClicked()));
    connect(toolsInsertcolxAction, SIGNAL(triggered()), SLOT(slotInsertColXClicked()));
    connect(toolsInsertcolyAction, SIGNAL(triggered()), SLOT(slotInsertColYClicked()));
    connect(toolsInserttransposeAction, SIGNAL(triggered()), SLOT(slotInsertTransposeClicked()));
    connect(toolsInsertresultAction, SIGNAL(triggered()), SLOT(slotInsertResultClicked()));

    connect(pTabBar, SIGNAL(currentChanged(int)), widgetStack10, SLOT(setCurrentIndex(int)));
/**
    connect(table5_1, SIGNAL(currentCellChanged(int, int, int, int)), SLOT(slotVectorSelected()));
    connect(table5_2, SIGNAL(itemChanged(QTableWidgetItem *)), SLOT(slotVectorChanged()));
    connect(table6_1, SIGNAL(currentCellChanged(int, int, int, int)), SLOT(slotMatrixSelected()));
    connect(table6_2, SIGNAL(itemChanged(QTableWidgetItem *)), SLOT(slotMatrixChanged()));
    connect(table4_1, SIGNAL(itemChanged(QTableWidgetItem *)), SLOT(slotVariableChanged()));
*/
    connect(QApplication::desktop(), SIGNAL(workAreaResized(int)), SLOT(workAreaResized(int)));

	// reload settings if possible
	load();

    viewMixed_FractionAction->setEnabled(option.mixed);
    viewSexagesimal_NotationAction->setEnabled(option.sexagesimal);
    viewStatus_BarAction->setEnabled(option.status);
    viewScientific_CalculatorAction->setEnabled(option.scientific);

    toolsReverse_Polish_NotationAction->setChecked(option.rpn);

    //! hard coded states
    viewStatus_BarAction->setEnabled(false);
    //pTabBar->hide();

	// call slots which weren't yet called
	slotBaseNClicked(option.basen);
	slotTrigoClicked(option.trigo);
	slotMixed();
	slotSexagesimal();

	slotStatusBar();
    slotScientific();

	// postpone initialization
    //QApplication::postEvent(currentLineEdit(), new QEvent(QEvent::FocusIn));
    QApplication::postEvent(this, new QEvent(QEvent::CloseSoftwareInputPanel));
    //currentLineEdit()->setFocus();
    //QEvent event(QEvent::CloseSoftwareInputPanel);
    //QApplication::sendEvent(this, &event);

    QApplication::postEvent(this, new QCustomEvent());
}


QLineEdit * MainUI::currentLineEdit()
{
    switch (pTabBar->currentIndex())
    {
    case 0:
        return lineEdit1_1;

    case 1:
        return static_cast<QLineEdit *>(table4_1->cellWidget(table4_1->currentRow(), 1));

    case 2:
        return worksheetUI2_1->lineEdit();

    case 3:
        return static_cast<QLineEdit *>(table5_1->cellWidget(table5_1->currentRow(), 1));

    case 4:
        return static_cast<QLineEdit *>(table6_1->cellWidget(table6_1->currentRow(), 1));
    }
}


void MainUI::load()
{
	QSettings settings;
    ///settings.setPath("Fornux", "Calculator", QSettings::UserScope);

    option.restore = settings.value("/startup/restore", false).toBool();
    option.reload = settings.value("/startup/reload", false).toBool();
    option.utility = settings.value("/startup/utility", "mspaint.exe").toString().toUtf8().constData();

	if (option.restore)
	{
        option.precision = settings.value("/option/precision", numeric_limits<floating::type>::digits10).toInt();
        option.trim = settings.value("/option/trim", true).toBool();
        option.exponent = settings.value("/option/exponent", false).toBool();
        option.magnitude = settings.value("/option/magnitude", true).toBool();
        option.imperial = settings.value("/option/imperial", true).toBool();

        option.basen = settings.value("/option/basen", 1).toInt();
        option.trigo = settings.value("/option/trigo", 0).toInt();
        option.width = settings.value("/option/width", minimumWidth()).toInt();
        option.height = settings.value("/option/height", minimumHeight()).toInt();
        option.mixed = settings.value("/option/mixed", false).toBool();
        option.sexagesimal = settings.value("/option/sexagesimal", false).toBool();

        option.rpn = settings.value("/option/rpn", false).toBool();
        option.status = settings.value("/option/status", true).toBool();
        option.scientific = settings.value("/option/scientific", true).toBool();
        option.brackets = settings.value("/option/brackets", true).toBool();

        graphUI1_1->option.min.x = settings.value("/graphic/min/x", -100).toDouble();
        graphUI1_1->option.max.x = settings.value("/graphic/max/x", 100).toDouble();
        graphUI1_1->option.scl.x = settings.value("/graphic/scl/x", 25).toDouble();
        graphUI1_1->option.min.y = settings.value("/graphic/min/y", -100).toDouble();
        graphUI1_1->option.max.y = settings.value("/graphic/max/y", 100).toDouble();
        graphUI1_1->option.scl.y = settings.value("/graphic/scl/y", 25).toDouble();
        graphUI1_1->option.min.z = settings.value("/graphic/min/z", -100).toDouble();
        graphUI1_1->option.max.z = settings.value("/graphic/max/z", 100).toDouble();
        graphUI1_1->option.scl.z = settings.value("/graphic/scl/z", 25).toDouble();

        graphUI1_1->option.header = settings.value("/graphic/header").toBool();
        graphUI1_1->option.legend = settings.value("/graphic/legend").toBool();
        graphUI1_1->option.title_font.fromString(settings.value("/graphic/title_font").toString());
        graphUI1_1->option.unit_font.fromString(settings.value("/graphic/unit_font").toString());
        graphUI1_1->option.scale_font.fromString(settings.value("/graphic/scale_font").toString());

        graphUI1_1->option.color = settings.value("/graphic/scientific_color").toInt();
        graphUI1_1->option.stat_color = settings.value("/graphic/statistic_color").toInt();
        graphUI1_1->option.precision = settings.value("/graphic/precision").toInt();
        graphUI1_1->option.title = settings.value("/graphic/title").toString();
        graphUI1_1->option.grid = settings.value("/graphic/grid").toBool();
        graphUI1_1->option.lightning = settings.value("/graphic/lightning").toBool();
        graphUI1_1->option.adjust = settings.value("/graphic/adjust").toBool();
        graphUI1_1->option.rescale = settings.value("/graphic/rescale").toBool();

        graphUI1_1->option.mode = (GraphUI::eMode) settings.value("/graphic/mode").toInt();
        graphUI1_1->option.stat = (GraphUI::eStat) settings.value("/graphic/stat").toInt();

        graphUI1_1->option.report[0].hcenter = settings.value("/graphic/report1/hcenter").toBool();
        graphUI1_1->option.report[0].vcenter = settings.value("/graphic/report1/vcenter").toBool();
        graphUI1_1->option.report[0].width = settings.value("/graphic/report1/width").toDouble();
        graphUI1_1->option.report[0].height = settings.value("/graphic/report1/height").toDouble();
        graphUI1_1->option.report[0].left = settings.value("/graphic/report1/left").toDouble();
        graphUI1_1->option.report[0].right = settings.value("/graphic/report1/right").toDouble();
        graphUI1_1->option.report[0].top = settings.value("/graphic/report1/top").toDouble();
        graphUI1_1->option.report[0].bottom = settings.value("/graphic/report1/bottom").toDouble();
        graphUI1_1->option.report[1].hcenter = settings.value("/graphic/report2/hcenter").toBool();
        graphUI1_1->option.report[1].vcenter = settings.value("/graphic/report2/vcenter").toBool();
        graphUI1_1->option.report[1].width = settings.value("/graphic/report2/width").toDouble();
        graphUI1_1->option.report[1].height = settings.value("/graphic/report2/height").toDouble();
        graphUI1_1->option.report[1].left = settings.value("/graphic/report2/left").toDouble();
        graphUI1_1->option.report[1].right = settings.value("/graphic/report2/right").toDouble();
        graphUI1_1->option.report[1].top = settings.value("/graphic/report2/top").toDouble();
        graphUI1_1->option.report[1].bottom = settings.value("/graphic/report2/bottom").toDouble();

        graphUI1_1->option.x.color = settings.value("/graphic/x/color").toInt();
        graphUI1_1->option.x.unit = settings.value("/graphic/x/unit").toString();
        graphUI1_1->option.x.axis = settings.value("/graphic/x/axis").toBool();
        graphUI1_1->option.x.numbers = settings.value("/graphic/x/numbers").toBool();
        graphUI1_1->option.x.grid = settings.value("/graphic/x/grid").toBool();
        graphUI1_1->option.x.trace = settings.value("/graphic/x/trace").toBool();
        graphUI1_1->option.y.color = settings.value("/graphic/y/color").toInt();
        graphUI1_1->option.y.unit = settings.value("/graphic/y/unit").toString();
        graphUI1_1->option.y.axis = settings.value("/graphic/y/axis").toBool();
        graphUI1_1->option.y.numbers = settings.value("/graphic/y/numbers").toBool();
        graphUI1_1->option.y.grid = settings.value("/graphic/y/grid").toBool();
        graphUI1_1->option.y.trace = settings.value("/graphic/y/trace").toBool();
        graphUI1_1->option.z.color = settings.value("/graphic/z/color").toInt();
        graphUI1_1->option.z.unit = settings.value("/graphic/z/unit").toString();
        graphUI1_1->option.z.axis = settings.value("/graphic/z/axis").toBool();
        graphUI1_1->option.z.numbers = settings.value("/graphic/z/numbers").toBool();
        graphUI1_1->option.z.grid = settings.value("/graphic/z/grid").toBool();
        graphUI1_1->option.z.trace = settings.value("/graphic/z/trace").toBool();
	}

	if (option.reload)
	{
        filename = settings.value("/startup/filename", "default.fcv").toString().toUtf8().constData();
	}
}


void MainUI::save()
{
	QSettings settings;
    ///settings.setPath("Fornux", "Calculator", QSettings::UserScope);

    settings.setValue("/startup/restore", option.restore);
    settings.setValue("/startup/reload", option.reload);
    settings.setValue("/startup/utility", QString::fromStdString(option.utility));

	if (option.restore)
	{
        settings.setValue("/option/precision", option.precision);
        settings.setValue("/option/trim", option.trim);
        settings.setValue("/option/exponent", option.exponent);
        settings.setValue("/option/magnitude", option.magnitude);
        settings.setValue("/option/imperial", option.imperial);

        settings.setValue("/option/basen", option.basen);
        settings.setValue("/option/trigo", option.trigo);
        settings.setValue("/option/width", width());
        settings.setValue("/option/height", height());
        settings.setValue("/option/mixed", option.mixed);
        settings.setValue("/option/sexagesimal", option.sexagesimal);

        settings.setValue("/option/rpn", option.rpn);
        settings.setValue("/option/status", option.status);
        settings.setValue("/option/scientific", option.scientific);
        settings.setValue("/option/brackets", option.brackets);

        settings.setValue("/graphic/min/x", (double)(graphUI1_1->option.min.x));
        settings.setValue("/graphic/max/x", (double)(graphUI1_1->option.max.x));
        settings.setValue("/graphic/scl/x", (double)(graphUI1_1->option.scl.x));
        settings.setValue("/graphic/min/y", (double)(graphUI1_1->option.min.y));
        settings.setValue("/graphic/max/y", (double)(graphUI1_1->option.max.y));
        settings.setValue("/graphic/scl/y", (double)(graphUI1_1->option.scl.y));
        settings.setValue("/graphic/min/z", (double)(graphUI1_1->option.min.z));
        settings.setValue("/graphic/max/z", (double)(graphUI1_1->option.max.z));
        settings.setValue("/graphic/scl/z", (double)(graphUI1_1->option.scl.z));

        settings.setValue("/graphic/header", graphUI1_1->option.header);
        settings.setValue("/graphic/legend", graphUI1_1->option.legend);
        settings.setValue("/graphic/title_font", graphUI1_1->option.title_font.toString());
        settings.setValue("/graphic/unit_font", graphUI1_1->option.unit_font.toString());
        settings.setValue("/graphic/scale_font", graphUI1_1->option.scale_font.toString());

        settings.setValue("/graphic/scientific_color", graphUI1_1->option.color);
        settings.setValue("/graphic/statistic_color", graphUI1_1->option.stat_color);
        settings.setValue("/graphic/precision", graphUI1_1->option.precision);
        settings.setValue("/graphic/title", graphUI1_1->option.title);
        settings.setValue("/graphic/grid", graphUI1_1->option.grid);
        settings.setValue("/graphic/lightning", graphUI1_1->option.lightning);
        settings.setValue("/graphic/adjust", graphUI1_1->option.adjust);
        settings.setValue("/graphic/rescale", graphUI1_1->option.rescale);

        settings.setValue("/graphic/mode", graphUI1_1->option.mode);
        settings.setValue("/graphic/stat", graphUI1_1->option.stat);

        settings.setValue("/graphic/report1/hcenter", graphUI1_1->option.report[0].hcenter);
        settings.setValue("/graphic/report1/vcenter", graphUI1_1->option.report[0].vcenter);
        settings.setValue("/graphic/report1/width", (double)(graphUI1_1->option.report[0].width));
        settings.setValue("/graphic/report1/height", (double)(graphUI1_1->option.report[0].height));
        settings.setValue("/graphic/report1/left", (double)(graphUI1_1->option.report[0].left));
        settings.setValue("/graphic/report1/right", (double)(graphUI1_1->option.report[0].right));
        settings.setValue("/graphic/report1/top", (double)(graphUI1_1->option.report[0].top));
        settings.setValue("/graphic/report1/bottom", (double)(graphUI1_1->option.report[0].bottom));
        settings.setValue("/graphic/report2/hcenter", graphUI1_1->option.report[1].hcenter);
        settings.setValue("/graphic/report2/vcenter", graphUI1_1->option.report[1].vcenter);
        settings.setValue("/graphic/report2/width", (double)(graphUI1_1->option.report[1].width));
        settings.setValue("/graphic/report2/height", (double)(graphUI1_1->option.report[1].height));
        settings.setValue("/graphic/report2/left", (double)(graphUI1_1->option.report[1].left));
        settings.setValue("/graphic/report2/right", (double)(graphUI1_1->option.report[1].right));
        settings.setValue("/graphic/report2/top", (double)(graphUI1_1->option.report[1].top));
        settings.setValue("/graphic/report2/bottom", (double)(graphUI1_1->option.report[1].bottom));

        settings.setValue("/graphic/x/color", graphUI1_1->option.x.color);
        settings.setValue("/graphic/x/unit", graphUI1_1->option.x.unit);
        settings.setValue("/graphic/x/axis", graphUI1_1->option.x.axis);
        settings.setValue("/graphic/x/numbers", graphUI1_1->option.x.numbers);
        settings.setValue("/graphic/x/grid", graphUI1_1->option.x.grid);
        settings.setValue("/graphic/x/trace", graphUI1_1->option.x.trace);
        settings.setValue("/graphic/y/color", graphUI1_1->option.y.color);
        settings.setValue("/graphic/y/unit", graphUI1_1->option.y.unit);
        settings.setValue("/graphic/y/axis", graphUI1_1->option.y.axis);
        settings.setValue("/graphic/y/numbers", graphUI1_1->option.y.numbers);
        settings.setValue("/graphic/y/grid", graphUI1_1->option.y.grid);
        settings.setValue("/graphic/y/trace", graphUI1_1->option.y.trace);
        settings.setValue("/graphic/z/color", graphUI1_1->option.z.color);
        settings.setValue("/graphic/z/unit", graphUI1_1->option.z.unit);
        settings.setValue("/graphic/z/axis", graphUI1_1->option.z.axis);
        settings.setValue("/graphic/z/numbers", graphUI1_1->option.z.numbers);
        settings.setValue("/graphic/z/grid", graphUI1_1->option.z.grid);
        settings.setValue("/graphic/z/trace", graphUI1_1->option.z.trace);
	}

	if (option.reload)
	{
		if (filename.empty())
		{
			filename = "default.fcv";
		}

        settings.setValue("/startup/filename", QString::fromStdString(filename));
	}
}


void MainUI::group1clicked(int)
{
}


void MainUI::group2clicked(int)
{
}


void MainUI::group3clicked(int)
{
}


void MainUI::group5clicked(int i)
{
    QLineEdit * pLineEdit = currentLineEdit();

    // memory handler
    if (buttonGroup5->button(i))
	{
		switch (i)
		{
		case 1:
			mem = floating(0.0);
			textLabel1_1->clear();
			textLabel7_1->clear();
			pushButton1_1->setEnabled(false);
			pushButton1_2->setEnabled(false);
			pushButton4_1->setEnabled(false);
			pushButton4_2->setEnabled(false);
			return;

		case 21:
			mem = mem - res;
			textLabel1_1->setText("M");
			textLabel7_1->setText("M");
			pushButton1_1->setEnabled(true);
			pushButton1_2->setEnabled(true);
			pushButton4_1->setEnabled(true);
			pushButton4_2->setEnabled(true);
			return;

		case 31:
			mem = mem + res;
			textLabel1_1->setText("M");
			textLabel7_1->setText("M");
			pushButton1_1->setEnabled(true);
			pushButton1_2->setEnabled(true);
			pushButton4_1->setEnabled(true);
			pushButton4_2->setEnabled(true);
			return;
		}
	}

	if (pLineEdit->alignment() == Qt::AlignRight)
	{
		setReadOnly(false);

		switch (i)
		{
		case 8: case 18: case 28: case 38:
		case 13: case 14: case 33:
			writeResult(res, pLineEdit);
			break;
		}
	}

	// space ai
	int c = pLineEdit->cursorPosition();
	
	QString const sFix[][4] = 
	{
		{
			c == 0 || pLineEdit->text()[c - 1] == ' ' ? "" : " ", 
			c != pLineEdit->text().length() && pLineEdit->text()[c] == ' ' ? "" : " ", 
			"()", 
			"(, )"
		}, 
		{"", "", "", ""}
	};

	QString const * pFix = option.rpn ? sFix[1] : sFix[0];

    if (QAbstractButton * p = buttonGroup5->button(i))
	{
		switch (i)
		{
		case 40: stat = false; break;
		}

		switch (i)
		{
		case 5: case 6: case 7: case 9: case 10:
		case 15: case 16: case 17:
		case 19: case 20: case 25: case 26:
		case 27: case 29: case 30:
		case 35: case 37:
			pLineEdit->insert(p->text());
			break;

		case 8:
		case 18:
			pLineEdit->insert(pFix[0] + p->text() + pFix[1]);
			break;

		case 28: case 38:
			if (int c = pLineEdit->cursorPosition())
			{
                if (c > 1 && pLineEdit->text().mid(c - 2, 2).indexOf(QRegExp("([0-9]|\\.)e")) != -1)
				{
					pLineEdit->insert(p->text());
					break;
				}
				else
				{
                    int i = pLineEdit->text().lastIndexOf(QRegExp("\\S"), c - 1);
                    int j = pLineEdit->text().lastIndexOf(QRegExp("[0-9a-zA-Z.!\\)\\]\\}\\|\\x0435\\x03C0]"), c - 1);

					if (i >= 0 && i == j)
					{
						if (pLineEdit->text()[i] != '|' || (bool) pLineEdit->text().left(c - 1).contains('|') % 2)
						{
							pLineEdit->insert(pFix[0] + p->text() + pFix[1]);
							break;
						}
					}
				}
			}

			pLineEdit->insert(p->text());
			break;

		case 2:
			if (option.brackets)
			{
				pLineEdit->insert("()");
				pLineEdit->cursorBackward(false);
			}
			else
			{
				pLineEdit->insert("(");
			}
			break;

		case 3:
			pLineEdit->insert(" = ");
			break;

		case 4:
			pLineEdit->insert(", ");
			break;

		case 12: case 22: case 32:
			if (checkBox1->isChecked() && checkBox2->isChecked())
				pLineEdit->insert(QString("a") + p->text() + "h" + pFix[2]);
			else if (checkBox1->isChecked())
				pLineEdit->insert(QString("a") + p->text() + pFix[2]);
			else if (checkBox2->isChecked())
				pLineEdit->insert(p->text() + "h" + pFix[2]);
			else
				pLineEdit->insert(p->text() + pFix[2]);

			if (! option.rpn) 
				pLineEdit->cursorBackward(false);

			checkBox1->setChecked(false);
			checkBox2->setChecked(false);
			break;

		case 24: case 34:
			pLineEdit->insert(p->text() + pFix[2]);
			if (! option.rpn) 
				pLineEdit->cursorBackward(false);
			break;

		case 11:
			writeResult(mem, pLineEdit);
			break;

		case 13:
			pLineEdit->insert("!");
			break;

		case 14:
			pLineEdit->insert("^-1");
			break;

		case 23:
			pLineEdit->insert(trUtf8("\xe2\x88\x9a"));
			break;

		case 33:
            switch (comboBox2->currentIndex())
			{
			case 3:
				pLineEdit->insert("^10");
				break;

			default:
				pLineEdit->insert("^2");
				break;
			}
			break;

		case 36:
			pLineEdit->insert("e");
			break;

		case 39:
			writeResult(res, pLineEdit);
			break;

		case 40:
			if (pLineEdit->alignment() != Qt::AlignRight && pLineEdit->text().length() > 0)
			{
				slotEnter();
			}

            break;
		}
	}
}

void MainUI::group6clicked(int i)
{
    QLineEdit * pLineEdit = currentLineEdit();

	if (pLineEdit->alignment() == Qt::AlignRight)
	{
		setReadOnly(false);

		switch (i)
		{
		case 23: case 33:
		case 9: case 19: case 29: case 39:
		case 10: case 20: case 40:
			writeResult(res, pLineEdit);
			break;
		}
	}

	// space ai
	int c = pLineEdit->cursorPosition();
	
	QString const sFix[][4] = 
	{
		{
			c == 0 || pLineEdit->text()[c - 1] == ' ' ? "" : " ", 
			c != pLineEdit->text().length() && pLineEdit->text()[c] == ' ' ? "" : " ", 
			"()", 
			"(, )"
		}, 
		{"", "", "", ""}
	};

	QString const * pFix = option.rpn ? sFix[1] : sFix[0];

    if (QAbstractButton * p = buttonGroup6->button(i))
	{
		switch (i)
		{
		case 1:
			pLineEdit->insert(trUtf8("\xc2\xb0\x20\x27"));
			if (! option.rpn) 
				pLineEdit->cursorBackward(false, 1);
			break;

		case 9: case 10: case 19: case 20: case 29: case 39: case 40:
			pLineEdit->insert(pFix[0] + p->text() + pFix[1]);
			break;

		case 30:
			pLineEdit->insert(p->text() + pFix[1]);
			break;

		case 2:
			if (option.brackets)
			{
				pLineEdit->insert("[]");
				pLineEdit->cursorBackward(false);
			}
			else
			{
				pLineEdit->insert("[");
			}
			break;

        case 12:
            if (option.brackets)
            {
                pLineEdit->insert("{}");
                pLineEdit->cursorBackward(false);
            }
            else
            {
                pLineEdit->insert("{");
            }
            break;

        case 3:
			{
				unsigned short n[] = {0x00D7, 0};
                pLineEdit->insert(pFix[0] + QString::fromUtf16(n) + pFix[1]);
			}
			break;

		case 4:
			{
				unsigned short n[] = {0x2219, 0};
                pLineEdit->insert(pFix[0] + QString::fromUtf16(n) + pFix[1]);
			}
			break;

		case 5:
			pLineEdit->insert(QString("size") + pFix[2]);
			pLineEdit->cursorBackward(false);
			break;

		case 6: case 7: case 8:
			pLineEdit->insert(p->text() + pFix[2]);
			if (! option.rpn) 
				pLineEdit->cursorBackward(false);
			break;

		case 11:
			pLineEdit->insert(trUtf8("\xc2\xb0\x20\x27\x20\x22"));
			if (! option.rpn) 
				pLineEdit->cursorBackward(false, 3);
			break;

		case 14:
			pLineEdit->insert(trUtf8("\xcf\x80"));
			break;

        case 22:
            pLineEdit->insert(p->text() + pFix[3]);
            if (! option.rpn)
                pLineEdit->cursorBackward(false, 3);
            break;
/*
		case 22:
			pLineEdit->insert(p->text() + pFix[2]);
			break;
*/
		case 32:
            pLineEdit->insert(p->text() + pFix[2]);
            break;

		case 13:
			pLineEdit->insert("||");
			pLineEdit->cursorBackward(false);
			break;

		case 23:
			pLineEdit->insert("^(1/)");
			pLineEdit->cursorBackward(false);
			break;

		case 24:
			//pLineEdit->insert(trUtf8("\xd0\xb5\x5e"));
			pLineEdit->insert(QString("exp") + pFix[2]);
			if (! option.rpn) 
				pLineEdit->cursorBackward(false);
			break;

		case 33:
			pLineEdit->insert("^");
			break;

		case 34:
            switch (comboBox2->currentIndex())
			{
			case 3:
				pLineEdit->insert("1010^");
				break;

			default:
				pLineEdit->insert("10^");
				break;
			}
			break;

		case 21:
			pLineEdit->insert(":");
			break;

		case 31:
			pLineEdit->insert("::");
			pLineEdit->cursorBackward(false);
			break;
		}
	}
}


void MainUI::group7clicked(int i)
{
    QLineEdit * pLineEdit = currentLineEdit();

	if (pLineEdit->alignment() == Qt::AlignRight)
	{
		setReadOnly(false);
	}

    if (QAbstractButton * p = buttonGroup7->button(i))
	{
		switch (i)
		{
		case 30:
			pLineEdit->insert(" ");
			break;

		default:
			pLineEdit->insert(p->text());
			break;
		}
	}
}


void MainUI::group8clicked(int i)
{
    QLineEdit * pLineEdit = currentLineEdit();

    if (buttonGroup8->button(i))
	{
		switch (i)
		{
		case 1:
			mem = floating(0.0);
			textLabel1_1->clear();
			textLabel7_1->clear();
			pushButton1_1->setEnabled(false);
			pushButton1_2->setEnabled(false);
			pushButton4_1->setEnabled(false);
			pushButton4_2->setEnabled(false);
			return;

		case 13:
			mem = mem - res;
			textLabel1_1->setText("M");
			textLabel7_1->setText("M");
			pushButton1_1->setEnabled(true);
			pushButton1_2->setEnabled(true);
			pushButton4_1->setEnabled(true);
			pushButton4_2->setEnabled(true);
			return;

		case 19:
			mem = mem + res;
			textLabel1_1->setText("M");
			textLabel7_1->setText("M");
			pushButton1_1->setEnabled(true);
			pushButton1_2->setEnabled(true);
			pushButton4_1->setEnabled(true);
			pushButton4_2->setEnabled(true);
			return;
		}
	}

	QString sBrackets = option.rpn ? "" : "()";
	QString sPreSpace = option.rpn ? " " : "";
	QString sPostSpace = option.rpn ? "" : " ";

	if (pLineEdit->alignment() == Qt::AlignRight)
	{
		setReadOnly(false);

		switch (i)
		{
		case 5: case 11: case 17: case 23:
		case 18:
			writeResult(res, pLineEdit);
			break;
		}
	}

    if (QAbstractButton * p = buttonGroup8->button(i))
	{
		switch (i)
		{
//		case 18: stat = true; break;
		case 24: stat = false; break;
		}

		switch (i)
		{
		case 2: case 3: case 4:
		case 8: case 9: case 10:
		case 14: case 15: case 16:
		case 20: case 22:
			pLineEdit->insert(p->text());
			break;

		case 5: case 11:
			pLineEdit->insert(QString(" ") + p->text() + " ");
			break;

		case 17: case 23:
            if (pLineEdit->text().mid(pLineEdit->cursorPosition() - 2, 2).indexOf(QRegExp("([0-9]|\\.)e")) != -1)
			{
				pLineEdit->insert(p->text());
			}
			else
			{
                int i = pLineEdit->text().lastIndexOf(QRegExp("\\S"), pLineEdit->cursorPosition() - 1);
                int j = pLineEdit->text().lastIndexOf(QRegExp("[0-9a-zA-Z.!\\)\\]\\}\\x0435\\x03C0]"), pLineEdit->cursorPosition() - 1);

				if (i >= 0 && i == j)
					pLineEdit->insert(QString(" ") + p->text() + " ");
				else
					pLineEdit->insert(p->text());
			}

			break;

		case 6:
			pLineEdit->insert("()");
			pLineEdit->cursorBackward(false);
			break;

		case 12:
			pLineEdit->insert(trUtf8("\xe2\x88\x9a"));
			break;

		case 18:
			pLineEdit->insert("^-1");
			break;

		case 7:
			writeResult(mem, pLineEdit);
			break;

		case 21:
			pLineEdit->insert("e");
			break;

		case 24:
			if (pLineEdit->alignment() != Qt::AlignRight && pLineEdit->text().length() > 0)
			{
				slotEnter();
			}

			return;
		}
	}
}


void MainUI::group6_1clicked(int i)
{
    QLineEdit * pLineEdit = currentLineEdit();

	if (pLineEdit->alignment() == Qt::AlignRight)
	{
		setReadOnly(false);
	}

    if (QAbstractButton * p = buttonGroup6_1->button(i))
	{
		switch (i)
		{
		case 1:
			pLineEdit->insert("permutation(, )");
			pLineEdit->cursorBackward(false, 3);
			break;

		case 2:
			pLineEdit->insert("combination(, )");
			pLineEdit->cursorBackward(false, 3);
			break;

		case 3:
			pLineEdit->insert("polar()");
			pLineEdit->cursorBackward(false);
			break;

		case 4:
			pLineEdit->insert("rectangular()");
			pLineEdit->cursorBackward(false);
			break;

		case 5:
			pLineEdit->insert("mean()");
			pLineEdit->cursorBackward(false);
			break;

		case 6:
			pLineEdit->insert("median()");
			pLineEdit->cursorBackward(false);
			break;

		case 7:
			pLineEdit->insert(trUtf8("\xe2\x88\x9a"));
			pLineEdit->insert("variancemle()");
			pLineEdit->cursorBackward(false);
			break;

		case 8:
			pLineEdit->insert(trUtf8("\xe2\x88\x9a"));
			pLineEdit->insert("variance()");
			pLineEdit->cursorBackward(false);
			break;

		case 9:
			pLineEdit->insert("sum()");
			pLineEdit->cursorBackward(false);
			break;

		case 10:
			pLineEdit->insert("sum(^2)");
			pLineEdit->cursorBackward(false, 3);
			break;

		case 11:
			pLineEdit->insert("length()");
			pLineEdit->cursorBackward(false);
			break;

		case 12:
			pLineEdit->insert("product()");
			pLineEdit->cursorBackward(false);
			break;

		default:
			pLineEdit->insert(p->text());
			break;
		}
	}
}


void MainUI::group6_2clicked(int i)
{
    QLineEdit * pLineEdit = currentLineEdit();

	if (pLineEdit->alignment() == Qt::AlignRight)
	{
		setReadOnly(false);
	}

    if (QAbstractButton * p = buttonGroup6_2->button(i))
	{
		switch (i)
		{
		case 1:
			pLineEdit->insert("mean(coly())");
			pLineEdit->cursorBackward(false, 2);
			break;

		case 2:
			pLineEdit->insert("median(coly())");
			pLineEdit->cursorBackward(false, 2);
			break;

		case 3:
			pLineEdit->insert(trUtf8("\xe2\x88\x9a"));
			pLineEdit->insert("variancemle(coly())");
			pLineEdit->cursorBackward(false, 2);
			break;

		case 4:
			pLineEdit->insert(trUtf8("\xe2\x88\x9a"));
			pLineEdit->insert("variance(coly())");
			pLineEdit->cursorBackward(false, 2);
			break;

		case 5:
			pLineEdit->insert("mean(colx())");
			pLineEdit->cursorBackward(false, 2);
			break;

		case 6:
			pLineEdit->insert("median(colx())");
			pLineEdit->cursorBackward(false, 2);
			break;

		case 7:
			pLineEdit->insert(trUtf8("\xe2\x88\x9a"));
			pLineEdit->insert("variancemle(colx())");
			pLineEdit->cursorBackward(false, 2);
			break;

		case 8:
			pLineEdit->insert(trUtf8("\xe2\x88\x9a"));
			pLineEdit->insert("variance(colx())");
			pLineEdit->cursorBackward(false, 2);
			break;

		case 9:
			pLineEdit->insert("sumxy()");
			pLineEdit->cursorBackward(false);
			break;

		case 10:
			pLineEdit->insert(p->text() + "()");
			pLineEdit->cursorBackward(false);
			break;

		case 11:
			pLineEdit->insert(p->text() + "()");
			pLineEdit->cursorBackward(false);
			break;

		case 12:
			pLineEdit->insert(p->text() + "()");
			pLineEdit->cursorBackward(false);
			break;

		default:
			pLineEdit->insert(p->text());
			break;
		}
	}
}


void MainUI::group10clicked(int i)
{
    if (toolsTrace_ModeAction->isChecked())
    {
        point<2> t(0, 0);

        switch (i)
        {
        case 0:
        case 1:
            t.x = -1;
            break;

        case 2:
        case 3:
            t.x = 1;
            break;

        case 4:
            t.y = 1;
            break;

        case 5:
            t.y = -1;
            break;
        }

        switch (i)
        {
        case 0:
        case 3:
            t.x *= graphUI1_1->precisionx * 10;
            t.y *= graphUI1_1->precisiony * 10;

            break;
        }

        if (graphUI1_1->trace((int) t.x, (int) t.y))
        {
            writeCoordinates();
        }
    }
    else
    {
        QLineEdit * pLineEdit = currentLineEdit();

        switch (i)
        {
        case 0:
            pLineEdit->home(pLineEdit->hasSelectedText());
            break;

        case 1:
            pLineEdit->cursorBackward(pLineEdit->hasSelectedText());
            break;

        case 2:
            pLineEdit->cursorForward(pLineEdit->hasSelectedText());
            break;

        case 3:
            pLineEdit->end(pLineEdit->hasSelectedText());
            break;

        case 4:
            slotPreviousExpression();
            break;

        case 5:
            slotNextExpression();
            break;
        }
    }
}


void MainUI::slotVectorSelected()
{
    int const angle_t = comboBox3->currentIndex();
    normed const result = evaluate(variable[static_cast<QLineEdit *>(table5_1->cellWidget(table5_1->currentRow(), 0))->text()[0].toLower().toLatin1()].get(), angle_t);

	writeVector(result, table5_2);
}


void MainUI::slotVectorChanged()
{
    QString result = table5_1->item(table5_1->currentRow(), 0)->text() + " = ";

    if (table5_2->rowCount() == 0)
	{
		result += "0";
	}
    else for (int i = 0; i < table5_2->rowCount(); ++ i)
	{
		if (i == 0) result += "[";
		else result += ", ";

        result += table5_2->item(i, 0)->text();

        if (table5_2->item(i, 1)->text() != "1") result += " {" + table5_2->item(i, 1)->text() + "}";

        if (i == table5_2->rowCount() - 1) result += "]";
	}

	execute(result);
}


void MainUI::slotMatrixSelected()
{
    int const angle_t = comboBox3->currentIndex();
    normed const result = evaluate(variable[static_cast<QLineEdit *>(table6_1->cellWidget(table6_1->currentRow(), 0))->text()[0].toLower().toLatin1()].get(), angle_t);

	writeMatrix(result, table6_2);
}


void MainUI::slotMatrixChanged()
{
    QString result = table6_1->item(table6_1->currentRow(), 0)->text() + " = ";

    if (table6_2->rowCount() == 0)
	{
		result += "0";
	}
    else for (int i = 0; i < table6_2->rowCount(); ++ i)
	{
		if (i == 0) 
			result += "[";
		else 
			result += ", ";

        if (table6_2->columnCount() == 0)
		{
			result += "0";
		}
        else for (int j = 0; j < table6_2->columnCount(); ++ j)
		{
			if (j == 0) 
				result += "[";
			else 
				result += ", ";

            result += table6_2->item(i, j)->text();

            if (j == table6_2->columnCount() - 1)
				result += "]";
		}

        if (i == table6_2->rowCount() - 1)
			result += "]";
	}

	execute(result);
}


void MainUI::slotVariableChanged()
{
    QString result = static_cast<QLineEdit *>(table4_1->cellWidget(table4_1->currentRow(), 0))->text() + " = " + static_cast<QLineEdit *>(table4_1->cellWidget(table4_1->currentRow(), 1))->text();

	execute(result);

    table4_1->setFocus();
}


void MainUI::slotInsertSortClicked()
{
    if (currentLineEdit()->alignment() == Qt::AlignRight)
	{
		setReadOnly(false);
	}

    currentLineEdit()->insert("sort()");
    currentLineEdit()->cursorBackward(false);
}


void MainUI::slotInsertPartClicked()
{
    if (currentLineEdit()->alignment() == Qt::AlignRight)
	{
		setReadOnly(false);
	}

    currentLineEdit()->insert("part(, )");
    currentLineEdit()->cursorBackward(false, 3);
}


void MainUI::slotInsertMergeClicked()
{
    if (currentLineEdit()->alignment() == Qt::AlignRight)
	{
		setReadOnly(false);
	}

    currentLineEdit()->insert("merge(, )");
    currentLineEdit()->cursorBackward(false, 3);
}


void MainUI::slotInsertColXClicked()
{
    if (currentLineEdit()->alignment() == Qt::AlignRight)
	{
		setReadOnly(false);
	}

    currentLineEdit()->insert("colx()");
    currentLineEdit()->cursorBackward(false);
}


void MainUI::slotInsertColYClicked()
{
    if (currentLineEdit()->alignment() == Qt::AlignRight)
	{
		setReadOnly(false);
	}

    currentLineEdit()->insert("coly()");
    currentLineEdit()->cursorBackward(false);
}


void MainUI::slotInsertTransposeClicked()
{
    if (currentLineEdit()->alignment() == Qt::AlignRight)
	{
		setReadOnly(false);
	}

    currentLineEdit()->insert("transpose()");
    currentLineEdit()->cursorBackward(false);
}


void MainUI::slotInsertResultClicked()
{
    if (currentLineEdit()->alignment() == Qt::AlignRight)
	{
		setReadOnly(false);
	}

    currentLineEdit()->insert("result()");
    currentLineEdit()->cursorBackward(false);
}


void MainUI::slotInsertFunction1stClicked()
{
    if (currentLineEdit()->alignment() == Qt::AlignRight)
	{
		setReadOnly(false);
	}

    currentLineEdit()->insert("function1st()");
    currentLineEdit()->cursorBackward(false);
}


void MainUI::slotInsertFunction2ndClicked()
{
    if (currentLineEdit()->alignment() == Qt::AlignRight)
	{
		setReadOnly(false);
	}

    currentLineEdit()->insert("function2nd(, )");
    currentLineEdit()->cursorBackward(false, 3);
}


void MainUI::button3clicked()
{
    if (currentLineEdit()->alignment() != Qt::AlignRight)
	{
        currentLineEdit()->backspace();
	}
}


void MainUI::button4clicked()
{
	setReadOnly(false);
}


void MainUI::setReadOnly(bool b, bool c)
{
	if (b)
	{
		if (c) 
            currentLineEdit()->clear();
        currentLineEdit()->setReadOnly(true);
        currentLineEdit()->setAlignment(Qt::AlignRight);

		pushButton1_3->setEnabled(true);
		pushButton1_4->setEnabled(true);
		pushButton4_3->setEnabled(true);
		pushButton4_4->setEnabled(true);
	}
	else
	{
		if (c) 
            currentLineEdit()->clear();
        currentLineEdit()->setReadOnly(false);
        currentLineEdit()->setAlignment(Qt::AlignLeft);

		pushButton1_3->setEnabled(false);
		pushButton1_4->setEnabled(false);
		pushButton4_3->setEnabled(false);
		pushButton4_4->setEnabled(false);

        //QApplication::postEvent(currentLineEdit(), new QEvent(QEvent::FocusIn));
        //QApplication::postEvent(this, new QEvent(QEvent::CloseSoftwareInputPanel));
        currentLineEdit()->setFocus();
        QEvent event(QEvent::CloseSoftwareInputPanel);
        QApplication::sendEvent(this, &event);
    }

	slotSelectionChanged();
}


void MainUI::setButtons()
{
	int basen = option.basen;

	{
		bool b = basen == 0;

		pushButton1_33->setEnabled(b);
		pushButton1_34->setEnabled(b);
		pushButton1_35->setEnabled(b);
		pushButton1_37->setEnabled(b);
		pushButton1_38->setEnabled(b);
		pushButton1_39->setEnabled(b);
	}

	{
		bool b = basen != 0 && basen != 2 && basen != 3;

		pushButton2->setEnabled(b);
		if (! b) 
            pushButton2->setDown(false), slotToggleClicked(1);

        boxGroup1->setEnabled(b);
        comboBox3->setEnabled(b);
        boxGroup6_3->setEnabled(b && ! option.rpn);
		widgetStack6_1->setEnabled(b && ! option.rpn);

		pushButton1_5->setEnabled(b);
		pushButton1_6->setEnabled(b);
		pushButton1_7->setEnabled(b);
		pushButton1_8->setEnabled(b);

        if (QAbstractButton * p = buttonGroup5->button(2))
			p->setEnabled(b && ! option.rpn);
        if (QAbstractButton * p = buttonGroup5->button(3))
			p->setEnabled(b && ! option.rpn);
        if (QAbstractButton * p = buttonGroup5->button(4))
			p->setEnabled(b && ! option.rpn);
        if (QAbstractButton * p = buttonGroup5->button(12))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup5->button(14))
			p->setEnabled(b && ! option.rpn);
        if (QAbstractButton * p = buttonGroup5->button(22))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup5->button(24))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup5->button(32))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup5->button(33))
			p->setEnabled(! option.rpn);
        if (QAbstractButton * p = buttonGroup5->button(34))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup5->button(36))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup5->button(37))
			p->setEnabled(b);

        if (QAbstractButton * p = buttonGroup6->button(1))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup6->button(2))
			p->setEnabled(b && ! option.rpn);
        if (QAbstractButton * p = buttonGroup6->button(3))
			p->setEnabled(b && ! option.rpn);
        if (QAbstractButton * p = buttonGroup6->button(4))
			p->setEnabled(b && ! option.rpn);
        if (QAbstractButton * p = buttonGroup6->button(5))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup6->button(6))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup6->button(7))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup6->button(8))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup6->button(11))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup6->button(12))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup6->button(13))
			p->setEnabled(! option.rpn);
        if (QAbstractButton * p = buttonGroup6->button(14))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup6->button(15))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup6->button(16))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup6->button(17))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup6->button(18))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup6->button(21))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup6->button(22))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup6->button(23))
			p->setEnabled(! option.rpn);
        if (QAbstractButton * p = buttonGroup6->button(24))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup6->button(25))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup6->button(26))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup6->button(27))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup6->button(28))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup6->button(31))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup6->button(32))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup6->button(34))
			p->setEnabled(b && ! option.rpn);
        if (QAbstractButton * p = buttonGroup6->button(35))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup6->button(36))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup6->button(37))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup6->button(38))
			p->setEnabled(b);

        toolsInsertsortAction->setEnabled(b);
        toolsInsertpartAction->setEnabled(b);
        toolsInsertmergeAction->setEnabled(b);
        toolsInsertcolxAction->setEnabled(b);
        toolsInsertcolyAction->setEnabled(b);
        toolsInserttransposeAction->setEnabled(b);
        toolsInsertresultAction->setEnabled(b);
        toolsInsertifnullAction->setEnabled(b);
        toolsInsertifnotnullAction->setEnabled(b);
        toolsInsertfunction1stAction->setEnabled(b);
        toolsInsertfunction2ndAction->setEnabled(b);
	}

	{
		bool b = basen != 2 && basen != 3;

        if (QAbstractButton * p = buttonGroup5->button(7))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup5->button(6))
			p->setEnabled(b);
	}

	{
		bool b = basen != 3;

        if (QAbstractButton * p = buttonGroup5->button(5))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup5->button(17))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup5->button(16))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup5->button(15))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup5->button(27))
			p->setEnabled(b);
        if (QAbstractButton * p = buttonGroup5->button(26))
			p->setEnabled(b);
	}
/*
	switch (a_i)
	{
    case 0:	currentLineEdit()->setValidator(new QRegExpValidator(QRegExp("[^\\.]*", false), this)); break;
    case 2:	currentLineEdit()->setValidator(new QRegExpValidator(QRegExp("[^8-9\\.]*", false), this)); break;
    case 3:	currentLineEdit()->setValidator(new QRegExpValidator(QRegExp("[^2-9\\.]*", false), this)); break;
    default:currentLineEdit()->setValidator(0); break;
	}
*/
}


void MainUI::slotBaseNClicked(int a_i)
{
	option.basen = a_i;

	if (a_i == 0)
        viewHexadecimalAction->setEnabled(true);
	else if (a_i == 1)
        viewDecimalAction->setEnabled(true);
	else if (a_i == 2)
        viewOctalAction->setEnabled(true);
	else if (a_i == 3)
        viewBinaryAction->setEnabled(true);

	setButtons();

	// refresh expression
    if (toolsTrace_ModeAction->isChecked())
	{
		writeCoordinates();
	}
    else if (currentLineEdit()->alignment() == Qt::AlignRight)
	{
        writeResult(res, currentLineEdit(), option.precision);
	}
}


void MainUI::slotTrigoClicked(int a_i)
{
	option.trigo = a_i;

	if (a_i == 0)
        viewDegreesAction->setEnabled(true);
	else if (a_i == 1)
        viewRadiansAction->setEnabled(true);
	else if (a_i == 2)
        viewGradsAction->setEnabled(true);
}


void MainUI::slotToggleClicked(int a_i)
{
	if (a_i == 0)
	{
        pushButton2->setChecked(false);
	}

    if (pushButton2->isChecked() || pushButton1->isChecked() && pushButton2->isChecked())
	{
        widgetStack1->setCurrentIndex(2);
	}
    else if (pushButton1->isChecked())
	{
        widgetStack1->setCurrentIndex(1);
	}
	else
	{
        widgetStack1->setCurrentIndex(0);
	}
}


void MainUI::slotKeypadClicked(int a_i)
{
    if (! pushButton1->isChecked() || ! pushButton2->isChecked())
	{
        widgetStack1->setCurrentIndex(0);

        pushButton1->setChecked(false);
        pushButton2->setChecked(false);
	}
}


void MainUI::slotNew()
{
	if
	(
		! saved
	)
	switch
	(
		QMessageBox::warning
		(
			this,
			"Warning",
            QString::fromStdString(string("Save changes to ") + (filename.size() ? filename.substr(filename.find_last_of('/') + 1) : "default.fcv") + "?"),
			QMessageBox::Yes | QMessageBox::Default,
			QMessageBox::No,
			QMessageBox::Cancel | QMessageBox::Escape
		)
	)
	{
	case QMessageBox::Yes:
		slotSave();

		if (! saved) 
			return;
		break;

	case QMessageBox::Cancel:
		return;
	};

	for (int i = 'a'; i <= (int) ('z'); ++ i)
	{
		expression[i] = expression_t();
		(mutable_ptr<type> &) (variable[i]) = variable_t();
	}

	saved = true;
	filename = "default.fcv";
    setWindowTitle(CAPTION);
}


void MainUI::slotRefresh(bool z)
{
	// update graphing mode if necessary
	if (graph) switch (graph)
	{
	case 'v':
		graphUI1_1->option.stat = GraphUI::eChart;
		graphUI1_1->option.mode = GraphUI::e2d;
		break;

	case 'w':
		graphUI1_1->option.stat = GraphUI::eScatter;
		graphUI1_1->option.mode = GraphUI::e2d;
		break;

	case 'y':
		graphUI1_1->option.stat = GraphUI::eScientific;
		graphUI1_1->option.mode = GraphUI::e2d;
		break;

	case 'z':
		graphUI1_1->option.stat = GraphUI::eScientific;
		graphUI1_1->option.mode = GraphUI::e3d;
		break;
	}

	// reset
	graph = 0;

	// refresh graphics
	switch (graphUI1_1->option.stat)
	{
	case GraphUI::eChart:
		slotBarChart(z);
		break;

	case GraphUI::eScatter:
		slotScatterPlot(z);
		break;

	case GraphUI::eScientific:
		switch (graphUI1_1->option.mode)
		{
		case GraphUI::e2d:
			slotPlotY(z);
			break;

		case GraphUI::e3d:
			slotPlotZ(z);
			break;

		default:
			break;
		}
		break;
	}
}


void MainUI::slotImportSingleVariables()
{
#if 0
	if (!saved)
	switch
	(
		QMessageBox::warning
		(
			this,
			"Warning",
            QString::fromStdString(string("Save changes to ") + (filename.size() ? filename.substr(filename.find_last_of('/') + 1) : "default.fcv") + "?"),
			QMessageBox::Yes | QMessageBox::Default,
			QMessageBox::No,
			QMessageBox::Cancel | QMessageBox::Escape
		)
	)
	{
	case QMessageBox::Yes:
		slotSave();

		if (! saved) 
			return;
		break;

	case QMessageBox::Cancel:
		return;
	};

    QString input = QFileDialog::getOpenFileName(this, "Open", "", "Microsoft Excel Files (*.xls; *.xla; *.xlt; *.xlm; *.xlc; *.xlm);;Text Files (*.txt; *.csv)");

	if (! input.isEmpty())
	{
		QString filename;
        QString ext = input.mid(input.lastIndexOf('.') + 1).toLower();

		if (ext == "txt" || ext == "csv")
		{
			filename = input;
		}
		else if (ext == "xls" || ext == "xla" || ext == "xlt" || ext == "xlm" || ext == "xlc" || ext == "xlm")
		{
			filename = QString(tempnam(0, "fcalc-cache")) + ".csv";

            if (! convert_xls_csv(input.toUtf8().constData(), filename.toUtf8().constData()))
			{
				setReadOnly(true, false);

				QMessageBox::critical
				(
					this,
					"Error",
					"Error reading file.\n\n",
					"Ok"
				);

				return;
			}
		}

		if (! filename.isEmpty())
		{
			QFile file(filename);

			if (file.open(QIODevice::ReadOnly))
			{
				QString line;
				QString matrix;
                QTextStream stream(& file);
				int basen = option.basen;

				slotBaseNClicked(1);

				matrix = "V = [";

				// load lines from the temporary file
				for (bool first = true; ! stream.atEnd(); first = false)
				{
					if (!first)
					{
						matrix += ", ";
					}

					line = stream.readLine();

					matrix += line.section(',', 0, 0) + (line.section(',', 1, 1).isEmpty() ? "" : " {" + line.section(',', 1, 1) + "}");
				}

				matrix += "]";

				slotBaseNClicked(basen);

				if (! execute(matrix))
				{
					setReadOnly(true, false);

					QMessageBox::critical
					(
						this,
						"Error",
						"Error processing file.\n\n",
						"Ok"
					);

					return;
				}

				setReadOnly(false);

				file.close();

				if (graph)
				{
					slotRefresh();
				}
			}
		}
	}
#endif
}


void MainUI::slotImportPairedVariables()
{
#if 0
	if (!saved)
	switch
	(
		QMessageBox::warning
		(
			this,
			"Warning",
            QString::fromStdString(string("Save changes to ") + (filename.size() ? filename.substr(filename.find_last_of('/') + 1) : "default.fcv") + "?"),
			QMessageBox::Yes | QMessageBox::Default,
			QMessageBox::No,
			QMessageBox::Cancel | QMessageBox::Escape
		)
	)
	{
	case QMessageBox::Yes:
		slotSave();

		if (! saved) 
			return;
		break;

	case QMessageBox::Cancel:
		return;
	};

    QString input = QFileDialog::getOpenFileName(this, "Open" ,"", "Microsoft Excel Files (*.xls; *.xla; *.xlt; *.xlm; *.xlc; *.xlm);;Text Files (*.txt; *.csv)");

	if (! input.isEmpty())
	{
		QString filename;
        QString ext = input.mid(input.lastIndexOf('.') + 1).toLower();

		if (ext == "txt" || ext == "csv")
		{
			filename = input;
		}
		else if (ext == "xls" || ext == "xla" || ext == "xlt" || ext == "xlm" || ext == "xlc" || ext == "xlm")
		{
			filename = QString(tempnam(0, "fcalc-cache")) + ".csv";

            if (! convert_xls_csv(input.toUtf8().constData(), filename.toUtf8().constData()))
			{
				setReadOnly(true, false);

				QMessageBox::critical
				(
					this,
					"Error",
					"Error reading file.\n\n",
					"Ok"
				);

				return;
			}
		}

		if (! filename.isEmpty())
		{
			QFile file(filename);

			if (file.open(QIODevice::ReadOnly))
			{
				QString line;
				QString matrix;
                QTextStream stream(& file);
				int basen = option.basen;

				slotBaseNClicked(1);

				matrix = "W = [";

				// load lines from the temporary file
				for (bool first = true; ! stream.atEnd(); first = false)
				{
					if (!first)
					{
						matrix += ", ";
					}

					line = stream.readLine();

					matrix += "[" + line + "]";
				}

				matrix += "]";

				slotBaseNClicked(basen);

				if (! execute(matrix))
				{
					setReadOnly(true, false);

					QMessageBox::critical
					(
						this,
						"Error",
						"Error processing file.\n\n",
						"Ok"
					);

					return;
				}

				setReadOnly(false);

				file.close();

				if (graph)
				{
					slotRefresh();
				}
			}
		}
	}
#endif
}


void MainUI::slotOpen(bool b)
{
	if
	(
		b
		&&
		! saved
	)
	switch
	(
		QMessageBox::warning
		(
			this,
			"Warning",
            QString::fromStdString(string("Save changes to ") + (filename.size() ? filename.substr(filename.find_last_of('/') + 1) : "default.fcv") + "?"),
			QMessageBox::Yes | QMessageBox::Default,
			QMessageBox::No,
			QMessageBox::Cancel | QMessageBox::Escape
		)
	)
	{
	case QMessageBox::Yes:
		slotSave();

		if (! saved) 
			return;
		break;

	case QMessageBox::Cancel:
		return;
	};

	QString input = QString::fromStdString(filename);

	if (b)
	{
        input = QFileDialog::getOpenFileName(this, "Open", "", "Fornux Calculator (*.fcv);;All files (*.*)");
	}
#ifdef _WIN32
	else
	{
		char buffer[BUFSIZE];

		if (GetLongPathNameA(input.ascii(), buffer, BUFSIZE) > 0)
		{
			input = buffer;
		}
	}
#endif

	if (! input.isEmpty())
	{
		QFile file(input);

        if (file.open(QIODevice::ReadOnly))
		{
			QString line;
            QTextStream stream(& file);
			bool rpn = option.rpn;
			int basen = option.basen;

			slotRPN(false);
			slotBaseNClicked(1);
            ///stream.setEncoding(QTextStream::Unicode);

			// clear variables first
			for (int i = 'a'; i <= (int) ('z'); ++ i)
			{
				expression[i] = expression_t();
				(mutable_ptr<type> &) (variable[i]) = variable_t();
			}

			// load variables from the file
			while (! stream.atEnd())
			{
				line = stream.readLine();

				if (! execute(line))
				{
					setReadOnly(true, false);

					QMessageBox::critical
					(
						this,
						"Error",
						"Error reading file.\n\n",
						"Ok"
					);

					slotRPN(rpn);
					slotBaseNClicked(basen);
					return;
				}
			}

			setReadOnly(false);
			slotRPN(rpn);
			slotBaseNClicked(basen);

			file.close();
			graph = 0;
			saved = true;
            filename = input.toUtf8().constData();

            setWindowTitle(QString(CAPTION) + " - [" + QString::fromStdString(filename.substr(filename.find_last_of('\\') + 1)) + "]");

			slotRefresh();
		}
		else
		{
			QMessageBox::critical
			(
				this,
				"Error",
				"Unexpected error opening file.\n\n",
				"Ok"
			);
		}
	}
}


void MainUI::slotSave()
{
	if (filename != "default.fcv")
	{
        QFile file(QString::fromStdString(filename));

		if (file.open(QIODevice::WriteOnly))
		{
            QTextStream stream(& file);

            ///stream.setEncoding(QTextStream::Unicode);

			for (int i = 'a'; i <= (int) ('z'); ++ i)
			{
                stream << QString(QChar(char(i))).toUpper() << " = " << expression[i] << endl;
			}

			file.close();
			saved = true;
		}
		else
		{
			QMessageBox::critical
			(
				this,
				"Error",
				"Unexpected error saving file.\n\n",
				"Ok"
			);
		}
	}
	else
	{
		slotSaveAs();
	}
}


void MainUI::slotSaveAs(bool b)
{
	QString input = QString::fromStdString(filename);

	if (b)
	{
        input = QFileDialog::getSaveFileName(this, "Save As", "", "Fornux Calculator (*.fcv);;All files (*.*)");
	}

	if (! input.isEmpty())
	{
		if (input.right(4) != ".fcv")
		{
			input += ".fcv";
		}

        filename = input.toUtf8().constData();

		slotSave();

        setWindowTitle(QString(CAPTION) + " - [" + QString::fromStdString(filename.substr(filename.find_last_of('/') + 1)) + "]");
	}
}

/*
struct pagesize_t { QPrinter::PageSize e; char const * p; double x, y; } const pagesize[] =
{
	{QPrinter::A4, "A4", 210, 297},
	{QPrinter::B5, "B5", 182, 257},
	{QPrinter::Letter, "Letter", 216, 279},
	{QPrinter::Legal, "Legal", 216, 356},
	{QPrinter::Executive, "Executive", 191, 254},
	{QPrinter::A0, "A0", 841, 1189},
	{QPrinter::A1, "A1", 594, 841},
	{QPrinter::A2, "A2", 420, 594},
	{QPrinter::A3, "A3", 297, 420},
	{QPrinter::A5, "A5", 148, 210},
	{QPrinter::A6, "A6", 105, 148},
	{QPrinter::A7, "A7", 74, 105},
	{QPrinter::A8, "A8", 52, 74},
	{QPrinter::A9, "A9", 37, 52},
	{QPrinter::B0, "B0", 1030, 1456},
	{QPrinter::B1, "B1", 728, 1030},
	{QPrinter::B10, "B10", 32, 45},
	{QPrinter::B2, "B2", 515, 728},
	{QPrinter::B3, "B3", 364, 515},
	{QPrinter::B4, "B4", 257, 364},
	{QPrinter::B6, "B6", 128, 182},
	{QPrinter::B7, "B7", 91, 128},
	{QPrinter::B8, "B8", 64, 91},
	{QPrinter::B9, "B9", 45, 64},
	{QPrinter::C5E, "C5E", 163, 229},
	{QPrinter::Comm10E, "Comm10E", 105, 241},
	{QPrinter::DLE, "DLE", 110, 220},
	{QPrinter::Folio, "Folio", 210, 330},
	{QPrinter::Ledger, "Ledger", 432, 279},
	{QPrinter::Tabloid, "Tabloid", 279, 432}
};


struct papersource_t { QPrinter::PaperSource e; char const * p; } const papersource[] =
{
	{QPrinter::OnlyOne, "OnlyOne"},
	{QPrinter::Lower, "Lower"},
	{QPrinter::Middle, "Middle"},
	{QPrinter::Manual, "Manual"},
	{QPrinter::Envelope, "Envelope"},
	{QPrinter::EnvelopeManual, "EnvelopeManual"},
	{QPrinter::Auto, "Auto Select"},
	{QPrinter::Tractor, "Tractor"},
	{QPrinter::SmallFormat, "SmallFormat"},
	{QPrinter::LargeFormat, "LargeFormat"},
	{QPrinter::LargeCapacity, "LargeCapacity"},
	{QPrinter::Cassette, "Cassette"},
	{QPrinter::FormSource, "FormSource"}
};
*/

bool MainUI::slotSetup(bool printer)
{
    pSetupUI = new SetupUI(this);

	pSetupUI->checkBox1_1->setChecked(graphUI1_1->option.header);
	pSetupUI->checkBox1_2->setChecked(graphUI1_1->option.legend);

	pSetupUI->lineEdit1_2->setText(graphUI1_1->option.title);
	pSetupUI->lineEdit1_4->setText(graphUI1_1->option.x.unit);
	pSetupUI->lineEdit1_5->setText(graphUI1_1->option.y.unit);
	pSetupUI->lineEdit1_6->setText(graphUI1_1->option.z.unit);

	pSetupUI->lineEdit1_3->setText(graphUI1_1->option.title_font.toString());
	pSetupUI->lineEdit1_7->setText(graphUI1_1->option.unit_font.toString());
	pSetupUI->lineEdit1_1->setText(graphUI1_1->option.scale_font.toString());

	if (printer)
	{
#if 0
        QPaintDevice & metrics = *pPrinter;

		int dpix = metrics.logicalDpiX();
		int dpiy = metrics.logicalDpiY();

		QRegExpValidator * pValidator = new QRegExpValidator(QRegExp("(\\.[0-9]+)|([0-9]+(\\.[0-9]*)?)"), pSetupUI);

		pSetupUI->lineEdit3_1->setValidator(pValidator);
		pSetupUI->lineEdit3_2->setValidator(pValidator);
		pSetupUI->lineEdit3_3->setValidator(pValidator);
		pSetupUI->lineEdit3_4->setValidator(pValidator);
		pSetupUI->lineEdit3_5->setValidator(pValidator);
		pSetupUI->lineEdit3_6->setValidator(pValidator);

        pSetupUI->tabWidget1->removeTab(1);

		pSetupUI->lineEdit3_1->setText(QString::number(graphUI1_1->option.report[1].width / dpix));
		pSetupUI->lineEdit3_2->setText(QString::number(graphUI1_1->option.report[1].height / dpiy));

/*
		unsigned int top, left, bottom, right;

		pPrinter->margins(& top, & left, & bottom, & right);

		pSetupUI->lineEdit3_3->setText(QString::number(float(left) / dpix));
		pSetupUI->lineEdit3_4->setText(QString::number(float(right) / dpix));
		pSetupUI->lineEdit3_5->setText(QString::number(float(top) / dpiy));
		pSetupUI->lineEdit3_6->setText(QString::number(float(bottom) / dpiy));
*/

		pSetupUI->lineEdit3_3->setText(QString::number(graphUI1_1->option.report[1].left / dpix));
		pSetupUI->lineEdit3_4->setText(QString::number(graphUI1_1->option.report[1].right / dpix));
		pSetupUI->lineEdit3_5->setText(QString::number(graphUI1_1->option.report[1].top / dpix));
		pSetupUI->lineEdit3_6->setText(QString::number(graphUI1_1->option.report[1].bottom / dpix));

		for (unsigned int i = 0; i < sizeof(pagesize) / sizeof(pagesize_t); ++ i)
		{
            pSetupUI->comboBox4_1->addItem(pagesize[i].p);
		}

		for (unsigned int i = 0; i < sizeof(papersource) / sizeof(papersource_t); ++ i)
		{
            pSetupUI->comboBox4_2->addItem(papersource[i].p);
		}

        pSetupUI->comboBox4_1->setCurrentIndex(pPrinter->pageSize());
        pSetupUI->comboBox4_2->setCurrentIndex(pPrinter->paperSource());
        pSetupUI->buttonGroup4_1->buttons().at(pPrinter->orientation())->setDown(true);
		pSetupUI->checkBox4_1->setChecked(graphUI1_1->option.report[1].hcenter);
		pSetupUI->checkBox4_2->setChecked(graphUI1_1->option.report[1].vcenter);

		connect(pSetupUI->lineEdit3_1, SIGNAL(textChanged(const QString &)), SLOT(slotSetupRepaint()));
		connect(pSetupUI->lineEdit3_2, SIGNAL(textChanged(const QString &)), SLOT(slotSetupRepaint()));
		connect(pSetupUI->lineEdit3_3, SIGNAL(textChanged(const QString &)), SLOT(slotSetupRepaint()));
		connect(pSetupUI->lineEdit3_4, SIGNAL(textChanged(const QString &)), SLOT(slotSetupRepaint()));
		connect(pSetupUI->lineEdit3_5, SIGNAL(textChanged(const QString &)), SLOT(slotSetupRepaint()));
		connect(pSetupUI->lineEdit3_6, SIGNAL(textChanged(const QString &)), SLOT(slotSetupRepaint()));
		connect(pSetupUI->comboBox4_1, SIGNAL(activated(int)), SLOT(slotSetupRepaint()));
		connect(pSetupUI->comboBox4_2, SIGNAL(activated(int)), SLOT(slotSetupRepaint()));
        connect(pSetupUI->buttonGroup4_1, SIGNAL(buttonClicked(int)), SLOT(slotSetupRepaint()));
        connect(pSetupUI->buttonGroup4_2, SIGNAL(buttonClicked(int)), SLOT(slotSetupRepaint()));

		slotSetupRepaint();
#endif
    }
	else
	{
        pSetupUI->tabWidget1->removeTab(3);
        pSetupUI->tabWidget1->removeTab(2);

		pSetupUI->spinBox2_1->setValue((int) graphUI1_1->option.report[0].width);
		pSetupUI->spinBox2_2->setValue((int) graphUI1_1->option.report[0].height);
	}

	if (graphUI1_1->option.mode == GraphUI::e2d)
	{
		pSetupUI->lineEdit1_6->hide();
		pSetupUI->textLabel1_6->hide();
	}

	if (! graphUI1_1->option.x.numbers && ! graphUI1_1->option.y.numbers && ! graphUI1_1->option.z.numbers)
	{
		pSetupUI->groupBox1_1->setEnabled(false);
	}

	bool res = false;

	if (pSetupUI->exec() == QDialog::Accepted)
	{
		graphUI1_1->option.header = pSetupUI->checkBox1_1->isChecked();
		graphUI1_1->option.legend = pSetupUI->checkBox1_2->isChecked();

		graphUI1_1->option.title = pSetupUI->lineEdit1_2->text();
		graphUI1_1->option.x.unit = pSetupUI->lineEdit1_4->text();
		graphUI1_1->option.y.unit = pSetupUI->lineEdit1_5->text();
		graphUI1_1->option.z.unit = pSetupUI->lineEdit1_6->text();

		graphUI1_1->option.title_font.fromString(pSetupUI->lineEdit1_3->text());
		graphUI1_1->option.unit_font.fromString(pSetupUI->lineEdit1_7->text());
		graphUI1_1->option.scale_font.fromString(pSetupUI->lineEdit1_1->text());

		if (printer)
		{
#if 0
			pPrinter->setFullPage(true);
            pPrinter->setPageSize((QPrinter::PageSize) (pSetupUI->comboBox4_1->currentIndex()));
            pPrinter->setPaperSource((QPrinter::PaperSource) (pSetupUI->comboBox4_2->currentIndex()));
            pPrinter->setOrientation((QPrinter::Orientation) (pSetupUI->buttonGroup4_1->checkedId()));

            QPaintDevice & metrics = *pPrinter;

			int dpix = metrics.logicalDpiX();
			int dpiy = metrics.logicalDpiY();

			graphUI1_1->option.report[1].width = pSetupUI->lineEdit3_1->text().toDouble() * dpix;
			graphUI1_1->option.report[1].height = pSetupUI->lineEdit3_2->text().toDouble() * dpiy;

			graphUI1_1->option.report[1].left = pSetupUI->lineEdit3_3->text().toDouble() * dpix;
			graphUI1_1->option.report[1].right = pSetupUI->lineEdit3_4->text().toDouble() * dpix;
			graphUI1_1->option.report[1].top = pSetupUI->lineEdit3_5->text().toDouble() * dpiy;
			graphUI1_1->option.report[1].bottom = pSetupUI->lineEdit3_6->text().toDouble() * dpiy;

/*
			pPrinter->setMargins((unsigned int) graphUI1_1->option.report[1].top, (unsigned int) graphUI1_1->option.report[1].left, (unsigned int) graphUI1_1->option.report[1].bottom, (unsigned int) graphUI1_1->option.report[1].right);
*/

			graphUI1_1->option.report[1].hcenter = pSetupUI->checkBox4_1->isChecked();
			graphUI1_1->option.report[1].vcenter = pSetupUI->checkBox4_2->isChecked();
#endif
        }
		else
		{
			graphUI1_1->option.report[0].width = pSetupUI->spinBox2_1->value();
			graphUI1_1->option.report[0].height = pSetupUI->spinBox2_2->value();
		}

		res = true;
	}

	delete pSetupUI;

	return res;
}


void MainUI::slotSetupRepaint()
{
	static QColor const black = QColor(0, 0, 0);
	static QColor const white = QColor(255, 255, 255);

    int item = pSetupUI->comboBox4_1->currentIndex();

	QPixmap pixmap(pSetupUI->pixmapLabel4_1->size());
	QPainter painter(& pixmap);

#if 0
	// layout
    if (pSetupUI->buttonGroup4_1->checkedId() == 0)
	{
		dim[20] = pagesize[item].x;
		dim[21] = pagesize[item].y;
	}
	else
	{
		dim[20] = pagesize[item].y;
		dim[21] = pagesize[item].x;
	}
#endif
    
	// pixmap size
	dim[0] = pixmap.width() - 8;
	dim[1] = pixmap.height() - 8;

	// page size
	float ratio = dim[21] / dim[20];

	if (ratio > 1)
	{
		dim[2] = dim[1] / ratio;
		dim[3] = dim[1];
	}
	else
	{
		dim[2] = dim[1];
		dim[3] = dim[1] * ratio;
	}

	// offset
	dim[4] = (dim[0] - dim[2]) / 2;
	dim[5] = (dim[1] - dim[3]) / 2;

	// margin
	dim[6] = pSetupUI->lineEdit3_3->text().toFloat() * 25.4 / dim[20] * dim[2];
	dim[7] = pSetupUI->lineEdit3_4->text().toFloat() * 25.4 / dim[20] * dim[2];
	dim[8] = pSetupUI->lineEdit3_5->text().toFloat() * 25.4 / dim[21] * dim[3];
	dim[9] = pSetupUI->lineEdit3_6->text().toFloat() * 25.4 / dim[21] * dim[3];

	// graphic size
	dim[10] = pSetupUI->lineEdit3_1->text().toFloat() * 25.4 / dim[20] * dim[2];
	dim[11] = pSetupUI->lineEdit3_2->text().toFloat() * 25.4 / dim[21] * dim[3];

	// top left of graphic center bounding rectangle
	dim[12] = pSetupUI->checkBox4_1->isChecked() ? dim[4] + dim[6] + (dim[2] - dim[6] - dim[7] - dim[10]) / 2 : dim[4] + dim[6];
	dim[13] = pSetupUI->checkBox4_2->isChecked() ? dim[5] + dim[8] + (dim[3] - dim[8] - dim[9] - dim[11]) / 2 : dim[5] + dim[8];

	// draw
    painter.fillRect(0, 0, (int) dim[0] + 8, (int) dim[1] + 8, pSetupUI->pixmapLabel4_1->palette().color(QWidget::backgroundRole()));

	painter.fillRect((int) dim[4] + 8, (int) dim[5] + 8, (int) dim[2], (int) dim[3], QBrush(black, Qt::Dense4Pattern));
	painter.fillRect((int) dim[4], (int) dim[5], (int) dim[2], (int) dim[3], QBrush(white, Qt::SolidPattern));

	painter.setPen(Qt::black);
	painter.drawRect((int) dim[4], (int) dim[5], (int) dim[2], (int) dim[3]);

	painter.setPen(QPen(Qt::black, 0, Qt::DotLine));
	painter.drawRect((int) dim[4] + (int) dim[6], (int) dim[5] + (int) dim[8], (int) dim[2] - (int) dim[6] - (int) dim[7], (int) dim[3] - (int) dim[8] - (int) dim[9]);
	painter.setClipRegion(QRegion((int) dim[4] + (int) dim[6], (int) dim[5] + (int) dim[8], (int) dim[2] - (int) dim[6] - (int) dim[7], (int) dim[3] - (int) dim[8] - (int) dim[9]));

	painter.fillRect((int) dim[12], (int) dim[13], (int) dim[10], (int) dim[11], QBrush(black, Qt::Dense4Pattern));
	painter.setPen(Qt::black);
	painter.drawRect((int) dim[12], (int) dim[13], (int) dim[10], (int) dim[11]);

	painter.end();

	pSetupUI->pixmapLabel4_1->setPixmap(pixmap);
}


#if 0
void MainUI::slotPrint()
{
    QPrintDialog dialog(pPrinter, this);

    if (dialog.exec())
	{
        QPixmap pixmap;

		floating::fast = true;
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

		switch (graphUI1_1->option.stat)
		{
		case GraphUI::eChart:
		case GraphUI::eScatter:
			graphUI1_1->draw_report(pixmap, 1);
			break;

		case GraphUI::eScientific:
			switch (graphUI1_1->option.mode)
			{
			case GraphUI::e2d:
				stack[2].push_back(variable_t()), (mutable_ptr<type> &) stack[2].back() = (mutable_ptr<type> &) (variable[(int) ('x')]);
				(mutable_ptr<type> &) (variable[(int) ('x')]) = new type_t<floating>(0);
				graphUI1_1->draw_report(pixmap, 1);
				(mutable_ptr<type> &) (variable[(int) ('x')]) = (mutable_ptr<type> &) stack[2].back(), stack[2].pop_back();
				break;

			case GraphUI::e3d:
				stack[2].push_back(variable_t()), (mutable_ptr<type> &) stack[2].back() = (mutable_ptr<type> &) (variable[(int) ('x')]);
				stack[3].push_back(variable_t()), (mutable_ptr<type> &) stack[3].back() = (mutable_ptr<type> &) (variable[(int) ('y')]);
				(mutable_ptr<type> &) (variable[(int) ('x')]) = new type_t<floating>(0);
				(mutable_ptr<type> &) (variable[(int) ('y')]) = new type_t<floating>(0);
				graphUI1_1->draw_report(pixmap, 1);
				(mutable_ptr<type> &) (variable[(int) ('x')]) = (mutable_ptr<type> &) stack[2].back(), stack[2].pop_back();
				(mutable_ptr<type> &) (variable[(int) ('y')]) = (mutable_ptr<type> &) stack[3].back(), stack[3].pop_back();
				break;

			default:
				break;
			}
			break;
		}

		floating::fast = false;
		QApplication::restoreOverrideCursor();

		// print out
		QPainter p(pPrinter);
        QPaintDevice & metrics = *pPrinter;

/*
		double x, y, w, h;
		int dpix = metrics.logicalDpiX();
		int dpiy = metrics.logicalDpiY();

		// Layout
		if (pPrinter->orientation() == QPrinter::Portrait)
		{
			w = pagesize[pPrinter->pageSize()].x / 25.4 * dpix;
			h = pagesize[pPrinter->pageSize()].y / 25.4 * dpiy;
		}
		else
		{
			w = pagesize[pPrinter->pageSize()].y / 25.4 * dpiy;
			h = pagesize[pPrinter->pageSize()].x / 25.4 * dpix;
		}
*/

		double x, y;

		if (! graphUI1_1->option.report[1].hcenter)
			x = 0;
		else
			x = (metrics.width() - graphUI1_1->option.report[1].left - graphUI1_1->option.report[1].right - pixmap.width()) / 2;

		if (! graphUI1_1->option.report[1].vcenter)
			y = 0;
		else
			y = (metrics.height() - graphUI1_1->option.report[1].top - graphUI1_1->option.report[1].bottom - pixmap.height()) / 2;

		p.drawPixmap(int(x + graphUI1_1->option.report[1].left), int(y + graphUI1_1->option.report[1].top), pixmap);
		///p.flush();
	}
}
#endif


#if 0
void MainUI::slotPaint()
{
	if (slotSetup(false))
	{
		QPixmap pixmap;

		floating::fast = true;
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

		switch (graphUI1_1->option.stat)
		{
		case GraphUI::eChart:
		case GraphUI::eScatter:
			graphUI1_1->draw_report(pixmap, 0);
			break;

		case GraphUI::eScientific:
			switch (graphUI1_1->option.mode)
			{
			case GraphUI::e2d:
				stack[2].push_back(variable_t()), (mutable_ptr<type> &) stack[2].back() = (mutable_ptr<type> &) (variable[(int) ('x')]);
				(mutable_ptr<type> &) (variable[(int) ('x')]) = new type_t<floating>(0);
				graphUI1_1->draw_report(pixmap, 0);
				(mutable_ptr<type> &) (variable[(int) ('x')]) = (mutable_ptr<type> &) stack[2].back(), stack[2].pop_back();
				break;

			case GraphUI::e3d:
				graphUI1_1->draw_report(pixmap, 0);
				break;

			default:
				break;
			}
			break;
		}

		floating::fast = false;
		QApplication::restoreOverrideCursor();

		QString name = QString(tempnam(0, "fcalc-graphic")) + ".bmp";

		if (pixmap.save(name, "BMP"))
		{
			Q3Process * process = new Q3Process(this);

			process->addArgument(QString::fromStdString(option.utility));
			process->addArgument(name);

			if (process->start())
			{
				return;
			}

			unlink(name.ascii());
		}

		QMessageBox::critical
		(
			this,
			"Error",
            QString::fromStdString(string("Unexpected error starting '").append(option.utility).append(" ").append(name.toUtf8().constData()).append("'.\n\n")),
			"Ok"
		);
	}
}
#endif


void MainUI::slotExit()
{
	if
	(
		! saved
	)
	switch
	(
		QMessageBox::warning
		(
			this,
			"Warning",
            QString::fromStdString(string("Save changes to ") + (filename.size() ? filename.substr(filename.find_last_of('/') + 1) : "default.fcv") + "?"),
			QMessageBox::Yes | QMessageBox::Default,
			QMessageBox::No,
			QMessageBox::Cancel | QMessageBox::Escape
		)
	)
	{
	case QMessageBox::Yes:
		slotSave();

		if (! saved) 
			return;
		break;

	case QMessageBox::Cancel:
		return;
	};

	qApp->quit();
}


void MainUI::slotCut()
{
    if (toolsTrace_ModeAction->isChecked())
	{
	}
	else
	{
        currentLineEdit()->cut();
	}
}


void MainUI::slotCopy()
{
    if (toolsTrace_ModeAction->isChecked())
	{
		if (lineEdit1_2->hasSelectedText()) 
			lineEdit1_2->copy();
		else if (lineEdit1_3->hasSelectedText()) 
			lineEdit1_3->copy();
		else if (lineEdit1_4->hasSelectedText()) 
			lineEdit1_4->copy();
	}
	else
	{
        currentLineEdit()->copy();
	}
}


void MainUI::slotPaste()
{
    currentLineEdit()->paste();
}


void MainUI::slotClearText()
{
    currentLineEdit()->clear();
}


void MainUI::slotSelectAll()
{
    currentLineEdit()->selectAll();
}


void MainUI::slotSelectionChanged()
{
    if (toolsTrace_ModeAction->isChecked())
	{
        editCutAction->setEnabled(false);
        editCopyAction->setEnabled(lineEdit1_2->hasSelectedText() || lineEdit1_3->hasSelectedText() || lineEdit1_4->hasSelectedText());
        editPasteAction->setEnabled(false);

        editClearAction->setEnabled(false);
        editSelect_AllAction->setEnabled(false);
	}
	else
	{
        editCutAction->setEnabled(! currentLineEdit()->isReadOnly() && currentLineEdit()->hasSelectedText());
        editCopyAction->setEnabled(currentLineEdit()->hasSelectedText());
        editPasteAction->setEnabled(! currentLineEdit()->isReadOnly() && ! QApplication::clipboard()->text(QClipboard::Clipboard).isEmpty());

        editClearAction->setEnabled(! currentLineEdit()->isReadOnly() && ! currentLineEdit()->text().isEmpty());
        editSelect_AllAction->setEnabled(! currentLineEdit()->text().isEmpty());
	}
}


void MainUI::slotEnter()
{
	if
	(
		statement[pTabBar->currentIndex()].l.size() == 1
		||
        (-- -- statement[pTabBar->currentIndex()].l.end())->first != currentLineEdit()->text()
		||
        (-- -- statement[pTabBar->currentIndex()].l.end())->second != comboBox2->currentIndex()
		||
        (-- -- statement[pTabBar->currentIndex()].l.end())->third != comboBox3->currentIndex()
	)
	{
        statement[pTabBar->currentIndex()].l.back() = fdi::tuple<QString, int, int>(currentLineEdit()->text(), comboBox2->currentIndex(), comboBox3->currentIndex());
        statement[pTabBar->currentIndex()].l.push_back(fdi::tuple<QString, int, int>(QString(""), comboBox2->currentIndex(), comboBox3->currentIndex()));
	}

	statement[pTabBar->currentIndex()].i = -- statement[pTabBar->currentIndex()].l.end();

    execute(currentLineEdit()->text());

	switch (pTabBar->currentIndex())
	{
	case 0:
	case 5:
		setReadOnly(true, false);
		break;
	}

    //viewPrevious_ExpressionAction->setEnabled(& * statement[pTabBar->currentIndex()].i != & * statement[pTabBar->currentIndex()].l.begin());
    //viewNext_ExpressionAction->setEnabled(& * statement[pTabBar->currentIndex()].i != & * -- statement[pTabBar->currentIndex()].l.end());
    viewFirst_ExpressionAction->setEnabled(& * statement[pTabBar->currentIndex()].i != & * statement[pTabBar->currentIndex()].l.begin());
    viewLast_ExpressionAction->setEnabled(false);

	if (graph)
	{
		slotRefresh();
	}
}


void MainUI::slotPreviousExpression()
{
    if (currentLineEdit()->alignment() == Qt::AlignRight)
	{
		setReadOnly(false);
	}

	if (statement[pTabBar->currentIndex()].i != statement[pTabBar->currentIndex()].l.begin())
	{
		-- statement[pTabBar->currentIndex()].i;
	}
	else
	{
		return;
	}

    currentLineEdit()->setText(statement[pTabBar->currentIndex()].i->first);
	slotBaseNClicked(statement[pTabBar->currentIndex()].i->second);
    comboBox2->setCurrentIndex(statement[pTabBar->currentIndex()].i->second);
    comboBox3->setCurrentIndex(statement[pTabBar->currentIndex()].i->third);

    //viewPrevious_ExpressionAction->setEnabled(& * statement[pTabBar->currentIndex()].i != & * statement[pTabBar->currentIndex()].l.begin());
    //viewNext_ExpressionAction->setEnabled(& * statement[pTabBar->currentIndex()].i != & * -- statement[pTabBar->currentIndex()].l.end());
    viewFirst_ExpressionAction->setEnabled(& * statement[pTabBar->currentIndex()].i != & * statement[pTabBar->currentIndex()].l.begin());
    viewLast_ExpressionAction->setEnabled(& * statement[pTabBar->currentIndex()].i != & * -- statement[pTabBar->currentIndex()].l.end());
}


void MainUI::slotNextExpression()
{
    if (currentLineEdit()->alignment() == Qt::AlignRight)
	{
		setReadOnly(false);
	}

	if (statement[pTabBar->currentIndex()].i != -- statement[pTabBar->currentIndex()].l.end())
	{
		++ statement[pTabBar->currentIndex()].i;
	}
	else
	{
		return;
	}

    currentLineEdit()->setText(statement[pTabBar->currentIndex()].i->first);
	slotBaseNClicked(statement[pTabBar->currentIndex()].i->second);
    comboBox2->setCurrentIndex(statement[pTabBar->currentIndex()].i->second);
    comboBox3->setCurrentIndex(statement[pTabBar->currentIndex()].i->third);

    //viewPrevious_ExpressionAction->setEnabled(& * statement[pTabBar->currentIndex()].i != & * statement[pTabBar->currentIndex()].l.begin());
    //viewNext_ExpressionAction->setEnabled(& * statement[pTabBar->currentIndex()].i != & * -- statement[pTabBar->currentIndex()].l.end());
    viewFirst_ExpressionAction->setEnabled(& * statement[pTabBar->currentIndex()].i != & * statement[pTabBar->currentIndex()].l.begin());
    viewLast_ExpressionAction->setEnabled(& * statement[pTabBar->currentIndex()].i != & * -- statement[pTabBar->currentIndex()].l.end());
}


void MainUI::slotFirstExpression()
{
    if (currentLineEdit()->alignment() == Qt::AlignRight)
	{
		setReadOnly(false);
	}

	statement[pTabBar->currentIndex()].i = statement[pTabBar->currentIndex()].l.begin();

    currentLineEdit()->setText(statement[pTabBar->currentIndex()].i->first);
	slotBaseNClicked(statement[pTabBar->currentIndex()].i->second);
    comboBox2->setCurrentIndex(statement[pTabBar->currentIndex()].i->second);
    comboBox3->setCurrentIndex(statement[pTabBar->currentIndex()].i->third);

    //viewPrevious_ExpressionAction->setEnabled(& * statement[pTabBar->currentIndex()].i != & * statement[pTabBar->currentIndex()].l.begin());
    //viewNext_ExpressionAction->setEnabled(& * statement[pTabBar->currentIndex()].i != & * -- statement[pTabBar->currentIndex()].l.end());
    viewFirst_ExpressionAction->setEnabled(& * statement[pTabBar->currentIndex()].i != & * statement[pTabBar->currentIndex()].l.begin());
    viewLast_ExpressionAction->setEnabled(& * statement[pTabBar->currentIndex()].i != & * -- statement[pTabBar->currentIndex()].l.end());
}


void MainUI::slotLastExpression()
{
    if (currentLineEdit()->alignment() == Qt::AlignRight)
	{
		setReadOnly(false);
	}

	statement[pTabBar->currentIndex()].i = -- statement[pTabBar->currentIndex()].l.end();

    currentLineEdit()->setText(statement[pTabBar->currentIndex()].i->first);
	slotBaseNClicked(statement[pTabBar->currentIndex()].i->second);
    comboBox2->setCurrentIndex(statement[pTabBar->currentIndex()].i->second);
    comboBox3->setCurrentIndex(statement[pTabBar->currentIndex()].i->third);

    //viewPrevious_ExpressionAction->setEnabled(& * statement[pTabBar->currentIndex()].i != & * statement[pTabBar->currentIndex()].l.begin());
    //viewNext_ExpressionAction->setEnabled(& * statement[pTabBar->currentIndex()].i != & * -- statement[pTabBar->currentIndex()].l.end());
    viewFirst_ExpressionAction->setEnabled(& * statement[pTabBar->currentIndex()].i != & * statement[pTabBar->currentIndex()].l.begin());
    viewLast_ExpressionAction->setEnabled(& * statement[pTabBar->currentIndex()].i != & * -- statement[pTabBar->currentIndex()].l.end());
}


void MainUI::slotMixed()
{
    option.mixed = viewMixed_FractionAction->isEnabled();

	// refresh expression
    if (toolsTrace_ModeAction->isChecked())
	{
		writeCoordinates();
	}
    else if (currentLineEdit()->alignment() == Qt::AlignRight)
	{
        writeResult(res, currentLineEdit(), option.precision);
	}
}


void MainUI::slotSexagesimal()
{
    option.sexagesimal = viewSexagesimal_NotationAction->isEnabled();

	// refresh expression
    if (toolsTrace_ModeAction->isChecked())
	{
		writeCoordinates();
	}
    else if (currentLineEdit()->alignment() == Qt::AlignRight)
	{
        writeResult(res, currentLineEdit(), option.precision);
	}
}


void MainUI::slotStatusBar()
{
    option.status = viewStatus_BarAction->isEnabled();

    if (viewStatus_BarAction->isEnabled())
	{
		statusBar()->addWidget(new QLabel("For Help, press F1.", statusBar()), 100);
	}
	else
	{
		delete statusBar();
	}

    if (viewScientific_CalculatorAction->isEnabled())
	{
		setMaximumSize(nMaximumSize);
		resize(option.width, option.height);
	}
	else
	{
		setUpdatesEnabled(false);
		option.width = width();
		option.height = height();
		setFixedSize(0, 0);
		adjustSize();
		setUpdatesEnabled(true);
	}
}


void MainUI::slotScientific(bool b)
{
    option.scientific = viewScientific_CalculatorAction->isEnabled();

    if (viewScientific_CalculatorAction->isEnabled())
	{
        boxGroup5->show();
        boxGroup6->show();
        boxGroup7->show();
        boxGroup8->hide();

        if (pushButton2->isDown() || pushButton1->isDown() && pushButton2->isDown())
		{
            widgetStack1->setCurrentIndex(2);
		}
        else if (pushButton1->isDown())
		{
            widgetStack1->setCurrentIndex(1);
		}
		else
		{
            widgetStack1->setCurrentIndex(0);
		}
	}
	else
	{
        boxGroup5->hide();
        boxGroup6->hide();
        boxGroup7->hide();
        boxGroup8->show();

        widgetStack1->setCurrentIndex(3);
	}

    if (viewScientific_CalculatorAction->isEnabled())
	{
		graphUI1_1->show();
		frame1->show();
        boxGroup4->show();
	}
	else
	{
		graphUI1_1->hide();
		frame1->hide();
        boxGroup4->hide();
	}

    if (viewScientific_CalculatorAction->isEnabled())
	{
		bool graph = graphUI1_1->option.mode != GraphUI::eNone || graphUI1_1->option.stat != GraphUI::eScientific;

        filePage_SetupAction->setEnabled(graph);
        filePrintAction->setEnabled(graph);
        fileSend_ToMicrosoft_PaintAction->setEnabled(graph);

        filePage_SetupAction->setEnabled(true);
        filePrintAction->setEnabled(true);
        fileSend_ToMicrosoft_PaintAction->setEnabled(true);

		viewBaseNActionGroup->setEnabled(true);
		viewTrigoActionGroup->setEnabled(true);
        settingsRangeAction->setEnabled(true);
        settingsGraphicAction->setEnabled(true);
        toolsPlotYAction->setEnabled(true);
        toolsPlotZAction->setEnabled(true);
        toolsBar_ChartAction->setEnabled(true);
        toolsScatter_PlotAction->setEnabled(true);
        toolsClearAction->setEnabled(graph);
        toolsZoom25Action->setEnabled(graph);
        toolsZoom50Action->setEnabled(graph);
        toolsZoom100Action->setEnabled(graph);
        toolsZoom200Action->setEnabled(graph);
        toolsZoom400Action->setEnabled(graph);
        toolsZoom_InAction->setEnabled(graph);
        toolsZoom_OutAction->setEnabled(graph);
        toolsTrace_ModeAction->setEnabled(graph);
        toolsInsertsortAction->setEnabled(true);
        toolsInsertpartAction->setEnabled(true);
        toolsInsertmergeAction->setEnabled(true);
        toolsInsertcolxAction->setEnabled(true);
        toolsInsertcolyAction->setEnabled(true);
        toolsInserttransposeAction->setEnabled(true);
        toolsInsertresultAction->setEnabled(true);
	}
	else
	{
        filePage_SetupAction->setEnabled(false);
        filePrintAction->setEnabled(false);
        filePrint_PreviewAction->setEnabled(false);
        fileSend_ToMicrosoft_PaintAction->setEnabled(false);
		viewBaseNActionGroup->setEnabled(false);
		viewTrigoActionGroup->setEnabled(false);
        settingsRangeAction->setEnabled(false);
        settingsGraphicAction->setEnabled(false);
        toolsPlotYAction->setEnabled(false);
        toolsPlotZAction->setEnabled(false);
        toolsBar_ChartAction->setEnabled(false);
        toolsScatter_PlotAction->setEnabled(false);
        toolsClearAction->setEnabled(false);
        toolsZoom25Action->setEnabled(false);
        toolsZoom50Action->setEnabled(false);
        toolsZoom100Action->setEnabled(false);
        toolsZoom200Action->setEnabled(false);
        toolsZoom400Action->setEnabled(false);
        toolsZoom_InAction->setEnabled(false);
        toolsZoom_OutAction->setEnabled(false);
        toolsTrace_ModeAction->setEnabled(false);
        toolsInsertsortAction->setEnabled(false);
        toolsInsertpartAction->setEnabled(false);
        toolsInsertmergeAction->setEnabled(false);
        toolsInsertcolxAction->setEnabled(false);
        toolsInsertcolyAction->setEnabled(false);
        toolsInserttransposeAction->setEnabled(false);
        toolsInsertresultAction->setEnabled(false);
	}

	if (b)
	{
        if (viewScientific_CalculatorAction->isEnabled())
		{
			setMaximumSize(nMaximumSize);
			resize(option.width, option.height);
		}
		else
		{
			setUpdatesEnabled(false);
			option.width = width();
			option.height = height();
			setFixedSize(0, 0);
			adjustSize();
			setUpdatesEnabled(true);
		}
	}
}


void MainUI::slotBaseN(QAction * a)
{
	int id = 1;

	if (a == viewHexadecimalAction)
		id = 0;
	else if (a == viewDecimalAction)
		id = 1;
	else if (a == viewOctalAction)
		id = 2;
	else if (a == viewBinaryAction)
		id = 3;

	option.basen = id;

	setButtons();
    comboBox2->setCurrentIndex(id);

	// refresh expression
    if (toolsTrace_ModeAction->isChecked())
	{
		writeCoordinates();
	}
    else if (currentLineEdit()->alignment() == Qt::AlignRight)
	{
        writeResult(res, currentLineEdit(), option.precision);
	}
}


void MainUI::slotTrigo(QAction * a)
{
	int id = 0;

	if (a == viewDegreesAction)
		id = 0;
	else if (a == viewRadiansAction)
		id = 1;
	else if (a == viewGradsAction)
		id = 2;

	option.trigo = id;

    comboBox3->setCurrentIndex(id);
}


void MainUI::slotWidgetStack(int aPage)
{
	switch (aPage)
	{
	case 3:
        table5_1->scrollToItem(table5_1->takeItem(table5_1->currentRow(), 0));
		slotVectorSelected();
		break;

	case 4:
        table6_1->scrollToItem(table6_1->takeItem(table6_1->currentRow(), 0));
		slotMatrixSelected();
		break;

	case 1:
        for (int i = 0; i < table4_1->rowCount(); ++ i)
		{
            table4_1->setCellWidget(i, 1, new QLineEdit(expression[static_cast<QLineEdit *>(table4_1->cellWidget(i, 0))->text()[0].toLower().toLatin1()]));
            table4_1->cellWidget(i, 1)->setFont(lineEdit1_1->font());
            //static_cast<QLineEdit *>(table4_1->cellWidget(i, 1))->setReadOnly(true);
            static_cast<QLineEdit *>(table4_1->cellWidget(i, 1))->installEventFilter(this);
            connect(static_cast<QLineEdit *>(table4_1->cellWidget(i, 1)), SIGNAL(editingFinished()), SLOT(slotVariableChanged()));
        }

		table4_1->setColumnWidth(1, widgetStack10->width() - table4_1->verticalScrollBar()->width() - table4_1->columnWidth(0) - 2);
		break;

	default:
		break;
	}

    currentLineEdit()->setFocus();
}


void MainUI::slotRPN(bool b)
{
    option.rpn = b && toolsReverse_Polish_NotationAction->isChecked();

	setButtons();
}


void MainUI::slotPlotY(bool z)
{
	// backup
	floating::fast = true;
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	stack[2].push_back(variable_t()), (mutable_ptr<type> &) stack[2].back() = (mutable_ptr<type> &) (variable[(int) ('x')]);
	(mutable_ptr<type> &) (variable[(int) ('x')]) = new type_t<floating>(0);

	// rescale
	if (z && graphUI1_1->option.rescale)
	{
		point<2> c;
		point<2> min = (point<2> const &) (graphUI1_1->option.min);
		point<2> max = (point<2> const &) (graphUI1_1->option.max);

		graphUI1_1->fx.min = (point<2> const &) (graphUI1_1->option.min);
		graphUI1_1->fx.max = (point<2> const &) (graphUI1_1->option.max);

		if (! graphUI1_1->init_fx(graphUI1_1->width(), graphUI1_1->height()))
		{
			// restore
			(mutable_ptr<type> &) (variable[(int) ('x')]) = (mutable_ptr<type> &) stack[2].back(), stack[2].pop_back();

			floating::fast = false;
			QApplication::restoreOverrideCursor();

			return;
		}

		for (real x = graphUI1_1->fx.min.x; x <= graphUI1_1->fx.max.x; x += graphUI1_1->fx.stp.x)
		{
			c = point<2>(x, plot(x));

			if (x == graphUI1_1->fx.min.x)
			{
				min.y = c.y;
				max.y = c.y;
			}
			else if (! isnan(c.y))
			{
				min.y = std::min(c.y, min.y);
				max.y = std::max(c.y, max.y);
			}
		}

		if (min.y < max.y)
		{
			graphUI1_1->option.scl.y = (max.y - min.y) / (graphUI1_1->option.max.x - graphUI1_1->option.min.x) * graphUI1_1->option.scl.x;
			graphUI1_1->option.max.y = max.y;
			graphUI1_1->option.min.y = min.y;
		}
	}

	// zoom
	real const xdelta = graphUI1_1->option.max.x - graphUI1_1->option.min.x;
    real const xrange = xdelta - xdelta * pow(2.0L, zoom);
	graphUI1_1->fx.min.x = graphUI1_1->option.min.x + xrange / 2;
	graphUI1_1->fx.max.x = graphUI1_1->option.max.x - xrange / 2;
	graphUI1_1->fx.scl.x = graphUI1_1->option.scl.x;

	real const ydelta = graphUI1_1->option.max.y - graphUI1_1->option.min.y;
    real const yrange = ydelta - ydelta * pow(2.0L, zoom);
	graphUI1_1->fx.min.y = graphUI1_1->option.min.y + yrange / 2;
	graphUI1_1->fx.max.y = graphUI1_1->option.max.y - yrange / 2;
	graphUI1_1->fx.scl.y = graphUI1_1->option.scl.y;

	// plot
	graph = 0;

	// plot multiple graphics
	if (type_t< fdi::vector<type_p> > const * r = dynamic_cast<type_t< fdi::vector<type_p> > const *>(variable[(int) ('y')].get()))
	{
		stack[2].push_back(variable_t()), (mutable_ptr<type> &) stack[2].back() = (mutable_ptr<type> &) (variable[(int) ('y')]);

		for (unsigned int i = 0; i < r->value.size(); ++ i)
		{
			(mutable_ptr<type> &) (variable[(int) ('y')]) = dynamic_cast<type_t< fdi::vector<type_p> > const *>(stack[2].back().get())->value[i];

            graphUI1_1->plot((real (*)(real)) (plot), i == 0);
		}

		(mutable_ptr<type> &) (variable[(int) ('y')]) = (mutable_ptr<type> &) stack[2].back(), stack[2].pop_back();
	}

	// plot single graphic
	else
	{
        graphUI1_1->plot((real (*)(real)) (plot));
	}

	// restore
	(mutable_ptr<type> &) (variable[(int) ('x')]) = (mutable_ptr<type> &) stack[2].back(), stack[2].pop_back();

	floating::fast = false;
	QApplication::restoreOverrideCursor();

	// refresh menu
	slotScientific(false);

    graphUI1_1->update();
}


void MainUI::slotPlotZ(bool z)
{
	floating::fast = true;
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	// zoom
	real const xdelta = graphUI1_1->option.max.x - graphUI1_1->option.min.x;
    real const xrange = xdelta - xdelta * pow(2.0L, zoom);
	graphUI1_1->fxy.min.x = graphUI1_1->option.min.x + xrange / 2;
	graphUI1_1->fxy.max.x = graphUI1_1->option.max.x - xrange / 2;
	graphUI1_1->fxy.scl.x = graphUI1_1->option.scl.x;

	real const ydelta = graphUI1_1->option.max.y - graphUI1_1->option.min.y;
    real const yrange = ydelta - ydelta * pow(2.0L, zoom);
	graphUI1_1->fxy.min.y = graphUI1_1->option.min.y + yrange / 2;
	graphUI1_1->fxy.max.y = graphUI1_1->option.max.y - yrange / 2;
	graphUI1_1->fxy.scl.y = graphUI1_1->option.scl.y;

	real const zdelta = graphUI1_1->option.max.z - graphUI1_1->option.min.z;
    real const zrange = zdelta - zdelta * pow(2.0L, zoom);
	graphUI1_1->fxy.min.z = graphUI1_1->option.min.z + zrange / 2;
	graphUI1_1->fxy.max.z = graphUI1_1->option.max.z - zrange / 2;
	graphUI1_1->fxy.scl.z = graphUI1_1->option.scl.z;

	// plot
    graph = 0;
    graphUI1_1->plot((real (*)(real, real)) (plot));

	floating::fast = false;
	QApplication::restoreOverrideCursor();

	// refresh menu
    slotScientific(false);

    graphUI1_1->update();
}

void MainUI::workAreaResized(int screen)
{
    QRect r(QApplication::desktop()->availableGeometry());
    resize(r.width(), r.height());
}

void MainUI::resizeEvent(QResizeEvent * p)
{
    //setGeometry(QApplication::desktop()->availableGeometry());

    graphUI1_1->setMinimumHeight(height() / 3);
    //comboBox2->setMinimumWidth(width() / 2);
    boxGroup10->setMinimumWidth(contentsRect().width() * 3 / 5);
    boxGroup1->setMaximumHeight(height() / 12);
    comboBox2->setMaximumHeight(height() / 12);
    comboBox3->setMaximumHeight(height() / 12);
    boxGroup4->setMaximumHeight(height() / 12);
    boxGroup9->setMaximumHeight(height() / 12);
    boxGroup10->setMaximumHeight(height() / 12);

    widgetStack6_1->setMaximumWidth(contentsRect().width() * 2 / 5);

    QWidget::resizeEvent(p);
}

void MainUI::slotBarChart(bool z)
{
	// backup
	floating::fast = true;
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	stack[2].push_back(variable_t()), (mutable_ptr<type> &) stack[2].back() = (mutable_ptr<type> &) (variable[(int) ('x')]);
	(mutable_ptr<type> &) (variable[(int) ('x')]) = new type_t<floating>(0);

    int const angle_t = comboBox3->currentIndex();

	normed result = evaluate(variable[(int) ('v')].get(), angle_t);

	if (type_t< fdi::vector<normed> > const * r = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& result)))
	{
		std::vector<real> a(r->value.size());

		if (z && graphUI1_1->option.adjust)
		{
			graphUI1_1->option.min.x = 0.0;
			graphUI1_1->option.max.x = 0.0;

			graphUI1_1->option.min.y = 0.0;
			graphUI1_1->option.max.y = 0.0;
		}

		for (unsigned int i = 0; i < r->value.size(); ++ i)
		{
			a[i] = r->value[i].to_floating();
		}

		// rescale
		if (z && graphUI1_1->option.adjust)
		{
			std::sort(a.begin(), a.end());

			for (std::vector<real>::iterator p = a.begin(), q; p != a.end(); p = q)
			{
				q = std::upper_bound(p, a.end(), * p);

				if (p == a.begin() && q != a.end())
				{
					graphUI1_1->option.min.x = * p - fabsl(* q - * p) / 2;
					graphUI1_1->option.scl.x = fabsl(* q - * p);
				}

				graphUI1_1->option.max.x = std::max(graphUI1_1->option.max.x, * p + graphUI1_1->option.scl.x) - graphUI1_1->option.scl.x / 2;
				graphUI1_1->option.max.y = std::max(graphUI1_1->option.max.y, real(q - p));
			}

            graphUI1_1->option.scl.y = ceil(graphUI1_1->option.max.y / (graphUI1_1->option.max.x - graphUI1_1->option.min.x) * graphUI1_1->option.scl.x);
            graphUI1_1->option.max.y = ceil(graphUI1_1->option.max.y / graphUI1_1->option.scl.y) * graphUI1_1->option.scl.y;
		}

		real const xdelta = graphUI1_1->option.max.x - graphUI1_1->option.min.x;
        real const xrange = xdelta - xdelta * pow(2.0L, zoom);
		graphUI1_1->fx.min.x = graphUI1_1->option.min.x + xrange / 2;
		graphUI1_1->fx.max.x = graphUI1_1->option.max.x - xrange / 2;
		graphUI1_1->fx.scl.x = graphUI1_1->option.scl.x;

		real const ydelta = graphUI1_1->option.max.y - graphUI1_1->option.min.y;
        real const yrange = ydelta - ydelta * pow(2.0L, zoom);
		graphUI1_1->fx.min.y = graphUI1_1->option.min.y + yrange / 2;
		graphUI1_1->fx.max.y = graphUI1_1->option.max.y - yrange / 2;
		graphUI1_1->fx.scl.y = graphUI1_1->option.scl.y;

		// plot
		graph = 0;
        graphUI1_1->plot(a, (real (*)(real)) (plot));
	}

	// restore
	(mutable_ptr<type> &) (variable[(int) ('x')]) = (mutable_ptr<type> &) stack[2].back(), stack[2].pop_back();
	floating::fast = false;
	QApplication::restoreOverrideCursor();

	// refresh menu
	slotScientific(false);

    graphUI1_1->update();
}


void MainUI::slotScatterPlot(bool z)
{
	// backup
	floating::fast = true;
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	stack[2].push_back(variable_t()), (mutable_ptr<type> &) stack[2].back() = (mutable_ptr<type> &) (variable[(int) ('x')]);
	(mutable_ptr<type> &) (variable[(int) ('x')]) = new type_t<floating>(0);

    int const angle_t = comboBox3->currentIndex();

	normed result = evaluate(variable[(int) ('w')].get(), angle_t);

	if (type_t< fdi::vector<normed> > const * r = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& result)))
	{
		std::vector<real> a(r->value.size()), b(r->value.size());

		for (unsigned int i = 0; i < r->value.size(); ++ i)
		{
			if (type_t< fdi::vector<normed> > const * s = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& r->value[i])))
			{
				if (s->value.size() != 2)
				{
					continue;
				}

				a[i] = s->value[0].to_floating();
				b[i] = s->value[1].to_floating();

				if (z && graphUI1_1->option.adjust)
				{
					if (i == 0)
					{
						graphUI1_1->option.min.x = a[i];
						graphUI1_1->option.max.x = a[i];
						graphUI1_1->option.min.y = b[i];
						graphUI1_1->option.max.y = b[i];
					}
					else
					{
						graphUI1_1->option.min.x = std::min(graphUI1_1->option.min.x, a[i]);
						graphUI1_1->option.max.x = std::max(graphUI1_1->option.max.x, a[i]);
						graphUI1_1->option.min.y = std::min(graphUI1_1->option.min.y, b[i]);
						graphUI1_1->option.max.y = std::max(graphUI1_1->option.max.y, b[i]);
					}
				}
			}
		}

		if (z && graphUI1_1->option.adjust)
		{
            real const xorder = std::max(floor(log10(graphUI1_1->option.max.x)), 1.0L);
            real const yorder = std::max(floor(log10(graphUI1_1->option.max.y)), 1.0L);

            graphUI1_1->option.scl.x = ceil((graphUI1_1->option.max.x - graphUI1_1->option.min.x) / pow(10, xorder)) * pow(10, xorder - 1);
            graphUI1_1->option.min.x = floor(graphUI1_1->option.min.x / graphUI1_1->option.scl.x) * graphUI1_1->option.scl.x;
            graphUI1_1->option.max.x = ceil(graphUI1_1->option.max.x / graphUI1_1->option.scl.x) * graphUI1_1->option.scl.x;
            graphUI1_1->option.scl.y = ceil((graphUI1_1->option.max.y - graphUI1_1->option.min.y) / pow(10, yorder)) * pow(10, yorder - 1);
            graphUI1_1->option.min.y = floor(graphUI1_1->option.min.y / graphUI1_1->option.scl.y) * graphUI1_1->option.scl.y;
            graphUI1_1->option.max.y = ceil(graphUI1_1->option.max.y / graphUI1_1->option.scl.y) * graphUI1_1->option.scl.y;
		}

		real const xdelta = graphUI1_1->option.max.x - graphUI1_1->option.min.x;
        real const xrange = xdelta - xdelta * pow(2.0L, zoom);
		graphUI1_1->fx.min.x = graphUI1_1->option.min.x + xrange / 2;
		graphUI1_1->fx.max.x = graphUI1_1->option.max.x - xrange / 2;
		graphUI1_1->fx.scl.x = graphUI1_1->option.scl.x;

		real const ydelta = graphUI1_1->option.max.y - graphUI1_1->option.min.y;
        real const yrange = ydelta - ydelta * pow(2.0L, zoom);
		graphUI1_1->fx.min.y = graphUI1_1->option.min.y + yrange / 2;
		graphUI1_1->fx.max.y = graphUI1_1->option.max.y - yrange / 2;
		graphUI1_1->fx.scl.y = graphUI1_1->option.scl.y;

		// plot
		graph = 0;
        graphUI1_1->plot(a, b, (real (*)(real)) (plot));
	}

	// restore
	(mutable_ptr<type> &) (variable[(int) ('x')]) = (mutable_ptr<type> &) stack[2].back(), stack[2].pop_back();
	floating::fast = false;
	QApplication::restoreOverrideCursor();

	// refresh menu
	slotScientific(false);

    graphUI1_1->update();
}


void MainUI::slotClear()
{
	graphUI1_1->plot();

	// refresh menu
	slotScientific(false);

    graphUI1_1->update();
}


void MainUI::slotZoom(QAction * a)
{
	if (a == toolsZoom25Action)
		zoom = 2;
	else if (a == toolsZoom50Action)
		zoom = 1;
	else if (a == toolsZoom100Action)
		zoom = 0;
	else if (a == toolsZoom200Action)
		zoom = -1;
	else if (a == toolsZoom400Action)
		zoom = -2;

	// refresh graphic
	QApplication::postEvent(graphUI1_1, new QResizeEvent(graphUI1_1->size(), graphUI1_1->size()));
}


void MainUI::slotZoomIn()
{
	if (zoom == -2) return;

	zoom = zoom - 1;

	switch (zoom)
	{
    case 2: toolsZoom25Action->setEnabled(true); break;
    case 1: toolsZoom50Action->setEnabled(true); break;
    case 0: toolsZoom100Action->setEnabled(true); break;
    case -1: toolsZoom200Action->setEnabled(true); break;
    case -2: toolsZoom400Action->setEnabled(true); break;
	}

	// refresh graphic
	QApplication::postEvent(graphUI1_1, new QResizeEvent(graphUI1_1->size(), graphUI1_1->size()));
}


void MainUI::slotZoomOut()
{
	if (zoom == 2) return;

	zoom = zoom + 1;

	switch (zoom)
	{
    case 2: toolsZoom25Action->setEnabled(true); break;
    case 1: toolsZoom50Action->setEnabled(true); break;
    case 0: toolsZoom100Action->setEnabled(true); break;
    case -1: toolsZoom200Action->setEnabled(true); break;
    case -2: toolsZoom400Action->setEnabled(true); break;
	}

	// refresh graphic
	QApplication::postEvent(graphUI1_1, new QResizeEvent(graphUI1_1->size(), graphUI1_1->size()));
}


void MainUI::slotTrace()
{
    if (toolsTrace_ModeAction->isChecked())
	{
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

		switch (graphUI1_1->option.mode)
		{
		case GraphUI::e2d:
			// backup
			stack[2].push_back(variable_t()), (mutable_ptr<type> &) stack[2].back() = (mutable_ptr<type> &) (variable[(int) ('x')]);

			(mutable_ptr<type> &) (variable[(int) ('x')]) = new type_t<floating>(0);

			if (graphUI1_1->trace(true))
			{
				frame1->setEnabled(false);
                boxGroup4->setEnabled(false);
                boxGroup9->setEnabled(false);
				widgetStack1->setEnabled(false);

				writeCoordinates();
                widgetStack1_1->setCurrentIndex(1);
			}
			else
			{
                toolsTrace_ModeAction->setEnabled(false);
			}

			break;

		case GraphUI::e3d:
			// backup
            ///stack[2].push_back(variable_t()), (mutable_ptr<type> &) stack[2].back() = (mutable_ptr<type> &) (variable[(int) ('x')]);
            ///stack[3].push_back(variable_t()), (mutable_ptr<type> &) stack[3].back() = (mutable_ptr<type> &) (variable[(int) ('y')]);

            ///(mutable_ptr<type> &) (variable[(int) ('x')]) = new type_t<floating>(0);
            ///(mutable_ptr<type> &) (variable[(int) ('y')]) = new type_t<floating>(0);

            if (false) ///graphUI1_1->trace(true))
			{
				frame1->setEnabled(false);
                boxGroup4->setEnabled(false);
                boxGroup9->setEnabled(false);
				widgetStack1->setEnabled(false);

				writeCoordinates();
                widgetStack1_1->setCurrentIndex(1);
			}
			else
			{
                toolsTrace_ModeAction->setChecked(false);
			}

			break;

		default:
			break;
		}

		QApplication::restoreOverrideCursor();
	}
	else
	{
		switch (graphUI1_1->option.mode)
		{
		case GraphUI::e2d:
			if (! graphUI1_1->trace(false))
			{
				QMessageBox message
				(
					"Error",
					"<br>Error #23 terminating tracing mode.</br>",
					QMessageBox::Warning,
					QMessageBox::Ok | QMessageBox::Default,
					Qt::NoButton,
					Qt::NoButton,
					this
				);

				message.exec();
			}

			// restore
			(mutable_ptr<type> &) (variable[(int) ('x')]) = (mutable_ptr<type> &) stack[2].back(), stack[2].pop_back();

			break;

		case GraphUI::e3d:
			if (! graphUI1_1->trace(false))
			{
				QMessageBox message
				(
					"Error",
					"<br>Error #23 terminating tracing mode.</br>",
					QMessageBox::Warning,
					QMessageBox::Ok | QMessageBox::Default,
					Qt::NoButton,
					Qt::NoButton,
					this
				);

				message.exec();
			}

			// restore
			(mutable_ptr<type> &) (variable[(int) ('x')]) = (mutable_ptr<type> &) stack[2].back(), stack[2].pop_back();
			(mutable_ptr<type> &) (variable[(int) ('y')]) = (mutable_ptr<type> &) stack[3].back(), stack[3].pop_back();

			break;

		default:
			break;
		}

		frame1->setEnabled(true);
        boxGroup4->setEnabled(true);
        boxGroup9->setEnabled(true);
		widgetStack1->setEnabled(true);

        widgetStack1_1->setCurrentIndex(0);
	}

	slotSelectionChanged();
}


void MainUI::slotRange()
{
	pRangeUI = new RangeUI(this);

    pRangeUI->spinBoxUI1->setRange(- numeric_limits<double>::max(), numeric_limits<double>::max());
    pRangeUI->spinBoxUI2->setRange(- numeric_limits<double>::max(), numeric_limits<double>::max());
    pRangeUI->spinBoxUI3->setRange(- numeric_limits<double>::max(), numeric_limits<double>::max());
    pRangeUI->spinBoxUI4->setRange(- numeric_limits<double>::max(), numeric_limits<double>::max());
    pRangeUI->spinBoxUI5->setRange(- numeric_limits<double>::max(), numeric_limits<double>::max());
    pRangeUI->spinBoxUI6->setRange(- numeric_limits<double>::max(), numeric_limits<double>::max());
    pRangeUI->spinBoxUI7->setRange(- numeric_limits<double>::max(), numeric_limits<double>::max());
    pRangeUI->spinBoxUI8->setRange(- numeric_limits<double>::max(), numeric_limits<double>::max());
    pRangeUI->spinBoxUI9->setRange(- numeric_limits<double>::max(), numeric_limits<double>::max());

    connect(pRangeUI->spinBoxUI1, SIGNAL(valueChanged(double)), SLOT(slotRangeChanged()));
    connect(pRangeUI->spinBoxUI2, SIGNAL(valueChanged(double)), SLOT(slotRangeChanged()));
    connect(pRangeUI->spinBoxUI3, SIGNAL(valueChanged(double)), SLOT(slotRangeChanged()));
    connect(pRangeUI->spinBoxUI4, SIGNAL(valueChanged(double)), SLOT(slotRangeChanged()));
    connect(pRangeUI->spinBoxUI5, SIGNAL(valueChanged(double)), SLOT(slotRangeChanged()));
    connect(pRangeUI->spinBoxUI6, SIGNAL(valueChanged(double)), SLOT(slotRangeChanged()));
    connect(pRangeUI->spinBoxUI7, SIGNAL(valueChanged(double)), SLOT(slotRangeChanged()));
    connect(pRangeUI->spinBoxUI8, SIGNAL(valueChanged(double)), SLOT(slotRangeChanged()));
    connect(pRangeUI->spinBoxUI9, SIGNAL(valueChanged(double)), SLOT(slotRangeChanged()));

	pRangeUI->spinBoxUI1->setValue(graphUI1_1->option.min.x);
	pRangeUI->spinBoxUI4->setValue(graphUI1_1->option.max.x);
	pRangeUI->spinBoxUI7->setValue(graphUI1_1->option.scl.x);
	pRangeUI->spinBoxUI2->setValue(graphUI1_1->option.min.y);
	pRangeUI->spinBoxUI5->setValue(graphUI1_1->option.max.y);
	pRangeUI->spinBoxUI8->setValue(graphUI1_1->option.scl.y);
	pRangeUI->spinBoxUI3->setValue(graphUI1_1->option.min.z);
	pRangeUI->spinBoxUI6->setValue(graphUI1_1->option.max.z);
	pRangeUI->spinBoxUI9->setValue(graphUI1_1->option.scl.z);

	if (pRangeUI->exec() == QDialog::Accepted)
	{
		graphUI1_1->option.min.x = pRangeUI->spinBoxUI1->value();
		graphUI1_1->option.max.x = pRangeUI->spinBoxUI4->value();
		graphUI1_1->option.scl.x = pRangeUI->spinBoxUI7->value();
		graphUI1_1->option.min.y = pRangeUI->spinBoxUI2->value();
		graphUI1_1->option.max.y = pRangeUI->spinBoxUI5->value();
		graphUI1_1->option.scl.y = pRangeUI->spinBoxUI8->value();
		graphUI1_1->option.min.z = pRangeUI->spinBoxUI3->value();
		graphUI1_1->option.max.z = pRangeUI->spinBoxUI6->value();
		graphUI1_1->option.scl.z = pRangeUI->spinBoxUI9->value();

		slotRefresh(false);
	}

	delete pRangeUI;
}


void MainUI::slotRangeChanged()
{
    pRangeUI->spinBoxUI1->setMaximum(pRangeUI->spinBoxUI4->value() - pow(10, floor(log10(fabsl(pRangeUI->spinBoxUI4->value()))) - 5));
	pRangeUI->spinBoxUI4->setMinimum(pRangeUI->spinBoxUI1->value());
    double const xrange = pRangeUI->spinBoxUI4->value() - pRangeUI->spinBoxUI1->value();
	pRangeUI->spinBoxUI7->setMinimum(xrange / 50.0L);
	pRangeUI->spinBoxUI7->setMaximum(xrange);

    pRangeUI->spinBoxUI2->setMaximum(pRangeUI->spinBoxUI5->value() - pow(10, floor(log10(fabsl(pRangeUI->spinBoxUI5->value()))) - 5));
	pRangeUI->spinBoxUI5->setMinimum(pRangeUI->spinBoxUI2->value());
    double const yrange = pRangeUI->spinBoxUI5->value() - pRangeUI->spinBoxUI2->value();
	pRangeUI->spinBoxUI8->setMinimum(yrange / 50.0L);
	pRangeUI->spinBoxUI8->setMaximum(yrange);

    pRangeUI->spinBoxUI3->setMaximum(pRangeUI->spinBoxUI6->value() - pow(10, floor(log10(fabsl(pRangeUI->spinBoxUI6->value()))) - 5));
	pRangeUI->spinBoxUI6->setMinimum(pRangeUI->spinBoxUI3->value());
    double const zrange = pRangeUI->spinBoxUI6->value() - pRangeUI->spinBoxUI3->value();
	pRangeUI->spinBoxUI9->setMinimum(zrange / 50.0L);
	pRangeUI->spinBoxUI9->setMaximum(zrange);
}


void MainUI::slotGraphic()
{
	pGraphicUI = new GraphicUI(this);

//	pGraphicUI->spinBox1_3->setValue((int) graphUI1_1->option.precision);
//	pGraphicUI->checkBox1_4->setChecked(graphUI1_1->option.grid);
	pGraphicUI->checkBox1_5->setChecked(graphUI1_1->option.lightning);
    pGraphicUI->comboBox1_1->setCurrentIndex(graphUI1_1->option.color);
    pGraphicUI->comboBox1_2->setCurrentIndex(graphUI1_1->option.stat_color);
	pGraphicUI->checkBox1_6->setChecked(graphUI1_1->option.adjust);
	pGraphicUI->checkBox1_7->setChecked(graphUI1_1->option.rescale);

	pGraphicUI->checkBox2_1->setChecked(graphUI1_1->option.x.axis);
	pGraphicUI->checkBox2_2->setChecked(graphUI1_1->option.x.numbers);
	pGraphicUI->checkBox2_3->setChecked(graphUI1_1->option.x.grid);
	pGraphicUI->checkBox2_4->setChecked(graphUI1_1->option.x.trace);
    pGraphicUI->comboBox2_1->setCurrentIndex(graphUI1_1->option.y.color);

	pGraphicUI->checkBox3_1->setChecked(graphUI1_1->option.y.axis);
	pGraphicUI->checkBox3_2->setChecked(graphUI1_1->option.y.numbers);
	pGraphicUI->checkBox3_3->setChecked(graphUI1_1->option.y.grid);
	pGraphicUI->checkBox3_4->setChecked(graphUI1_1->option.y.trace);
    pGraphicUI->comboBox3_1->setCurrentIndex(graphUI1_1->option.y.color);

	pGraphicUI->checkBox4_1->setChecked(graphUI1_1->option.z.axis);
	pGraphicUI->checkBox4_2->setChecked(graphUI1_1->option.z.numbers);
	pGraphicUI->checkBox4_3->setChecked(graphUI1_1->option.z.grid);
	pGraphicUI->checkBox4_4->setChecked(graphUI1_1->option.z.trace);
    pGraphicUI->comboBox4_1->setCurrentIndex(graphUI1_1->option.y.color);

	pGraphicUI->buttonApply->setEnabled(false);

	connect(pGraphicUI->buttonApply, SIGNAL(clicked()), this, SLOT(slotGraphicApply()));

	if (pGraphicUI->exec() == QDialog::Accepted && pGraphicUI->buttonApply->isEnabled())
	{
		slotGraphicApply();
	}

	delete pGraphicUI;
}


void MainUI::slotGraphicApply()
{
	pGraphicUI->buttonApply->setEnabled(false);

//	graphUI1_1->option.precision = pGraphicUI->spinBox1_3->value();
//	graphUI1_1->option.grid = pGraphicUI->checkBox1_4->isChecked();
	graphUI1_1->option.lightning = pGraphicUI->checkBox1_5->isChecked();
    graphUI1_1->option.color = pGraphicUI->comboBox1_1->currentIndex();
    graphUI1_1->option.stat_color = pGraphicUI->comboBox1_2->currentIndex();
	graphUI1_1->option.adjust = pGraphicUI->checkBox1_6->isChecked();
	graphUI1_1->option.rescale = pGraphicUI->checkBox1_7->isChecked();

	graphUI1_1->option.x.axis = pGraphicUI->checkBox2_1->isChecked();
	graphUI1_1->option.x.numbers = pGraphicUI->checkBox2_2->isChecked();
	graphUI1_1->option.x.grid = pGraphicUI->checkBox2_3->isChecked();
	graphUI1_1->option.x.trace = pGraphicUI->checkBox2_4->isChecked();
    graphUI1_1->option.x.color = pGraphicUI->comboBox2_1->currentIndex();

	graphUI1_1->option.y.axis = pGraphicUI->checkBox3_1->isChecked();
	graphUI1_1->option.y.numbers = pGraphicUI->checkBox3_2->isChecked();
	graphUI1_1->option.y.grid = pGraphicUI->checkBox3_3->isChecked();
	graphUI1_1->option.y.trace = pGraphicUI->checkBox3_4->isChecked();
    graphUI1_1->option.y.color = pGraphicUI->comboBox3_1->currentIndex();

	graphUI1_1->option.z.axis = pGraphicUI->checkBox4_1->isChecked();
	graphUI1_1->option.z.numbers = pGraphicUI->checkBox4_2->isChecked();
	graphUI1_1->option.z.grid = pGraphicUI->checkBox4_3->isChecked();
	graphUI1_1->option.z.trace = pGraphicUI->checkBox4_4->isChecked();
    graphUI1_1->option.z.color = pGraphicUI->comboBox4_1->currentIndex();

	// Refresh graphic
	slotRefresh();
}


void MainUI::slotOptions()
{
	pOptionUI = new OptionUI(this);

	pOptionUI->spinBox1_1->setValue((int) option.precision);
	pOptionUI->checkBox1_1->setChecked(option.trim);
	pOptionUI->checkBox1_2->setChecked(option.magnitude);
    pOptionUI->buttonGroup1_1->buttons().at(option.imperial ? 1 : 0)->setDown(true);
	pOptionUI->checkBox1_3->setChecked(option.exponent);
	pOptionUI->checkBox1_4->setChecked(option.brackets);
	pOptionUI->lineEdit2_1->setText(QString::fromStdString(option.utility));
	pOptionUI->checkBox2_2->setChecked(option.restore);
	pOptionUI->checkBox2_3->setChecked(option.reload);

	pOptionUI->buttonApply->setEnabled(false);

	connect(pOptionUI->buttonApply, SIGNAL(clicked()), this, SLOT(slotOptionsApply()));

	if (pOptionUI->exec() == QDialog::Accepted && pOptionUI->buttonApply->isEnabled())
	{
		slotOptionsApply();
	}

	delete pOptionUI;
}


void MainUI::slotOptionsApply()
{
	pOptionUI->buttonApply->setEnabled(false);

	option.precision = pOptionUI->spinBox1_1->value();
	option.trim = pOptionUI->checkBox1_1->isChecked();
	option.magnitude = pOptionUI->checkBox1_2->isChecked();
    option.imperial = pOptionUI->buttonGroup1_1->checkedId() == 1;
	option.exponent = pOptionUI->checkBox1_3->isChecked();
	option.brackets = pOptionUI->checkBox1_4->isChecked();
    option.utility = pOptionUI->lineEdit2_1->text().toUtf8().constData();
	option.restore = pOptionUI->checkBox2_2->isChecked();
	option.reload = pOptionUI->checkBox2_3->isChecked();

	// Refresh expression
    if (toolsTrace_ModeAction->isChecked())
	{
		writeCoordinates();
	}
    else if (currentLineEdit()->alignment() == Qt::AlignRight)
	{
        writeResult(res, currentLineEdit(), option.precision);
	}
}


void MainUI::writeCoordinates()
{
	switch (graphUI1_1->option.mode)
	{
	case GraphUI::e2d:
		lineEdit1_4->hide();

        writeResult(floating(graphUI1_1->fx.trace.x), lineEdit1_2, 8, "X = ");
        writeResult(floating(graphUI1_1->fx.trace.y), lineEdit1_3, 8, "Y = ");

		break;

	case GraphUI::e3d:
		lineEdit1_4->show();

        writeResult(floating(graphUI1_1->fxy.trace.x), lineEdit1_2, 8, "X = ");
        writeResult(floating(graphUI1_1->fxy.trace.y), lineEdit1_3, 8, "Y = ");
        writeResult(floating(graphUI1_1->fxy.trace.z), lineEdit1_4, 8, "Z = ");

		break;

	default:
		break;
	}
}


/**
	Appends text to the line edit (write mode).
*/

void MainUI::writeResult(normed const & f, QLineEdit * p)
{
	ostringstream o;

    switch (comboBox2->currentIndex())
	{
	case 0:
		o.flags(o.flags() | ios_base::hex);
		break;

	case 2:
		o.flags(o.flags() | ios_base::oct);
		break;

	case 3:
		o.flags(o.flags() | normed::bin);
		break;

	default:
		o.precision(option.precision);

		o.flags(std::ios_base::showpoint);
		if (option.trim)	
			o.flags(o.flags() | normed::trim);
		if (option.sexagesimal)	
			o.flags(o.flags() | normed::sexagesimal);
		if (option.mixed)	
			o.flags(o.flags() | fraction::mixed);
		if (option.exponent)	
			o.flags(o.flags() | std::ios_base::scientific);
		break;
	}

	o << f;

	p->insert(QString::fromStdString(o.str()));
}


/**
	Writes out the result (read mode).
*/

void MainUI::writeResult(normed const & f, QLineEdit * p, int precision, QString const & s)
{
	ostringstream o;

    switch (comboBox2->currentIndex())
	{
	case 0:
		o.flags(o.flags() | ios_base::hex);
		break;

	case 2:
		o.flags(o.flags() | ios_base::oct);
		break;

	case 3:
		o.flags(o.flags() | normed::bin);
		break;

	default:
		o.precision(precision);

		o.flags(std::ios_base::showpoint);
		if (option.trim)	
			o.flags(o.flags() | normed::trim);
		if (option.sexagesimal)	
			o.flags(o.flags() | normed::sexagesimal);
		if (option.mixed)	
			o.flags(o.flags() | fraction::mixed);
		if (option.magnitude)	
			o.flags(o.flags() | floating::showmagnitude);
		if (option.imperial)	
			o.flags(o.flags() | floating::imperialsystem);
		if (option.exponent)	
			o.flags(o.flags() | std::ios_base::scientific);
		break;
	}

	o << f;

	res = f;

	switch (pTabBar->currentIndex())
	{
	case 0:
		if (graph)
		{
			p->setText("0.");
		}
		else
		{
			p->setText(s + QString::fromStdString(o.str()));
		}
		break;

	case 2:
    {
        QPalette p(palette());
        p.setColor(QPalette::Background, Qt::black);
        worksheetUI2_1->setAutoFillBackground(true);
        worksheetUI2_1->setPalette(p);
		worksheetUI2_1->setText(s + QString::fromStdString(o.str()));
    }
		break;

	case 5:
		if (type_t< fdi::vector<normed> > const * r = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& f)))
		{
			if (r->value.size() > 0)
			{
				if (type_t< fdi::vector<normed> > const * s = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& r->value[0])))
				{
                    widgetStack7_1->setCurrentIndex(1);
					writeMatrix(f, table7_2);
					p->setText("0.");
					break;
				}
			}

            widgetStack7_1->setCurrentIndex(0);
			writeVector(f, table7_1);
			p->setText("0.");
			break;
		}

		p->setText(s + QString::fromStdString(o.str()));
		break;

	default:
		p->setText(s + QString::fromStdString(o.str()));
		break;
	}
}


void MainUI::writeVector(normed const & f, MatrixUI * p)
{
    p->setRowCount(0);

	if (type_t< fdi::vector<normed> > const * r = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& f)))
	{
		// make sure we're not dealing with a matrix
		if (r->value.size() > 0)
		{
			if (type_t< fdi::vector<normed> > const * s = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& r->value[0])))
			{
				return;
			}
		}

		// fetch the vector
		QLineEdit * pLineEdit = new QLineEdit(this);

		for (unsigned int i = 0, j, k = 0; i < r->value.size(); i = j, ++ k)
		{
			pLineEdit->clear();

			for (j = i + 1; j < r->value.size(); ++ j)
			{
				if (r->value[j - 1].to_floating() != r->value[j].to_floating())
				{
					break;
				}
			}

            p->setRowCount(p->rowCount() + 1);

			writeResult(r->value[i], pLineEdit);

            p->setItem(k, 0, new QTableWidgetItem(pLineEdit->text()));
            p->setItem(k, 1, new QTableWidgetItem(QString::number(j - i)));
		}
	}
}


void MainUI::writeMatrix(normed const & f, MatrixUI * p)
{
    p->setRowCount(0);
    p->setColumnCount(0);

	if (type_t< fdi::vector<normed> > const * r = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& f)))
	{
		QLineEdit * pLineEdit = new QLineEdit(this);

		for (unsigned int i = 0; i < r->value.size(); ++ i)
		{
			if (type_t< fdi::vector<normed> > const * s = dynamic_cast<type_t< fdi::vector<normed> > const *>(reinterpret_cast<type const *>(& r->value[i])))
			{
				if (i == 0)
				{
                    p->setColumnCount(s->value.size());
				}

                p->setRowCount(p->rowCount() + 1);

				for (unsigned int j = 0; j < s->value.size(); ++ j)
				{
					pLineEdit->clear();

					writeResult(s->value[j], pLineEdit);

                    p->setItem(i, j, new QTableWidgetItem(pLineEdit->text()));
				}
			}
		}
	}
}


void MainUI::slotHelp()
{
#ifdef _WIN32
	if (! HtmlHelpA(GetDesktopWindow(), ".\\fcalc-help.chm", HH_DISPLAY_TOPIC, 0))
	{
        QMessageBox::critical(this, "Console", QString("ERROR: Cannot button 'fcalc-help.chm'"));
	}
#else
    QDesktopServices::openUrl(QUrl("http://www.fornux.com/calculator/help.html"));
#endif
}


void MainUI::slotAbout()
{
    pAboutUI = new AboutUI(this);

	pAboutUI->textLabel10->setText(ABOUT);

	pAboutUI->pixmapLabel1->setPixmap(QPixmap(":/images/logo.png"));

	if (pAboutUI->exec() == QDialog::Accepted)
	{
	}

	delete pAboutUI;
}



