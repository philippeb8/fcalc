/**
    Copyright 2019 Fornux Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "app.h"

#include "ui.h"

#include <signal.h>

#include <string>
#include <fstream>
#include <algorithm>

#include <qlabel.h>
#include <qlineedit.h>
#include <qstatusbar.h>
#include <QMainWindow>
#include <qmessagebox.h>
#include <qapplication.h>
#include <QSplashScreen>
#include <QTimer>
#include <QProxyStyle>

#ifdef _WIN32
#include <qwindowsxpstyle.h>
#endif


using namespace std;


/**
	Shared variables and utilities.
*/

extern bool saved;




App * App::self = 0;


class MyProxyStyle : public QProxyStyle
{
public:
    int pixelMetric(PixelMetric metric, const QStyleOption * option = 0, const QWidget * widget = 0 ) const
    {
        if (metric == QStyle::PM_SmallIconSize)
            return 46;
        else
            return QProxyStyle::pixelMetric(metric, option, widget);
    }
};

void translateFontSize( QWidget *ui )
{
    QObjectList l;
    QObject *w;

    if (!ui)
        return;

    //if (QToolButton *ww = dynamic_cast<QToolButton *>(ui))
    {
        QFont font = QFont(ui->font());
        font.setPointSize(16);
        ui->setFont(font);
    }

    l = ui->children();

    foreach (w, l)
    {
        if (QWidget *ww = dynamic_cast<QWidget *>(w))
            translateFontSize( ww );
    }
}

App::App(int & argc, char * argv[], const char * name) : QApplication(argc, argv)
{
	self = this;


	// UI
	QApplication::setFont(QFont("Tahoma", 8));
#ifdef MINGW32
	QApplication::setStyle(new QWindowsXPStyle());
#endif

	// Main application
	pMainUI = new MainUI(0, "PowerCalc");

/**
	// Internal fault handlers
	signal(SIGFPE, termination_handler);
	signal(SIGILL, termination_handler);
	signal(SIGSEGV, termination_handler);

	// External fault handlers
	signal(SIGINT, termination_handler);
	signal(SIGABRT, termination_handler);
	signal(SIGTERM, termination_handler);
*/

    // Filename option
	if (1 < argc && argv[1][0] != '/')
	{
		pMainUI->filename = argv[1];
		pMainUI->option.commandline = true;
    }

/**
	// execute application
    setCentralWidget(pMainUI);

	pMainUI->statusBar()->setSizeGripEnabled(false);

	pMainUI->show();
	pSplash->finish(pMainUI);
*/

#if 1
    pMainUI->setStyleSheet(
                //"QGroupBox { border:0; } QToolButton { font-size: 16px; font-weight: bold; } "
                "QGroupBox { border:0; } QToolButton { font-weight: bold; } "
#if 0
                "QMenuBar {"
                "    font-size: 18px; font-weight: bold;"
                "    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
                "                                      stop:0 lightgray, stop:1 darkgray);"
                "}"

                "QMenuBar::item {"
                "    spacing: 3px; /* spacing between menu bar items */"
                "    padding: 1px 4px;"
                "    background: transparent;"
                "    border-radius: 4px;"
                "}"

                "QMenuBar::item:selected { /* when selected using mouse or keyboard */"
                "    background: #a8a8a8;"
                "}"

                "QMenuBar::item:pressed {"
                "    background: #888888;"
                "}"
#endif
                );
#endif

    translateFontSize(pMainUI);

    pMainUI->setStyle(new MyProxyStyle);

    pMainUI->show();
}

/**
void App::termination_handler(int signum)
{
	switch (signum)
	{
	case SIGFPE:
	case SIGILL:
	case SIGSEGV:
		QMessageBox::critical
		(
			App::self->pMainUI,
			"Program Termination",
			QString
			(
				QString("Internal fault") + " (#" + QString::number(signum) + ").\n\n"
			),
			QMessageBox::Ok,
			Qt::NoButton
		);

		QApplication::exit(-1);
		break;

	case SIGINT:
	case SIGABRT:
	case SIGTERM:
		if
		(
			! saved
			&&
			QMessageBox::critical
			(
				App::self->pMainUI,
				"Program Termination",
				QString
				(
					QString("Abnormal program termination") + " (#" + QString::number(signum) + ").\n\n"
					"Click OK to save current variables.\n\n"
				),
				QMessageBox::Ok,
				QMessageBox::Cancel
			)
			==
			QMessageBox::Ok
		)
		{
			App::self->pMainUI->slotSave();
		}

		QApplication::exit(-1);
		break;
	}
}
*/
