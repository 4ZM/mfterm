#ifndef DICTIONARY__H
#define DICTIONARY__H

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

#include <stdint.h>
#include <stdio.h>

typedef struct key_list_t_ {
  uint8_t key[6];
  struct key_list_t_* next;
} key_list_t;

/**
 * Parse the input file and import all keys found in the dictionary.
 */
int dictionary_import(FILE* input);

/**
 * Clear the dictionary and free all allocated memory.
 */
void dictionary_clear();

/**
 * Add a new key to the dictionary. If the dictionary does not exist
 * (is empty), it will be created and the key inserted. If the key
 * already exists in the list, it will be moved to the head of the
 * list and 0 will be returned; else != 0 is returned.
 * Note: this operation is O(n)
 */
int dictionary_add(const uint8_t* key);

/**
 * Return a head pointer to the current dictionary (or NULL if it is
 * empty). Don't hang on to this pointer after an add operation, since
 * the list head migt change; rather, use this function again
 * everywhere a ref. to the list is required.
 */
key_list_t* dictionary_get();

#endif
