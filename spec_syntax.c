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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include "util.h"
#include "spec_syntax.h"

// Forward declarations
int sp_parse();
extern FILE* sp_in;

// Parse the input file and set up type_root and instance_root.
// Return 0 on success.
int spec_import(FILE* input) {

  // Parse the input to build a type hierarchy
  sp_in = input;
  if (sp_parse()) {
    printf("Syntax Error: Specification not imported\n");
    goto error;
  }

  // Check for missing definitions
  type_t* partial = tt_contains_partial_types();
  if (partial) {
    printf("Error: Incomplete declaration '%s' in specification.\n",
           partial->composite_extras->name);
    goto error;
  }

  // Make sure we have a root type defined
  if (type_root == NULL) {
    printf("Error: No root type '.' found in specification.\n");
    goto error;
  }

  // Create the instance tree
  instance_root = make_instance_tree(type_root);

  printf("Specification sucessfully imported.\n");

  return 0;

 error:
  clear_instance_tree();
  tt_clear();
  return 1;
}

// The primitive Byte type
type_t byte_type = {
  .type_category = BYTE_TYPE,
  .composite_extras = NULL,
};

// The primitive Bit type
type_t bit_type = {
  .type_category = BIT_TYPE,
  .composite_extras = NULL,
};

// Allocate and return a composite type instance. The type will assume
// ownership of the heap allocated name.
type_t* make_composite_type(char* name) {
  type_t* t = malloc(sizeof(type_t));
  t->type_category = COMPOSITE_TYPE;
  t->composite_extras = (composite_type_extras_t*)
    malloc(sizeof(composite_type_extras_t));

  if (name)
    t->composite_extras->name = name;
  else
    t->composite_extras->name = NULL; // Anonymous type

  t->composite_extras->fields = NULL;
  return t;
}

// Free a composite type. This function will also free it's fields.
void free_composite_type(type_t* t) {

  // Free the fields
  field_list_t* iter = t->composite_extras->fields;
  while (iter) {
    field_list_t* tmp = iter;
    iter = iter->next_;
    free_field(tmp->field);
    free(tmp);
  }

  // Free the type data
  free(t->composite_extras->name);
  free(t->composite_extras);
  free(t);
}

// Allocate a new field with the given parameters. Anonymous '-'
// filler fields use NULL as name. The field will assume ownership of
// the heap allocated name.
field_t* make_field(char* name, type_t* type, size_t length) {
  field_t* f = (field_t*) malloc(sizeof(field_t));
  f->name = name; // NULL for fillers
  f->type = type;
  f->length = length;
  return f;
}

// Free the memory used by a field.
void free_field(field_t* field) {
  if (field == NULL)
    return;
  free(field->name);
  free(field);
}

// Add a field to an existing list of fields or, if the field_list
// parameter is NULL, create a new field list. The order of fields is
// significant and this function will append the field to the end of
// the field_list.
field_list_t* append_field(field_list_t* field_list, field_t* field) {

  // Create the list node
  field_list_t* flist = (field_list_t*) malloc(sizeof(field_list_t));
  flist->field = field;
  flist->next_ = NULL;

  // Create the list or append to the end
  if (field_list != NULL) {
    field_list_t* it = field_list;
    while(it->next_)
      it = it->next_;
    it->next_ = flist;
  }
  else {
    field_list = flist; // Won't effect the inarg
  }

  // Return the start of the list
  return field_list;
}

// Search the field list for a field with the given name
field_t* get_field(field_list_t* field_list, const char* name) {
  if (name == NULL || field_list == NULL)
    return NULL;

  field_list_t* it = field_list;
  while(it) {
    field_t* f = it->field;
    if (f && f->name && strcmp(f->name, name) == 0)
      return f;
    it = it->next_;
  }

  // Not found
  return NULL;
}


// The global instance of the type table. If there isn't any, the
// variable will be NULL. All the type table operations (tt_) operate
// on this global variable.
type_table_t* type_table = NULL;

// The root type of the type hierarchy
type_t* type_root = NULL;


// Internal functions for allocating and freeing type table list nodes.
type_table_t* tt_make_node_(type_t* t);
void tt_free_node_(type_table_t* tt);

// Clear the type table - freeing the memory used by the table and by
// all the types.
void tt_clear() {

  // Reset the root
  type_root = NULL;

  // Free all types and the table itself
  type_table_t* it = type_table;
  while(it) {
    type_table_t* next = it->next_;
    free_composite_type(it->type);
    tt_free_node_(it);
    it = next;
  };

  type_table = NULL;
}

// Add a type to the type table.
type_t* tt_add_type(type_t* t) {
  if (type_table == NULL) {
    type_table = tt_make_node_(t);
  }
  else {
    type_table_t* it = type_table;
    while(it->next_)
      it = it->next_;
    it->next_ = tt_make_node_(t);
  }
  return t;
}

// Search the type table for a type with the given name. The first
// type found will be returned. If no type is found, NULL is returned.
type_t* tt_get_type(const char* type_name) {
  if (type_name == NULL)
    return NULL;

  type_table_t* it = type_table;
  while(it) {
    // Anonymous types (name == NULL) will never match
    if (it->type->composite_extras->name &&
        strcmp(type_name, it->type->composite_extras->name) == 0)
      return it->type; // Type was found!
    it = it->next_;
  }

  // Not found
  return NULL;
}

// Check if there are any partially declared types in the type
// table. Return a pointer to the first incomplete type or NULL if
// none exists.
type_t* tt_contains_partial_types() {
  type_table_t* it = type_table;
  while(it) {
    if (it->type->composite_extras->decl_status == PARTIAL_DECL)
      return it->type;
    it = it->next_;
  }
  return NULL;
}

// Allocate a new list entry for the type table
type_table_t* tt_make_node_(type_t* t) {
  type_table_t* tt = malloc(sizeof(type_table_t));
  tt->type = t;
  tt->next_ = NULL;
  return tt;
}

// Free a type table list entry (this won't free the type)
void tt_free_node_(type_table_t* tt) {
  free(tt);
}


// The global variable representing the root instance; it is an
// instanciation of the '.' type.
instance_t* instance_root = NULL;

// Forward decls of internal functions used during creation and
// destruction of the instance tree.
void clear_instance_tree_(instance_t* root);
void make_instance_(instance_t* root,
                    size_t* obytes, size_t* obits,
                    size_t* sbytes, size_t* sbits);
instance_t* make_byte_instance_(field_t* field, size_t* obytes, size_t* obits);
instance_t* make_bit_instance_(field_t* field, size_t* obytes, size_t* obits);
instance_t* make_composite_instance_(field_t* field,
                                     size_t* obytes, size_t* obits);
instance_list_t* append_instance_(instance_list_t** end_ptr,
                                  instance_t* new_field);


// Create an instance tree matching the type tree starting at
// type_root. The global instance tree is constructed with type_root '.'.
instance_t* make_instance_tree(type_t* type_root) {
  instance_t* root = malloc(sizeof(instance_t));
  root->offset_bytes = 0;
  root->offset_bits = 0;
  root->size_bytes = 0;
  root->size_bits = 0;
  root->field = make_field(strdup("."), type_root, 1);
  root->fields = NULL;

  size_t obytes = 0;
  size_t obits = 0;
  make_instance_(root, &obytes, &obits,
                 &(root->size_bytes), &(root->size_bits));
  return root;
}

// Clear the global instance tree. Free it and set instance_root NULL
void clear_instance_tree() {
  if (instance_root == NULL)
    return;
  clear_instance_tree_(instance_root);
  instance_root = NULL;
}

/* Get the child instance with a given name. Only look to children, */
/* not grand children. If no child with the given name exists, return */
/* null. */
instance_t* get_instance_child(instance_t* inst, const char* name) {
  if (name == NULL)
    return NULL;

  return get_instance_child_n(inst, name, strlen(name));
}

/**
 * Like get_instance_child(inst, name), but name does not have to be
 * null terminated. Instead the length of the name string is given by
 * the last argument.
 */
instance_t* get_instance_child_n(instance_t* inst,
                                 const char* name,
                                 size_t nlen) {

  if (inst == NULL || name == NULL || nlen == 0)
    return NULL;

  instance_list_t* iter = inst->fields;
  while(iter) {
    field_t* f = iter->instance->field;

    // Make sure the field has a name (isn't anaonymous) before comparing.
    if (f && f->name &&
        strlen(f->name) == nlen &&
        strncmp(f->name, name, nlen) == 0)
      return iter->instance;
    iter = iter->next_;
  }

  return NULL;
}

/**
 * Parse a specification path of the form '.fu.bar.baz' and return the
 * instance pointed to by baz. In case the path doesn't point to a
 * loaded instance, return NULL. */
instance_t* parse_spec_path(const char* path) {

  const char* lastpath;
  instance_t* inst;

  // Parse the path up to the last instance
  if (parse_partial_spec_path(path, &lastpath, &inst) != 0)
    return NULL;

  // Find and return the leaf node
  return get_instance_child(inst, lastpath);
}

/**
 * Parse the path to produce a parent section and an instance that
 * points to the head of the parent.
 *
 * The format is .fu.bar.ba(z). Where .fu.bar.ba is the path, fu, bar
 * and baz are nested fields. The function should return parent_end
 * pointing into path to the point after the last '.', i.e. to the 'b'
 * in the last 'ba'. parent_inst will point to bar.
 *
 * The function returns 0 on success.
 */
int parse_partial_spec_path(const char* path,
                            const char** parent_end,
                            instance_t** parent_inst) {

  // Check input. Paths start with '.'
  if (path == NULL || path[0] != '.')
    return 1;

  instance_t* inst = instance_root;
  const char* remaining_path = path + 1;

  char* tok_end;
  while((tok_end = strchr(remaining_path, '.')) != NULL) {
    // There is still a part of the path before the last '.'

    ptrdiff_t tok_name = tok_end - remaining_path;
    if (tok_name <= 0)
      return 1;

    inst = get_instance_child_n(inst, remaining_path, (size_t)tok_name);

    // Exit early (error in ancestor path)
    if (inst == NULL)
      return 1;

    // Remove the token and the '.' from the start of remaining string
    remaining_path = tok_end + 1;
  }

  // Set the output arguments
  if (parent_end != NULL)
    *parent_end = remaining_path;
  if (parent_inst != NULL)
    *parent_inst = inst;

  return 0;
}

// Count the number of fields in the instance
int instance_fields_count(instance_t* inst) {
  if (inst == NULL)
    return 0;

  int count = 0;
  instance_list_t* it = inst->fields;
  while (it) {
    ++count;
    it = it->next_;
  }

  return count;
}

void clear_instance_tree_(instance_t* root) {
  instance_list_t* iter = root->fields;
  while (iter) {
    instance_list_t* tmp = iter->next_;
    clear_instance_tree_(iter->instance);
    free(iter);
    iter = tmp;
  }
  free(root);
}

void make_instance_(instance_t* root,
                    size_t* obytes, size_t* obits,
                    size_t* sbytes, size_t* sbits) {

  // Pointers to the matching type field and instance field beign processed.
  field_list_t* tf_iter = root->field->type->composite_extras->fields;
  instance_list_t* if_iter = root->fields;

  // Iterate over all the type fields and add matching instance fields.
  while(tf_iter) {
    field_t* f = tf_iter->field;

    // Create the child sub-tree or instance.
    instance_t* child;
    // Byte
    if (f->type == &byte_type) {
      child = make_byte_instance_(f, obytes, obits);
    }
    // Bit
    else if (f->type == &bit_type) {
      child = make_bit_instance_(f, obytes, obits);
    }
    // Composite type
    else {
      child = make_composite_instance_(f, obytes, obits);
      child->size_bytes = 0;
      child->size_bits = 0;

      make_instance_(child, obytes, obits,
                     &(child->size_bytes), &(child->size_bits));

      // Update size with bits % 8
      child->size_bytes =
        child->size_bytes * f->length +
        (child->size_bits * f->length) / 8;
      child->size_bits = (child->size_bits * f->length) % 8;
    }

    // Add the new instance or instance tree, to the root instance
    // field list.
    if_iter = if_iter == NULL ?
      append_instance_(&(root->fields), child) :
      append_instance_(&(if_iter->next_), child);

    // Note that we are changing the value of the input parameters here
    // this is part of the 'return value' of the function. Wrap the
    // bit size mod 8.
    *sbytes = *sbytes + child->size_bytes + (*sbits + child->size_bits) / 8;
    *sbits = (*sbits + child->size_bits) % 8;

    tf_iter = tf_iter->next_;
  }
}

instance_t* make_byte_instance_(field_t* field, size_t* obytes, size_t* obits) {
  instance_t* i = malloc(sizeof(instance_t));
  i->field = field;
  i->fields = NULL;
  i->offset_bytes = *obytes;
  i->offset_bits = *obits;
  i->size_bytes = field->length;
  i->size_bits = 0;
  *obytes += i->size_bytes;
  // *obits += 0;
  return i;
}

instance_t* make_bit_instance_(field_t* field, size_t* obytes, size_t* obits) {
  instance_t* i = malloc(sizeof(instance_t));
  i->field = field;
  i->fields = NULL;
  i->offset_bytes = *obytes;
  i->offset_bits = *obits;

  // Bits % 8, overflow in bytes field
  i->size_bytes = field->length / 8;
  i->size_bits = field->length % 8;
  *obytes += i->size_bytes + (*obits + i->size_bits) / 8;
  *obits = (*obits + i->size_bits) % 8;
  return i;
}

instance_t* make_composite_instance_(field_t* field,
                                     size_t* obytes, size_t* obits) {
  instance_t* i = malloc(sizeof(instance_t));
  i->field = field;
  i->fields = NULL;
  i->offset_bytes = *obytes;
  i->offset_bits = *obits;

  // The size will be updated later, after recusion into all child fields.
  i->size_bytes = 0;
  i->size_bits = 0;
  return i;
}

instance_list_t* append_instance_(instance_list_t** end_ptr,
                                  instance_t* new_field) {
  instance_list_t* il = (instance_list_t*) malloc(sizeof(instance_list_t));
  il->instance = new_field;
  il->next_ = NULL;
  *end_ptr = il;
  return il;
}

// Forward declaration
void print_instance_tree_(instance_t* i, int indent);
void print_instance_(instance_t* i);


void print_instance_tree() {
  if (instance_root == NULL) {
    printf("No specification loaded.\n");
    return;
  }

  printf("[%d, %d] [%d, %d]  --  .  (root)\n",
         instance_root->offset_bytes,
         instance_root->offset_bits,
         instance_root->size_bytes,
         instance_root->size_bits);

  print_instance_tree_(instance_root, 1);
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
    print_instance_(inst);
    if (inst->field->type->composite_extras != NULL)
      print_instance_tree_(inst, indent + 1);

    il = il->next_;
  }
}

void print_instance_(instance_t* i) {
    if (i->field->type == &byte_type) {
      printf("[%d, %d] [%d, %d]  --  Byte[%d]  %s\n",
             i->offset_bytes,
             i->offset_bits,
             i->size_bytes,
             i->size_bits,
             i->field->length,
             i->field->name);
    }
    else if (i->field->type == &bit_type) {
      printf("[%d, %d] [%d, %d]  --  Bit[%d]  %s\n",
             i->offset_bytes,
             i->offset_bits,
             i->size_bytes,
             i->size_bits,
             i->field->length,
             i->field->name);
    }
    else {
      printf("[%d, %d] [%d, %d]  --  %s[%d]  %s\n",
             i->offset_bytes,
             i->offset_bits,
             i->size_bytes,
             i->size_bits,
             i->field->type->composite_extras->name,
             i->field->length,
             i->field->name);
    }
}
