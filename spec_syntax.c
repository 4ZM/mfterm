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
#include "util.h"
#include "spec_syntax.h"

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
    if (f->name && strcmp(f->name, name) == 0)
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
    type_table_t* next = type_table->next_;
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
