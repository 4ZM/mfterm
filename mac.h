#ifndef MAC__H
#define MAC__H

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

/**
 * Compute a DES MAC, use DES in CBC mode. Key and output should be 8
 * bytes. The length specifies the length of the input in bytes. It
 * will be zero padded to 8 byte alignment if required.
 */
int compute_mac(const unsigned char* input,
                unsigned char* output,
                const unsigned char* key,
                long length);

unsigned char* compute_block_mac(int block,
                                 const unsigned char* key);

#endif
