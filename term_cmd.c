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
#include "term_cmd.h"

command_t commands[] = {
  { "help", com_help, "Display this text" },
  { "?", com_help, "Synonym for 'help'" },
  { "quit", com_quit, "Quit" },
  { "exit", com_quit, "Synonym for 'quit'" },
  { "read-file", com_read_file, "Read card data from a file" },
  { "write-file", com_write_file, "Write card data to a file" },
  { "read-device", com_read_dev, "Read card data from a device" },
  { "write-device", com_write_dev, "Write card data to a card" },
  { (char *)NULL, (cmd_func_t)NULL, (char *)NULL }
};


/* Look up NAME as the name of a command, and return a pointer to that
   command.  Return a NULL pointer if NAME isn't a command name. */
command_t * find_command(char *name) {
  for (int i = 0; commands[i].name; i++)
    if (strcmp (name, commands[i].name) == 0)
      return &commands[i];

  return (command_t *)NULL;
}

int com_help(char* arg) {
  if (arg != 0) {
    for (int i = 0; commands[i].name; i++) {
      if (strcmp (arg, commands[i].name) == 0) {
        printf ("\t%s\t\t%s.\n", commands[i].name, commands[i].doc);
        return 0;
      }
    }
    printf ("No commands match `%s'\n", arg);
  }

  // Fall though case...
  for (int i = 0; commands[i].name; i++) {
    printf ("\t%s\t\t%s.\n", commands[i].name, commands[i].doc);
  }

  return 0;
}

int com_quit(char *arg) {
  stop_input_loop();
  return 0;
}

int com_read_file() {
  printf("TBD - read from file\n");
  return 0;
}

int com_write_file() {
  printf("TBD - read write to file\n");
  return 0;
}

int com_read_dev() {
  printf("TBD - read from device\n");
  return 0;
}

int com_write_dev() {
  printf("TBD - write to device\n");
  return 0;
}

