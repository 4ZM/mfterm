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
#include "dictionary.h"

static uint8_t key_token[6];

typedef enum {
  DT_KEY = 1,
  DT_ERR = 2
} dict_token_t;

%}

%option yylineno

hex_digit      [0-9a-fA-F]
key            {hex_digit}{12}
comment        #[^\n]*

%%

{key}          {
                 read_key(key_token, dp_text);
                 return DT_KEY;
               }
{comment}      {} // eat comments
[ \t\r\n]+     {} // eat white space
.              {
                 printf("Line: %d - Unrecognized input: %s\n", dp_lineno, dp_text);
                 return DT_ERR;
               }

%%

// Reset the line numbering at the end of a file
int dp_wrap() { dp_lineno = 1; return 1; }

// Parser loop. Import keys to the dictionary as they are found.
int dictionary_import(FILE* input) {
  dict_token_t token;
  int res = 0;
  int keys_count = 0;
  int imported_count = 0;
  dp_in = input;

  while((token = dp_lex())) {
    switch(token) {
    case DT_KEY:
      ++keys_count;
      if (dictionary_add(key_token))
        ++imported_count;
      break;

    case DT_ERR:
    default:
      res = -1;
      break;
    }
  }

  if (res)
    printf("There were errors.\n");

  printf("%d keys (of %d) imported.\n", imported_count, keys_count);

  return res;
}
