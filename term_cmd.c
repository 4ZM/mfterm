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

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include "mfterm.h"
#include "tag.h"
#include "term_cmd.h"
#include "mifare_ctrl.h"
#include "dictionary.h"
#include "spec_syntax.h"
#include "util.h"
#include "mac.h"

command_t commands[] = {
  { "help",  com_help, 0, 0, "Display this text" },
  { "?",     com_help, 0, 0, "Synonym for 'help'" },

  { "quit",  com_quit, 0, 1, "Exit the program" },
  { "exit",  com_quit, 0, 0, "Synonym for 'quit'" },

  { "load",  com_load_tag, 1, 1, "Load tag data from a file" },
  { "save",  com_save_tag, 1, 1, "Save tag data to a file" },
  { "clear", com_clear_tag, 0, 1, "Clear the current tag data" },

  { "read",  com_read_tag,  0, 1, "A|B : Read tag data from a physical tag" },
  { "read unlocked", com_read_tag_unlocked, 0, 1, "On pirate cards, read card without keys" },
  { "write", com_write_tag, 0, 1, "A|B : Write tag data to a physical tag" },
  { "write unlocked", com_write_tag_unlocked, 0, 1, "On pirate cards, write 1k tag with block 0" },

  { "print",      com_print,      0, 1, "1k|4k : Print tag data" },
  { "print head", com_print_head, 0, 1, "Print first sector" },
  { "print keys", com_print_keys, 0, 1, "1k|4k : Print tag's keys" },
  { "print ac",   com_print_ac,   0, 1, "Print access conditions" },

  { "set", com_set, 0, 1, "#block #offset = xx xx xx : Set tag data" },
  { "setuid", com_setuid, 0, 1, "xx xx xx xx: Set tag UID" },

  { "keys load",   com_keys_load,   1, 1, "Load keys from a file" },
  { "keys save",   com_keys_save,   1, 1, "Save keys to a file" },
  { "keys clear",  com_keys_clear,  0, 1, "Clear the keys" },
  { "keys set",    com_keys_set,    0, 1, "A|B #S key : Set a key value" },
  { "keys import", com_keys_import, 0, 1, "Import keys from the current tag" },
  { "keys test",   com_keys_test,   0, 1, "Try to authenticate with the keys" },
  { "keys",        com_keys_print,  0, 1, "1k|4k : Print the keys" },

  { "dict load",   com_dict_load,   1, 1, "Load a dictionary key file" },
  { "dict clear",  com_dict_clear,  0, 1, "Clear the key dictionary" },
  { "dict attack", com_dict_attack, 0, 1, "Find keys of a physical tag"},
  { "dict",        com_dict_print,  0, 1, "Print the key dictionary" },

  { "spec load",   com_spec_load,   1, 1, "Load a specification file" },
  { "spec clear",  com_spec_clear,  0, 1, "Unload the specification" },
  { "spec",        com_spec_print,  0, 1, "Print the specification" },

  { "mac key", com_mac_key_get_set, 0, 1, "<k0..k7> : Get or set MAC key" },
  { "mac compute", com_mac_block_compute, 0, 1, "#block : Compute block MAC" },
  { "mac update", com_mac_block_update, 0, 1, "#block : Compute block MAC" },
  { "mac validate", com_mac_validate, 0, 1, "1k|4k : Validates block MAC of the whole tag" },

  { (char *)NULL, (cmd_func_t)NULL, 0, 0, (char *)NULL }
};

// Parse a Mifare size type argument (1k|4k)
mf_size_t parse_size(const char* str);

// Parse a Mifare size type argument (1k|4k). Return the default
// argument value if the string is NULL.
mf_size_t parse_size_default(const char* str, mf_size_t default_size);

// Parse a Mifare key type argument (A|B)
mf_key_type_t parse_key_type(const char* str);

// Parse a Mifare key type argument (A|B). Return the default
// argument value if the string is NULL.
mf_key_type_t parse_key_type_default(const char* str,
                                     mf_key_type_t default_type);

// Compute the MAC using the current_mac_key. If update is nonzero,
// the mac of the current tag is updated. If not, the MAC is simply
// printed.
int com_mac_block_compute_impl(char* arg, int update);

/* Look up NAME as the name of a command, and return a pointer to that
   command.  Return a NULL pointer if NAME isn't a command name. */
command_t* find_command(const char *name) {
  command_t* cmd = NULL;
  size_t cmd_len = 0;

  for (size_t i = 0; commands[i].name; ++i) {
    size_t l = strlen(commands[i].name);
    if (l > cmd_len && strncmp(name, commands[i].name, l) == 0) {
      cmd = &commands[i];
      cmd_len = l;
    }
  }

  return cmd;
}

/**
 * Helper function to print the specified command alligned with the longest
 * command name.
 */
void print_help_(size_t cmd) {

  // Find longest command (and cache the result)
  static size_t cmd_len_max = 0;
  if (cmd_len_max == 0) {
    for (int i = 0; commands[i].name; ++i) {
      size_t cmd_len = strlen(commands[i].name);
      cmd_len_max = cmd_len > cmd_len_max ? cmd_len : cmd_len_max;
    }
  }

  // Format: 4x' ' | cmd | ' '-pad-to-longest-cmd | 4x' ' | doc
  printf ("    %s", commands[cmd].name);
  for (int j = (int)(cmd_len_max - strlen(commands[cmd].name)); j >= 0; --j)
    printf(" ");
  printf ("    %s.\n", commands[cmd].doc);
}

int com_help(char* arg) {

  // Help request for specific command?
  if (arg && *arg != '\0') {
    for (size_t i = 0; commands[i].name; ++i) {
      if (strcmp(arg, commands[i].name) == 0) {
        print_help_(i);
        return 0;
      }
    }
    printf ("No commands match '%s'\n", arg);
  }

  // Help for all commands (with doc flag)
  for (size_t i = 0; commands[i].name; i++) {
    if (commands[i].document)
      print_help_(i);
  }

  return 0;
}

int com_quit(char *arg) {
  stop_input_loop();
  return 0;
}

int com_load_tag(char *arg) {
  int res = load_tag(arg);
  if (res == 0)
    printf("Successfully loaded tag from: %s\n", arg);
  return 0;
}

int com_save_tag(char* arg) {
  int res = save_tag(arg);
  if (res == 0)
    printf("Successfully wrote tag to: %s\n", arg);
  return 0;
}

int com_clear_tag(char* arg) {
  clear_tag(&current_tag);
  return 0;
}

int com_read_tag(char* arg) {
  // Add option to choose key
  char* ab = strtok(arg, " ");

  if (ab && strtok(NULL, " ") != (char*)NULL) {
    printf("Too many arguments\n");
    return -1;
  }
  if (!ab)
    printf("No key argument (A|B) given. Defaulting to A\n");

  // Parse key selection
  mf_key_type_t key_type = parse_key_type_default(ab, MF_KEY_A);
  if (key_type == MF_INVALID_KEY_TYPE) {
    printf("Invalid argument (A|B): %s\n", ab);
    return -1;
  }

  // Issue the read request
  mf_read_tag(&current_tag, key_type);
  return 0;
}

int com_read_tag_unlocked(char* arg) {
  char* ab = strtok(arg, " ");
  if (ab) {
    printf("This command doesn't take any arguments\n");
    return -1;
  }

  // Issue the read request
  mf_read_tag(&current_tag, MF_KEY_UNLOCKED);
  return 0;
}

int com_write_tag(char* arg) {
  // Add option to choose key
  char* ab = strtok(arg, " ");

  if (!ab) {
    printf("Too few arguments: (A|B)\n");
    return -1;
  }

  if (strtok(NULL, " ") != (char*)NULL) {
    printf("Too many arguments\n");
    return -1;
  }

  // Parse key selection
  mf_key_type_t key_type = parse_key_type(ab);
  if (key_type == MF_INVALID_KEY_TYPE) {
    printf("Invalid argument (A|B): %s\n", ab);
    return -1;
  }

  // Issue the read request
  mf_write_tag(&current_tag, key_type);
  return 0;
}

int com_write_tag_unlocked(char* arg) {
  char* ab = strtok(arg, " ");
  if (ab) {
    printf("This command doesn't take any arguments\n");
    return -1;
  }

  // Issue the write request
  mf_write_tag(&current_tag, MF_KEY_UNLOCKED);
  return 0;
}

int com_print(char* arg) {

  char* a = strtok(arg, " ");

  if (a && strtok(NULL, " ") != (char*)NULL) {
    printf("Too many arguments\n");
    return -1;
  }

  mf_size_t size = parse_size_default(a, MF_1K);

  if (size == MF_INVALID_SIZE) {
    printf("Unknown argument: %s\n", a);
    return -1;
  }

  print_tag(size);

  return 0;
}

int com_print_head(char* arg) {
  print_tag_head();
  return 0;
}

int com_set(char* arg) {
  char* block_str = strtok(arg, " ");
  char* offset_str = strtok(NULL, " ");
  char* byte_str = strtok(NULL, " ");

  if (!block_str || !offset_str || !byte_str) {
    printf("Too few arguments: #block #offset xx xx xx .. xx\n");
    return -1;
  }

  unsigned int block = (unsigned int) strtoul(block_str, &block_str, 16);
  if (*block_str != '\0') {
    printf("Invalid block character (non hex): %s\n", block_str);
    return -1;
  }
  if (block < 0 || block > 0xff) {
    printf("Invalid block [0,ff]: %x\n", block);
    return -1;
  }

  unsigned int offset = (unsigned int) strtoul(offset_str, &offset_str, 16);
  if (*offset_str != '\0') {
    printf("Invalid offset character (non hex): %s\n", offset_str);
    return -1;
  }
  if (offset < 0 || offset > 0x0f) {
    printf("Invalid offset [0,f]: %x\n", offset);
    return -1;
  }

  // Consume the byte tokens
  do {
    long int byte = strtol(byte_str, &byte_str, 16);
    if (*byte_str != '\0') {
      printf("Invalid byte character (non hex): %s\n", byte_str);
      return -1;
    }
    if (byte < 0 || byte > 0xff) {
      printf("Invalid byte value [0,ff]: %lx\n", byte);
      return -1;
    }

    if (offset > 0x0f) {
      printf("Too many bytes specified.\n");
      return -1;
    }

    // Write the data
    current_tag.amb[block].mbd.abtData[offset++] = (uint8_t)byte;

  } while((byte_str = strtok(NULL, " ")) != (char*)NULL);

  return 0;
}

int com_setuid(char* arg) {
  char* byte_str = strtok(arg, " ");
  int block = 0;

  /// TODO : Check arg size (display warning if < 4)
  if (!byte_str) {
    printf("Too few arguments: xx xx xx xx\n");
    return -1;
  }

  // Consume the byte tokens
  do {
    long int byte = strtol(byte_str, &byte_str, 16);

    if (byte < 0 || byte > 0xff) {
      printf("Invalid byte value [0,ff]: %lx\n", byte);
      return -1;
    }

    // Write the data
    current_tag.amb[0].mbd.abtData[block++] = (uint8_t)byte;

  } while(((byte_str = strtok(NULL, " ")) != (char*)NULL) && (block < 4));
  // Compute and write BCC
  current_tag.amb[0].mbd.abtData[4] = (uint8_t)current_tag.amb[0].mbd.abtData[0] ^
    (uint8_t)current_tag.amb[0].mbd.abtData[1] ^
    (uint8_t)current_tag.amb[0].mbd.abtData[2] ^
    (uint8_t)current_tag.amb[0].mbd.abtData[3];

  return 0;
}

int com_print_keys(char* arg) {
  char* a = strtok(arg, " ");

  if (a && strtok(NULL, " ") != (char*)NULL) {
    printf("Too many arguments\n");
    return -1;
  }

  mf_size_t size = parse_size_default(a, MF_1K);

  if (size == MF_INVALID_SIZE) {
    printf("Unknown argument: %s\n", a);
    return -1;
  }

  print_keys(&current_tag, size);

  return 0;
}

int com_print_ac(char* arg) {
  if (strtok(arg, " ") != (char*)NULL) {
    printf("Too many arguments\n");
    return -1;
  }

  print_ac(&current_tag);

  return 0;
}

int com_keys_load(char* arg) {
  int res = load_auth(arg);
  if (res == 0)
    printf("Successfully loaded keys from: %s\n", arg);
  return 0;
}

int com_keys_save(char* arg) {
  int res = save_auth(arg);
  if (res == 0)
    printf("Successfully wrote keys to: %s\n", arg);
  return 0;
}

int com_keys_clear(char* arg) {
  clear_tag(&current_auth);
  return 0;
}

int com_keys_set(char* arg) {
  // Arg format: A|B #S key

  char* ab = strtok(arg, " ");
  char* sector_str = strtok(NULL, " ");
  char* key_str = strtok(NULL, " ");

  if (strtok(NULL, " ") != (char*)NULL) {
    printf("Too many arguments\n");
    return -1;
  }

  if (!ab || !sector_str || !key_str) {
    printf("Too few arguments: (A|B) #sector key\n");
    return -1;
  }

  // Read sector
  long int sector = strtol(sector_str, &sector_str, 16);

  // Sanity check sector range
  if (*sector_str != '\0') {
    printf("Invalid sector character (non hex): %s\n", sector_str);
    return -1;
  }
  if (sector < 0 || sector > 0x1b) {
    printf("Invalid sector [0,1b]: %lx\n", sector);
    return -1;
  }

  // Sanity check key length
  if (strncmp(key_str, "0x", 2) == 0)
    key_str += 2;
  if (strlen(key_str) != 12) {
    printf("Invalid key (6 byte hex): %s\n", key_str);
    return -1;
  }

  // Compute the block that houses the key for the desired sector
  size_t block = sector_to_trailer((size_t)sector);

  // Parse key selection and point to appropriate key
  uint8_t* key;
  mf_key_type_t key_type = parse_key_type(ab);
  if (key_type == MF_KEY_A)
    key = current_auth.amb[block].mbt.abtKeyA;
  else if (key_type == MF_KEY_B)
    key = current_auth.amb[block].mbt.abtKeyB;
  else {
    printf("Invalid argument (A|B): %s\n", ab);
    return -1;
  }

  // Parse the key
  if (read_key(key, key_str) == NULL) {
    printf("Invalid key character (non hex)\n");
    return -1;
  }

  return 0;
}

int com_keys_import(char* arg) {
  import_auth();
  return 0;
}

int com_keys_test(char* arg) {
  // Arg format: 1k|4k A|B

  char* s = strtok(arg, " ");
  char* ab = strtok(NULL, " ");

  if (s && ab && strtok(NULL, " ") != NULL) {
    printf("Too many arguments\n");
    return -1;
  }

  if (!s || !ab) {
    printf("Too few arguments: (1k|4k) (A|B)\n");
    return -1;
  }

  // Parse arguments
  mf_size_t size = parse_size(s);
  if (size == MF_INVALID_SIZE) {
    printf("Unknown size argument (1k|4k): %s\n", s);
    return -1;
  }

  mf_key_type_t key_type = parse_key_type(ab);
  if (key_type == MF_INVALID_KEY_TYPE) {
    printf("Unknown key type argument (A|B): %s\n", ab);
    return -1;
  }

  // Run the auth test
  mf_test_auth(&current_auth, size, key_type);
  return 0;
}

int com_keys_print(char* arg) {
  char* a = strtok(arg, " ");

  if (a && strtok(NULL, " ") != (char*)NULL) {
    printf("Too many arguments\n");
    return -1;
  }

  mf_size_t size = parse_size_default(a, MF_1K);

  if (size == MF_INVALID_SIZE) {
    printf("Unknown argument: %s\n", a);
    return -1;
  }

  print_keys(&current_auth, size);

  return 0;
}

int com_dict_load(char* arg) {
  FILE* dict_file = fopen(arg, "r");

  if (dict_file == NULL) {
    printf("Could not open file: %s\n", arg);
    return 1;
  }

  dictionary_import(dict_file);

  fclose(dict_file);
  return 0;
}

int com_dict_clear(char* arg) {
  dictionary_clear();
  return 0;
}

int com_dict_attack(char* arg) {

  // Not much point if we don't have any keys
  if (!dictionary_get()) {
    printf("Dictionary is empty!\n");
    return -1;
  }

  mf_dictionary_attack(&current_auth);
  return 0;
}

int com_dict_print(char* arg) {
  key_list_t* kl = dictionary_get();

  int count = 0;
  while(kl) {
    printf("%s\n", sprint_key(kl->key));
    kl = kl->next;
    ++count;
  }

  printf("Dictionary contains: %d keys\n", count);

  return 0;
}


int com_spec_print(char* arg) {
  print_instance_tree();

  return 0;
}

int com_spec_load(char* arg) {
  // Start by clearing the current hierarcy
  clear_instance_tree();
  tt_clear();

  // Open the file
  FILE* spec_file = fopen(arg, "r");
  if (spec_file == NULL) {
    printf("Could not open file: %s\n", arg);
    return 1;
  }

  // Parse the specification
  spec_import(spec_file);

  fclose(spec_file);

  return 0;
}

int com_spec_clear(char* arg) {

  clear_instance_tree();
  tt_clear();

  return 0;
}

int com_mac_key_get_set(char* arg) {
  char* key_str = strtok(arg, " ");

  if (key_str == 0) {
    printf("Current MAC key: \n");
    print_hex_array_sep(current_mac_key, 8, " ");
    printf("\n");
    return 0;
  }

  uint8_t key[8];
  int key_ptr = 0;

  // Consume the key tokens
  do {
    long int byte = strtol(key_str, &key_str, 16);
    if (*key_str != '\0') {
      printf("Invalid key character (non hex): %s\n", key_str);
      return -1;
    }
    if (byte < 0 || byte > 0xff) {
      printf("Invalid byte value [0,ff]: %lx\n", byte);
      return -1;
    }

    if (key_ptr > sizeof(key)) {
      printf("Too many bytes specified in key (should be 8).\n");
      return -1;
    }

    // Accept the byte and add it to the key
    key[key_ptr++] = (uint8_t)byte;

  } while((key_str = strtok(NULL, " ")) != (char*)NULL);

  if (key_ptr != sizeof(key)) {
    printf("Too few bytes specified in key (should be 8).\n");
    return -1;
  }

  // Everything ok, so update the global
  memcpy(current_mac_key, key, 8);
  return 0;
}

int com_mac_block_compute(char* arg) {
  return com_mac_block_compute_impl(arg, 0);
}

int com_mac_block_update(char* arg) {
  return com_mac_block_compute_impl(arg, 1);
}

int com_mac_block_compute_impl(char* arg, int update) {
  char* block_str = strtok(arg, " ");

  if (!block_str) {
    printf("Too few arguments: #block\n");
    return -1;
  }

  unsigned int block = (unsigned int) strtoul(block_str, &block_str, 16);
  if (*block_str != '\0') {
    printf("Invalid block character (non hex): %s\n", block_str);
    return -1;
  }
  if (block < 0 || block > 0xff) {
    printf("Invalid block [0,ff]: %x\n", block);
    return -1;
  }

  // Use the key
  unsigned char* mac = compute_block_mac(block, current_mac_key, update);

  // MAC is null on error, else 8 bytes
  if (mac == 0)
    return -1;

  // Only need 16 MSBs.
  printf("Block %2.2x, MAC : ", block);
  print_hex_array_sep(mac, 2, " ");
  printf("\n");

  return 0;
}

int com_mac_validate(char* arg) {
  char* a = strtok(arg, " ");

  if (a && strtok(NULL, " ") != (char*)NULL) {
    printf("Too many arguments\n");
    return -1;
  }

  mf_size_t size = parse_size_default(a, MF_1K);

  if (size == MF_INVALID_SIZE) {
    printf("Unknown argument: %s\n", a);
    return -1;
  }

  for (unsigned int i = 1; i < block_count(size); ++i) {
    if (is_trailer_block(i))
      continue;

    unsigned char* mac = compute_block_mac(i, current_mac_key, 0);
    printf("Block: %2x ", i);
    printf("Tag: ");
    print_hex_array_sep(&current_tag.amb[i].mbd.abtData[14], 2, " ");
    printf(" Computed: ");
    print_hex_array_sep(mac, 2, " ");
    printf(" Result: ");

    if (memcmp(mac, &current_tag.amb[i].mbd.abtData[14], 2) == 0)
      printf("VALID");
    else
      printf("IN-VALID");

    printf("\n");
  }
  return 0;
}

mf_size_t parse_size(const char* str) {

  if (str == NULL)
    return MF_INVALID_SIZE;

  if (strcasecmp(str, "1k") == 0)
    return MF_1K;

  if (strcasecmp(str, "4k") == 0)
    return MF_4K;

  return MF_INVALID_SIZE;
}

mf_size_t parse_size_default(const char* str, mf_size_t default_size) {
  if (str == NULL)
    return default_size;
  return parse_size(str);
}

mf_key_type_t parse_key_type(const char* str) {

  if (str == NULL)
    return MF_INVALID_KEY_TYPE;

  if (strcasecmp(str, "a") == 0)
    return MF_KEY_A;

  if (strcasecmp(str, "b") == 0)
    return MF_KEY_B;

  return MF_INVALID_KEY_TYPE;
}

mf_key_type_t parse_key_type_default(const char* str,
                                     mf_key_type_t default_type) {
  if (str == NULL)
    return default_type;
  return parse_key_type(str);
}

// Any command starting with '.' - path spec
int exec_path_command(const char *line) {

  instance_t* inst = parse_spec_path(line);

  if (inst)
    print_tag_data_range(inst->offset_bytes, inst->offset_bits,
                         inst->size_bytes, inst->size_bits);
  else
    printf("Invalid Path\n");


  return 0;
}
