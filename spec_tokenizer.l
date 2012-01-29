/**
 * Copyright (C) 2011 Anders Sundman <anders@4zm.org>
 *
 * This file is part of mfterm.
 *
 * mfterm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mfterm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mfterm.  If not, see <http://www.gnu.org/licenses/>.
 */

%{

#include <stdio.h>
#include "util.h"
#include "spec_syntax.h"

#include "spec_parser.tab.h"

int colnum = 0;

#define YY_USER_ACTION {           \
  sp_lloc.first_line = sp_lineno;  \
  sp_lloc.first_column = colnum;   \
  colnum = colnum + sp_leng;       \
  sp_lloc.last_column=colnum;      \
  sp_lloc.last_line = sp_lineno;   \
}



%}

%option yylineno bison-bridge bison-locations

ws             [ \t]
nl             [\r\n]
comment        #[^\n]*

hex_digit      [0-9a-fA-F]
hex_number     0x{hex_digit}+
dec_number     [0-9]+

identifier     [a-zA-Z][a-zA-Z0-9_]*
byte           Byte(\[|{ws})
bit            Bit(\[|{ws})
br             [\[\]\{\}]

%%


{identifier}   {
                 sp_lval.string = strdup(sp_text);
                 return IDENTIFIER;
               }
{dec_number}   {
                 sp_lval.string = strdup(sp_text);
                 return DEC_NUM;
               }
{hex_number}   {
                 sp_lval.string = strdup(sp_text);
                 return HEX_NUM;
               }

{byte}         { yyless(sp_leng - 1); return BYTE; }
{bit}          { yyless(sp_leng - 1); return BIT; }
{br}|[\.-]     { return sp_text[0]; }

{comment}      {} // eat comments
{ws}+          {} // eat white space
{nl}+          { colnum = 0; } // eat new lines and reset column

.              {
                 printf("Line: %d - Unrecognized input: %s\n", sp_lineno, sp_text);
                 return 0;
               }

%%

// Reset the line numbering at the end of a file
int sp_wrap() { sp_lineno = 1; return 1; }


