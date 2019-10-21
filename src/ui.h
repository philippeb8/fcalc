/**
    Copyright 2019 Fornux Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef UI_H
#define UI_H


#include <list>
#include <string>
#include <sstream>

#include <qstring.h>
#include <qmainwindow.h>
#include <qbuttongroup.h>
//Added by qt3to4:
#include <QEvent>
#include <QVBoxLayout>
#include <QActionGroup>

#include "tuple.h"
#include "point.h"
#include "normed.h"
#include "ui_calculatorui.h"




/**
	Main application.
*/

class App;
class QWidget;
class QTabBar;
class QPrinter;
class Q3VBoxLayout;
class Q3ActionGroup;
class QSplashScreen;
class QAssistantClient;

class TitleUI;
class MatrixUI;
class VariableUI;
struct RangeUI;
struct AboutUI;
struct SetupUI;
struct OptionUI;
struct GraphicUI;
///class Ui_RegisterUI;


class QCustomEvent : public QEvent {
public:
    QCustomEvent() : QEvent(QEvent::User) {}
};


class MainUI : public QMainWindow, public Ui_CalculatorUI
{
	Q_OBJECT

	friend class App;
	friend void SDP_ExecuteRegistered();
	friend void SDP_ExecuteTrialExpired();
	friend void SDP_ExecuteTrialNotExpired();
    friend fdi::normed const & flush(fdi::normed const & res);

	class History;

public:
	MainUI(QWidget * = 0, char const * = 0);

	virtual void load();
	virtual void save();

	void writeResult(fdi::normed const &, QLineEdit *);
	void writeResult(fdi::normed const &, QLineEdit *, int, QString const & = "");
	void writeVector(fdi::normed const &, MatrixUI *);
	void writeMatrix(fdi::normed const &, MatrixUI *);

protected:
    QLineEdit * currentLineEdit();
    bool event(QEvent *);
	bool eventFilter(QObject *, QEvent *);
    void error(std::string const &);
	bool execute(QString const &);
	void writeCoordinates();
	void setButtons();
	void setReadOnly(bool, bool = true);
    void resizeEvent(QResizeEvent *);

protected slots:
    void workAreaResized(int);
    void group1clicked(int);
	void group2clicked(int);
	void group3clicked(int);
	void group5clicked(int);
	void group6clicked(int);
	void group7clicked(int);
	void group8clicked(int);
	void group6_1clicked(int);
	void group6_2clicked(int);
	void button3clicked();
	void button4clicked();
	void group10clicked(int);

	void slotVectorSelected();
	void slotVectorChanged();
	void slotMatrixSelected();
	void slotMatrixChanged();
	void slotVariableChanged();
	void slotBaseNClicked(int);
	void slotTrigoClicked(int);
	void slotToggleClicked(int);
	void slotKeypadClicked(int);
	void slotInsertSortClicked();
	void slotInsertPartClicked();
	void slotInsertMergeClicked();
	void slotInsertColXClicked();
	void slotInsertColYClicked();
	void slotInsertTransposeClicked();
	void slotInsertResultClicked();
	void slotInsertFunction1stClicked();
	void slotInsertFunction2ndClicked();

	void slotNew();
	void slotOpen(bool = true);
	void slotSave();
	void slotSaveAs(bool = true);
	bool slotSetup(bool = true);
	void slotSetupRepaint();
    ///void slotPrint();
    ///void slotPaint();
	void slotExit();
	void slotImportSingleVariables();
	void slotImportPairedVariables();
	void slotRefresh(bool = true);

	void slotCut();
	void slotCopy();
	void slotPaste();
	void slotClearText();
	void slotSelectAll();
	void slotSelectionChanged();

	void slotEnter();
	void slotPreviousExpression();
	void slotNextExpression();
	void slotFirstExpression();
	void slotLastExpression();
	void slotBaseN(QAction *);
	void slotTrigo(QAction *);
	void slotMixed();
	void slotSexagesimal();
	void slotStatusBar();
	void slotScientific(bool = true);

	void slotWidgetStack(int);
	void slotRPN(bool = true);
	void slotPlotY(bool = true);
	void slotPlotZ(bool = true);
	void slotBarChart(bool = true);
	void slotScatterPlot(bool = true);
	void slotClear();
	void slotZoom(QAction *);
	void slotZoomIn();
	void slotZoomOut();
	void slotTrace();

	void slotRange();
	void slotRangeChanged();
	void slotGraphic();
	void slotGraphicApply();
	void slotOptions();
	void slotOptionsApply();

    ///void slotRegister();
    ///void slotRegisterChanged();
    void slotHelp();
	void slotAbout();

public:
	static MainUI * pMainUI;
	static char pRegistrationKey[192];

	QTabBar * pTabBar;
	std::vector<History> statement;

	struct
	{
		bool commandline;
		std::string utility;

		std::vector<int> color, stat_color;

		int precision, basen, trigo, width, height;
		bool trim, magnitude, exponent, imperial, mixed, sexagesimal, restore, reload, status, scientific, brackets, accelshift, accelalpha, rpn;
	} option;

private:
	struct History
	{
		std::list< fdi::tuple<QString, int, int> > l;
		std::list< fdi::tuple<QString, int, int> >::iterator i;
	};

	int zoom;
	bool stat;
	float dim[22];
	fdi::normed res, mem;
	std::string filename;

    QPrinter * pPrinter;

	TitleUI * pTitleUI;
	VariableUI * pVariableUI;
    RangeUI * pRangeUI;
    AboutUI * pAboutUI;
    SetupUI * pSetupUI;
    OptionUI * pOptionUI;
    GraphicUI * pGraphicUI;
    ///RegisterUI * pRegisterUI;
	QAssistantClient * pAssistantClient;

	struct EE
	{
		int nResizeId, nFlashId, nTranslateId, nWarpId;

		static int const n = 10;				// number of control point<3, double>s = n+1
		static int const t = 4;					// degree of polynomial = t-1
		static int const resolution = 120;		// how many point<3, double>s our in our output array

		int i[2], k[2];
		enum {translate, energize, warp} state;
		point<3, double> pts[n + 1];			// allocate our control point<3, double> array
		point<3, double> out_pts[resolution];

		QPoint pos;
		QSplashScreen * pSplash;
	} ee;

	QActionGroup * viewBaseNActionGroup, * viewTrigoActionGroup, * toolsZoomActionGroup, * advActionGroup;

	enum {eNothing, eGraph2d, eGraph3d, eTrace2d, eTrace3d} mode;

	bool bRegistered;
	QPoint nMousePos;
	QSize const nMaximumSize;
};


#endif

