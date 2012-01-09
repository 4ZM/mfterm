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
  { "keys set",    com_keys_set,    0, 1, "A|B = key : Set a key value" },
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
  printf("TBD - com_save_tag\n");
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
  char* args[128];
  size_t arg_index = 0;

  if (arg) {
    args[arg_index++] = strtok(arg, " ");
    while(arg_index < sizeof(args) &&
          (args[arg_index] = strtok(NULL, " "))) {
      ++arg_index;
    }
    if (arg_index == sizeof(args)) {
      printf("Too many arguments.\n");
      return -1;
    }
  }
  else {
    args[0] = (char*)NULL;
  }

  if (args[0] == (char*)NULL)
    print_tag(MF_1K);
  else if (args[1] != (char*)NULL) {
    printf("Too many arguments\n");
    return -1;
  }
  else if (strcmp(args[0], "1k") == 0)
    print_tag(MF_1K);
  else if (strcmp(args[0], "4k") == 0)
    print_tag(MF_4K);
  else {
    printf("Unknown argument: %s\n", args[0]);
    return -1;
  }

  return 0;
}

int com_set(char* arg) {
  printf("TBD - com_set\n");
  return 0;
}

int com_print_keys(char* arg) {
  print_keys(&mt_current, MF_1K);
  return 0;
}

int com_keys_load(char* arg) {
  int res = load_auth(arg);
  if (res == 0)
    printf("Successfully loaded keys from: %s\n", arg);
  return 0;
}

int com_keys_save(char* arg) {
  printf("TBD - com_keys_save\n");
  return 0;
}

int com_keys_set(char* arg) {
  printf("TBD - com_keys_set\n");
  return 0;
}

int com_keys_import(char* arg) {
  import_auth();
  return 0;
}

int com_keys_print(char* arg) {
  print_keys(&mt_auth, MF_1K);
  return 0;
}
