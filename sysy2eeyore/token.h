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
    INT = 258,
    VOID = 259,
    RET = 260,
    WHILE = 261,
    IF = 262,
    ELSE = 263,
    BREAK = 264,
    CONT = 265,
    ADD = 266,
    SUB = 267,
    MUL = 268,
    DIV = 269,
    MOD = 270,
    ASG = 271,
    NOT = 272,
    LT = 273,
    GT = 274,
    LE = 275,
    GE = 276,
    EQ = 277,
    NE = 278,
    AND = 279,
    OR = 280,
    CONS = 281,
    ID = 282,
    IMM = 283,
    LBS = 284,
    LBM = 285,
    LBL = 286,
    RBS = 287,
    RBM = 288,
    RBL = 289,
    COMMA = 290,
    SEMI = 291
  };
#endif
/* Tokens.  */
#define INT 258
#define VOID 259
#define RET 260
#define WHILE 261
#define IF 262
#define ELSE 263
#define BREAK 264
#define CONT 265
#define ADD 266
#define SUB 267
#define MUL 268
#define DIV 269
#define MOD 270
#define ASG 271
#define NOT 272
#define LT 273
#define GT 274
#define LE 275
#define GE 276
#define EQ 277
#define NE 278
#define AND 279
#define OR 280
#define CONS 281
#define ID 282
#define IMM 283
#define LBS 284
#define LBM 285
#define LBL 286
#define RBS 287
#define RBM 288
#define RBL 289
#define COMMA 290
#define SEMI 291

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
