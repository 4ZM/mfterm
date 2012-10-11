/**
 * Copyright (C) 2011 Anders Sundman <anders@4zm.org>
 *
 * This file is part of mfterm.
 *
 * mfterm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * mfterm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with mfterm.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <openssl/des.h>
#include <string.h>
#include "util.h"
#include "mac.h"
#include "tag.h"

// The DES MAC key in use
unsigned char current_mac_key[] = { 0, 0, 0, 0, 0, 0, 0, 0 };


/**
 * Compute a DES MAC, use DES in CBC mode. Key and output should be 8
 * bytes. The length specifies the length of the input in bytes. It
 * will be zero padded to 8 byte alignment if required.
 */
int compute_mac(const unsigned char* input,
		unsigned char* output,
		const unsigned char* key,
		long length) {

  DES_cblock des_key =
    { key[0], key[1], key[2], key[3], key[4], key[5], key[6], key[7] };

  // todo zeropad input if required
  const unsigned char* padded_input = input;

  // Generate a key schedule. Don't be picky, allow bad keys.
  DES_key_schedule schedule;
  DES_set_key_unchecked(&des_key, &schedule);

  // IV is all zeroes
  unsigned char ivec[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

  // Compute the DES in CBC
  DES_ncbc_encrypt(padded_input, output, length, &schedule, &ivec, 1);

  // Move up and truncate (we only want 8 bytes)
  for (int i = 0; i < 8; ++i)
    output[i] = output[length - 8 + i];
  for (int i = 8; i < length; ++i)
    output[i] = 0;

  return 0;
}

/**
 * Compute the MAC of a given block with the specified 8 byte
 * key. Return a 8 byte MAC value.
 *
 * The input to MAC algo [ 4 serial | 14 data | 6 0-pad ]
 *
 * If update is * nonzero, the mac of the current tag is updated. If
 * not, the MAC is simply printed.
 */
unsigned char* compute_block_mac(unsigned int block,
                                 const unsigned char* key,
                                 int update) {

  static unsigned char output[8];

  // Input to MAC algo [ 4 serial | 14 data | 6 0-pad ]
  unsigned char input[24];
  memcpy(&input, current_tag.amb[0].mbm.abtUID, 4);
  memcpy(&input[4], current_tag.amb[block].mbd.abtData, 14);
  memset(&input[18], 0, 6);

  int res = 0;
  res = compute_mac(input, output, key, 24);

  // Ret null on error
  if (res != 0) return NULL;

  // Should the new MAC be written back?
  if (update) {
    memcpy(&current_tag.amb[block].mbd.abtData[14], output, 2);
  }

  return output;
}
