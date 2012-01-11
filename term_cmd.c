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

command_t commands[] = {
  { "help",  com_help, 0, 0, "Display this text" },
  { "?",     com_help, 0, 0, "Synonym for 'help'" },

  { "quit",  com_quit, 0, 1, "Exit the program" },
  { "exit",  com_quit, 0, 0, "Synonym for 'quit'" },

  { "load",  com_load_tag, 1, 1, "Load tag data from a file" },
  { "save",  com_save_tag, 1, 1, "Save tag data to a file" },

  { "read",  com_read_tag,  0, 1, "Read tag data from a physical tag" },
  { "write", com_write_tag, 0, 1, "Write tag data to a physical tag" },

  { "print",      com_print,      0, 1, "1k|4k : Print tag data" },
  { "print keys", com_print_keys, 0, 1, "Print tag's keys" },

  { "set", com_set, 0, 1, "#block #offset = xx xx xx : Set tag data" },

  { "keys load",   com_keys_load,   1, 1, "Load keys from a file" },
  { "keys save",   com_keys_save,   1, 1, "Save keys to a file" },
  { "keys set",    com_keys_set,    0, 1, "A|B #S key : Set a key value" },
  { "keys import", com_keys_import, 0, 1, "Import keys from the current tag" },
  { "keys",        com_keys_print,  0, 1, "Print the keys" },

  { (char *)NULL, (cmd_func_t)NULL, 0, 0, (char *)NULL }
};


/* Look up NAME as the name of a command, and return a pointer to that
   command.  Return a NULL pointer if NAME isn't a command name. */
command_t * find_command(char *name) {
  command_t* cmd = NULL;
  size_t cmd_len = 0;

  for (int i = 0; commands[i].name; i++) {
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
  static int cmd_len_max = 0;
  if (cmd_len_max == 0) {
    for (int i = 0; commands[i].name; i++) {
      size_t cmd_len = strlen(commands[i].name);
      cmd_len_max = cmd_len > cmd_len_max ? cmd_len : cmd_len_max;
    }
  }

  // Format: 4x' ' | cmd | ' '-pad-to-longest-cmd | 4x' ' | doc
  printf ("    %s", commands[cmd].name);
  for (int j = cmd_len_max - strlen(commands[cmd].name); j >= 0; --j)
    printf(" ");
  printf ("    %s.\n", commands[cmd].doc);
}

int com_help(char* arg) {

  // Help request for specific command?
  if (arg) {
    for (int i = 0; commands[i].name; ++i) {
      if (strcmp(arg, commands[i].name) == 0) {
        print_help_(i);
        return 0;
      }
    }
    printf ("No commands match '%s'\n", arg);
  }

  // Help for all commands (with doc flag)
  for (int i = 0; commands[i].name; i++) {
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

int com_read_tag(char* arg) {
  printf("TBD - com_read_tag\n");
  return 0;
}

int com_write_tag(char* arg) {
  printf("TBD - com_write_tag\n");
  return 0;
}

int com_print(char* arg) {

  char* a = strtok(arg, " ");

  if (a == (char*)NULL)
    print_tag(MF_1K);
  else if (strtok(NULL, " ") != (char*)NULL) {
    printf("Too many arguments\n");
    return -1;
  }
  else if (strcmp(a, "1k") == 0)
    print_tag(MF_1K);
  else if (strcmp(a, "4k") == 0)
    print_tag(MF_4K);
  else {
    printf("Unknown argument: %s\n", a);
    return -1;
  }

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

  int block = strtol(block_str, NULL, 16);
  if (block < 0 || block > 0xff) {
    printf("Invalid block [0,ff]: %x\n", block);
    return -1;
  }

  int offset = strtol(offset_str, NULL, 16);
  if (offset < 0 || offset > 0x0f) {
    printf("Invalid offset [0,f]: %x\n", offset);
    return -1;
  }

  do {
    int byte = strtol(byte_str, NULL, 16);

    if (byte < 0 || byte > 0xff) {
      printf("Invalid byte value [0,ff]: %x\n", byte);
      return -1;
    }

    if (offset > 0x0f) {
      printf("Too many bytes specified.\n");
      return -1;
    }

    // Write the data
    mt_current.amb[block].mbd.abtData[offset++] = byte;

  } while((byte_str = strtok(NULL, " ")) != (char*)NULL);

  return 0;
}

int com_print_keys(char* arg) {
  char* a = strtok(arg, " ");

  if (a == (char*)NULL)
    print_keys(&mt_current, MF_1K);
  else if (strtok(NULL, " ") != (char*)NULL) {
    printf("Too many arguments\n");
    return -1;
  }
  else if (strcmp(a, "1k") == 0)
    print_keys(&mt_current, MF_1K);
  else if (strcmp(a, "4k") == 0)
    print_keys(&mt_current, MF_4K);
  else {
    printf("Unknown argument: %s\n", a);
    return -1;
  }

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
  int sector = strtol(sector_str, NULL, 16);

  // Sanity check sector range
  if (sector < 0 || sector > 0x1b) {
    printf("Invalid sector [0,1b]: %x\n", sector);
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
  size_t block;
  if (sector < 0x10)
    block = sector * 4 + 3;
  else
    block = 0x10 * 4 + (sector - 0x10) * 0x10 + 0xf;

  // Parse key selection and point to appropriate key
  byte_t* key;
  if (strcasecmp(ab, "a") == 0)
    key = mt_auth.amb[block].mbt.abtKeyA;
  else if (strcasecmp(ab, "b") == 0)
    key = mt_auth.amb[block].mbt.abtKeyB;
  else {
    printf("Invalid argument (A|B): %s\n", ab);
    return -1;
  }

  // Write the key data
  char byte_tok[] = {0, 0, 0};
  for (int i = 0; i < 6; ++i) {
    byte_tok[0] = key_str[i*2];
    byte_tok[1] = key_str[i*2+1];
    key[i] = strtol(byte_tok, NULL, 16);
  }

  return 0;
}

int com_keys_import(char* arg) {
  import_auth();
  return 0;
}

int com_keys_print(char* arg) {
  char* a = strtok(arg, " ");

  if (a == (char*)NULL)
    print_keys(&mt_auth, MF_1K);
  else if (strtok(NULL, " ") != (char*)NULL) {
    printf("Too many arguments\n");
    return -1;
  }
  else if (strcmp(a, "1k") == 0)
    print_keys(&mt_auth, MF_1K);
  else if (strcmp(a, "4k") == 0)
    print_keys(&mt_auth, MF_4K);
  else {
    printf("Unknown argument: %s\n", a);
    return -1;
  }

  return 0;
}
