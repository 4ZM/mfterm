#ifndef TAG__H_
#define TAG__H_

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

#include "mifare.h"

// Convenience typedefs (shortening)
typedef mifare_classic_tag mf_tag_t;
typedef mifare_classic_block mf_block_t;

// The active tag
extern mf_tag_t mt_current;

// The ACL + keys used
extern mf_tag_t mt_auth;

// Load tag or keys from file
int load_tag(char* fn);
int load_auth(char* fn);

// Output
void print_tag();
void print_tag_range(size_t first, size_t last);

#endif
