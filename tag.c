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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mifare.h"
#include "util.h"
#include "tag.h"

mf_tag_t mt_current;
mf_tag_t mt_auth;

void strip_non_auth_data(mf_tag_t* tag);
int load_mfd(const char* fn, mf_tag_t* tag);
int save_mfd(const char* fn, const mf_tag_t* tag);

int load_mfd(const char* fn, mf_tag_t* tag) {
  FILE* mfd_file = fopen(fn, "rb");

  if (mfd_file == NULL) {
    printf("Could not open file: %s\n", fn);
    return 1;
  }

  if (fread(tag, 1, sizeof(mf_tag_t), mfd_file) != sizeof(mf_tag_t)) {
    printf("Could not read file: %s\n", fn);
    fclose(mfd_file);
    return 1;
  }

  fclose(mfd_file);
  return 0;
}

int save_mfd(const char* fn, const mf_tag_t* tag) {
  FILE* mfd_file = fopen(fn, "w");

  if (mfd_file == NULL) {
    printf("Could not open file for writing: %s\n", fn);
    return 1;
  }

  if (fwrite(tag, 1, sizeof(mf_tag_t), mfd_file) != sizeof(mf_tag_t)) {
    printf("Could not write file: %s\n", fn);
    fclose(mfd_file);
    return 1;
  }

  fclose(mfd_file);
  return 0;
}

int load_tag(const char* fn) {
  return load_mfd(fn, &mt_current);
}

int save_tag(const char* fn) {
  return save_mfd(fn, &mt_current);
}

int load_auth(const char* fn) {
  if (load_mfd(fn, &mt_auth))
    return 1;

  strip_non_auth_data(&mt_auth);
  return 0;
}

int save_auth(const char* fn) {
  return save_mfd(fn, &mt_auth);
}


int import_auth() {
  memcpy(&mt_auth, &mt_current, sizeof(mf_tag_t));
  strip_non_auth_data(&mt_auth);
  return 0;
}

void print_tag(mf_size size) {
  if (size == MF_1K)
    print_tag_range(0, MF_1K / sizeof(mf_block_t) - 1);
  else if (size == MF_4K)
    print_tag_range(0, MF_4K / sizeof(mf_block_t) - 1);
  else {
    printf("Unsupported tag size.\n");
  }
  return;
}

void print_tag_range(size_t first, size_t last) {

  // Print header
  printf("xS  xB  00                   07 08                   0f\n");
  printf("-------------------------------------------------------\n");

  // Iterate over all blocks
  for (int block = 0; block <= last; ++block) {

    // Sector number
    printf("%02x  ",
           block < 0x10*4 ? block / 4 : 0x10 + (block - 0x10*4) / 0x10);

    // Block number
    printf("%02x  ", block);

    // then print the block data
    print_hex_array_sep(mt_current.amb[block].mbd.abtData,
                    sizeof(mf_block_t), " ");

    printf("\n");

    // Indicate sector bondaries with extra nl
    if (block < last && block < 16*4 && (block + 1) % 4 == 0)
      printf("\n");
    else if (block < last && block > 16*4 && (block + 1) % 16 == 0)
      printf("\n");
  }
}

void print_keys(const mf_tag_t* tag, mf_size size) {
  printf("xS  xB  KeyA          KeyB\n");
  for (int block = 3; block < 0x10 * 4; block += 4) {
    printf("%02x  %02x  ", block / 4, block);
    print_hex_array(tag->amb[block].mbt.abtKeyA, 6);
    printf("  ");
    print_hex_array(tag->amb[block].mbt.abtKeyB, 6);
    printf("\n");
  }

  if (size == MF_1K)
    return;

  printf("\n");

  for (int block = 0xf; block < 0x0c * 0x10; block += 0x10) {
    printf("%02x  %02x  ", 0x10 + block/0x10, 0x10*4 + block);
    print_hex_array(tag->amb[0x10*4 + block].mbt.abtKeyA, 6);
    printf("  ");
    print_hex_array(tag->amb[0x10*4 + block].mbt.abtKeyB, 6);
    printf("\n");
  }
}

const char* sprint_key(const byte_t* key) {
  static char str_buff[13];

  if (!key)
    return NULL;

  sprintf(str_buff, "%02x%02x%02x%02x%02x%02x",
          (unsigned int)(key[0]),
          (unsigned int)(key[1]),
          (unsigned int)(key[2]),
          (unsigned int)(key[3]),
          (unsigned int)(key[4]),
          (unsigned int)(key[5]));

  return str_buff;
}

byte_t* read_key(byte_t* key, const char* str) {
  if (!key || !str)
    return NULL;

  static char byte_tok[] = {0, 0, 0};
  char* byte_tok_end;
  for (int i = 0; i < 6; ++i) {
    byte_tok[0] = str[i*2];
    byte_tok[1] = str[i*2+1];
    key[i] = strtol(byte_tok, &byte_tok_end, 16);
    if (*byte_tok_end != '\0') {
      return NULL;
    }
  }

  return key;
}

void clear_tag(mf_tag_t* tag) {
  memset((void*)tag, 0x00, MF_4K);
}

void strip_non_auth_data(mf_tag_t* tag) {
  static const size_t bs = sizeof(mf_block_t);

  // Clear 1k sector data 16 á 4 - only keep sector trailer
  for (int i = 0; i < 0x10; ++i)
    memset(((void*)tag) + i * 4 * bs, 0x00, 3 * bs);

  // Clear 2-4k sector data 12 á 16 - only keep sector trailer
  for (int i = 0; i < 0x0c; ++i)
    memset(((void*)tag) + 0x10 * 4 * bs + i * 0x10 * bs, 0x00, 0x0f * bs);
}


size_t block_count(mf_size size) {
  return size / 0x10;
}

size_t sector_count(mf_size size) {
  return size == MF_1K ? 0x10 : 0x1c;
}

int is_trailer_block(size_t block) {
  return (block + 1) % (block < 0x40 ? 4 : 0x10) == 0;
}

size_t block_to_sector(size_t block) {
  if (block < 0x10*4)
    return block / 4;

  return 0x10 + (block - 0x10*4) / 0x10;
}

// Return the trailer block for the specified block
size_t block_to_trailer(size_t block)
{
  if (block < 0x10*4)
    return block + (3 - (block % 4));

  return block + (0xf - (block % 0x10));
}

// Return the sector size (in blocks) that contains the block
size_t sector_size(size_t block) {
  return block < 0x10*4 ? 4 : 16;
}

// Extract a key from a tag and return it
byte_t* key_from_tag(const mf_tag_t* tag, mf_key_type key_type, size_t block) {
  static byte_t key[6];

  size_t trailer_block = block_to_trailer(block);

  if (key_type == MF_KEY_A)
    memcpy(key, tag->amb[trailer_block].mbt.abtKeyA, 6);
  else
    memcpy(key, tag->amb[trailer_block].mbt.abtKeyB, 6);

  return key;
}

/**
 * Return block index of the first block in every sector in turn on
 * repeated calls. Initialize the iterator by calling with state
 * 0. Subsequent calls should use the tag size as state. The iterator
 * returns -1 as an end marker.
 */
int sector_iterator(int state) {
  static int block;

  if (state == 0)
    return block = 0;

  if (block + 4 < 0x10*4)
    return block += 4;

  if (state == MF_1K) // End marker for 1k state
    return -1;

  if (block + 0x10 < 0x100)
    return block += 0x10;

  return -1; // End marker for 4k state
}

