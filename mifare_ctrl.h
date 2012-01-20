#ifndef MIFARE_CTRL__H
#define MIFARE_CTRL__H

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
 *
 * Parts of code used in this file are from the GNU readline library file
 * fileman.c (GPLv3). Copyright (C) 1987-2009 Free Software Foundation, Inc
 */

#include "tag.h"
#include "dictionary.h"

/**
 * Connect to an nfc device. Then read the tag data, authenticating with the
 * 'current_auth' keys of specified type, and store it in the
 * 'current_tag' state variable. Finally, disconnect from the device.
 * If there are authentication errors, those sectors will be set to
 * all zeroes.
 * Return 0 on success != 0 on failure.
 */
int mf_read_tag(mf_tag_t* tag, mf_key_type_t key_type);

/**
 * Connect to an nfc device. The write the tag data, authenticating with
 * the 'current_auth' keys of specified type. Finally, disconnect from
 * the device.  If there are authentication errors, those sectors will
 * not be written.
 * Return 0 on success != 0 on failure.
 */
int mf_write_tag(const mf_tag_t* tag, mf_key_type_t key_type);

/**
 * Connect to an nfc device.  Then, for each sector in turn, try keys in the
 * dictionary for authentication. Report success or failure. If a key
 * is found, set it in the state variable 'current_auth'. Finally,
 * disconnect from the device.
 * Return 0 on success != 0 on failure.
 */
int mf_dictionary_attack(mf_tag_t* tag);

/**
 * Connect to an nfc device. Then test the keys in the 'current_auth'
 * by trying to authenticate to the sectors of the tag. Report success
 * or failure for each sector. Finally, disconnect from the device.
 * Return 0 on success != 0 on failure.
 */
int mf_test_auth(const mf_tag_t* keys,
                 mf_size_t size,
                 mf_key_type_t key_type);

#endif
