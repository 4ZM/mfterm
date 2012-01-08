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
 *
 * Parts of code used in this file are from the GNU readline library file
 * fileman.c (GPLv3). Copyright (C) 1987-2009 Free Software Foundation, Inc
 */

#include <string.h>
#include <stdio.h>
#include "util.h"

int is_space(char c) {
  return c == ' ' || c == '\t';
}

/* Strip whitespace from the start and end of STRING.  Return a pointer
   into STRING. */
char* trim(char* string) {

  char* s = string;
  while (is_space(*s))
    ++s;

  if (*s == 0)
    return s;

  char* t = s + strlen(s) - 1;
  while (t > s && is_space(*t))
    --t;
  *++t = '\0';

  return s;
}

void print_hex_array(unsigned char* data, size_t nbytes) {
  print_hex_array_sep(data, nbytes, NULL);
}

void print_hex_array_sep(unsigned char* data, size_t nbytes, char* sep) {
    for (int i = 0; i < nbytes; ++i) {
      printf("%02x", data[i]);
      if (sep)
        printf("%s", sep);
    }
}
