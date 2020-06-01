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

#ifndef YY_LIBCONFIG_YY_GRAMMAR_H_INCLUDED
# define YY_LIBCONFIG_YY_GRAMMAR_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int libconfig_yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    TOK_BOOLEAN = 258,
    TOK_INTEGER = 259,
    TOK_HEX = 260,
    TOK_BIN = 261,
    TOK_OCT = 262,
    TOK_INTEGER64 = 263,
    TOK_HEX64 = 264,
    TOK_BIN64 = 265,
    TOK_OCT64 = 266,
    TOK_FLOAT = 267,
    TOK_STRING = 268,
    TOK_NAME = 269,
    TOK_EQUALS = 270,
    TOK_NEWLINE = 271,
    TOK_ARRAY_START = 272,
    TOK_ARRAY_END = 273,
    TOK_LIST_START = 274,
    TOK_LIST_END = 275,
    TOK_COMMA = 276,
    TOK_GROUP_START = 277,
    TOK_GROUP_END = 278,
    TOK_SEMICOLON = 279,
    TOK_GARBAGE = 280,
    TOK_ERROR = 281
  };
#endif
/* Tokens.  */
#define TOK_BOOLEAN 258
#define TOK_INTEGER 259
#define TOK_HEX 260
#define TOK_BIN 261
#define TOK_OCT 262
#define TOK_INTEGER64 263
#define TOK_HEX64 264
#define TOK_BIN64 265
#define TOK_OCT64 266
#define TOK_FLOAT 267
#define TOK_STRING 268
#define TOK_NAME 269
#define TOK_EQUALS 270
#define TOK_NEWLINE 271
#define TOK_ARRAY_START 272
#define TOK_ARRAY_END 273
#define TOK_LIST_START 274
#define TOK_LIST_END 275
#define TOK_COMMA 276
#define TOK_GROUP_START 277
#define TOK_GROUP_END 278
#define TOK_SEMICOLON 279
#define TOK_GARBAGE 280
#define TOK_ERROR 281

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 86 "grammar.y" /* yacc.c:1909  */

  int ival;
  long long llval;
  double fval;
  char *sval;

#line 113 "grammar.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int libconfig_yyparse (void *scanner, struct parse_context *ctx, struct scan_context *scan_ctx);

#endif /* !YY_LIBCONFIG_YY_GRAMMAR_H_INCLUDED  */
