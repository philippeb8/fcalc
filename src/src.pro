TEMPLATE  = app
LANGUAGE  = C++

INCLUDEPATH += ../include ../libparser ../libqd ../libui ../libcole
unix:LIBS   += -L../libparser -L../libqd -L../libui -L../libcole -lparser -lqd -lui -lcole #-lGL -lGLU
win32:LIBS  += -L../libparser/release -L../libqd/release -L../libui/release -L../libcole/release -L../lib parser.lib qd.lib ui.lib cole.lib -lhtmlhelp #-lopengl32 -lglu32

DEFINES  += EDITION=2

TARGET    = fcalc

RC_FILE   = fcalc.rc

HEADERS	 += app.h ui.h
SOURCES	 += calculus.cpp app.cpp ui.cpp main.cpp xlhtml.c

QT +=  printsupport

#QMAKE_CXXFLAGS+=-mfloat-abi=hard -mfpu=vfp

QMAKE_INFO_PLIST = fcalc.plist

ICON = fcalc.icns
