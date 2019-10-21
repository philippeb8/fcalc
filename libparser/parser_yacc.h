#ifndef YY_CalculatorParser_h_included
#define YY_CalculatorParser_h_included
#define YY_USE_CLASS
/* before anything */
#ifdef c_plusplus
 #ifndef __cplusplus
  #define __cplusplus
 #endif
#endif


#ifndef PARSER_H
#define PARSER_H

#define yyFlexLexer lexerFlexLexer
#include <FlexLexer.h>

#include <list>
#include <string>
#include <vector>
#include <typeinfo>
#include "type.h"
#include "tuple.h"
#include "normed.h"
#include "calculus.h"
#include "mutable_ptr.h"

using namespace fdi;

typedef mutable_ptr<type> type_p;

struct val
{
	integer l;
	floating d;
        fdi::tuple<char, integer> i;

	union
	{
		char c;
		fmpd ed;
		ufmpn fn;
		bfmpn gn;
		ufmpd fd;
		bfmpd gd;
		tfmpd hd;
		ufmpl fl;
		bfmpl gl;
		bmmpv gm;
		bfmpv gv;

		type * t;
		type_t< fdi::vector<type_p> > * v;
                type_t< std::list< fdi::tuple<type_p, int> > > * s;
	};
};

#define ON symbol = true;

#define OFF symbol = false;

#endif
#define YY_CalculatorParser_INHERIT  : public lexerFlexLexer
#define YY_CalculatorParser_STYPE  val
#define YY_CalculatorParser_LEX_BODY  { return lexerFlexLexer::yylex(); }
#define YY_CalculatorParser_ERROR_BODY  { * yyout << string("Invalid Statement"); }
#define YY_CalculatorParser_CONSTRUCTOR_INIT  : symbol(false)
#define YY_CalculatorParser_MEMBERS  bool symbol; type_t<type_p> config; void state(int s) { yy_start = 1 + 2 * s; }
 /* %{ and %header{ and %union, during decl */
#ifndef YY_CalculatorParser_COMPATIBILITY
 #ifndef YY_USE_CLASS
  #define  YY_CalculatorParser_COMPATIBILITY 1
 #else
  #define  YY_CalculatorParser_COMPATIBILITY 0
 #endif
#endif

#if YY_CalculatorParser_COMPATIBILITY != 0
/* backward compatibility */
 #ifdef YYLTYPE
  #ifndef YY_CalculatorParser_LTYPE
   #define YY_CalculatorParser_LTYPE YYLTYPE
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
/* use %define LTYPE */
  #endif
 #endif
/*#ifdef YYSTYPE*/
  #ifndef YY_CalculatorParser_STYPE
   #define YY_CalculatorParser_STYPE YYSTYPE
  /* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
   /* use %define STYPE */
  #endif
/*#endif*/
 #ifdef YYDEBUG
  #ifndef YY_CalculatorParser_DEBUG
   #define  YY_CalculatorParser_DEBUG YYDEBUG
   /* WARNING obsolete !!! user defined YYDEBUG not reported into generated header */
   /* use %define DEBUG */
  #endif
 #endif 
 /* use goto to be compatible */
 #ifndef YY_CalculatorParser_USE_GOTO
  #define YY_CalculatorParser_USE_GOTO 1
 #endif
#endif

/* use no goto to be clean in C++ */
#ifndef YY_CalculatorParser_USE_GOTO
 #define YY_CalculatorParser_USE_GOTO 0
#endif

#ifndef YY_CalculatorParser_PURE
/* YY_CalculatorParser_PURE */
#endif

#define YY_CalculatorParser_PARSE parserparse
#define YY_CalculatorParser_LEX parserlex
#define YY_CalculatorParser_ERROR parsererror
#define YY_CalculatorParser_LVAL parserlval
#define YY_CalculatorParser_CHAR parserchar
#define YY_CalculatorParser_DEBUG parserdebug
/* prefix */

#ifndef YY_CalculatorParser_DEBUG
/* YY_CalculatorParser_DEBUG */
#endif

#ifndef YY_CalculatorParser_LSP_NEEDED
 /* YY_CalculatorParser_LSP_NEEDED*/
#endif

/* DEFAULT LTYPE*/
#ifdef YY_CalculatorParser_LSP_NEEDED
 #ifndef YY_CalculatorParser_LTYPE
  #ifndef BISON_YYLTYPE_ISDECLARED
   #define BISON_YYLTYPE_ISDECLARED
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;
  #endif

  #define YY_CalculatorParser_LTYPE yyltype
 #endif
#endif

/* DEFAULT STYPE*/
#ifndef YY_CalculatorParser_STYPE
 #define YY_CalculatorParser_STYPE int
#endif

/* DEFAULT MISCELANEOUS */
#ifndef YY_CalculatorParser_PARSE
 #define YY_CalculatorParser_PARSE yyparse
#endif

#ifndef YY_CalculatorParser_LEX
 #define YY_CalculatorParser_LEX yylex
#endif

#ifndef YY_CalculatorParser_LVAL
 #define YY_CalculatorParser_LVAL yylval
#endif

#ifndef YY_CalculatorParser_LLOC
 #define YY_CalculatorParser_LLOC yylloc
#endif

#ifndef YY_CalculatorParser_CHAR
 #define YY_CalculatorParser_CHAR yychar
#endif

#ifndef YY_CalculatorParser_NERRS
 #define YY_CalculatorParser_NERRS yynerrs
#endif

#ifndef YY_CalculatorParser_DEBUG_FLAG
 #define YY_CalculatorParser_DEBUG_FLAG yydebug
#endif

#ifndef YY_CalculatorParser_ERROR
 #define YY_CalculatorParser_ERROR yyerror
#endif

#ifndef YY_CalculatorParser_PARSE_PARAM
 #ifndef __STDC__
  #ifndef __cplusplus
   #ifndef YY_USE_CLASS
    #define YY_CalculatorParser_PARSE_PARAM
    #ifndef YY_CalculatorParser_PARSE_PARAM_DEF
     #define YY_CalculatorParser_PARSE_PARAM_DEF
    #endif
   #endif
  #endif
 #endif
 #ifndef YY_CalculatorParser_PARSE_PARAM
  #define YY_CalculatorParser_PARSE_PARAM void
 #endif
#endif

/* TOKEN C */
#ifndef YY_USE_CLASS

 #ifndef YY_CalculatorParser_PURE
  #ifndef yylval
   extern YY_CalculatorParser_STYPE YY_CalculatorParser_LVAL;
  #else
   #if yylval != YY_CalculatorParser_LVAL
    extern YY_CalculatorParser_STYPE YY_CalculatorParser_LVAL;
   #else
    #warning "Namespace conflict, disabling some functionality (bison++ only)"
   #endif
  #endif
 #endif

#define	EOL	258
#define	PI	259
#define	EXP	260
#define	DEGREE	261
#define	FUNCTION1stRESULT	262
#define	IF	263
#define	ELSE	264
#define	FOR	265
#define	WHILE	266
#define	EXIT	267
#define	CHAR	268
#define	INDEX	269
#define	LONG	270
#define	DOUBLE	271
#define	INTEGER	272
#define	FUNCTION0ed	273
#define	FUNCTION1st	274
#define	FUNCTION1stNORMED	275
#define	FUNCTION1stNOT	276
#define	FUNCTION2nd	277
#define	FUNCTION2ndVECTOR	278
#define	FUNCTION2ndNORMED	279
#define	FUNCTION2ndMIXED	280
#define	FUNCTION2ndSHIFT	281
#define	FUNCTION2ndCMP	282
#define	FUNCTION2ndPOSTFIX	283
#define	FUNCTION2ndAND	284
#define	FUNCTION2ndXOR	285
#define	FUNCTION2ndOR	286
#define	FUNCTION2ndMODULO	287
#define	FUNCTION3rd	288
#define	CROSS	289
#define	SCALAR	290
#define	SQUAREROOT	291

 /* #defines token */
/* after #define tokens, before const tokens S5*/
#else
 #ifndef YY_CalculatorParser_CLASS
  #define YY_CalculatorParser_CLASS CalculatorParser
 #endif

 #ifndef YY_CalculatorParser_INHERIT
  #define YY_CalculatorParser_INHERIT
 #endif

 #ifndef YY_CalculatorParser_MEMBERS
  #define YY_CalculatorParser_MEMBERS 
 #endif

 #ifndef YY_CalculatorParser_LEX_BODY
  #define YY_CalculatorParser_LEX_BODY  
 #endif

 #ifndef YY_CalculatorParser_ERROR_BODY
  #define YY_CalculatorParser_ERROR_BODY  
 #endif

 #ifndef YY_CalculatorParser_CONSTRUCTOR_PARAM
  #define YY_CalculatorParser_CONSTRUCTOR_PARAM
 #endif
 /* choose between enum and const */
 #ifndef YY_CalculatorParser_USE_CONST_TOKEN
  #define YY_CalculatorParser_USE_CONST_TOKEN 0
  /* yes enum is more compatible with flex,  */
  /* so by default we use it */ 
 #endif
 #if YY_CalculatorParser_USE_CONST_TOKEN != 0
  #ifndef YY_CalculatorParser_ENUM_TOKEN
   #define YY_CalculatorParser_ENUM_TOKEN yy_CalculatorParser_enum_token
  #endif
 #endif

class YY_CalculatorParser_CLASS YY_CalculatorParser_INHERIT
{
public: 
 #if YY_CalculatorParser_USE_CONST_TOKEN != 0
  /* static const int token ... */
  static const int EOL;
static const int PI;
static const int EXP;
static const int DEGREE;
static const int FUNCTION1stRESULT;
static const int IF;
static const int ELSE;
static const int FOR;
static const int WHILE;
static const int EXIT;
static const int CHAR;
static const int INDEX;
static const int LONG;
static const int DOUBLE;
static const int INTEGER;
static const int FUNCTION0ed;
static const int FUNCTION1st;
static const int FUNCTION1stNORMED;
static const int FUNCTION1stNOT;
static const int FUNCTION2nd;
static const int FUNCTION2ndVECTOR;
static const int FUNCTION2ndNORMED;
static const int FUNCTION2ndMIXED;
static const int FUNCTION2ndSHIFT;
static const int FUNCTION2ndCMP;
static const int FUNCTION2ndPOSTFIX;
static const int FUNCTION2ndAND;
static const int FUNCTION2ndXOR;
static const int FUNCTION2ndOR;
static const int FUNCTION2ndMODULO;
static const int FUNCTION3rd;
static const int CROSS;
static const int SCALAR;
static const int SQUAREROOT;

 /* decl const */
 #else
  enum YY_CalculatorParser_ENUM_TOKEN { YY_CalculatorParser_NULL_TOKEN=0
  	,EOL=258
	,PI=259
	,EXP=260
	,DEGREE=261
	,FUNCTION1stRESULT=262
	,IF=263
	,ELSE=264
	,FOR=265
	,WHILE=266
	,EXIT=267
	,CHAR=268
	,INDEX=269
	,LONG=270
	,DOUBLE=271
	,INTEGER=272
	,FUNCTION0ed=273
	,FUNCTION1st=274
	,FUNCTION1stNORMED=275
	,FUNCTION1stNOT=276
	,FUNCTION2nd=277
	,FUNCTION2ndVECTOR=278
	,FUNCTION2ndNORMED=279
	,FUNCTION2ndMIXED=280
	,FUNCTION2ndSHIFT=281
	,FUNCTION2ndCMP=282
	,FUNCTION2ndPOSTFIX=283
	,FUNCTION2ndAND=284
	,FUNCTION2ndXOR=285
	,FUNCTION2ndOR=286
	,FUNCTION2ndMODULO=287
	,FUNCTION3rd=288
	,CROSS=289
	,SCALAR=290
	,SQUAREROOT=291

 /* enum token */
     }; /* end of enum declaration */
 #endif
public:
 int YY_CalculatorParser_PARSE(YY_CalculatorParser_PARSE_PARAM);
 virtual void YY_CalculatorParser_ERROR(char *msg) YY_CalculatorParser_ERROR_BODY;
 #ifdef YY_CalculatorParser_PURE
  #ifdef YY_CalculatorParser_LSP_NEEDED
   virtual int  YY_CalculatorParser_LEX(YY_CalculatorParser_STYPE *YY_CalculatorParser_LVAL,YY_CalculatorParser_LTYPE *YY_CalculatorParser_LLOC) YY_CalculatorParser_LEX_BODY;
  #else
   virtual int  YY_CalculatorParser_LEX(YY_CalculatorParser_STYPE *YY_CalculatorParser_LVAL) YY_CalculatorParser_LEX_BODY;
  #endif
 #else
  virtual int YY_CalculatorParser_LEX() YY_CalculatorParser_LEX_BODY;
  YY_CalculatorParser_STYPE YY_CalculatorParser_LVAL;
  #ifdef YY_CalculatorParser_LSP_NEEDED
   YY_CalculatorParser_LTYPE YY_CalculatorParser_LLOC;
  #endif
  int YY_CalculatorParser_NERRS;
  int YY_CalculatorParser_CHAR;
 #endif
 #if YY_CalculatorParser_DEBUG != 0
  public:
   int YY_CalculatorParser_DEBUG_FLAG;	/*  nonzero means print parse trace	*/
 #endif
public:
 YY_CalculatorParser_CLASS(YY_CalculatorParser_CONSTRUCTOR_PARAM);
public:
 YY_CalculatorParser_MEMBERS 
};
/* other declare folow */
#endif


#if YY_CalculatorParser_COMPATIBILITY != 0
 /* backward compatibility */
 /* Removed due to bison problems
 /#ifndef YYSTYPE
 / #define YYSTYPE YY_CalculatorParser_STYPE
 /#endif*/

 #ifndef YYLTYPE
  #define YYLTYPE YY_CalculatorParser_LTYPE
 #endif
 #ifndef YYDEBUG
  #ifdef YY_CalculatorParser_DEBUG 
   #define YYDEBUG YY_CalculatorParser_DEBUG
  #endif
 #endif

#endif
/* END */
#endif
