TEMPLATE	= lib
LANGUAGE	= C++

CONFIG	+= staticlib 

INCLUDEPATH	+= ../libqd ../include

HEADERS	+= graphui.h \
	spinboxui.h \
	tableui.h \
	worksheetui.h \
	matrixui.h \
	q3spinbox.h \
	q3rangecontrol.h

SOURCES	+= graphui.cpp \
	spinboxui.cpp \
	tableui.cpp \
	worksheetui.cpp \
	matrixui.cpp \
	q3spinbox.cpp \
	q3rangecontrol.cpp

#The following line was changed from FORMS to FORMS3 by qt3to4
FORMS3	= calculatorui.ui \
	graphicui.ui \
	rangeui.ui \
	variableui.ui \
	aboutui.ui \
	registerui.ui \
	setupui.ui \
	optionui.ui \
	childui.ui
	
RESOURCES     = images.qrc

TARGET    = ui

#The following line was inserted by qt3to4
#QT +=  opengl
#The following line was inserted by qt3to4
CONFIG += uic

#The following line was inserted by qt3to4
QT += widgets gui

FORMS += \
    calculatorui.ui \
    rangeui.ui \
    aboutui.ui \
    childui.ui \
    graphicui.ui \
    optionui.ui \
    setupui.ui \
    titleui.ui \
    variableui.ui
