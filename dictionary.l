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
#include "dictionary.h"

uint8_t key_token[6];

typedef enum {
  DT_KEY = 1,
  DT_ERR = 2
} dict_token_t;

key_list_t* key_list = NULL;

key_list_t* kl_add(key_list_t** list, const uint8_t* key);
void kl_clear(key_list_t** list);

key_list_t* kl_make_node(const uint8_t* key);
int key_cmp(const uint8_t* k1, const uint8_t* k2);

%}

%option yylineno

hex_digit      [0-9a-fA-F]
key            {hex_digit}{12}
comment        #[^\n]*

%%

{key}          {
                 read_key(key_token, yytext);
                 return DT_KEY;
               }
{comment}      {} // eat comments
[ \t\r\n]+     {} // eat white space
.              {
                 printf("Line: %d - Unrecognized input: %s\n", yylineno, yytext);
                 return DT_ERR;
               }

%%

int yywrap() { yylineno = 1; return 1; }


int dictionary_import(FILE* input) {
  dict_token_t token;
  int res = 0;
  int keys_count = 0;
  int imported_count = 0;
  yyin = input;

  while((token = yylex())) {
    switch(token) {
    case DT_KEY:
      ++keys_count;
      if (dictionary_add(key_token))
        ++imported_count;
      break;

    case DT_ERR:
      res = -1;
      break;
    }
  }

  if (res)
    printf("There were errors.\n");

  printf("%d keys (of %d) imported.\n", imported_count, keys_count);

  return res;
}

void dictionary_clear() {
  kl_clear(&key_list);
}

int dictionary_add(const uint8_t* key) {
  return kl_add(&key_list, key) != NULL;
}

key_list_t* dictionary_get() {
  return key_list;
}

// Append a node to the list. Don't append duplicates, O(n) operation.
key_list_t* kl_add(key_list_t** list, const uint8_t* key) {
  if (list == NULL)
    return NULL;

  // A new list
  if (*list == NULL)
    return *list = kl_make_node(key);

  // Append (but no duplicates)
  key_list_t* it = *list;
  key_list_t* last = NULL;
  while(it) {

    // Don't add duplicates, but move the key first in the list
    if (key_cmp(it->key, key) == 0) {
      if (last) {
        last->next = it->next; // disconnect it
        it->next = *list;      // move it fisrt
        *list = it;            // re-point the head
      }

      return NULL; // Return null on duplicates
    }
    last = it;
    it = it->next;
  }

  return last->next = kl_make_node(key);
}

void kl_clear(key_list_t** list) {
  if (list == NULL || *list == NULL)
    return;

  // Free the list nodes
  key_list_t* it = *list;
  do {
    key_list_t* next = it->next;
    free(it);
    it = next;
  } while(it);

  *list = 0;
}

key_list_t* kl_make_node(const uint8_t* key) {
  // Create the list node
  key_list_t* new_node = (key_list_t*) malloc(sizeof(key_list_t));
  memcpy((void*)new_node, key, 6);
  new_node->next = NULL;
  return new_node;
}

int key_cmp(const uint8_t* k1, const uint8_t* k2) {
  for (int i = 0; i < 6; ++i) {
    if (k1[i] != k2[i])
      return -1;
  }
  return 0;
}
