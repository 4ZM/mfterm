#include <stdio.h>
#include "spec_syntax.h"

int sp_parse();

int main(int argc, char** argv) {
  sp_parse();

  printf("type table\n");
  type_table_t* tt = type_table;
  while(tt) {
    printf("+%s\n", tt->type->composite_extras->name);
    field_t* f = tt->type->composite_extras->fields;
    while(f) {
      printf("  %s\n", f->name);
      f = f->next_;
    }
    tt = tt->next_;
  }
  return 0;
}
