#include <stdio.h>
#include "spec_syntax.h"

int sp_parse();

int main(int argc, char** argv) {
  sp_parse();

  printf("Type table:\n");
  type_table_t* tt = type_table;
  while(tt) {
    printf("+%s\n", tt->type->composite_extras->name);
    field_list_t* fl = tt->type->composite_extras->fields;
    while(fl) {
      printf("  %s\n", fl->field->name);
      fl = fl->next_;
    }
    tt = tt->next_;
  }
  printf("\n");

  // check for missing definitions
  type_t* partial = tt_contains_partial_types();
  if (partial) {
    printf("Incomplete declarations! Here is one: %s\n",
           partial->composite_extras->name);
    return 1;
  }

  if (type_root == NULL) {
    printf("Root type not found\n");
    return 1;
  }

  printf("Instance tree:\n");
  instance_root = make_instance_tree(type_root);
  print_instance_tree();

  return 0;
}
