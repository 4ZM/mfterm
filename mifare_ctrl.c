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
 * Parts of code used in this file are based on the Public platform
 * independent Near Field Communication (NFC) library example
 * nfc-mfclassic.c. It is thus covered by that license as well:
 *
 * Copyright (C) 2009, Roel Verdult
 * Copyright (C) 2010, Romuald Conty, Romain Tartière
 * Copyright (C) 2011, Adam Laurie
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  1) Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  2 )Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include <nfc/nfc.h>
#include "mifare.h"
#include "tag.h"
#include "mifare_ctrl.h"

// State of the device/tag - should be NULL between high level calls.
static nfc_device_t* device = NULL;
static nfc_target_t target;
static mf_size_t size;

static const nfc_modulation_t mf_nfc_modulation = {
  .nmt = NMT_ISO14443A,
  .nbr = NBR_106,
};

int mf_connect();
int mf_disconnect(int ret_state);

bool mf_configure_device();
bool mf_select_target();

bool mf_authenticate(byte_t block,
                     const byte_t* key,
                     mf_key_type_t key_type);

bool mf_read_tag_internal(mf_tag_t* tag,
                          const mf_tag_t* keys,
                          mf_key_type_t key_type);

bool mf_dictionary_attack_internal(mf_tag_t* tag);

bool mf_test_auth_internal(const mf_tag_t* keys,
                           mf_size_t size,
                           mf_key_type_t key_type);


int mf_disconnect(int ret_state) {
  nfc_disconnect(device);
  device = NULL;
  return ret_state;
}

int mf_connect() {

  // Connect to (any) NFC reader
  device = nfc_connect(NULL);
  if (device == NULL) {
    printf ("Could not connect to any NFC device\n");
    return -1; // Don't jump here, since we don't need to disconnect
  }

  // Initialize the device as a reader
  if (!mf_configure_device()) {
    printf("Error initializing NFC device\n");
    return mf_disconnect(-1);
  }

  // Try to find a tag
  if (!mf_select_target()) {
    printf("Connected to device, but no tag found.\n");
    return mf_disconnect(-1);
  }

  // Test if we are dealing with a Mifare compatible tag
  if ((target.nti.nai.btSak & 0x08) == 0) {
    printf("Incompatible tag type: 0x%02x (i.e. not Mifare).\n",
           target.nti.nai.btSak);
    return mf_disconnect(-1);
  }

  // Guessing tag size
  if ((target.nti.nai.abtAtqa[1] & 0x02)) { // 4K
    size = MF_4K;
  }
  else if ((target.nti.nai.abtAtqa[1] & 0x04)) { // 1K
    size = MF_1K;
  }
  else {
    printf("Unsupported tag size [1|4]K.\n");
    return mf_disconnect(-1);
  }

  return 0; // Indicate success - we are now connected
}


int mf_read_tag(mf_tag_t* tag, mf_key_type_t key_type) {

  if (mf_connect())
    return -1; // No need to disconnect here

  if (!mf_read_tag_internal(tag, &current_auth, key_type)) {
    printf("Read failed!\n");
    return mf_disconnect(-1);
  }

  return mf_disconnect(0);
}


int mf_write_tag(const mf_tag_t* tag, mf_key_type_t key_type) {
  printf("TBD - com_write_tag\n");
  return 0;
}

int mf_dictionary_attack(mf_tag_t* tag) {

  if (mf_connect()) {
    return -1; // No need to disconnect here
  }

  if (!mf_dictionary_attack_internal(tag)) {
    printf("Dictionary attack failed!\n");
    return mf_disconnect(-1);
  }

  return mf_disconnect(0);
}


int mf_test_auth(const mf_tag_t* keys,
                 mf_size_t size,
                 mf_key_type_t key_type) {

  if (mf_connect()) {
    return -1; // No need to disconnect here
  }

  if (!mf_test_auth_internal(keys, size, key_type)) {
    printf("Test authentication failed!\n");
    return mf_disconnect(-1);
  }

  return mf_disconnect(0);
}


bool mf_configure_device() {

  // Disallow invalid frame
  if (!nfc_configure(device, NDO_ACCEPT_INVALID_FRAMES, false))
    return false;

  // Disallow multiple frames
  if (!nfc_configure(device, NDO_ACCEPT_MULTIPLE_FRAMES, false))
    return false;

  // Make sure we reset the CRC and parity to chip handling.
  if (!nfc_configure(device, NDO_HANDLE_CRC, true))
    return false;
  if (!nfc_configure(device, NDO_HANDLE_PARITY, true))
    return false;

  // Disable ISO14443-4 switching in order to read devices that emulate
  // Mifare Classic with ISO14443-4 compliance.
  if (!nfc_configure(device, NDO_AUTO_ISO14443_4, false))
    return false;

  // Activate "easy framing" feature by default
  if (!nfc_configure(device, NDO_EASY_FRAMING, true))
    return false;

  // Deactivate the CRYPTO1 cipher, it may could cause problems when
  // still active
  if (!nfc_configure(device, NDO_ACTIVATE_CRYPTO1, false))
    return false;

  // Drop explicitely the field
  if (!nfc_configure(device, NDO_ACTIVATE_FIELD, false))
    return false;

  // Override default initialization option, only try to select a tag once.
  if (!nfc_configure(device, NDO_INFINITE_SELECT, false))
    return false;

  return true;
}

bool mf_select_target() {
  if (!nfc_initiator_select_passive_target(device,
                                           mf_nfc_modulation,
                                           NULL,   // init data
                                           0,      // init data len
                                           &target)) {
    return false;
  }
  return true;
}

bool mf_read_tag_internal(mf_tag_t* tag,
                      const mf_tag_t* keys, mf_key_type_t key_type) {
  mifare_param mp;

  static mf_tag_t buffer_tag;
  clear_tag(&buffer_tag);

  int error = 0;

  printf("Reading tag ["); fflush(stdout);

  // Read the card from end to begin
  for (int block = block_count(size) - 1; block >= 0; --block) {

    // Authenticate everytime we reach a trailer block
    if (is_trailer_block(block)) {

      // Try to authenticate for the current sector
      byte_t* key = key_from_tag(keys, key_type, block);
      if (!mf_authenticate(block, key, key_type)) {
        // Progress indication and error report
        printf("0x%02x", block_to_sector(block));
        if (block != 3) printf(".");
        fflush(stdout);

        block -= sector_size(block) - 1; // Skip the rest of the sector blocks
        error = 1;
      }
      else {
        // Try to read the trailer (only to *read* the access bits)
        if (nfc_initiator_mifare_cmd(device, MC_READ, block, &mp)) {
          // Copy the keys over to our tag buffer
          key_to_tag(&buffer_tag, keys->amb[block].mbt.abtKeyA, MF_KEY_A, block);
          key_to_tag(&buffer_tag, keys->amb[block].mbt.abtKeyB, MF_KEY_B, block);

          // Store the retrieved access bits in the tag buffer
          memcpy(buffer_tag.amb[block].mbt.abtAccessBits,
                 mp.mpd.abtData + 6, 4);
        } else {
          printf ("\nUnable to read trailer block: 0x%02x.\n", block);
          return false;
        }
        printf("."); fflush(stdout); // Progress indicator
      }
    }

    else { // I.e. not a sector trailer
      // Try to read out the block
      if (!nfc_initiator_mifare_cmd(device, MC_READ, block, &mp)) {
        printf("\nUnable to read block: 0x%02x.\n", block);
        return false;
      }
      memcpy(buffer_tag.amb[block].mbd.abtData, mp.mpd.abtData, 0x10);
    }
  }

  // Terminate progress indicator
  if (error)
    printf("] Auth errors in indicated sectors.\n");
  else
    printf("] Success!\n");

  // Success! Copy the data
  // todo: Or return static ptr?
  memcpy(tag, &buffer_tag, MF_4K);

  return true;
}


bool mf_dictionary_attack_internal(mf_tag_t* tag) {

  // Tag buffer to swap in if we find all keys
  int all_keys_found = 1;
  static mf_tag_t buffer_tag;
  clear_tag(&buffer_tag);

  // Iterate over the start blocks in all sectors
  for (int block = sector_iterator(0);
       block >= 0; block = sector_iterator(size)) {

    printf("Working on sector: %02x [", block_to_sector(block));

    const byte_t* key_a = NULL;
    const byte_t* key_b = NULL;

    // Iterate we run out of dictionary keys or the sector is cracked
    const key_list_t* key_it = dictionary_get();
    while(key_it && (key_a == NULL || key_b == NULL)) {

      // Try to authenticate for the current sector
      if (key_a == NULL &&
          mf_authenticate(block, key_it->key, MF_KEY_A)) {
        key_a = key_it->key;
      }

      // Try to authenticate for the current sector
      if (key_b == NULL &&
          mf_authenticate(block, key_it->key, MF_KEY_B)) {
        key_b = key_it->key;
      }

      key_it = key_it->next;

      printf("."); fflush(stdout); // Progress indicator
    }

    printf("]\n");

    printf("  A Key: ");
    if (key_a) {
      printf("%s\n", sprint_key(key_a));

      // Optimize dictionary by moving key to the front
      dictionary_add(key_a);

      // Save key in the buffer
      key_to_tag(&buffer_tag, key_a, MF_KEY_A, block);
    }
    else {
      all_keys_found = 0;
      printf("Not found\n");
    }

    printf("  B Key: ");
    if (key_b) {
      printf("%s\n", sprint_key(key_b));

      // Optimize dictionary by moving key to the front
      dictionary_add(key_b);

      // Save key in the buffer
      key_to_tag(&buffer_tag, key_b, MF_KEY_B, block);
    }
    else {
      all_keys_found = 0;
      printf("Not found\n");
    }

  }

  // All keys found, use them as current keys
  if (all_keys_found)
    memcpy(tag, &buffer_tag, MF_4K);

  return true;
}


bool mf_test_auth_internal(const mf_tag_t* keys,
                          mf_size_t size,
                          mf_key_type_t key_type) {

  printf("xS  T  Key           Status\n");
  printf("----------------------------\n");

  for (int block = sector_iterator(0);
       block != -1;
       block = sector_iterator(size)) {

    byte_t* key = key_from_tag(keys, key_type, block);
    printf("%02x  %c  %s  ",
           block_to_sector(block),
           key_type,
           sprint_key(key));


    if (!mf_authenticate(block, key, key_type)) {
      printf("Failure");
    }
    else {
      printf("Success");
    }

    printf("\n");
  }

  return true;
}


bool mf_authenticate(byte_t block, const byte_t* key, mf_key_type_t key_type) {

  mifare_param mp;

  // Set the authentication information (uid)
  memcpy(mp.mpa.abtUid, target.nti.nai.abtUid + target.nti.nai.szUidLen - 4, 4);

  // Select key for authentication
  mifare_cmd mc = (key_type == MF_KEY_A) ? MC_AUTH_A : MC_AUTH_B;

  // Set the key
  memcpy(mp.mpa.abtKey, key, 6);

  // Try to authenticate for the current sector
  if (nfc_initiator_mifare_cmd(device, mc, block, &mp))
    return true;

  // Do the hand shaking again if auth failed
  nfc_initiator_select_passive_target(device, mf_nfc_modulation,
                                      NULL, 0, &target);

  return false;
}
