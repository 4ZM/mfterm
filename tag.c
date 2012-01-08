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
#include <string.h>
#include "mifare.h"
#include "util.h"
#include "tag.h"

mf_tag_t mt_current;
mf_tag_t mt_auth;

int load_mfd(char* fn, mf_tag_t* tag) {
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

int load_tag(char* fn) {
  return load_mfd(fn, &mt_current);
}

int load_auth(char* fn) {

  if (load_mfd(fn, &mt_auth))
    return 1;

  // Clear 1k sector data 16 á 4 - only keep sector trailer
  for (int i = 0; i < 0x10; ++i)
    for (int j = 0; j < 0x03; ++j)
      memset(&mt_auth + i * sizeof(mf_block_t) + j, 0x00, sizeof(mf_block_t));

  // Clear 2-4k sector data 12 á 16 - only keep sector trailer
  for (int i = 0; i < 0x0C; ++i)
    for (int j = 0; j < 0x10; ++j)
      memset(&mt_auth + i * sizeof(mf_block_t) + j, 0x00, sizeof(mf_block_t));

  return 0;
}

void print_tag() {
  print_tag_range(0, sizeof(mf_tag_t) / sizeof(mf_block_t) - 1);
  return;
}

void print_tag_range(size_t first, size_t last) {
  // Iterate over all blocks
  for (int block = 0; block <= last; ++block) {

    // Start with block number
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

void print_keys() {
  printf("xS  xB  KeyA          KeyB\n");
  for (int block = 3; block < 0x10 * 4; block += 4) {
    printf("%02x  %02x  ", block / 4, block);
    print_hex_array(mt_current.amb[block].mbt.abtKeyA, 6);
    printf("  ");
    print_hex_array(mt_current.amb[block].mbt.abtKeyB, 6);
    printf("\n");
  }

  printf("\n");

  for (int block = 0xf; block < 0x0c * 0x10; block += 0x10) {
    printf("%02x  %02x  ", 0x10 + block/0x10, 0x10*4 + block);
    print_hex_array(mt_current.amb[0x10*4 + block].mbt.abtKeyA, 6);
    printf("  ");
    print_hex_array(mt_current.amb[0x10*4 + block].mbt.abtKeyB, 6);
    printf("\n");
  }
}
