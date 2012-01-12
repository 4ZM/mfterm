/*-
 * Public platform independent Near Field Communication (NFC) library examples
 * 
 * Copyright (C) 2009, Roel Verdult
 * Copyright (C) 2010, Romuald Conty, Romain Tartière
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
 * 
 * Note that this license only applies on the examples, NFC library itself is under LGPL
 *
 */

/**
 * @file mifaretag.h
 * @brief provide samples structs and functions to manipulate MIFARE Classic and Ultralight tags using libnfc
 */

#ifndef _LIBNFC_MIFARE_H_
#  define _LIBNFC_MIFARE_H_

#  include <nfc/nfc-types.h>

// Compiler directive, set struct alignment to 1 byte_t for compatibility
#  pragma pack(1)

typedef enum {
  MC_AUTH_A = 0x60,
  MC_AUTH_B = 0x61,
  MC_READ = 0x30,
  MC_WRITE = 0xA0,
  MC_TRANSFER = 0xB0,
  MC_DECREMENT = 0xC0,
  MC_INCREMENT = 0xC1,
  MC_STORE = 0xC2
} mifare_cmd;

// MIFARE command params
typedef struct {
  byte_t  abtKey[6];
  byte_t  abtUid[4];
} mifare_param_auth;

typedef struct {
  byte_t  abtData[16];
} mifare_param_data;

typedef struct {
  byte_t  abtValue[4];
} mifare_param_value;

typedef union {
  mifare_param_auth mpa;
  mifare_param_data mpd;
  mifare_param_value mpv;
} mifare_param;

// Reset struct alignment to default
#  pragma pack()

bool    nfc_initiator_mifare_cmd (nfc_device_t * pnd, const mifare_cmd mc, const uint8_t ui8Block, mifare_param * pmp);

// Compiler directive, set struct alignment to 1 byte_t for compatibility
#  pragma pack(1)

// MIFARE Classic
typedef struct {
  byte_t  abtUID[4];
  byte_t  btBCC;
  byte_t  btUnknown;
  byte_t  abtATQA[2];
  byte_t  abtUnknown[8];
} mifare_classic_block_manufacturer;

typedef struct {
  byte_t  abtData[16];
} mifare_classic_block_data;

typedef struct {
  byte_t  abtKeyA[6];
  byte_t  abtAccessBits[4];
  byte_t  abtKeyB[6];
} mifare_classic_block_trailer;

typedef union {
  mifare_classic_block_manufacturer mbm;
  mifare_classic_block_data mbd;
  mifare_classic_block_trailer mbt;
} mifare_classic_block;

typedef struct {
  mifare_classic_block amb[256];
} mifare_classic_tag;

// MIFARE Ultralight
typedef struct {
  byte_t  sn0[3];
  byte_t  btBCC0;
  byte_t  sn1[4];
  byte_t  btBCC1;
  byte_t  internal;
  byte_t  lock[2];
  byte_t  otp[4];
} mifareul_block_manufacturer;

typedef struct {
  byte_t  abtData[16];
} mifareul_block_data;

typedef union {
  mifareul_block_manufacturer mbm;
  mifareul_block_data mbd;
} mifareul_block;

typedef struct {
  mifareul_block amb[4];
} mifareul_tag;

// Reset struct alignment to default
#  pragma pack()

#endif // _LIBNFC_MIFARE_H_
