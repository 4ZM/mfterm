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

int com_help(char* arg);
int com_quit(char* arg);
int com_read_file(char* arg);
int com_write_file(char* arg);
int com_read_dev(char* arg);
int com_write_dev(char* arg);
int com_display_raw(char* arg);
int com_display_keys(char* arg);

typedef struct {
  char *name;
  cmd_func_t func;
  char *doc;
} command_t;

extern command_t commands[];

command_t *find_command();

#endif
