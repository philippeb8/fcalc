/**
    Copyright 2019 Fornux Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef GRAPHUI_H
#define GRAPHUI_H


#include <list>
#include <vector>
#include <string>
#include <utility>
#include <sstream>

#include <qfont.h>
#include <qwidget.h>
//Added by qt3to4:
#include <QPixmap>
#include <QPaintEvent>

#include "point.h"


class QTimer;


/**
	Converts any object quickly into a string.
*/

template <typename T>
	QString QStreamString(T const & o)
	{
		return QString::fromStdString(static_cast<std::ostringstream const &>(std::ostringstream() << o).str());
	}




class MainUI;
class GLWidgetUI;

class QPixmap;
class QGLWidget;


class GraphUI : public QWidget
{
	Q_OBJECT

	friend class MainUI;
	friend class GLWidgetUI;

public:
	GraphUI(QWidget * = 0, char const * = 0);

	void plot();
	void plot(real (*)(real), bool = true);
	void plot(real (*)(real, real), bool = true);
	void plot(std::vector<real> const &, real (*)(real), bool = true);
	void plot(std::vector<real> const &, std::vector<real> const &, real (*)(real), bool = true);

	bool trace(bool);
	bool trace(int, int);

	bool init_fx(int, int);
	bool init_fxy(int, int);

protected:
	void draw_marker(int, int);
    void draw_report(QPainter &, int);
	void draw_fx(QPainter &, int, int, bool = true);
	void draw_fxy(QPainter &, int, int, bool = true);

protected slots:
	void timeout();

	virtual void paintEvent(QPaintEvent *);

public:
	static bool fast;

	enum eMode {eNone, e2d, e3d};
	enum eStat {eScientific, eChart, eScatter};

	struct
	{
		point<3> min, max, scl;

		bool header, legend;
		QFont title_font, unit_font, scale_font;

		struct
		{
			bool hcenter, vcenter;

			double width, height, left, right, top, bottom;
		} report[2];

		int color, stat_color, precision;

		QString title;
		bool grid, lightning, adjust, rescale;

		struct
		{
			int color;
			QString unit;
			bool axis, numbers, grid, trace;
		} x, y, z;

		enum eMode mode;
		enum eStat stat;
	} option;

private:
	struct fx_t
	{
		bool draw[2];
		point<2, int> top[2];
		point<2> trace, axis;
		point<2> min, max, scl, stp;

		real (* plot)(real);
		std::vector<real> data[2];

		int sx(real, int);
		int sy(real, int);
		real rx(int, int);
		real ry(int, int);
	} fx;

	struct fxy_t
	{
		bool draw[3];
		point<2, int> top[3];
		point<3> trace, axis;
		point<3> min, max, scl, stp;

		real (* plot)(real, real);

		int sx(real, real, int);
		int sy(real, real, int);
		real rx(int, real, int);
		real ry(int, real, int);
	} fxy;

	bool on;

	int benchmark;
	int sx, sy, dx, dy, cx, cy;
	real precisionx, precisiony;

	QTimer * pTimer;
	QPixmap * pPixmap;
#if 1
    QWidget * pGLWidget;
#else
    QGLWidget * pGLWidget;
#endif
};


#endif

