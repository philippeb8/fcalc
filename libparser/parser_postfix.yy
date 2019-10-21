/**
    Fornux PowerCalc - Scientific calculator for your desktop.

    Copyright (C) 2012  Phil Bouchard <pbouchard8@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


%header{
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

#endif
%}

%name CalculatorParserRPN

%define INHERIT : public lexerFlexLexer

%define STYPE val

%define LEX_BODY { return lexerFlexLexer::yylex(); }

%define ERROR_BODY { * yyout << string("Invalid Statement"); }

%define CONSTRUCTOR_INIT : symbol(false)

%define MEMBERS bool symbol; type_t<type_p> config; void state(int s) { yy_start = 1 + 2 * s; }


%token		EOL
%token		PI
%token		EXP
%token		DEGREE
%token		FUNCTION1stRESULT

%token		IF
%token		ELSE
%token		FOR
%token		WHILE
%token		EXIT

%token	<c>	CHAR
%token	<i>	INDEX
%token	<l>	LONG
%token	<d>	DOUBLE
%token	<d>	INTEGER


%token	<ed>	FUNCTION0ed

%token	<fd>	FUNCTION1st
%token	<fn>	FUNCTION1stNORMED
%token	<fl>	FUNCTION1stNOT

%token	<gd>	FUNCTION2nd
%token	<gv>	FUNCTION2ndVECTOR
%token	<gn>	FUNCTION2ndNORMED
%token	<gm>	FUNCTION2ndMIXED
%token	<gl>	FUNCTION2ndSHIFT
%token	<gd>	FUNCTION2ndCMP
%token	<gd>	FUNCTION2ndPOSTFIX
%token	<gl>	FUNCTION2ndAND
%token	<gl>	FUNCTION2ndXOR
%token	<gl>	FUNCTION2ndOR
%token	<gl>	FUNCTION2ndMODULO

%token	<hd>	FUNCTION3rd


%token		CROSS
%token		SCALAR
%token		SQUAREROOT


%type	<t>	start
%type	<t>	statement
%type	<s>	statement_list
%type	<t>	number
%type	<t>	symbol
%type	<t>	terminal
%type	<t>	expression


%%

start:			statement_list
			{
				config.value = $1;
				YYACCEPT;
			}
			;

statement_list:		statement_list statement
			{
				$$ = $1;
                                $$->value.push_back(fdi::tuple<type_p, int>($2, yylineno));
			}
			|
			statement
			{
                                $$ = new type_t< std::list< fdi::tuple<type_p, int> > >();
                                $$->value.push_back(fdi::tuple<type_p, int>($1, yylineno));
			}
			;

statement:		expression EOL
			{
				$$ = $1;
			}
			|
			'{' statement_list '}'
			{
				$$ = $2;
			}
			|
			IF '(' expression ')' statement
			{
                                $$ = new type_t< fdi::tuple<type_p, type_p, type_p> >(fdi::tuple<type_p, type_p, type_p>($3, $5, 0));
			}
			|
			IF '(' expression ')' statement ELSE statement
			{
                                $$ = new type_t< fdi::tuple<type_p, type_p, type_p> >(fdi::tuple<type_p, type_p, type_p>($3, $5, $7));
			}
			|
			WHILE '(' expression ')' statement
			{
                                $$ = new type_t< fdi::tuple<type_p, type_p, type_p, type_p> >(fdi::tuple<type_p, type_p, type_p, type_p>(0, $3, 0, $5));
			}
			|
			FOR '(' expression EOL expression EOL expression ')' statement
			{
                                $$ = new type_t< fdi::tuple<type_p, type_p, type_p, type_p> >(fdi::tuple<type_p, type_p, type_p, type_p>($3, $5, $7, $9));
			}
			|
			EXIT
			{
				$$ = new type_t<void>();
			}
			;

expression:		expression FUNCTION1stNOT
			{
                                $$ = new type_t< fdi::tuple<ufmpl, type_p> >(fdi::tuple<ufmpl, type_p>($2, $1));
			}
			|
			expression expression FUNCTION2ndOR
			{
                                $$ = new type_t< fdi::tuple<bfmpl, type_p, type_p> >(fdi::tuple<bfmpl, type_p, type_p>($3, $1, $2));
			}
			|
			expression expression FUNCTION2ndXOR
			{
                                $$ = new type_t< fdi::tuple<bfmpl, type_p, type_p> >(fdi::tuple<bfmpl, type_p, type_p>($3, $1, $2));
			}
			|
			expression expression FUNCTION2ndAND
			{
                                $$ = new type_t< fdi::tuple<bfmpl, type_p, type_p> >(fdi::tuple<bfmpl, type_p, type_p>($3, $1, $2));
			}
			|
			expression expression FUNCTION2ndCMP
			{
                                $$ = new type_t< fdi::tuple<bfmpd, type_p, type_p> >(fdi::tuple<bfmpd, type_p, type_p>($3, $1, $2));
			}
			|
			expression expression FUNCTION2ndSHIFT
			{
                                $$ = new type_t< fdi::tuple<bfmpl, type_p, type_p> >(fdi::tuple<bfmpl, type_p, type_p>($3, $1, $2));
			}
			|
			expression expression '+'
			{
                                $$ = new type_t< fdi::tuple<bfmpd, type_p, type_p> >(fdi::tuple<bfmpd, type_p, type_p>((bfmpd) & plus<floating>::operator (), $1, $2));
			}
			|
			expression expression '-'
			{
                                $$ = new type_t< fdi::tuple<bfmpd, type_p, type_p> >(fdi::tuple<bfmpd, type_p, type_p>((bfmpd) & minus<floating>::operator (), $1, $2));
			}
			|
			expression expression '*'
			{
                                $$ = new type_t< fdi::tuple<bfmpd, type_p, type_p> >(fdi::tuple<bfmpd, type_p, type_p>((bfmpd) & multiplies<floating>::operator (), $1, $2));
			}
			|
			expression expression '/'
			{
                                $$ = new type_t< fdi::tuple<bfmpd, type_p, type_p> >(fdi::tuple<bfmpd, type_p, type_p>((bfmpd) & divides<floating>::operator (), $1, $2));
			}
			|
			expression expression FUNCTION2ndMODULO
			{
                                $$ = new type_t< fdi::tuple<bfmpl, type_p, type_p> >(fdi::tuple<bfmpl, type_p, type_p>($3, $1, $2));
			}
			|
			expression expression '^'
			{
                                $$ = new type_t< fdi::tuple<bfmpd, type_p, type_p> >(fdi::tuple<bfmpd, type_p, type_p>((bfmpd) & binary_cmath_pow::operator (), $1, $2));
			}
			|
			expression SQUAREROOT
			{
                                $$ = new type_t< fdi::tuple<ufmpd, type_p> >(fdi::tuple<ufmpd, type_p>((ufmpd) & unary_cmath_sqrt::operator (), $1));
			}
			|
			expression '~'
			{
                                $$ = new type_t< fdi::tuple<ufmpd, type_p> >(fdi::tuple<ufmpd, type_p>((ufmpd) & negate<floating>::operator (), $1));
			}
			|
			expression '!'
			{
                                $$ = new type_t< fdi::tuple<ufmpd, type_p> >(fdi::tuple<ufmpd, type_p>((ufmpd) & fac::operator (), $1));
			}
			|
			CHAR FUNCTION2ndPOSTFIX
			{
                                $$ = new type_t< fdi::tuple<char, type_p> >(fdi::tuple<char, type_p>($1, new type_t< fdi::tuple<normed, type_p> >(fdi::tuple<normed, type_p>(normed(), new type_t< fdi::tuple<bfmpd, type_p, type_p> >(fdi::tuple<bfmpd, type_p, type_p>($2, new type_t<char>($1), new type_t<floating>(1)))))));
			}
			|
			terminal
			{
				$$ = $1;
			}
			|
			FUNCTION0ed
			{
				$$ = new type_t<fmpd>($1);
			}
			|
			expression FUNCTION1st
			{
                                $$ = new type_t< fdi::tuple<ufmpd, type_p> >(fdi::tuple<ufmpd, type_p>($2, $1));
			}
			|
			expression expression FUNCTION2nd
			{
                                $$ = new type_t< fdi::tuple<bfmpd, type_p, type_p> >(fdi::tuple<bfmpd, type_p, type_p>($3, $1, $2));
			}
			|
			expression expression expression FUNCTION3rd
			{
                                $$ = new type_t< fdi::tuple<tfmpd, type_p, type_p, type_p> >(fdi::tuple<tfmpd, type_p, type_p, type_p>($4, $1, $2, $3));
			}
			|
			expression FUNCTION1stRESULT
			{
                                $$ = new type_t< fdi::tuple<normed, type_p> >(fdi::tuple<normed, type_p>(normed(), $1));
			}
			|
			expression FUNCTION1stNORMED
			{
                                $$ = new type_t< fdi::tuple<ufmpn, type_p> >(fdi::tuple<ufmpn, type_p>($2, $1));
			}
			|
			expression expression FUNCTION2ndNORMED
			{
                                $$ = new type_t< fdi::tuple<bfmpn, type_p, type_p> >(fdi::tuple<bfmpn, type_p, type_p>($3, $1, $2));
			}
			|
			expression expression FUNCTION2ndVECTOR
			{
                                $$ = new type_t< fdi::tuple<bmmpv, type_p, type_p> >(fdi::tuple<bmmpv, type_p, type_p>((bmmpv) & dot::operator (), $1, $2));
			}
			|
			expression expression FUNCTION2ndMIXED
			{
                                $$ = new type_t< fdi::tuple<bfmpv, type_p, type_p> >(fdi::tuple<bfmpv, type_p, type_p>((bfmpv) & cross::operator (), $1, $2));
			}
			;

terminal:		number
			{
				$$ = $1;
			}
			|
			symbol
			{
				$$ = $1;
			}
			|
			INTEGER ':' INTEGER ':' INTEGER
			{
				$$ = new type_t<fraction>(fraction($1 * $5 + $3, $5));
			}
			|
			INTEGER ':' INTEGER
			{
				$$ = new type_t<fraction>(fraction($1, $3));
			}
			|
			INTEGER DEGREE INTEGER '\'' INTEGER '"'
			{
				$$ = new type_t<floating>(floating($1 + $3 / floating(60.0) + $5 / floating(3600.0)));
			}
			|
			INTEGER DEGREE INTEGER '\'' DOUBLE '"'
			{
				$$ = new type_t<floating>(floating($1 + $3 / floating(60.0) + $5 / floating(3600.0)));
			}
			|
			INTEGER DEGREE INTEGER '\''
			{
				$$ = new type_t<floating>(floating($1 + $3 / floating(60.0)));
			}
			;

number:			INTEGER
			{
				$$ = new type_t<floating>($1);
			}
			|
			DOUBLE
			{
				$$ = new type_t<floating>($1);
			}
			|
			LONG
			{
				$$ = new type_t<integer>($1);
			}
			;

symbol:			CHAR
			{
				$$ = new type_t<char>($1);
			}
			|
			INDEX
			{
                                $$ = new type_t< fdi::tuple<char, integer> >(fdi::tuple<char, integer>($1.first, $1.second));
			}
			|
			PI
			{
				$$ = new type_t<floating>(floating::_pi);
			}
			|
			EXP
			{
				$$ = new type_t<floating>(floating::_e);
			}
			;


%%
