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
  #include <string.h>
  #include "spec_syntax.h"

  int sp_lex(void);
  void sp_error(char const *);
%}

%union {
  type_t* type_t_ptr;
  field_t* field_t_ptr;
  char* string;
  int integer;
}

%token BYTE BIT
%token <string> IDENTIFIER
%token <string> DEC_NUM HEX_NUM

%type <type_t_ptr> data_type
%type <type_t_ptr> primitive_data_type
%type <type_t_ptr> named_composite_type_decl
%type <field_t_ptr> composite_type_decl
%type <field_t_ptr> field_decl
%type <field_t_ptr> field_decl_list
%type <integer> number

%% /* Grammar rules and actions follow.  */

input
: /* empty */ { }
| input named_composite_type_decl { }
;

named_composite_type_decl
: IDENTIFIER composite_type_decl {

    type_t* t = tt_get_type($1);
    if (t) {
      // We know of this type name
      // If it is allready complete, there is an error .. todo add error handling
    }
    else {
      t = make_composite_type($1);
      tt_add_type(t);
    }

    t->composite_extras->fields = $2;
    t->composite_extras->decl_status = COMPLETE_DECL;
    $$ = t;
  }
;

composite_type_decl
: '{' field_decl_list '}' {
    $$ = $2;
  }
;

field_decl_list
: /* empty */ {
    $$ = NULL;
  }
| field_decl_list field_decl {
    if ($1 == NULL)
      $$ = $2;
    else
      $$ = append_field($1, $2);
  }
;

field_decl
: data_type IDENTIFIER {
    $$ = make_field($2, $1, 1);
  }
| data_type '-' {
    $$ = make_field(NULL, $1, 1); // Anonymous
  }
| data_type '[' number ']' IDENTIFIER {
    $$ = make_field($5, $1, $3);
  }
| data_type '[' number ']' '-' {
    $$ = make_field(NULL, $1, $3);
  }
;

data_type
: composite_type_decl {
    type_t* t;
    t = make_composite_type(NULL); // Anonymous
    tt_add_type(t);
    t->composite_extras->fields = $1;
    t->composite_extras->decl_status = COMPLETE_DECL;
    $$ = t;
  }
| primitive_data_type { $$ = $1; }
| IDENTIFIER {
  type_t* t = tt_get_type($1);
    if (t) {
      $$ = t; // Re-discovered a known type
    }
    else {
      // This is a new type name - decl will hopefully come later.
      t = make_composite_type($1);
      t->composite_extras->decl_status = PARTIAL_DECL;
      tt_add_type(t);
      $$ = t;
    }
  }
;

primitive_data_type
: BYTE { $$ = &byte_type; }
| BIT { $$ = &bit_type; }
;

number
: DEC_NUM { $$ = strtol($1, NULL, 10); }
| HEX_NUM { $$ = strtol($1, NULL, 16); }
;

%%

void sp_error(const char *str)
{
  printf("error: %s\n",str);
}
