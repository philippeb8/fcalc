TEMPLATE  = app
LANGUAGE  = C++

INCLUDEPATH += include libui libqd libparser

DEFINES  += EDITION=2

TARGET    = fcalc

HEADERS	 += src/app.h src/ui.h
SOURCES	 += src/calculus.cpp src/app.cpp src/ui.cpp src/main.cpp

QT +=  printsupport

DEPLOYMENT.display_name = Calculator
QMAKE_INFO_PLIST = fcalc.plist
ICON = fcalc.icns




HEADERS	+= libui/graphui.h \
	libui/spinboxui.h \
	libui/tableui.h \
	libui/worksheetui.h \
	libui/matrixui.h \
	libui/q3spinbox.h \
	libui/q3rangecontrol.h

SOURCES	+= libui/graphui.cpp \
	libui/spinboxui.cpp \
	libui/tableui.cpp \
	libui/worksheetui.cpp \
	libui/matrixui.cpp \
	libui/q3spinbox.cpp \
	libui/q3rangecontrol.cpp
	
RESOURCES     = libui/images.qrc

CONFIG += uic

#The following line was inserted by qt3to4
QT += widgets gui

FORMS += \
    libui/calculatorui.ui \
    libui/rangeui.ui \
    libui/aboutui.ui \
    libui/childui.ui \
    libui/graphicui.ui \
    libui/optionui.ui \
    libui/setupui.ui \
    libui/titleui.ui \
    libui/variableui.ui

    
    
    
HEADERS	 += libqd/inline.h \
	    libqd/floating.h
SOURCES	 += \
    libqd/floating.cpp

    
    
    
    
HEADERS	 += libparser/parser_yacc.h libparser/parser_postfix_yacc.h
SOURCES	 += libparser/parser_yacc.cpp libparser/parser_postfix_yacc.cpp libparser/lexer_lex.cpp

DISTFILES += \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
