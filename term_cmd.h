#ifndef TERM_CMD__H
#define TERM_CMD__H

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

typedef int (*cmd_func_t)(char*);

// Print help command
int com_help(char* arg);

// Exit mfterm command
int com_quit(char* arg);

// Load/Save tag file operations
int com_load_tag(char* arg);
int com_save_tag(char* arg);

// Clear (zero) tag command
int com_clear_tag(char* arg);

// Read/Write tag NFC operations
int com_read_tag(char* arg);
int com_write_tag(char* arg);

// Tag print commands
int com_print(char* arg);
int com_print_keys(char* arg);

// Tag set (value) command
int com_set(char* arg);

// Key operations
int com_keys_load(char* arg);
int com_keys_save(char* arg);
int com_keys_clear(char* arg);
int com_keys_set(char* arg);
int com_keys_import(char* arg);
int com_keys_print(char* arg);
int com_keys_test(char* arg);

// Dictionary operations
int com_dict_load(char* arg);
int com_dict_clear(char* arg);
int com_dict_attack(char* arg);
int com_dict_print(char* arg);

// Specification operations
int com_spec_load(char* arg);
int com_spec_clear(char* arg);
int com_spec_print(char* arg);

// MAC operations
int com_mac_block_compute(char* arg);

typedef struct {
  char *name;       // The command
  cmd_func_t func;  // Function to call on command
  int fn_arg;       // File name completion if > 0
  int document;     // Show in documentation if > 0
  char *doc;        // String documenting the command
} command_t;

extern command_t commands[];

// Lookup a command by name. Return a ptr to the command function, or
// NULL if the command isn't found.
command_t* find_command(const char *name);

// Any command starting with '.' - path spec
int exec_path_command(const char *line);

#endif
