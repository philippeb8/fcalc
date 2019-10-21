/**
    Copyright 2019 Fornux Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "graphui.h"

#include <list>
#include <cmath>
#include <vector>
#include <iostream>
#include <algorithm>

#include <QtOpenGL/qgl.h>
#ifdef QT_OPENGL_ES
//#include <QGLWidget>
#endif
#include <qtimer.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qdatetime.h>
#include <qfontmetrics.h>
//Added by qt3to4:
#include <QPolygon>
#include <QPaintEvent>
#include <QPolygon>

#include "calculus.h"
#include "portability.h"



namespace fdi
{
#if __cplusplus > 201100L
inline real floor(const real &a) { return (real)(integer)(a); }
#else
inline real floor(const real &a) { return std::floor(a); }
#endif
}

#define self (* this)

using namespace std;
using namespace fdi;

typedef mutable_ptr<type> type_p;


extern variable_t variable[128];

extern std::list<variable_t> stack[6];


#if 1
class GLWidgetUI : public QWidget
{
public:
    GLWidgetUI(QWidget * parent) : QWidget(parent) {}
};

#else
class GLWidgetUI : public QGLWidget
{
public:
	GLWidgetUI(QWidget * parent);

    virtual void initializeGL();
	virtual void resizeGL(int, int);
	virtual void paintGL();

private:
	int nWidth, nHeight;
};

inline GLWidgetUI::GLWidgetUI(QWidget * parent) : QGLWidget(parent)
{
}

inline void GLWidgetUI::initializeGL()
{
	GLfloat nLight[][4] =
	{
		{0.1, 0.1, 0.1, 1.0},
		{1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0},
		{-1.0, -1.0, -1.0, 0.0}
	};

	glLightfv(GL_LIGHT0, GL_AMBIENT, nLight[0]);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, nLight[1]);
	glLightfv(GL_LIGHT0, GL_SPECULAR, nLight[2]);
	glLightfv(GL_LIGHT0, GL_POSITION, nLight[3]);

	GLdouble nClip[][4] =
	{
		{0.0, -1.0, -0.67, 1.0},
		{0.0, 1.0, 0.67, 0.0}
	};

	glClipPlane(GL_CLIP_PLANE0, nClip[0]);
	glClipPlane(GL_CLIP_PLANE1, nClip[1]);

	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);

    qglClearColor(Qt::white);
}

inline void GLWidgetUI::resizeGL(int nWidth, int nHeight)
{
	GraphUI * const p = static_cast<GraphUI *>(parent());

	if (! p->init_fxy(nWidth, nHeight)) 
		return;

	self.nWidth = nWidth;
	self.nHeight = nHeight;

	// resize
    glViewport(0, 0, (GLint) (self.nWidth), (GLint) (self.nHeight));

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

	real rx = real(10) / p->cx;
	real ry = real(10) / p->cy;
	real rz = real(10) / (p->dy * 3 / 5);

    glOrtho(0.0, 1.0 + p->cy * 1.0 / p->cx + rx, 0.0, 1.0 + 0.67 + ry, 0.0, 1.0 + rz);

//	glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT, GL_FASTEST);

    glMatrixMode(GL_MODELVIEW);

//	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

inline void GLWidgetUI::paintGL()
{
	GraphUI * const p = static_cast<GraphUI *>(parent());

	QTime t;
	QPainter painter(p->pPixmap);

	GLdouble matrix[] =
	{
		1.0, 0.0, 0.0, 0.0,
		p->cy * 1.0 / p->cx, 0.67, -1.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	};

	glLoadMatrixd(matrix);

	t.start();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// backup
	stack[2].push_back(variable_t()), (mutable_ptr<type> &) stack[2].back() = (mutable_ptr<type> &) (variable[(int) ('x')]);
	stack[3].push_back(variable_t()), (mutable_ptr<type> &) stack[3].back() = (mutable_ptr<type> &) (variable[(int) ('y')]);

	(mutable_ptr<type> &) (variable[(int) ('x')]) = new type_t<floating>(0);
	(mutable_ptr<type> &) (variable[(int) ('y')]) = new type_t<floating>(0);

	// plot multiple graphics
    if (type_t< fdi::vector<type_p> > const * r = dynamic_cast<type_t< fdi::vector<type_p> > const *>(variable[(int) ('z')].get()))
	{
		stack[2].push_back(variable_t()), (mutable_ptr<type> &) stack[2].back() = (mutable_ptr<type> &) (variable[(int) ('z')]);

		for (unsigned int i = 0; i < r->value.size(); ++ i)
		{
            (mutable_ptr<type> &) (variable[(int) ('z')]) = dynamic_cast<type_t< fdi::vector<type_p> > const *>(stack[2].back().get())->value[i];

			p->draw_fxy(painter, self.nWidth, self.nHeight, i == 0);
		}

		(mutable_ptr<type> &) (variable[(int) ('z')]) = (mutable_ptr<type> &) stack[2].back(), stack[2].pop_back();
	}

	// plot single graphic
	else
	{
		p->draw_fxy(painter, self.nWidth, self.nHeight);
	}

	// restore
	(mutable_ptr<type> &) (variable[(int) ('x')]) = (mutable_ptr<type> &) stack[2].back(), stack[2].pop_back();
	(mutable_ptr<type> &) (variable[(int) ('y')]) = (mutable_ptr<type> &) stack[3].back(), stack[3].pop_back();

    if (GraphUI::fast != true)
	{
		p->benchmark = t.elapsed();
	}
}
#endif



/**
	Indexed colors.
*/

QColor pColor[] =
{
	QColor( 255,   0,   0 ),   // index 249   red
	QColor(   0, 255,   0 ),   // index 250   green
	QColor(   0,   0, 255 ),   // index 252   blue
	QColor(   0, 255, 255 ),   // index 254   cyan
	QColor( 255,   0, 255 ),   // index 253   magenta
	QColor( 255, 255,   0 ),   // index 251   yellow
	QColor( 255, 255, 255 ),   // index 255   white
	QColor( 192, 192, 192 ),   // index 7     light gray
	QColor( 128,   0,   0 ),   // index 1     dark red
	QColor(   0, 128,   0 ),   // index 2     dark green
	QColor(   0,   0, 128 ),   // index 4     dark blue
	QColor(   0, 128, 128 ),   // index 6     dark cyan
	QColor( 128,   0, 128 ),   // index 5     dark magenta
	QColor( 128, 128,   0 ),   // index 3     dark yellow
	QColor( 128, 128, 128 ),   // index 248   medium gray
	QColor(   0,   0,   0 )    // index 0     black
};


bool GraphUI::fast = false;


/**
	Main widget.
*/

GraphUI::GraphUI(QWidget * a_pcWidget, char const * a_pzName): QWidget(a_pcWidget)
{
	// instanciate
	pTimer = new QTimer(this);
    ///pPixmap = new QPixmap();
    //pGLWidget = new GLWidgetUI(this);

	// defaults
	on = false;
	benchmark = 0;

    option.header = true;
    option.legend = true;

	option.title_font = QFont("Arial", 16);
	option.unit_font = QFont("Arial", 8);
	option.scale_font = QFont("Arial", 8);

	option.title_font.setBold(true);
	option.title_font.setUnderline(true);

	option.unit_font.setBold(true);

	option.report[0].width = 500;
	option.report[0].height = 250;
/*
	option.report[0].left = 20;
	option.report[0].right = 60;
	option.report[0].top = 80;
	option.report[0].bottom = 20;
*/

	option.report[1].width = 480;
	option.report[1].height = 240;
	option.report[1].left = 96;
	option.report[1].right = 96;
	option.report[1].top = 96;
	option.report[1].bottom = 96;
	option.report[1].hcenter = true;
	option.report[1].vcenter = true;

	option.color = 0;
	option.stat_color = 8;
	option.grid = false;
	option.lightning = true;
	option.adjust = true;
	option.rescale = true;
    option.precision = 10;
	option.title = "<Untitled>";

	option.x.axis = true;
	option.x.numbers = true;
	option.x.grid = false;
	option.x.trace = true;
	option.x.color = 7;
	option.x.unit = "X";

	option.y.axis = true;
	option.y.numbers = true;
	option.y.grid = false;
	option.y.trace = true;
	option.y.color = 7;
	option.y.unit = "Y";

	option.z.axis = true;
	option.z.numbers = true;
	option.z.grid = true;
	option.z.trace = false;
	option.z.color = 7;
	option.z.unit = "Z";

	option.mode = eNone;
	option.stat = eScientific;
	
	fx.plot = 0;
	fxy.plot = 0;

    setStyleSheet("background-color:white;");

	// init
    //pGLWidget->hide();

	// connections
	connect(pTimer, SIGNAL(timeout()), SLOT(timeout()));
}


void GraphUI::plot()
{
	option.mode = eNone;
	option.stat = eScientific;

    //pGLWidget->hide();
    ///*pPixmap = pPixmap->copy(QRect(QPoint(0, 0), size()));
    ///pPixmap->fill(palette().color(QWidget::backgroundRole()));

    update();
}


void GraphUI::plot(real (* f)(real), bool bErase)
{
	QTime t;

	option.mode = e2d;
	option.stat = eScientific;

	fx.plot = f;

/**
	if (bErase)
	{
		pGLWidget->hide();
        *pPixmap = pPixmap->copy(QRect(QPoint(0, 0), size()));
        pPixmap->fill(palette().color(QWidget::backgroundRole()));
    }
*/

    ///QPainter painter(this);

	t.start();
    ///draw_fx(painter, width(), height(), bErase);

    if (GraphUI::fast != true)
	{
		benchmark = t.elapsed();
	}

    update();
}


void GraphUI::plot(real (* f)(real, real), bool bErase)
{
	QTime t;

	option.mode = e3d;
	option.stat = eScientific;

	fxy.plot = f;

    //pGLWidget->resize(size());
    //*pPixmap = pPixmap->copy(QRect(QPoint(0, 0), size()));

    ///QPainter painter(this);

	t.start();
    ///draw_fxy(painter, width(), height(), bErase);

	if (GraphUI::fast != true)
	{
		benchmark = t.elapsed();
	}

    //pGLWidget->show();
    //pGLWidget->update();
	
    update();
}


void GraphUI::plot(std::vector<real> const & a, real (* f)(real), bool bErase)
{
	QTime t;

	option.mode = e2d;
	option.stat = eChart;

	fx.plot = f;
	fx.data[0] = a;

/*
    pGLWidget->hide();
    *pPixmap = pPixmap->copy(QRect(QPoint(0, 0), size()));
    pPixmap->fill(palette().color(QWidget::backgroundRole()));
*/

    ///QPainter painter(this);

	t.start();
    ///draw_fx(painter, width(), height());

    if (GraphUI::fast != true)
	{
		benchmark = t.elapsed();
	}

	update();
}


void GraphUI::plot(std::vector<real> const & a, std::vector<real> const & b, real (* f)(real), bool bErase)
{
	QTime t;

	option.mode = e2d;
	option.stat = eScatter;

	fx.plot = f;
	fx.data[0] = a;
	fx.data[1] = b;

/*
    pGLWidget->hide();
    *pPixmap = pPixmap->copy(QRect(QPoint(0, 0), size()));
    pPixmap->fill(palette().color(QWidget::backgroundRole()));
*/

    ///QPainter painter(this);

	t.start();
    ///draw_fx(painter, width(), height());

    if (GraphUI::fast != true)
	{
		benchmark = t.elapsed();
	}

	update();
}


/**
	Tracing mode toggle.

	@return		Successful or not.
*/

bool GraphUI::trace(bool toggle)
{
	update();

	on = toggle;

	if (toggle)
	{
		int ox, oy;

		switch (option.mode)
		{
		case e2d:
			fx.trace.x = fx.rx(sx = fx.sx(fx.axis.x, dy), dy);

			for (ox = cx; ox > 0; -- ox)
			{
				fx.trace.y = (* fx.plot)(fx.trace.x);

				if (fx.trace.y >= fx.min.y && fx.trace.y <= fx.max.y) 
					break;

				fx.trace.x = fx.rx(++ sx, dy);

				if (fx.trace.x < fx.min.x) 
					fx.trace.x = fx.rx(sx = fx.sx(fx.max.x, dy), dy);
				else if (fx.trace.x > fx.max.x) 
					fx.trace.x = fx.rx(sx = fx.sx(fx.min.x, dy), dy);
			}

			if (ox == 0) 
				return false;

			break;

		case e3d:
			fxy.trace.x = fxy.rx(sx = fxy.sx(fxy.axis.x, 0., dy), 0., dy);
			fxy.trace.y = fxy.rx(sy = fxy.sy(fxy.axis.y, 0., dy), 0., dy);

			for (oy = cy; oy > 0; -- oy)
			{
				for (ox = cx; ox > 0; -- ox)
				{
					fxy.trace.z = (* fxy.plot)(fxy.trace.x, fxy.trace.y);

					if (fxy.trace.z >= fxy.min.z && fxy.trace.z <= fxy.max.z) 
						break;

					fxy.trace.x = fxy.rx(++ sx, 0., dy);

					if (fxy.trace.x < fxy.min.x) 
						fxy.trace.x = fxy.rx(sx = fxy.sx(fxy.max.x, 0., dy), 0., dy);
					else if (fxy.trace.x > fxy.max.x) 
						fxy.trace.x = fxy.rx(sx = fxy.sx(fxy.min.x, 0., dy), 0., dy);
				}

				if (ox > 0) 
					break;

				fxy.trace.y = fxy.ry(-- sy, 0., dy);

				if (fxy.trace.y < fxy.min.y) 
					fxy.trace.y = fxy.ry(sy = fxy.sy(fxy.max.y, 0., dy), 0., dy);
				else if (fxy.trace.y > fxy.max.y) 
					fxy.trace.y = fxy.ry(sy = fxy.sy(fxy.min.y, 0., dy), 0., dy);
			}

			if (oy == 0) 
				return false;

			break;

		default:
			break;
		}

		trace(0, 0);

		pTimer->start(530);

		return true;
	}
	else
	{
		pTimer->stop();

		return true;
	}

	return false;
}


/**
	Modifies Z tracing cursor.

	@param	x	Relative X translation based on the parameter's sign
		y	Relative Y translation based on the parameter's sign

	@return		Successful or not.
*/


bool GraphUI::trace(int x, int y)
{
	update();

	switch (option.mode)
	{
	case e2d:
		for (int ox = cx; sx += x, ox > 0; -- ox, x = x < 0 ? -1 : x > 0 ? 1 : 0)
		{
			fx.trace.x = fx.rx(sx, dy);

			if (fx.trace.x < fx.min.x) 
				fx.trace.x = fx.rx(sx = fx.sx(fx.max.x, dy), dy);
			else if (fx.trace.x > fx.max.x) 
				fx.trace.x = fx.rx(sx = fx.sx(fx.min.x, dy), dy);

			fx.trace.y = (* fx.plot)(fx.trace.x);

			if (fx.trace.y >= fx.min.y && fx.trace.y <= fx.max.y) 
				break;
		}

		on = true;

		return true;

	case e3d:
		for (int ox = cx; sx += x, ox > 0; -- ox, x = x < 0 ? -1 : x > 0 ? 1 : 0)
		{
			fxy.trace.x = fxy.rx(sx, 0., dy);

			if (fxy.trace.x < fxy.min.x) 
				fxy.trace.x = fxy.rx(sx = fxy.sx(fxy.max.x, 0., dy), 0., dy);
			else if (fxy.trace.x > fxy.max.x) 
				fxy.trace.x = fxy.rx(sx = fxy.sx(fxy.min.x, 0., dy), 0., dy);

			fxy.trace.z = (* fxy.plot)(fxy.trace.x, fxy.trace.y);

			if (fxy.trace.z >= fxy.min.z && fxy.trace.z <= fxy.max.z) 
				break;
		}

		for (int oy = cy; sy -= y, oy > 0; -- oy, y = y < 0 ? -1 : y > 0 ? 1 : 0)
		{
			fxy.trace.y = fxy.ry(sy, 0., dy);

			if (fxy.trace.y < fxy.min.y) 
				fxy.trace.y = fxy.ry(sy = fxy.sy(fxy.max.y, 0., dy), 0., dy);
			else if (fxy.trace.y > fxy.max.y) 
				fxy.trace.y = fxy.ry(sy = fxy.sy(fxy.min.y, 0., dy), 0., dy);

			fxy.trace.z = (* fxy.plot)(fxy.trace.x, fxy.trace.y);

			if (fxy.trace.z >= fxy.min.z && fxy.trace.z <= fxy.max.z) 
				break;
		}

		on = true;

		return true;

	default:
		break;
	}

	return false;
}


/**
	Screen X position from any real X coordinate.

	@return		Real X coordinate.
*/

inline int GraphUI::fx_t::sx(real x, int)
{
	return int((x - min.x) / stp.x + 34);
}


/**
	Screen Y position from any real Y coordinate.

	@return		Real Y coordinate.
*/

inline int GraphUI::fx_t::sy(real y, int)
{
	return int(- (y - max.y) / stp.y + 20);
}


/**
	Real X coordinate from any screen X position.

	@return		Real X coordinate.
*/

inline real GraphUI::fx_t::rx(int sx, int)
{
	return (real(sx) - 34) * stp.x + min.x;
}


/**
	Real Y coordinate from any screen Y position.

	@return		Real Y coordinate.
*/

inline real GraphUI::fx_t::ry(int sy, int dy)
{
	return (- real(sy) + 20) * stp.y - max.y;
}


bool GraphUI::init_fx(int a_dx, int a_dy)
{
	dx = a_dx;
	dy = a_dy;

	cx = dx - 34 * 2;
	cy = dy - 20 * 2;

	if (cx <= 0 || cy <= 0) 
		return false;

    if (GraphUI::fast != true)
	{
        precisionx = 1.0L;
	}
	else
	{
        precisionx = benchmark / 900.0L + 2;
	}

	fx.stp.x = (fx.max.x - fx.min.x) / cx;
	fx.stp.y = (fx.max.y - fx.min.y) / cy;

	fx.axis = point<2>(0 < fx.min.x ? fx.min.x : 0 > fx.max.x ? fx.max.x : 0, 0 < fx.min.y ? fx.min.y : 0 > fx.max.y ? fx.max.y : 0);

	fx.top[0] = point<2, int>(fx.sx(fx.max.x, dy), fx.sy(fx.axis.y, dy));
	fx.top[1] = point<2, int>(fx.sx(fx.axis.x, dy), fx.sy(fx.max.y, dy));

	return true;
}

void GraphUI::draw_fx(QPainter & painter, int a_dx, int a_dy, bool init)
{
    if (init)
	{
		if (! init_fx(a_dx, a_dy)) 
			return;

		if (option.z.grid)
		{
			// draw xy grid
            QPolygon a(4);

			painter.setPen(QPen(pColor[option.z.color], 1));

            for (real m = fdi::floor(fx.max.y / fx.scl.y) * fx.scl.y; m >= fx.min.y; m -= fx.scl.y)
			{
				painter.drawLine(fx.sx(fx.min.x, dy), fx.sy(m, dy), fx.sx(fx.max.x, dy), fx.sy(m, dy));
			}

            for (real m = fdi::floor(fx.max.x / fx.scl.x) * fx.scl.x; m >= fx.min.x; m -= fx.scl.x)
			{
				painter.drawLine(fx.sx(m, dy), fx.sy(fx.max.y, dy), fx.sx(m, dy), fx.sy(fx.min.y, dy));
			}
		}

        painter.setPen(QPen(Qt::black, 2));
		painter.setBrush(Qt::black);

		if (fx.draw[0] = option.x.axis || fx.axis.y == 0)
		{
			// draw x axis
			painter.drawLine(fx.sx(fx.min.x, dy), fx.sy(fx.axis.y, dy), fx.top[0].x + 10, fx.top[0].y);

			// draw x arrow
			{
                QPolygon a(4);
				a.setPoint(0, QPoint(fx.top[0].x + 10, fx.top[0].y - 4));
				a.setPoint(1, QPoint(fx.top[0].x + 20, fx.top[0].y - 1));
				a.setPoint(2, QPoint(fx.top[0].x + 20, fx.top[0].y));
				a.setPoint(3, QPoint(fx.top[0].x + 10, fx.top[0].y + 3));

				painter.drawConvexPolygon(a);
			}
		}

		if (fx.draw[1] = option.y.axis || fx.axis.y == 0)
		{
			// draw y axis
			painter.drawLine(fx.sx(fx.axis.x, dy), fx.sy(fx.min.y, dy), fx.top[1].x, fx.top[1].y - 10);

			// draw y arrow
			{
                QPolygon a(4);
				a.setPoint(0, QPoint(fx.top[1].x - 4, fx.top[1].y - 10));
				a.setPoint(1, QPoint(fx.top[1].x - 1, fx.top[1].y - 20));
				a.setPoint(2, QPoint(fx.top[1].x, fx.top[1].y - 20));
				a.setPoint(3, QPoint(fx.top[1].x + 3, fx.top[1].y - 10));

				painter.drawConvexPolygon(a);
			}
		}
	}

	if (option.stat == eChart)
    {
		painter.setPen(QPen(Qt::black, 1));
		painter.setBrush(pColor[option.stat_color].light(110));
		///painter.setClipRect(fx.sx(fx.min.x, dy), fx.sy(fx.max.y, dy), fx.sx(fx.max.x, dy) - fx.sx(fx.min.x, dy) + 1, fx.sy(fx.min.y, dy) - fx.sy(fx.max.y, dy), QPainter::CoordPainter);

        for (std::vector<real>::iterator p = fx.data[0].begin(), q; p != fx.data[0].end(); p = q)
		{
			q = std::upper_bound(p, fx.data[0].end(), * p);
			painter.drawRect(fx.sx(* p - fx.scl.x / 3, dy), fx.sy(q - p, dy), fx.sx(* p + fx.scl.x * 2 / 3, dy) - fx.sx(* p, dy) + 1, fx.sy(fx.axis.y, dy) - fx.sy(q - p, dy));
		}

		painter.setClipping(false);
	}

	if (option.stat == eScatter)
	{
		painter.setPen(pColor[option.stat_color].light(110));
		painter.setBrush(pColor[option.stat_color].light(110));

        for (std::vector<real>::iterator i = fx.data[0].begin(), j = fx.data[1].begin(); i != fx.data[0].end() && j != fx.data[1].end(); ++ i, ++ j)
		{
			if (* i >= fx.min.x && * i <= fx.max.x && * j >= fx.min.y && * j <= fx.max.y)
			{
				painter.drawEllipse(fx.sx(* i, dy) - 2, fx.sy(* j, dy) - 2, 4, 4);
			}
		}
	}

    if (option.mode == e2d)
    {
		int e = 0;
		point<2> c[3][2];

        QColor tint = pColor[option.color];

        painter.setPen(QPen(tint.light(110), 2));
        painter.setBrush(tint.light(110));

        for (real x = fx.min.x; x <= fx.max.x; x += fx.stp.x * precisionx)
		{
			c[e][1] = point<2>(x, (* fx.plot)(x));

            if (x != fx.min.x && ! (isnan(c[e][0].y) || isnan(c[e][1].y)))
			{
				if ((c[e][0].y >= fx.min.y || c[e][1].y >= fx.min.y) && (c[e][0].y <= fx.max.y || c[e][1].y <= fx.max.y))
				{
					struct
					{
						static bool intersects(point<2> const & y, point<2> const & v1, point<2> const & v2)
						{
							return (v2.y - v1.y) / (y.y - v1.y) > 1;
						}

						static point<2> intersection(point<2> const & y, point<2> const & v1, point<2> const & v2)
						{
							point<2> h = y - v1;
							point<2> e = v2 - v1;

							return point<2>(e.x / e.y * h.y + v1.x, y.y);
						}

                        std::vector< point<2> > operator () (std::vector< point<2> > & polygon, point<2> const & lim, bool upper)
						{
                            std::vector< point<2> > cliped;
							cliped.reserve(polygon.capacity());

							for (unsigned int i = 0; i < polygon.size(); ++ i)
							{
								unsigned int j = (i + 1) % polygon.size();

								if (upper ? (polygon[i].y > lim.y) : (polygon[i].y < lim.y))
								{
									if (intersects(lim, polygon[i], polygon[j]))
									{
										cliped.push_back(intersection(lim, polygon[i], polygon[j]));
									}
								}
								else
								{
									cliped.push_back(polygon[i]);

									if (intersects(lim, polygon[i], polygon[j]))
									{
										cliped.push_back(intersection(lim, polygon[i], polygon[j]));
									}
								}
							}

							return cliped;
						}
					} clip;

                    std::vector< point<2> > polygon;
					polygon.reserve(2);
					polygon.push_back(c[e][0]);
					polygon.push_back(c[e][1]);

					polygon = clip(polygon, fx.min, false);
					polygon = clip(polygon, fx.max, true);

                    QPolygon a(polygon.size());

					for (unsigned int i = 0; i < polygon.size(); ++ i)
					{
						a.setPoint(i, fx.sx(polygon[i].x, dy), fx.sy(polygon[i].y, dy));
					}

					painter.drawLine(a[0], a[1]);
				}
			}

			c[e][0] = c[e][1];
        }
	}

	painter.setPen(QPen(Qt::black, 2));
	painter.setBrush(Qt::black);

	if (init)
		painter.setFont(QFont("Arial", 8));
	else
		painter.setFont(option.scale_font);

	if (fx.draw[0] = option.x.axis || fx.axis.y == 0 || fx.axis.y == 0)
	{
        real const magnitude = fdi::floor(log10(fabs(fx.max.x))) - 2;

		// draw x numbers
        for (real m = fdi::floor(fx.max.x / fx.scl.x) * fx.scl.x; option.x.numbers && m >= fx.min.x; m -= fx.scl.x)
		{
            QString const number = QStreamString(rint(m / pow(10.0L, magnitude)) * pow(10.0L, magnitude));
			QPoint const pixel = QPoint(fx.sx(m, dy), fx.sy(fx.axis.y, dy));
			QRect const rect = painter.fontMetrics().boundingRect(number);

			painter.drawLine(pixel.x(), pixel.y() + 2, pixel.x(), pixel.y() - 2);
			painter.drawText(pixel.x() - rect.width() / 2, pixel.y() + 4 + rect.height(), number);
		}
	}

	if (fx.draw[1] = option.y.axis || fxy.axis.z == 0 || fxy.axis.z == 0)
	{
        real const magnitude = fdi::floor(log10(fabs(fx.max.y))) - 2;

		// draw y numbers
        for (real m = fdi::floor(fx.max.y / fx.scl.y) * fx.scl.y; option.y.numbers && m >= fx.min.y; m -= fx.scl.y)
		{
            QString const number = QStreamString(rint(m / pow(10.0L, magnitude)) * pow(10.0L, magnitude));
			QPoint const pixel = QPoint(fx.sx(fx.axis.x, dy), fx.sy(m, dy));
			QRect const rect = painter.fontMetrics().boundingRect(number);

			painter.drawLine(pixel.x() - 2, pixel.y(), pixel.x() + 2, pixel.y());
			painter.drawText(pixel.x() - 6 - rect.width(), pixel.y() + rect.height() / 2 - 4, number);
		}
	}
}


/**
	Screen X position from any real X coordinate.

	@return		Real X coordinate.
*/

inline int GraphUI::fxy_t::sx(real x, real y, int dy)
{
	return int((x - min.x) / stp.x + (y - max.y) / stp.y + (dy * 2 / 5));
}


/**
	Screen Y position from any real Z coordinate.

	@return		Real Y coordinate.
*/

inline int GraphUI::fxy_t::sy(real y, real z, int dy)
{
	return int(- (y - max.y) / stp.y - (z - min.z) / stp.z + (dy * 3 / 5));
}


/**
	Real X coordinate from any screen X position and real Y coordinate.

	@return		Real X coordinate.
*/

inline real GraphUI::fxy_t::rx(int sx, real y, int dy)
{
	return real(sx) * stp.x - (y - max.y) * stp.x / stp.y - (dy * 2 / 5) * stp.x + min.x;
}


/**
	Real Y coordinate from any screen Y position and real Z coordinate.

	@return		Real Y coordinate.
*/

inline real GraphUI::fxy_t::ry(int sy, real z, int dy)
{
	return - real(sy) * stp.y - (z - min.z) * stp.y / stp.z + (dy * 3 / 5) * stp.y + max.y;
}

bool GraphUI::init_fxy(int a_dx, int a_dy)
{
	dx = a_dx;
	dy = a_dy;

	if (dx <= 20 || dy <= 20) 
		return false;

	cx = dx - dy * 2 / 5;
	cy = dy * 2 / 5;

	fxy.stp.x = (fxy.max.x - fxy.min.x) / cx;
	fxy.stp.y = (fxy.max.y - fxy.min.y) / cy;
	fxy.stp.z = (fxy.max.z - fxy.min.z) / (dy * 3 / 5);

	fxy.axis = point<3>(0 < fxy.min.x ? fxy.min.x : 0 > fxy.max.x ? fxy.max.x : 0, 0 < fxy.min.y ? fxy.min.y : 0 > fxy.max.y ? fxy.max.y : 0, 0 < fxy.min.z ? fxy.min.z : 0 > fxy.max.z ? fxy.max.z : 0);

	fxy.top[0] = point<2, int>(fxy.sx(fxy.max.x, fxy.axis.y, dy), fxy.sy(fxy.axis.y, fxy.axis.z, dy));
	fxy.top[1] = point<2, int>(fxy.sx(fxy.axis.x, fxy.max.y, dy), fxy.sy(fxy.max.y, fxy.axis.z, dy));
	fxy.top[2] = point<2, int>(fxy.sx(fxy.axis.x, fxy.axis.y, dy), fxy.sy(fxy.axis.y, fxy.max.z, dy));

	return true;
}

#if 1
void GraphUI::draw_fxy(QPainter & painter, int a_dx, int a_dy, bool init)
{
    if (init)
        if (! init_fxy(a_dx, a_dy))
            return;

    // screen precision
    if (GraphUI::fast != true)
    {
        precisiony = 4.0L;
    }
    else
    {
        precisiony = benchmark / 300.0L + 4;
    }

    // Screen precision
    if (! option.grid)
    {
        precisionx = 2;
        precisiony = 1;
    }
    else
    {
        precisionx = option.precision;
        precisiony = option.precision / 2 ? : 1;
    }

    if (option.x.grid)
    {
        // Draw YZ grid
        QPolygon a(4);

        painter.setPen(QPen(pColor[option.x.color], 1));
/*
        a.setPoint(0, fxy.sx(fxy.min.x, fxy.min.y, dy), fxy.sy(fxy.min.y, fxy.min.z, dy));
        a.setPoint(1, fxy.sx(fxy.min.x, fxy.min.y, dy), fxy.sy(fxy.min.y, fxy.max.z, dy));
        a.setPoint(2, fxy.sx(fxy.min.x, fxy.max.y, dy), fxy.sy(fxy.max.y, fxy.max.z, dy));
        a.setPoint(3, fxy.sx(fxy.min.x, fxy.max.y, dy), fxy.sy(fxy.max.y, fxy.min.z, dy));

        painter.drawConvexPolygon(a);
*/
        for (real m = fdi::floor(fxy.max.y / fxy.scl.y) * fxy.scl.y; m >= fxy.min.y; m -= fxy.scl.y)
        {
            painter.drawLine(fxy.sx(fxy.min.x, m, dy), fxy.sy(m, fxy.min.z, dy), fxy.sx(fxy.min.x, m, dy), fxy.sy(m, fxy.max.z, dy));
        }

        for (real m = fdi::floor(fxy.max.z / fxy.scl.z) * fxy.scl.z; m >= fxy.min.z; m -= fxy.scl.z)
        {
            painter.drawLine(fxy.sx(fxy.min.x, fxy.min.y, dy), fxy.sy(fxy.min.y, m, dy), fxy.sx(fxy.min.x, fxy.max.y, dy), fxy.sy(fxy.max.y, m, dy));
        }
    }

    if (option.y.grid)
    {
        // Draw XZ grid
        QPolygon a(4);

        painter.setPen(QPen(pColor[option.y.color], 1));
/*
        a.setPoint(0, fxy.sx(fxy.min.x, fxy.max.y, dy), fxy.sy(fxy.max.y, fxy.min.z, dy));
        a.setPoint(1, fxy.sx(fxy.max.x, fxy.max.y, dy), fxy.sy(fxy.max.y, fxy.min.z, dy));
        a.setPoint(2, fxy.sx(fxy.max.x, fxy.max.y, dy), fxy.sy(fxy.max.y, fxy.max.z, dy));
        a.setPoint(3, fxy.sx(fxy.min.x, fxy.max.y, dy), fxy.sy(fxy.max.y, fxy.max.z, dy));

        painter.drawConvexPolygon(a);
*/
        for (real m = fdi::floor(fxy.max.x / fxy.scl.x) * fxy.scl.x; m >= fxy.min.x; m -= fxy.scl.x)
        {
            painter.drawLine(fxy.sx(m, fxy.max.y, dy), fxy.sy(fxy.max.y, fxy.min.z, dy), fxy.sx(m, fxy.max.y, dy), fxy.sy(fxy.max.y, fxy.max.z, dy));
        }

        for (real m = fdi::floor(fxy.max.z / fxy.scl.z) * fxy.scl.z; m >= fxy.min.z; m -= fxy.scl.z)
        {
            painter.drawLine(fxy.sx(fxy.min.x, fxy.max.y, dy), fxy.sy(fxy.max.y, m, dy), fxy.sx(fxy.max.x, fxy.max.y, dy), fxy.sy(fxy.max.y, m, dy));
        }
    }

    if (option.z.grid)
    {
        // Draw XY grid
        QPolygon a(4);

        painter.setPen(QPen(pColor[option.z.color], 1));
/*
        a.setPoint(0, fxy.sx(fxy.min.x, fxy.min.y, dy), fxy.sy(fxy.min.y, fxy.min.z, dy));
        a.setPoint(1, fxy.sx(fxy.max.x, fxy.min.y, dy), fxy.sy(fxy.min.y, fxy.min.z, dy));
        a.setPoint(2, fxy.sx(fxy.max.x, fxy.max.y, dy), fxy.sy(fxy.max.y, fxy.min.z, dy));
        a.setPoint(3, fxy.sx(fxy.min.x, fxy.max.y, dy), fxy.sy(fxy.max.y, fxy.min.z, dy));

        painter.drawConvexPolygon(a);
*/
        for (real m = fdi::floor(fxy.max.y / fxy.scl.y) * fxy.scl.y; m >= fxy.min.y; m -= fxy.scl.y)
        {
            painter.drawLine(fxy.sx(fxy.min.x, m, dy), fxy.sy(m, fxy.min.z, dy), fxy.sx(fxy.max.x, m, dy), fxy.sy(m, fxy.min.z, dy));
        }

        for (real m = fdi::floor(fxy.max.x / fxy.scl.x) * fxy.scl.x; m >= fxy.min.x; m -= fxy.scl.x)
        {
            painter.drawLine(fxy.sx(m, fxy.min.y, dy), fxy.sy(fxy.min.y, fxy.min.z, dy), fxy.sx(m, fxy.max.y, dy), fxy.sy(fxy.max.y, fxy.min.z, dy));
        }
    }

    painter.setFont(option.scale_font);
    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(Qt::black);

    if (fxy.draw[1] = option.y.axis || (fxy.axis.x == 0 || fxy.axis.x == 0) && (fxy.axis.z == 0 || fxy.axis.z == 0))
    {
        // Draw Y arrow
        painter.drawLine(fxy.top[1].x, fxy.top[1].y, fxy.top[1].x + 10, fxy.top[1].y - 10);

        QPolygon a(3);
        a.setPoint(0, QPoint(fxy.top[1].x + 10 - 2, fxy.top[1].y - 10 - 2));
        a.setPoint(1, QPoint(fxy.top[1].x + 10 + 8, fxy.top[1].y - 10 - 8));
        a.setPoint(2, QPoint(fxy.top[1].x + 10 + 2, fxy.top[1].y - 10 + 2));

        painter.drawConvexPolygon(a);
    }

    std::vector< std::vector< point<3> > > c(cx, std::vector< point<3> >(2));

    for (real y = fxy.max.y; y >= fxy.min.y; y -= fxy.stp.y * precisiony)
    {
        if (fxy.axis.y <= y + fxy.stp.y * precisiony && y < fxy.axis.y)
        {
            painter.setPen(QPen(Qt::black, 2));
            painter.setBrush(Qt::black);

            if (fxy.draw[0] = option.x.axis || (fxy.axis.y == 0 || fxy.axis.y == 0) && (fxy.axis.z == 0 || fxy.axis.z == 0))
            {
                // Draw X axis
                painter.drawLine(fxy.sx(fxy.min.x, fxy.axis.y, dy), fxy.sy(fxy.axis.y, fxy.axis.z, dy), fxy.top[0].x + 10, fxy.top[0].y);

                // Draw X arrow
                QPolygon a(3);
                a.setPoint(0, QPoint(fxy.top[0].x + 10, fxy.top[0].y - 3));
                a.setPoint(1, QPoint(fxy.top[0].x + 20, fxy.top[0].y));
                a.setPoint(2, QPoint(fxy.top[0].x + 10, fxy.top[0].y + 3));

                painter.drawConvexPolygon(a);

                // Draw X numbers
                for (real m = fdi::floor(fxy.max.x / fxy.scl.x) * fxy.scl.x; option.x.numbers && m >= fxy.min.x; m -= fxy.scl.x)
                {
                    QString const number = QStreamString(m);
                    QPoint const pixel = QPoint(fxy.sx(m, fxy.axis.y, dy), fxy.sy(fxy.axis.y, fxy.axis.z, dy) - 1);
                    QRect const rect = painter.fontMetrics().boundingRect(number);

                    painter.drawLine(pixel.x(), pixel.y(), pixel.x(), pixel.y() - 2);
                    painter.drawText(pixel.x() - rect.width() / 2, pixel.y() - 4, number);
                }
            }

            if (fxy.draw[2] = option.z.axis || (fxy.axis.y == 0 || fxy.axis.y == 0) && (fxy.axis.x == 0 || fxy.axis.x == 0))
            {
                // Draw Z axis
                painter.drawLine(fxy.sx(fxy.axis.x, fxy.axis.y, dy), fxy.sy(fxy.axis.y, fxy.min.z, dy), fxy.top[2].x, fxy.top[2].y - 10);

                // Draw Z arrow
                QPolygon a(3);
                a.setPoint(0, QPoint(fxy.top[2].x - 3, fxy.top[2].y - 10));
                a.setPoint(1, QPoint(fxy.top[2].x, fxy.top[2].y - 20));
                a.setPoint(2, QPoint(fxy.top[2].x + 3, fxy.top[2].y - 10));

                painter.drawConvexPolygon(a);

                // Draw Z numbers
                for (real m = fdi::floor(fxy.max.z / fxy.scl.z) * fxy.scl.z; option.z.numbers && m >= fxy.min.z; m -= fxy.scl.z)
                {
                    QString const number = QStreamString(m);
                    QPoint const pixel = QPoint(fxy.sx(fxy.axis.x, fxy.axis.y, dy), fxy.sy(fxy.axis.y, m, dy));
                    QRect const rect = painter.fontMetrics().boundingRect(number);

                    painter.drawLine(pixel.x(), pixel.y(), pixel.x() + 2, pixel.y());
                    painter.drawText(pixel.x() + 6, pixel.y() + rect.height() / 2 - 4, number);
                }
            }
        }

        int e = 0;

        for (real x = fxy.min.x; x <= fxy.max.x; x += fxy.stp.x * precisionx, ++ e)
        {
            c[e][0] = c[e][1];
            c[e][1] = point<3>(x, y, (* fxy.plot)(x, y));

            if ((x != fxy.min.x && y != fxy.max.y) && ! (isnan(c[e][1].z) || isnan(c[e - 1][1].z) || isnan(c[e][0].z) || isnan(c[e - 1][0].z)))
            {
                if ((c[e][1].z >= fxy.min.z || c[e - 1][1].z >= fxy.min.z || c[e][0].z >= fxy.min.z || c[e - 1][0].z >= fxy.min.z) && (c[e][1].z <= fxy.max.z || c[e - 1][1].z <= fxy.max.z || c[e][0].z <= fxy.max.z || c[e - 1][0].z <= fxy.max.z))
                {
                    struct
                    {
                        static bool intersects(point<3> const & z, point<3> const & v1, point<3> const & v2)
                        {
                            return (v2.z - v1.z) / (z.z - v1.z) > 1;
                        }

                        static point<3> intersection(point<3> const & z, point<3> const & v1, point<3> const & v2)
                        {
                            point<3> h = z - v1;
                            point<3> e = v2 - v1;

                            return point<3>(e.x * h.z / e.z + v1.x, e.y * h.z / e.z + v1.y, z.z);
                        }

                        std::vector< point<3> > operator () (std::vector< point<3> > & polygon, point<3> const & lim, bool upper)
                        {
                            std::vector< point<3> > cliped;
                            cliped.reserve(polygon.capacity());

                            for (int i = 0; i < polygon.size(); ++ i)
                            {
                                int j = (i + 1) % polygon.size();

                                if (upper ? (polygon[i].z > lim.z) : (polygon[i].z < lim.z))
                                {
                                    if (intersects(lim, polygon[i], polygon[j]))
                                    {
                                        cliped.push_back(intersection(lim, polygon[i], polygon[j]));
                                    }
                                }
                                else
                                {
                                    cliped.push_back(polygon[i]);

                                    if (intersects(lim, polygon[i], polygon[j]))
                                    {
                                        cliped.push_back(intersection(lim, polygon[i], polygon[j]));
                                    }
                                }
                            }

                            return cliped;
                        }
                    } clip;

                    QColor tint = pColor[option.color];

                    std::vector< point<3> > polygon;

                    polygon.reserve(6);
                    polygon.push_back(c[e - 1][0]);
                    polygon.push_back(c[e][0]);
                    polygon.push_back(c[e][1]);
                    polygon.push_back(c[e - 1][1]);

                    polygon = clip(polygon, fxy.min, false);
                    polygon = clip(polygon, fxy.max, true);

                    QPolygon a(polygon.size());

                    for (int i = 0; i < polygon.size(); ++ i)
                    {
                        a.setPoint(i, fxy.sx(polygon[i].x, polygon[i].y, dy), fxy.sy(polygon[i].y, polygon[i].z, dy));
                    }

                    if (! option.lightning)
                    {
                        painter.setPen(tint);
                        painter.setBrush(tint);
                    }
                    else
                    {
                        point<3> u = c[e - 1][0] - c[e][0];
                        point<3> v = c[e][1] - c[e][0];
                        point<3> w = u * v;

                        real dot = w.dot(point<3>(1, -1, 1));
                        real length = w.length();

                        if (dot > 0)
                        {
                            painter.setPen(tint.light(dot * 40  / length + 110));
                            painter.setBrush(tint.light(dot * 40  / length + 110));
                        }
                        else
                        {
                            painter.setPen(tint.light(dot * 40 / length + 90));
                            painter.setBrush(tint.light(dot * 40 / length + 90));
                        }
                    }

                    if (option.grid)
                    {
                        painter.setBrush(Qt::NoBrush);
                    }

                    painter.drawConvexPolygon(a);
                }
            }
        }

        painter.setPen(QPen(Qt::black, 2));
        painter.setBrush(Qt::black);

        if (option.y.axis || (fxy.axis.x == 0 || fxy.axis.x == 0) && (fxy.axis.z == 0 || fxy.axis.z == 0))
        {
            // Draw Y axis
            painter.drawLine(fxy.sx(fxy.axis.x, y, dy), fxy.sy(y, fxy.axis.z, dy), fxy.sx(fxy.axis.x, y + fxy.stp.y * precisiony, dy), fxy.sy(y + fxy.stp.y * precisiony, fxy.axis.z, dy));

            // Draw Y numbers
            for (real m = fdi::floor(y / fxy.scl.y) * fxy.scl.y; option.y.numbers && m > y - fxy.stp.y * precisiony; m -= fxy.scl.y)
            {
                QString const number = QStreamString(m);
                QPoint const pixel = QPoint(fxy.sx(fxy.axis.x, m, dy), fxy.sy(m, fxy.axis.z, dy));
                QRect const rect = painter.fontMetrics().boundingRect(number);

                painter.drawLine(pixel.x(), pixel.y(), pixel.x() + 4, pixel.y());
                painter.drawText(pixel.x() + 8, pixel.y() + rect.height() / 2 - 4, number);
            }
        }
    }
}

#else

void GraphUI::draw_fxy(QPainter & painter, int a_dx, int a_dy, bool init)
{
	if (init) 
		if (! init_fxy(a_dx, a_dy)) 
			return;

	// screen precision
    if (GraphUI::fast != true)
	{
		precisiony = 4.0L;
	}
	else
	{
		precisiony = benchmark / 300.0L + 4;
	}

	precisionx = precisiony * 2;

	point<3> d = fxy.max - fxy.min;
    std::vector< std::vector< point<3> > > cross(cx, std::vector< point<3> >(2));
    std::vector< std::vector< point<3> > > normal(cx, std::vector< point<3> >(2));
    std::vector< std::vector< point<3> > > vertex(cx, std::vector< point<3> >(3));

	// draw graph
	glEnable((GLenum) (GL_CLIP_PLANE0));
	glEnable((GLenum) (GL_CLIP_PLANE1));

    QColor cTint = pColor[option.color];
	GLfloat aTint[] = {cTint.red() / 384.0, cTint.green() / 384.0, cTint.blue() / 384.0, 1.0};
	//GLfloat aShininess[] = {option.lightning ? 128.0 : 1.0};
	
	glLineWidth(1.0);
    //pGLWidget->qglColor();
	glEnable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);
	glMaterialfv(GL_FRONT, GL_AMBIENT, aTint);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, aTint);
	//glMaterialfv(GL_FRONT, GL_SPECULAR, (GLfloat []) {0.0, 0.0, 0.0, 1.0});
	//glMaterialfv(GL_FRONT, GL_SHININESS, aShininess);

	// start by precalculating a line
	int i = 0;

	for (real y = fxy.max.y + fxy.stp.y * precisiony; y >= fxy.min.y - fxy.stp.y * precisiony; y -= fxy.stp.y * precisiony, ++ i)
	{
		if (i > 2)
		{
            glBegin(GL_TRIANGLE_STRIP);
		}

		int j = 0;

		for (real x = fxy.min.x - fxy.stp.x * precisionx; x <= fxy.max.x + fxy.stp.x * precisionx; x += fxy.stp.x * precisionx, ++ j)
		{
			vertex[j][2] = vertex[j][1];
			vertex[j][1] = vertex[j][0];
			vertex[j][0] = point<3>(x, y, (* fxy.plot)(x, y));

			if (i > 0 && j > 0)
			{
				point<3> u = vertex[j - 1][0] - vertex[j][1];
				point<3> v = vertex[j - 1][1] - vertex[j][0];
				point<3> w = u * v;

				cross[j][1] = cross[j][0];
				cross[j][0] = w / w.length();

				normal[j][1] = normal[j][0];
				
				if (! option.lightning)
					normal[j][0] = point<3>(-1.0, -1.0, -1.0);
				else
					normal[j][0] = (cross[j - 1][0] + cross[j - 1][1] + cross[j][0] + cross[j][1]) / 4;

				if (i > 2 && j > 1)
				{
					GLdouble z[] = {double((vertex[j - 1][1].z - fxy.min.z) / d.z), double((vertex[j - 1][2].z - fxy.min.z) / d.z)};

					// crash check (to be revised)
					if (z[0] > 100.0) 
						z[0] = 100.0;
					else if (z[0] < -100.0) 
						z[0] = -100.0;

					if (z[1] > 100.0) 
						z[1] = 100.0;
					else if (z[1] < -100.0) 
						z[1] = -100.0;

					glNormal3d(normal[j][1].x, normal[j][1].y, normal[j][1].z);
				    glVertex3d((vertex[j - 1][2].x - fxy.min.x) / d.x, (vertex[j - 1][2].y - fxy.min.y) / d.y, z[1]);
					glNormal3d(normal[j][0].x, normal[j][0].y, normal[j][0].z);
				    glVertex3d((vertex[j - 1][1].x - fxy.min.x) / d.x, (vertex[j - 1][1].y - fxy.min.y) / d.y, z[0]);
				}
			}
		}

		if (i > 2)
		{
		    glEnd();
		}
	}

	if (! init) 
		return;

	// draw grids
	glDisable((GLenum) (GL_CLIP_PLANE0));
	glDisable((GLenum) (GL_CLIP_PLANE1));

    glLineWidth(1.0);
	glDisable(GL_LIGHTING);
    pGLWidget->qglColor(Qt::gray);

	if (option.x.grid)
	{
	    pGLWidget->qglColor(pColor[option.x.color]);

        for (real m = fdi::floor(fxy.max.y / fxy.scl.y) * fxy.scl.y; m >= fxy.min.y; m -= fxy.scl.y)
		{
            glBegin(GL_LINES);
            glVertex3f(0, (m - fxy.min.y) / d.y, (fxy.min.z - fxy.min.z) / d.z);
		    glVertex3f(0, (m - fxy.min.y) / d.y, (fxy.max.z - fxy.min.z) / d.z);
            glEnd();
        }

        for (real m = fdi::floor(fxy.max.z / fxy.scl.z) * fxy.scl.z; m >= fxy.min.z; m -= fxy.scl.z)
		{
            glBegin(GL_LINES);
            glVertex3f(0, (fxy.min.y - fxy.min.y) / d.y, (m - fxy.min.z) / d.z);
		    glVertex3f(0, (fxy.max.y - fxy.min.y) / d.y, (m - fxy.min.z) / d.z);
            glEnd();
        }
	}

	if (option.y.grid)
	{
	    pGLWidget->qglColor(pColor[option.y.color]);

        for (real m = fdi::floor(fxy.max.x / fxy.scl.x) * fxy.scl.x; m >= fxy.min.x; m -= fxy.scl.x)
		{
            glBegin(GL_LINES);
            glVertex3f((m - fxy.min.x) / d.x, (fxy.max.y - fxy.min.y) / d.y, (fxy.min.z - fxy.min.z) / d.z);
		    glVertex3f((m - fxy.min.x) / d.x, (fxy.max.y - fxy.min.y) / d.y, (fxy.max.z - fxy.min.z) / d.z);
		    glEnd();
		}

        for (real m = fdi::floor(fxy.max.z / fxy.scl.z) * fxy.scl.z; m >= fxy.min.z; m -= fxy.scl.z)
		{
            glBegin(GL_LINES);
            glVertex3f((fxy.min.x - fxy.min.x) / d.x, (fxy.max.y - fxy.min.y) / d.y, (m - fxy.min.z) / d.z);
		    glVertex3f((fxy.max.x - fxy.min.x) / d.x, (fxy.max.y - fxy.min.y) / d.y, (m - fxy.min.z) / d.z);
            glEnd();
        }
	}

	if (option.z.grid)
	{
	    pGLWidget->qglColor(pColor[option.z.color]);

        for (real m = fdi::floor(fxy.max.y / fxy.scl.y) * fxy.scl.y; m >= fxy.min.y; m -= fxy.scl.y)
		{
            glBegin(GL_LINES);
            glVertex3f((fxy.min.x - fxy.min.x) / d.x, (m - fxy.min.y) / d.y, 0);
		    glVertex3f((fxy.max.x - fxy.min.x) / d.x, (m - fxy.min.y) / d.y, 0);
            glEnd();
        }

        for (real m = fdi::floor(fxy.max.x / fxy.scl.x) * fxy.scl.x; m >= fxy.min.x; m -= fxy.scl.x)
		{
            glBegin(GL_LINES);
            glVertex3f((m - fxy.min.x) / d.x, (fxy.min.y - fxy.min.y) / d.y, 0);
		    glVertex3f((m - fxy.min.x) / d.x, (fxy.max.y - fxy.min.y) / d.y, 0);
            glEnd();
        }
	}

	// draw axises
    glLineWidth(2.0);
	glDisable(GL_LIGHTING);
    pGLWidget->qglColor(Qt::black);

	real rx = real(1) / cx;
	real ry = real(1) / cy;
	real rz = real(1) / (dy * 3 / 5);

	if (fxy.draw[0] = option.x.axis)
	{
		// draw x axis
        glBegin(GL_LINES);
        glVertex3f(0 / d.x, (fxy.axis.y - fxy.min.y) / d.y, (fxy.axis.z - fxy.min.z) / d.z);
	    glVertex3f((fxy.max.x - fxy.min.x) / d.x, (fxy.axis.y - fxy.min.y) / d.y, (fxy.axis.z - fxy.min.z) / d.z);
        glEnd();

		// draw x arrow
        glBegin(GL_QUADS);
        glVertex3f((fxy.max.x - fxy.min.x) / d.x, (fxy.axis.y - fxy.min.y) / d.y, (fxy.axis.z - fxy.min.z) / d.z - 4 * rz);
	    glVertex3f((fxy.max.x - fxy.min.x) / d.x, (fxy.axis.y - fxy.min.y) / d.y, (fxy.axis.z - fxy.min.z) / d.z + 4 * rz);
	    glVertex3f((fxy.max.x - fxy.min.x) / d.x + 10 * rx, (fxy.axis.y - fxy.min.y) / d.y, (fxy.axis.z - fxy.min.z) / d.z + 1 * rz);
	    glVertex3f((fxy.max.x - fxy.min.x) / d.x + 10 * rx, (fxy.axis.y - fxy.min.y) / d.y, (fxy.axis.z - fxy.min.z) / d.z - 1 * rz);
        glEnd();

        real const magnitude = fdi::floor(log10(fabs(fxy.max.z))) - 2;

		// draw x numbers
        for (real m = fdi::floor(fxy.max.x / fxy.scl.x) * fxy.scl.x; option.x.numbers && m >= fxy.min.x; m -= fxy.scl.x)
		{
            QString const number = QStreamString(rint(m / pow(10.0L, magnitude)) * pow(10.0L, magnitude));
			QRect const rect = painter.fontMetrics().boundingRect(number);

            glBegin(GL_LINES);
            glVertex3f((m - fxy.min.x) / d.x, (fxy.axis.y - fxy.min.y) / d.y, (fxy.axis.z - fxy.min.z) / d.z - 2 * rz);
		    glVertex3f((m - fxy.min.x) / d.x, (fxy.axis.y - fxy.min.y) / d.y, (fxy.axis.z - fxy.min.z) / d.z + 2 * rz);
		    glEnd();

			pGLWidget->renderText((m - fxy.min.x) / d.x - rect.width() / 2 * rx, (fxy.axis.y - fxy.min.y) / d.y - 4 * ry, (fxy.axis.z - fxy.min.z) / d.z + (2 - rect.height()) * rz, number);
			//pGLWidget->renderText((m - fxy.min.x) / d.x, (fxy.axis.y - fxy.min.y) / d.y - 6 * ry, (fxy.axis.z - fxy.min.z) / d.z + (2 - rect.height()) * rz, number);
		}
	}

	if (fxy.draw[1] = option.y.axis)
	{
		// draw y axis
        glBegin(GL_LINES);
        glVertex3f((fxy.axis.x - fxy.min.x) / d.x, 0 / d.y, (fxy.axis.z - fxy.min.z) / d.z);
	    glVertex3f((fxy.axis.x - fxy.min.x) / d.x, (fxy.max.y - fxy.min.y) / d.y, (fxy.axis.z - fxy.min.z) / d.z);
        glEnd();

		// draw y arrow
	    glBegin(GL_QUADS);
	    glVertex3f((fxy.axis.x - fxy.min.x) / d.x - 4 * rx, (fxy.max.y - fxy.min.y) / d.y, (fxy.axis.z - fxy.min.z) / d.z);
	    glVertex3f((fxy.axis.x - fxy.min.x) / d.x + 4 * rx, (fxy.max.y - fxy.min.y) / d.y, (fxy.axis.z - fxy.min.z) / d.z);
	    glVertex3f((fxy.axis.x - fxy.min.x) / d.x + 1 * rx, (fxy.max.y - fxy.min.y) / d.y + 8 * ry, (fxy.axis.z - fxy.min.z) / d.z);
	    glVertex3f((fxy.axis.x - fxy.min.x) / d.x - 1 * rx, (fxy.max.y - fxy.min.y) / d.y + 8 * ry, (fxy.axis.z - fxy.min.z) / d.z);
	    glEnd();

        real const magnitude = fdi::floor(log10(fabs(fxy.max.z))) - 2;

		// draw y numbers
        for (real m = fdi::floor(fxy.max.y / fxy.scl.y) * fxy.scl.y; option.y.numbers && m >= fxy.min.y; m -= fxy.scl.y)
		{
            QString const number = QStreamString(rint(m / pow(10.0L, magnitude)) * pow(10.0L, magnitude));
			QRect const rect = painter.fontMetrics().boundingRect(number);

            glBegin(GL_LINES);
            glVertex3f((fxy.axis.x - fxy.min.x) / d.x - 2 * rx, (m - fxy.min.y) / d.y, (fxy.axis.z - fxy.min.z) / d.z);
		    glVertex3f((fxy.axis.x - fxy.min.x) / d.x + 2 * rx, (m - fxy.min.y) / d.y, (fxy.axis.z - fxy.min.z) / d.z);
            glEnd();

			pGLWidget->renderText((fxy.axis.x - fxy.min.x) / d.x - (6 + rect.width()) * rx, (m - fxy.min.y) / d.y, (fxy.axis.z - fxy.min.z) / d.z - (rect.height() / 2 - 2) * rz, number);
		}
	}

	if (fxy.draw[2] = option.z.axis)
	{
		// draw z axis
        glBegin(GL_LINES);
        glVertex3f((fxy.axis.x - fxy.min.x) / d.x, (fxy.axis.y - fxy.min.y) / d.y, 0 / d.z);
	    glVertex3f((fxy.axis.x - fxy.min.x) / d.x, (fxy.axis.y - fxy.min.y) / d.y, (fxy.max.z - fxy.min.z) / d.z);
        glEnd();

		// draw z arrow
        glBegin(GL_QUADS);
        glVertex3f((fxy.axis.x - fxy.min.x) / d.x - 4 * rx, (fxy.axis.y - fxy.min.y) / d.y, (fxy.max.z - fxy.min.z) / d.z);
	    glVertex3f((fxy.axis.x - fxy.min.x) / d.x + 4 * rx, (fxy.axis.y - fxy.min.y) / d.y, (fxy.max.z - fxy.min.z) / d.z);
	    glVertex3f((fxy.axis.x - fxy.min.x) / d.x + 1 * rx, (fxy.axis.y - fxy.min.y) / d.y, (fxy.max.z - fxy.min.z) / d.z + 10 * rz);
	    glVertex3f((fxy.axis.x - fxy.min.x) / d.x - 1 * rx, (fxy.axis.y - fxy.min.y) / d.y, (fxy.max.z - fxy.min.z) / d.z + 10 * rz);
	    glEnd();

        real const magnitude = fdi::floor(log10(fabs(fxy.max.z))) - 2;

		// draw z numbers
        for (real m = fdi::floor(fxy.max.z / fxy.scl.z) * fxy.scl.z; option.z.numbers && m >= fxy.min.z; m -= fxy.scl.z)
		{
            QString const number = QStreamString(rint(m / pow(10.0L, magnitude)) * pow(10.0L, magnitude));
			QRect const rect = painter.fontMetrics().boundingRect(number);

            glBegin(GL_LINES);
            glVertex3f((fxy.axis.x - fxy.min.x) / d.x - 2 * rx, (fxy.axis.y - fxy.min.y) / d.y, (m - fxy.min.z) / d.z);
		    glVertex3f((fxy.axis.x - fxy.min.x) / d.x + 2 * rx, (fxy.axis.y - fxy.min.y) / d.y, (m - fxy.min.z) / d.z);
            glEnd();

			pGLWidget->renderText((fxy.axis.x - fxy.min.x) / d.x - (6 + rect.width()) * rx, (fxy.axis.y - fxy.min.y) / d.y, (m - fxy.min.z) / d.z - (rect.height() / 2 - 2) * rz, number);
		}
	}
}
#endif

void GraphUI::draw_marker(int x, int y)
{
	QPainter painter(this);

	///painter.setRasterOp(Qt::NotROP);
	painter.setPen(QPen(Qt::black, 3));

	painter.drawPoint(x, y);
	painter.drawLine(x - 15, y, x - 5, y);
	painter.drawLine(x + 5, y, x + 15, y);
	painter.drawLine(x, y - 15, x, y - 5);
	painter.drawLine(x, y + 5, x, y + 15);

	painter.setPen(QPen(Qt::black, 1));

	switch (option.mode)
	{
	case e2d:
		if (option.x.trace)
		{
			painter.drawLine(x, y + 5, x, y + 15);
		}

		if (option.y.trace)
		{
			painter.drawLine(x, y + 5, x, y + 15);
		}
		break;

	case e3d:
		if (option.x.trace)
		{
			painter.drawLine(fxy.sx(fxy.axis.x, fxy.trace.y, dy), fxy.sy(fxy.trace.y, fxy.axis.z, dy), fxy.sx(fxy.trace.x, fxy.trace.y, dy), fxy.sy(fxy.trace.y, fxy.axis.z, dy));
		}

		if (option.y.trace)
		{
			painter.drawLine(fxy.sx(fxy.trace.x, fxy.axis.y, dy), fxy.sy(fxy.axis.y, fxy.axis.z, dy), fxy.sx(fxy.trace.x, fxy.trace.y, dy), fxy.sy(fxy.trace.y, fxy.axis.z, dy));
		}

		if (option.z.trace)
		{
			painter.drawLine(fxy.sx(fxy.trace.x, fxy.trace.y, dy), fxy.sy(fxy.trace.y, fxy.axis.z, dy), fxy.sx(fxy.trace.x, fxy.trace.y, dy), fxy.sy(fxy.trace.y, fxy.trace.z, dy));
		}
		break;

	default:
		break;
	}
}

void GraphUI::timeout()
{
	on = ! on;

	update();
}

void bitBlt( QWidget* dst, int x, int y, const QPixmap* src )
{
    QPainter p( dst );
    p.drawPixmap( x, y, *src );
}

void GraphUI::paintEvent(QPaintEvent * a_event)
{
    QPainter p(this);

    if (option.mode == GraphUI::e2d || option.stat != GraphUI::eScientific)
    {
        ///pixmap = pixmap.copy(QRect(QPoint(0, 0), rect[4].size()));
        ///pixmap.fill(palette().color(QWidget::backgroundRole()));

        QPainter & painter = p;

        stack[2].push_back(variable_t()), (mutable_ptr<type> &) stack[2].back() = (mutable_ptr<type> &) (variable[(int) ('x')]);
        (mutable_ptr<type> &) (variable[(int) ('x')]) = new type_t<floating>(0);

        // plot multiple graphics
        if (type_t< fdi::vector<type_p> > const * r = dynamic_cast<type_t< fdi::vector<type_p> > const *>(variable[(int) ('y')].get()))
        {
            stack[2].push_back(variable_t()), (mutable_ptr<type> &) stack[2].back() = (mutable_ptr<type> &) (variable[(int) ('y')]);

            for (unsigned int i = 0; i < r->value.size(); ++ i)
            {
                (mutable_ptr<type> &) (variable[(int) ('y')]) = dynamic_cast<type_t< fdi::vector<type_p> > const *>(stack[2].back().get())->value[i];

                draw_fx(painter, (int) width(), (int) height(), i == 0);
            }

            (mutable_ptr<type> &) (variable[(int) ('y')]) = (mutable_ptr<type> &) stack[2].back(), stack[2].pop_back();
        }

        // plot single graphic
        else
        {
            draw_fx(painter, (int) width(), (int) height());
        }

        (mutable_ptr<type> &) (variable[(int) ('x')]) = (mutable_ptr<type> &) stack[2].back(), stack[2].pop_back();
    }
    else if (option.mode == GraphUI::e3d)
    {
        ///pixmap = pixmap.copy(QRect(QPoint(0, 0), rect[4].size()));
        ///pixmap.fill(palette().color(QWidget::backgroundRole()));

        QPainter & painter = p;

        stack[2].push_back(variable_t()), (mutable_ptr<type> &) stack[2].back() = (mutable_ptr<type> &) (variable[(int) ('x')]);
        stack[2].push_back(variable_t()), (mutable_ptr<type> &) stack[2].back() = (mutable_ptr<type> &) (variable[(int) ('y')]);
        (mutable_ptr<type> &) (variable[(int) ('x')]) = new type_t<floating>(0);
        (mutable_ptr<type> &) (variable[(int) ('y')]) = new type_t<floating>(0);

        //painter.drawPixmap(0, 0, pGLWidget->renderPixmap((int) width(), (int) height()));

        draw_fxy(painter, (int) width(), (int) height());

        (mutable_ptr<type> &) (variable[(int) ('y')]) = (mutable_ptr<type> &) stack[2].back(), stack[2].pop_back();
        (mutable_ptr<type> &) (variable[(int) ('x')]) = (mutable_ptr<type> &) stack[2].back(), stack[2].pop_back();
    }

	if (on)
	{
		switch (option.mode)
		{
		case e2d:
			draw_marker(fx.sx(fx.trace.x, dy), fx.sy(fx.trace.y, dy));
			break;

		case e3d:
			draw_marker(fxy.sx(fxy.trace.x, fxy.trace.y, dy), fxy.sy(fxy.trace.y, fxy.trace.z, dy));
			break;

		default:
			break;
		}
	}

	QWidget::paintEvent(a_event);
}

void GraphUI::draw_report(QPainter & pixmap, int report)
{
	QRect rect[5];

	rect[4] = QRect(0, 0, (int) option.report[report].width, (int) option.report[report].height);

	if (option.mode == GraphUI::e2d || option.stat != GraphUI::eScientific)
	{
		if (! init_fx((int) option.report[report].width, (int) option.report[report].height)) 
			return;

		if (option.legend)
		{
			QFontMetrics fontmetrics(option.unit_font);

			rect[0] = fontmetrics.boundingRect(fx.top[0].x + 20 + 8, fx.top[0].y - 4, 0, 0, Qt::AlignLeft | Qt::AlignVCenter, option.x.unit);
            rect[4] = rect[4].united(rect[0]);

			rect[1] = fontmetrics.boundingRect(fx.top[1].x, fx.top[1].y - 20 - 8, 0, 0, Qt::AlignHCenter | Qt::AlignBottom, option.y.unit);
            rect[4] = rect[4].united(rect[1]);
		}

		if (option.header)
		{
			QFontMetrics fontmetrics(option.title_font);

			rect[3] = fontmetrics.boundingRect(int(option.report[report].width / 2), rect[1].y() - 40 - 4, 0, 0, Qt::AlignHCenter | Qt::AlignBottom, option.title);
            rect[4] = rect[4].united(rect[3]);
		}

		rect[4] = QRect(rect[4].topLeft() + QPoint(-10, -10), rect[4].size() + QSize(20, 20));

        ///pixmap = pixmap.copy(QRect(QPoint(0, 0), rect[4].size()));
        ///pixmap.fill(palette().color(QWidget::backgroundRole()));

        QPainter & painter = pixmap;

		if (rect[4].x() < 0) 
			painter.translate(abs(rect[4].x()), 0);
		if (rect[4].y() < 0) 
			painter.translate(0, abs(rect[4].y()));

		// plot multiple graphics
        if (type_t< fdi::vector<type_p> > const * r = dynamic_cast<type_t< fdi::vector<type_p> > const *>(variable[(int) ('y')].get()))
		{
			stack[2].push_back(variable_t()), (mutable_ptr<type> &) stack[2].back() = (mutable_ptr<type> &) (variable[(int) ('y')]);

			for (unsigned int i = 0; i < r->value.size(); ++ i)
			{
                (mutable_ptr<type> &) (variable[(int) ('y')]) = dynamic_cast<type_t< fdi::vector<type_p> > const *>(stack[2].back().get())->value[i];

				draw_fx(painter, (int) option.report[report].width, (int) option.report[report].height, i == 0);
			}

			(mutable_ptr<type> &) (variable[(int) ('y')]) = (mutable_ptr<type> &) stack[2].back(), stack[2].pop_back();
		}

		// plot single graphic
		else
		{
			draw_fx(painter, (int) option.report[report].width, (int) option.report[report].height);
		}

		painter.setPen(Qt::black);

		if (option.legend)
		{
			painter.setFont(option.unit_font);

			painter.drawText(rect[0].bottomLeft(), option.x.unit);
			painter.drawText(rect[1].bottomLeft(), option.y.unit);
		}

		if (option.header)
		{
			painter.setFont(option.title_font);

			painter.drawText(rect[3].bottomLeft(), option.title);
		}
	}
	else if (option.mode == GraphUI::e3d)
	{
		if (! init_fxy((int) option.report[report].width, (int) option.report[report].height)) return;

		if (option.legend)
		{
			QFontMetrics fontmetrics(option.unit_font);

			rect[0] = fontmetrics.boundingRect(fxy.top[0].x + 20 + 8, fxy.top[0].y + 4, 0, 0, Qt::AlignLeft | Qt::AlignVCenter, option.x.unit);
            rect[4] = rect[4].united(rect[0]);

			rect[1] = fontmetrics.boundingRect(fxy.top[1].x + 20 + 8, fxy.top[1].y - 20, 0, 0, Qt::AlignLeft | Qt::AlignBottom, option.y.unit);
            rect[4] = rect[4].united(rect[1]);

			rect[2] = fontmetrics.boundingRect(fxy.top[2].x - 4, fxy.top[2].y - 20 - 8, 0, 0, Qt::AlignHCenter | Qt::AlignBottom, option.z.unit);
            rect[4] = rect[4].united(rect[1]);
		}

		if (option.header)
		{
			QFontMetrics fontmetrics(option.title_font);

			rect[3] = fontmetrics.boundingRect(int(option.report[report].width / 2), rect[2].y() - 40 - 4, 0, 0, Qt::AlignHCenter | Qt::AlignBottom, option.title);
            rect[4] = rect[4].united(rect[3]);
		}

		rect[4] = QRect(rect[4].topLeft() + QPoint(-10, -10), rect[4].size() + QSize(20, 20));

        ///pixmap = pixmap.copy(QRect(QPoint(0, 0), rect[4].size()));
        ///pixmap.fill(palette().color(QWidget::backgroundRole()));

        QPainter & painter = pixmap;

		if (rect[4].x() < 0) 
			painter.translate(abs(rect[4].x()), 0);
		if (rect[4].y() < 0) 
			painter.translate(0, abs(rect[4].y()));

        //painter.drawPixmap(0, 0, pGLWidget->renderPixmap((int) option.report[report].width, (int) option.report[report].height));

        draw_fxy(painter, (int) option.report[report].width, (int) option.report[report].height, false);

		painter.setPen(Qt::black);

		if (option.legend)
		{
			painter.setFont(option.unit_font);

			painter.drawText(rect[0].bottomLeft(), option.x.unit);
			painter.drawText(rect[1].bottomLeft(), option.y.unit);
			painter.drawText(rect[2].bottomLeft(), option.z.unit);
		}

		if (option.header)
		{
			painter.setFont(option.title_font);

			painter.drawText(rect[3].bottomLeft(), option.title);
		}
	}
}

