TEMPLATE  = lib
LANGUAGE  = C++
CONFIG	 += staticlib 

TARGET    = cole

HEADERS	 += cole.h \
			cole.h.in \
			internal.h \
			support.h

SOURCES	 += cole.c \
			internal.c \
			olecod.c \
			oledecod.c \
			support.c \
			version.c

#The following line was inserted by qt3to4
QT +=  opengl
