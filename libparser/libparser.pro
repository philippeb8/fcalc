TEMPLATE  = lib
LANGUAGE  = C++
CONFIG	 += staticlib 
INCLUDEPATH += ../include ../libqd ../libui

QMAKE_LEX       = flex
QMAKE_LEXFLAGS  = -+ -8 -i -f -L -olex.lexer.c
QMAKE_YACC      = bison++
QMAKE_YACCFLAGS = -d -l

TARGET    = parser

#DESTDIR   = ..

#YACCSOURCES += parser.yy parser_postfix.yy

#LEXSOURCES  += lexer.ll

HEADERS	 += parser_yacc.h parser_postfix_yacc.h
SOURCES	 += parser_yacc.cpp parser_postfix_yacc.cpp lexer_lex.cpp

#The following line was inserted by qt3to4
QT +=  opengl
