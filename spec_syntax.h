#ifndef SPEC_SYNTAX__H
#define SPEC_SYNTAX__H

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

typedef enum {
  COMPOSITE_TYPE,
  BYTE_TYPE,
  BIT_TYPE,
} type_category_t;

typedef enum {
  PARTIAL_DECL,
  COMPLETE_DECL,
} type_decl_status_t;

typedef struct type_t type_t;
typedef struct field_t field_t;
typedef struct composite_type_extras_t composite_type_extras_t;
typedef struct type_table_t type_table_t;

/**
 * A struct representing a data type in the specification
 * language. There are two primitive types, Bit and Bytte. All other
 * types are user defined and use the composite_extras field to
 * express the type details.
 *
 * The primitive types are allocated statically (constants), while all
 * the composite types are allocated dynamically.
 */
struct type_t {
  type_category_t type_category;
  composite_type_extras_t* composite_extras;
};

// The primitive type instances
extern type_t byte_type;
extern type_t bit_type;

/**
 * A composite type is made up of an ordered list of fields. A field
 * is a named use of another type as an array. A field array of length
 * 1 can be treated without the array syntax in the language; but is
 * represented line all other fields.
 */
struct field_t {
  char* name; // Field name
  type_t* type;
  size_t length;  // Array length. 'no-array' types are realy length 1.

  field_t* next_; // The field list is implicit (rep. by the field type).
};

/**
 * This structure represents the content of a user defined type. It
 * holds the name and the fields of the type.
 *
 * The type also has a flag to indicate if it has been fully
 * declared. The specification language allows the use of types before
 * they have been declared. Once the complete specification has been
 * parsed, all types must be declared.
 */
struct composite_type_extras_t {
  char* name; // Type name
  type_decl_status_t decl_status;
  field_t* fields; // All fields of the type or NULL. Order is important.
};

// Allocate and return a composite type instance. The type will assume
// ownership of the heap allocated name.
type_t* make_composite_type(char* name);

// Free a composite type. This function will also free it's fields.
void free_composite_type(type_t* t);

// Allocate a new field with the given parameters. Anonymous '-'
// filler fields use NULL as name. The field will assume ownership of
// the heap allocated name.
field_t* make_field(char* name, type_t* type, size_t length);

// Free the memory used by a field.
void free_field(field_t* field);

// Add a field to an existing list of fields. The order of fields is
// significant and this function will append the field to the end of
// the field_list.
field_t* append_field(field_t* field_list, field_t* field);

// Search the field list for a field with the given name
field_t* get_field(field_t* field_list, const char* name);


/**
 * A 'table' of all the types in the language. This is part of the
 * output from the parsing process. The table is actually a list and
 * operations are typically O(n^2); but since there will probably
 * never be more than 50 types, this should be ok.
 */
struct type_table_t {
  type_t* type;
  type_table_t* next_;
};

// The global instance of the type table. If there isn't any, the
// variable will be NULL. All the type table operations (tt_) operate
// on this global variable.
extern type_table_t* type_table;

// Clear the type table - freeing the memory used by the table and by
// all the types.
void tt_clear();

// Add a type to the type table.
type_t* tt_add_type(type_t* t);

// Search the type table for a type with the given name. The first
// type found will be returned. If no type is found, NULL is returned.
type_t* tt_get_type(const char* type_name);

// Check if there are any partially declared types in the type
// table. Return a pointer to the first incomplete type or NULL if
// none exists.
type_t* tt_contains_partial_types();

#endif

