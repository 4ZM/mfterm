#ifndef TAG__H
#define TAG__H

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

typedef enum {
  MF_INVALID_SIZE = 0,
  MF_1K = 1024,
  MF_4K = 4096
} mf_size_t;

typedef enum {
  MF_INVALID_KEY_TYPE = 0,
  MF_KEY_A = 'a',
  MF_KEY_B = 'b',
  MF_KEY_UNLOCKED = 0xff,
} mf_key_type_t;

// Convenience typedefs (shortening)
typedef mifare_classic_tag mf_tag_t;
typedef mifare_classic_block mf_block_t;

// The active tag
extern mf_tag_t current_tag;

// The ACL + keys used
extern mf_tag_t current_auth;

// Load/Save tag or keys from file
int load_tag(const char* fn);
int load_auth(const char* fn);
int save_tag(const char* fn);
int save_auth(const char* fn);

// Copy key data from the 'current_tag' to the 'current_auth'
int import_auth();

// Output tag data
void print_tag();
void print_tag_block_range(size_t first, size_t last);
void print_tag_data_range(size_t byte_offset, size_t bit_offset,
                          size_t byte_len, size_t bit_len);
void print_tag_bytes(size_t first_byte, size_t last_byte);

void print_keys(const mf_tag_t* tag, mf_size_t size);
void print_ac(const mf_tag_t* tag);

// Return a hex string representationon of the key
const char* sprint_key(const uint8_t* key);

// Parse the string and set the key. Return the key, or NULL on error.
uint8_t* read_key(uint8_t* key, const char* str);

// Return a string describing the tag type 1k|4k
const char* sprint_size(mf_size_t size);

// Set the contents of a tag to zeroes
void clear_tag(mf_tag_t* tag);

// Return number of blocks for size
size_t block_count(mf_size_t size);

// Return number of sectors for size
size_t sector_count(mf_size_t size);

// Return > 0 if the block is a trailer, 0 otherwise.
int is_trailer_block(size_t block);

// Return the sector index of the block
size_t block_to_sector(size_t block);

// Return the head block for the specified block
size_t block_to_header(size_t block);

// Return the trailer block for the specified block
size_t block_to_trailer(size_t block);

// Return the trailer block for the specified sector
size_t sector_to_trailer(size_t sector);

// Return the sector size (in blocks) that contains the block
size_t sector_size(size_t block);

// Extract the key for the block parameters sector of the tag and return it
uint8_t* key_from_tag(const mf_tag_t* tag,
                     mf_key_type_t key_type,
                     size_t block);

// Write key to the sector of a tag, where the sector is specified by
// the block (anywhere in the sector).
void key_to_tag(mf_tag_t* tag, const uint8_t* key,
                mf_key_type_t key_type, size_t block);

/**
 * Return block index of the first block in every sector in turn on
 * repeated calls. Initialize the iterator by calling with state
 * 0. Subsequent calls should use the tag size as state. The iterator
 * returns -1 as an end marker.
 */
int sector_header_iterator(int state);


#endif
