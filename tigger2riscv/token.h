/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    MALLOC = 258,
    END = 259,
    IF = 260,
    GOTO = 261,
    CALL = 262,
    STORE = 263,
    LOAD = 264,
    LOADADDR = 265,
    RETURN = 266,
    REGNAME = 267,
    FUNCNAME = 268,
    LABNAME = 269,
    IMMNAME = 270,
    LMB = 271,
    RMB = 272,
    NOT = 273,
    MIN = 274,
    ASS = 275,
    OP2 = 276,
    COL = 277
  };
#endif
/* Tokens.  */
#define MALLOC 258
#define END 259
#define IF 260
#define GOTO 261
#define CALL 262
#define STORE 263
#define LOAD 264
#define LOADADDR 265
#define RETURN 266
#define REGNAME 267
#define FUNCNAME 268
#define LABNAME 269
#define IMMNAME 270
#define LMB 271
#define RMB 272
#define NOT 273
#define MIN 274
#define ASS 275
#define OP2 276
#define COL 277

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
