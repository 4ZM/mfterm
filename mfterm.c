/**
 * Copyright (C) 2011 Anders Sundman <anders@4zm.org>
 *
 * This file is part of mfterm.
 *
 * mfterm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mfterm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mfterm.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Parts of code used in this file are from the GNU readline library file
 * fileman.c (GPLv3). Copyright (C) 1987-2009 Free Software Foundation, Inc
 */

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "term_cmd.h"
#include "mfterm.h"
#include "util.h"

int stop_input_loop_ = 0;
void stop_input_loop() {
  stop_input_loop_ = 1;
}

char *completion_cmd_generator();
char **mft_completion();
int execute_line(char *line);
void initialize_readline();

int main(int argc, char** argv) {
  initialize_readline();
  input_loop();
  return 0;
}

// Request user input until stop_intput_loop_ == 0
void input_loop() {
  char *line, *s;
  while (stop_input_loop_ == 0) {
    line = readline ("$ ");

    if (!line)
      break;

    s = trim(line);

    if (*s) {
      add_history(s);
      execute_line(s);
    }
    free (line);
  }
}

/* Execute a command line. */
int execute_line (char *line) {
  char* word = strtok(line, " ");
  command_t* command;
  command = find_command(word);

  if (!command) {
    fprintf (stderr, "%s: No such command.\n", word);
    return -1;
  }

  char* arg = strtok(NULL, " ");
  // Pull out more args here...

  return (*(command->func))(arg);
}

void initialize_readline()
{
  rl_readline_name = "MFT";
  rl_attempted_completion_function = (rl_completion_func_t*)mft_completion;
}

/* Attempt to complete on the contents of TEXT.  START and END bound the
   region of rl_line_buffer that contains the word to complete.  TEXT is
   the word to complete.  We can use the entire contents of rl_line_buffer
   in case we want to do some simple parsing.  Return the array of matches,
   or NULL if there aren't any. */
char** mft_completion(char* text, int start, int end) {
  char **matches = (char **)NULL;


  if (start == 0) {
    // Commands start at 0
    matches = rl_completion_matches(text, completion_cmd_generator);
  }
  else {
    // Don't complete on anything else (default is complete on filename)
    rl_attempted_completion_over = 1;
  }

  return matches;
}


/**
  Called to generate completion suggestions.
  state == 0 on first call.
 */
char* completion_cmd_generator(char* text, int state) {
  static int list_index, len;
  char *name;

  // First call?
  if (!state) {
    list_index = 0;
    len = strlen(text); // Cached for performance
  }

  // Return next suggestion
  while ((name = commands[list_index].name)) {
    ++list_index;

    if (strncmp(name, text, len) == 0) {
      char* r = malloc(strlen(name) + 1);
      strcpy(r, name);
      return r;
    }
  }

  // No (more) matches
  return (char*) NULL;
}

// Add more completion_xxx_generators here...

