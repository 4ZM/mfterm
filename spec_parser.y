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
  #include <stdarg.h>
  #include "spec_syntax.h"

  /* #define YYDEBUG 1 */
  /* sp_debug = 1; */

  #define YYERROR_VERBOSE 1

  struct YYLTYPE;

  int sp_lex(void);
  void sp_error(const char* s, ...);
  void sp_lerror(struct YYLTYPE loc, const char* s, ...);
%}

%union {
  type_t* type_t_ptr;
  field_t* field_t_ptr;
  field_list_t* field_list_t_ptr;
  char* string;
  int integer;
}

%token BYTE "Byte"
%token BIT "Bit"
%token <string> IDENTIFIER "name"
%token <string> DEC_NUM "dec-number"
%token <string> HEX_NUM "hex-number"
%token end_of_file 0 "end-of-file"

%type <type_t_ptr> data_type
%type <type_t_ptr> primitive_data_type
%type <type_t_ptr> named_composite_type_decl
%type <field_list_t_ptr> composite_type_decl
%type <field_t_ptr> field_decl
%type <field_list_t_ptr> field_decl_list
%type <integer> number

%destructor { free($$); } IDENTIFIER DEC_NUM HEX_NUM <string>
%destructor { free_field($$);
            } field_decl <field_t_ptr>
%destructor { free_field($$->field); free($$);
            } composite_type_decl field_decl_list <field_list_t_ptr>
%destructor { if ($$ && $$->composite_extras)
                free_composite_type($$);
            } named_composite_type_decl data_type <type_t_ptr>

%% /* Grammar rules and actions follow.  */

input
: /* empty */ { }
| input named_composite_type_decl { (void) $2; }
| input '.' composite_type_decl {
    if (tt_get_type(".")) {
      // Error - the root type has allready been defined
      sp_lerror(@1, "Root type '.' allready defined.");
      YYERROR; // abort and initiate error recovery
    }

    // Create the type (named '.')
    type_t* t = make_composite_type(".");
    t->composite_extras->fields = $3;
    t->composite_extras->decl_status = COMPLETE_DECL;
    tt_add_type(t);
    type_root = t;
  }
;

named_composite_type_decl
: IDENTIFIER composite_type_decl {

    type_t* t = tt_get_type($1);
    if (t && t->composite_extras->decl_status == COMPLETE_DECL) {
      // Error - the type has been defined once before
      sp_lerror(@1, "Re-definition of type '%s'", $1);
      $$ = NULL;
      YYERROR; // abort and initiate error recovery
    }

    // If it's the first time we see the type, create it
    if(t == NULL) {
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
    if ($1 == NULL) {
      $$ = append_field(NULL, $2);
    }
    else {
      if ($2 == NULL) {
        $$ = NULL;
      }
      else if ($2->name == NULL || get_field($1, $2->name) == NULL) {
        // If it doesn't exist, then all is well. Add it.
        $$ = append_field($1, $2);
      }
      else {
        // If it allready exists, we have a semantic error.
        sp_lerror(@2, "A field with the name '%s' is allready defined.",
                  $2->name);
        $$ = $1;
        YYERROR; // abort and initiate error recovery
      }
    }
  }
| field_decl_list error {
    $$ = $1;
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
| data_type '[' error ']'{
    (void) $1;
    $$ = NULL;
    yyerrok;
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
: DEC_NUM { $$ = strtol($1, NULL, 10); free($1); }
| HEX_NUM { $$ = strtol($1, NULL, 16); free($1); }
;

%%

void sp_error(const char* s, ...) {
  va_list ap;
  va_start(ap, s);
  sp_lerror(sp_lloc, s, ap);
}

void sp_lerror(struct YYLTYPE t, const char* s, ...) {
  va_list ap;
  va_start(ap, s);

  if(t.first_line) {
    if (t.last_line == t.first_line)
      fprintf(stderr, "Error:%d:%d: ", t.first_line, t.first_column);
    else
      fprintf(stderr, "Error:%d-%d: ", t.first_line, t.last_line);
  }
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
}
