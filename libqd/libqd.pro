TEMPLATE  = lib
LANGUAGE  = C++
CONFIG	 += staticlib 
INCLUDEPATH += ../include

TARGET    = qd

HEADERS	 += inline.h \
	    floating.h
SOURCES	 += \
    floating.cpp
#The following line was inserted by qt3to4
QT +=  opengl
