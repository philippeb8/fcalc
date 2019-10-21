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


%{
#include "parser_yacc.h"

#include "qd.h"
#include "dispatch.h"

using namespace fdi;
//%option yylineno
%}

%option noyywrap

%x HEX
%x OCT
%x BIN

%%


"pi"				{
						return CalculatorParser::PI;
					}

"sqrt"				{
                        ((CalculatorParser *) (this))->parserlval.fd = (ufmpd) & unary_cmath_sqrt::operator ();
						return CalculatorParser::FUNCTION1st;
					}

\x80				{
						return CalculatorParser::PI;
					}

<*>\x83				{
						return CalculatorParser::SQUAREROOT;
					}


"sin"				{
                        ((CalculatorParser *) (this))->parserlval.fd = (ufmpd) & unary_cmath_sin::operator ();
						return CalculatorParser::FUNCTION1st;
					}

"cos"				{
                        ((CalculatorParser *) (this))->parserlval.fd = (ufmpd) & unary_cmath_cos::operator ();
						return CalculatorParser::FUNCTION1st;
					}

"tan"				{
                        ((CalculatorParser *) (this))->parserlval.fd = (ufmpd) & unary_cmath_tan::operator ();
						return CalculatorParser::FUNCTION1st;
					}

"asin"				{
                        ((CalculatorParser *) (this))->parserlval.fd = (ufmpd) & unary_cmath_asin::operator ();
						return CalculatorParser::FUNCTION1st;
					}

"acos"				{
                        ((CalculatorParser *) (this))->parserlval.fd = (ufmpd) & unary_cmath_acos::operator ();
						return CalculatorParser::FUNCTION1st;
					}

"atan"				{
                        ((CalculatorParser *) (this))->parserlval.fd = (ufmpd) & unary_cmath_atan::operator ();
						return CalculatorParser::FUNCTION1st;
					}

"sinh"				{
                        ((CalculatorParser *) (this))->parserlval.fd = (ufmpd) & unary_cmath_sinh::operator ();
						return CalculatorParser::FUNCTION1st;
					}

"cosh"				{
                        ((CalculatorParser *) (this))->parserlval.fd = (ufmpd) & unary_cmath_cosh::operator ();
						return CalculatorParser::FUNCTION1st;
					}

"tanh"				{
                        ((CalculatorParser *) (this))->parserlval.fd = (ufmpd) & unary_cmath_tanh::operator ();
						return CalculatorParser::FUNCTION1st;
					}

"asinh"				{
                        ((CalculatorParser *) (this))->parserlval.fd = (ufmpd) & unary_cmath_asinh::operator ();
						return CalculatorParser::FUNCTION1st;
					}

"acosh"				{
                        ((CalculatorParser *) (this))->parserlval.fd = (ufmpd) & unary_cmath_acosh::operator ();
						return CalculatorParser::FUNCTION1st;
					}

"atanh"				{
                        ((CalculatorParser *) (this))->parserlval.fd = (ufmpd) & unary_cmath_atanh::operator ();
						return CalculatorParser::FUNCTION1st;
					}

"ln"				{
                        ((CalculatorParser *) (this))->parserlval.fd = (ufmpd) & unary_cmath_log::operator ();
						return CalculatorParser::FUNCTION1st;
					}

"log"				{
                        ((CalculatorParser *) (this))->parserlval.fd = (ufmpd) & unary_cmath_log10::operator ();
						return CalculatorParser::FUNCTION1st;
					}

"exp"				{
                        ((CalculatorParser *) (this))->parserlval.fd = (ufmpd) & unary_cmath_exp::operator ();
						return CalculatorParser::FUNCTION1st;
					}

"floor"				{
                        ((CalculatorParser *) (this))->parserlval.fd = (ufmpd) & unary_cmath_floor::operator ();
						return CalculatorParser::FUNCTION1st;
					}

"atan2"				{
                        ((CalculatorParser *) (this))->parserlval.gd = (bfmpd) & binary_cmath_atan2::operator ();
						return CalculatorParser::FUNCTION2nd;
					}

"ran"				{
						((CalculatorParser *) (this))->parserlval.ed = (fmpd) & ran::operator ();
						return CalculatorParser::FUNCTION0ed;
					}

"permutation"		{
						((CalculatorParser *) (this))->parserlval.gd = (bfmpd) & permutation::operator ();
						return CalculatorParser::FUNCTION2nd;
					}

"combination"		{
						((CalculatorParser *) (this))->parserlval.gd = (bfmpd) & combination::operator ();
						return CalculatorParser::FUNCTION2nd;
					}


"dot"				{
						((CalculatorParser *) (this))->parserlval.gm = (bmmpv) & dot::operator ();
						return CalculatorParser::FUNCTION2ndMIXED;
					}

"cross"				{
						((CalculatorParser *) (this))->parserlval.gv = (bfmpv) & cross::operator ();
						return CalculatorParser::FUNCTION2ndVECTOR;
					}

\x84				{
						return CalculatorParser::CROSS;
					}

\x85				{
						return CalculatorParser::SCALAR;
					}

"length"			{
						((CalculatorParser *) (this))->parserlval.fn = (ufmpn) & vectorsize::operator ();
						return CalculatorParser::FUNCTION1stNORMED;
					}

"sort"				{
						((CalculatorParser *) (this))->parserlval.fn = (ufmpn) & sort::operator ();
						return CalculatorParser::FUNCTION1stNORMED;
					}

"part"				{
						((CalculatorParser *) (this))->parserlval.gn = (bfmpn) & part::operator ();
						return CalculatorParser::FUNCTION2ndNORMED;
					}

"merge"				{
						((CalculatorParser *) (this))->parserlval.gn = (bfmpn) & vectormerge::operator ();
						return CalculatorParser::FUNCTION2ndNORMED;
					}


"sum"				{
						((CalculatorParser *) (this))->parserlval.fn = (ufmpn) & sum::operator ();
						return CalculatorParser::FUNCTION1stNORMED;
					}

"product"			{
						((CalculatorParser *) (this))->parserlval.fn = (ufmpn) & product::operator ();
						return CalculatorParser::FUNCTION1stNORMED;
					}

"mean"				{
						((CalculatorParser *) (this))->parserlval.fn = (ufmpn) & mean::operator ();
						return CalculatorParser::FUNCTION1stNORMED;
					}

"median"			{
						((CalculatorParser *) (this))->parserlval.fn = (ufmpn) & median::operator ();
						return CalculatorParser::FUNCTION1stNORMED;
					}

"variance"			{
						((CalculatorParser *) (this))->parserlval.fn = (ufmpn) & variance::operator ();
						return CalculatorParser::FUNCTION1stNORMED;
					}

"variancemle"		{
						((CalculatorParser *) (this))->parserlval.fn = (ufmpn) & variancemle::operator ();
						return CalculatorParser::FUNCTION1stNORMED;
					}

"covariance"		{
						((CalculatorParser *) (this))->parserlval.fn = (ufmpn) & covariance::operator ();
						return CalculatorParser::FUNCTION1stNORMED;
					}


"sumxy"				{
						((CalculatorParser *) (this))->parserlval.fn = (ufmpn) & sumxy::operator ();
						return CalculatorParser::FUNCTION1stNORMED;
					}

"colx"				{
						((CalculatorParser *) (this))->parserlval.fn = (ufmpn) & colx::operator ();
						return CalculatorParser::FUNCTION1stNORMED;
					}

"coly"				{
						((CalculatorParser *) (this))->parserlval.fn = (ufmpn) & coly::operator ();
						return CalculatorParser::FUNCTION1stNORMED;
					}

"transpose"			{
						((CalculatorParser *) (this))->parserlval.fn = (ufmpn) & transpose::operator ();
						return CalculatorParser::FUNCTION1stNORMED;
					}

"alpha"				{
						((CalculatorParser *) (this))->parserlval.fn = (ufmpn) & alpha::operator ();
						return CalculatorParser::FUNCTION1stNORMED;
					}

"beta"				{
						((CalculatorParser *) (this))->parserlval.fn = (ufmpn) & beta::operator ();
						return CalculatorParser::FUNCTION1stNORMED;
					}

"correlation"		{
						((CalculatorParser *) (this))->parserlval.fn = (ufmpn) & correlation::operator ();
						return CalculatorParser::FUNCTION1stNORMED;
					}

\x87				{
						((CalculatorParser *) (this))->parserlval.fn = (ufmpn) & alpha::operator ();
						return CalculatorParser::FUNCTION1stNORMED;
					}

\x88				{
						((CalculatorParser *) (this))->parserlval.fn = (ufmpn) & beta::operator ();
						return CalculatorParser::FUNCTION1stNORMED;
					}

\x89				{
						((CalculatorParser *) (this))->parserlval.fn = (ufmpn) & correlation::operator ();
						return CalculatorParser::FUNCTION1stNORMED;
					}


"polar"				{
						((CalculatorParser *) (this))->parserlval.fn = (ufmpn) & polar::operator ();
						return CalculatorParser::FUNCTION1stNORMED;
					}

"rectangular"		{
						((CalculatorParser *) (this))->parserlval.fn = (ufmpn) & rectangular::operator ();
						return CalculatorParser::FUNCTION1stNORMED;
					}

"result"			{
						return CalculatorParser::FUNCTION1stRESULT;
					}


<*>"not"			{
						((CalculatorParser *) (this))->parserlval.fl = (ufmpl) & bitwise_not<integer>::operator ();
						return CalculatorParser::FUNCTION1stNOT;
					}

<*>"<<"				{
						((CalculatorParser *) (this))->parserlval.fl = (ufmpl) & bitwise_lsh<integer>::operator ();
						return CalculatorParser::FUNCTION2ndSHIFT;
					}

<*>">>"				{
						((CalculatorParser *) (this))->parserlval.fl = (ufmpl) & bitwise_rsh<integer>::operator ();
						return CalculatorParser::FUNCTION2ndSHIFT;
					}

<*>"and"			{
						((CalculatorParser *) (this))->parserlval.gl = (bfmpl) & bitwise_and<integer>::operator ();
						return CalculatorParser::FUNCTION2ndAND;
					}

<*>"xor"			{
						((CalculatorParser *) (this))->parserlval.gl = (bfmpl) & bitwise_xor<integer>::operator ();
						return CalculatorParser::FUNCTION2ndXOR;
					}

<*>"xnor"			{
						((CalculatorParser *) (this))->parserlval.gl = (bfmpl) & bitwise_xnor<integer>::operator ();
						return CalculatorParser::FUNCTION2ndXOR;
					}

<*>"or"				{
						((CalculatorParser *) (this))->parserlval.gl = (bfmpl) & bitwise_or<integer>::operator ();
						return CalculatorParser::FUNCTION2ndOR;
					}

<*>"mod"			{
						((CalculatorParser *) (this))->parserlval.gl = (bfmpl) & bitwise_mod<integer>::operator ();
						return CalculatorParser::FUNCTION2ndMODULO;
					}


<*>"=="				{
						((CalculatorParser *) (this))->parserlval.gd = (bfmpd) & equal_to<floating>::operator ();
						return CalculatorParser::FUNCTION2ndCMP;
					}

<*>"<"				{
						((CalculatorParser *) (this))->parserlval.gd = (bfmpd) & less<floating>::operator ();
						return CalculatorParser::FUNCTION2ndCMP;
					}

<*>">"				{
						((CalculatorParser *) (this))->parserlval.gd = (bfmpd) & greater<floating>::operator ();
						return CalculatorParser::FUNCTION2ndCMP;
					}

<*>"!="				{
						((CalculatorParser *) (this))->parserlval.gd = (bfmpd) & not_equal_to<floating>::operator ();
						return CalculatorParser::FUNCTION2ndCMP;
					}

<*>"<>"				{
						((CalculatorParser *) (this))->parserlval.gd = (bfmpd) & not_equal_to<floating>::operator ();
						return CalculatorParser::FUNCTION2ndCMP;
					}

<*>"<="				{
						((CalculatorParser *) (this))->parserlval.gd = (bfmpd) & less_equal<floating>::operator ();
						return CalculatorParser::FUNCTION2ndCMP;
					}

<*>">="				{
						((CalculatorParser *) (this))->parserlval.gd = (bfmpd) & greater_equal<floating>::operator ();
						return CalculatorParser::FUNCTION2ndCMP;
					}


<*>"++"				{
						((CalculatorParser *) (this))->parserlval.gd = (bfmpd) & plus<floating>::operator ();
						return CalculatorParser::FUNCTION2ndPOSTFIX;
					}

<*>"--"				{
						((CalculatorParser *) (this))->parserlval.gd = (bfmpd) & minus<floating>::operator ();
						return CalculatorParser::FUNCTION2ndPOSTFIX;
					}


[a-z]				{
						((CalculatorParser *) (this))->parserlval.c = * yytext;
						return CalculatorParser::CHAR;
					}

<BIN>[0-1]{1,64}	{
						((CalculatorParser *) (this))->parserlval.l = dispatch<char *, integer>().bin(yytext);
						return CalculatorParser::LONG;
					}

<OCT>[0-7]{1,22}	{
						((CalculatorParser *) (this))->parserlval.l = dispatch<char *, integer>().oct(yytext);
						return CalculatorParser::LONG;
					}

<HEX>[0-9a-f]{1,16}	{
						((CalculatorParser *) (this))->parserlval.l = dispatch<char *, integer>().hex(yytext);
						return CalculatorParser::LONG;
					}

[0-9]+				{
						((CalculatorParser *) (this))->parserlval.d = dispatch<char *, floating>()(yytext);
                        return CalculatorParser::INTEGER;
					}

"nan"				{
						((CalculatorParser *) (this))->parserlval.d = floating::_nan;
						return CalculatorParser::DOUBLE;
					}

\.[0-9]+(e[+-]?[0-9]+)?			{
						((CalculatorParser *) (this))->parserlval.d = dispatch<char *, floating>()(yytext);
						return CalculatorParser::DOUBLE;
					}

[0-9]+(\.[0-9]*)?(e[+-]?[0-9]+)?	{
						((CalculatorParser *) (this))->parserlval.d = dispatch<char *, floating>()(yytext);
						return CalculatorParser::DOUBLE;
					}


<*>\;				{
						return CalculatorParser::EOL;
					}

<*><<EOF>>			{
						return CalculatorParser::EOL;
					}

<*>\#.*				{
					}

<*>[ \t]+			{
					}


\x86				{
						return CalculatorParser::DEGREE;
					}



"todegrees"			{
						((CalculatorParser *) (this))->parserlval.hd = (tfmpd) & todegrees::operator ();
						return CalculatorParser::FUNCTION3rd;
					}

<*>.				{
						return * yytext;
					}

%%

/*
[a-z][0-9]+				{
						((CalculatorParser *) (this))->parserlval.i.first = * yytext;
						((CalculatorParser *) (this))->parserlval.i.second = dispatch<char *, integer>().bin(yytext + 1);
						return CalculatorParser::INDEX;
					}

"todegrees"				{
						((CalculatorParser *) (this))->parserlval.hd = (tfmpd) & todegrees::operator ();
						return CalculatorParser::FUNCTION3rd;
					}

"det"				{
						((CalculatorParser *) (this))->parserlval.fn = (ufmpn) & det::operator ();
						return CalculatorParser::FUNCTION1stNORMED;
					}

"inverse"				{
						((CalculatorParser *) (this))->parserlval.fn = (ufmpn) & inverse::operator ();
						return CalculatorParser::FUNCTION1stNORMED;
					}

"identity"			{
						((CalculatorParser *) (this))->parserlval.fn = (ufmpn) & identity::operator ();
						return CalculatorParser::FUNCTION1stNORMED;
					}

<*>"if"					{
						return CalculatorParser::IF;
					}

<*>"else"				{
						return CalculatorParser::ELSE;
					}

<*>"for"				{
						return CalculatorParser::FOR;
					}

<*>"while"				{
						return CalculatorParser::WHILE;
					}

<*>"exit"				{
						return CalculatorParser::EXIT;
					}

*/
