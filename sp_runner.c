#include <stdio.h>
#include "spec_syntax.h"

int sp_parse();

void print_instance_tree(instance_t* i);
void print_instance_tree_(instance_t* i, int indent);

void print_instance_tree(instance_t* root) {
  printf(". (root) [%d, %d] [%d, %d]\n",
         root->offset_bytes,
         root->offset_bits,
         root->size_bytes,
         root->size_bits);

  print_instance_tree_(root, 1);
}

void print_instance_tree_(instance_t* root, int indent) {

  // For each field of the root instance
  instance_list_t* il = root->fields;
  while(il) {

    // Indent
    int count = (indent - 1) * 2;
    while(count--)
      printf(" ");
    printf("+- ");

    // Print instance field
    instance_t* inst = il->instance;
    if (inst->field->type == &byte_type) {
      printf("Byte[%d] %s [%d, %d] [%d, %d]\n",
             inst->field->length,
             inst->field->name,
             inst->offset_bytes,
             inst->offset_bits,
             inst->size_bytes,
             inst->size_bits);
    }
    else if (inst->field->type == &bit_type) {
      printf("Bit[%d] %s [%d, %d] [%d, %d]\n",
             inst->field->length,
             inst->field->name,
             inst->offset_bytes,
             inst->offset_bits,
             inst->size_bytes,
             inst->size_bits);
    }
    else {
      printf("%s[%d] %s [%d, %d] [%d, %d]\n",
             inst->field->type->composite_extras->name,
             inst->field->length,
             inst->field->name,
             inst->offset_bytes,
             inst->offset_bits,
             inst->size_bytes,
             inst->size_bits);

      print_instance_tree_(inst, indent + 1);
    }

    il = il->next_;
  }
}

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
  instance_t* instance_root = make_instance_tree(type_root);
  print_instance_tree(instance_root);

  return 0;
}
