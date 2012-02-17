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
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "term_cmd.h"
#include "mfterm.h"
#include "util.h"
#include "spec_syntax.h"

int stop_input_loop_ = 0;
void stop_input_loop() {
  stop_input_loop_ = 1;
}

char* completion_cmd_generator(const char* text, int state);
char* completion_sub_cmd_generator(const char* text, int state);
char* completion_spec_generator(const char* text, int state);
int perform_filename_completion();
char** mft_completion(char* text, int start, int end);
int execute_line(char* line);
void initialize_readline();

typedef char** rl_completion_func_t(const char*, int, int);

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
int execute_line (char* line) {
  if (strncmp(line, ".", 1) == 0)
    return exec_path_command(line);

  command_t* command = find_command(line);

  if (!command) {
    fprintf (stderr, "%s: No such command.\n", line);
    return -1;
  }

  // Skip past command and ws to the arguments
  line += strlen(command->name);
  line = trim(line);

  return (*(command->func))(line);
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

  // Don't complete on files for most cases
  rl_attempted_completion_over = 1;

  // Add the trailing space unless told otherwise
  rl_completion_suppress_append = 0;

  // Complete strings starting with '.' as specification paths
  if (text[0] == '.' && instance_root) {
    rl_completion_suppress_append = 1; // no trailing space on paths
    return rl_completion_matches(text, completion_spec_generator);
  }

  // Commands start at 0
  if (start == 0)
    return rl_completion_matches(text, completion_cmd_generator);

  // else: Sub commands and file arguments start at > 0

  // Try to match sub commands
  matches = rl_completion_matches(text, completion_sub_cmd_generator);
  if (matches)
      return matches;

  if (perform_filename_completion()) {
    // Do complete on filenames
    rl_attempted_completion_over = 0;
    return matches;
  }

  return matches;
}


int perform_filename_completion() {
  for (int i = 0; commands[i].name; ++i) {
    if (commands[i].fn_arg &&
        strncmp(rl_line_buffer, commands[i].name,
                strlen(commands[i].name)) == 0) {
      return 1;
    }
  }
  return 0;
}

/**
  Called to generate completion suggestions.
  state == 0 on first call.
 */
char* completion_cmd_generator(const char* text, int state) {
  static int cmd_index, len;

  // First call?
  if (!state) {
    cmd_index = 0;
    len = strlen(text); // Cached for performance
  }

  // Return next suggestion
  char *name;
  while ((name = commands[cmd_index].name)) {
    ++cmd_index;

    // Check if the command is applicable
    if (strncmp(name, text, len) == 0) {
      char* r = malloc(strlen(name) + 1);
      strcpy(r, name);
      return r;
    }
  }

  // No (more) matches
  return (char*) NULL;
}

char* completion_sub_cmd_generator(const char* text, int state) {
  static int cmd_index, len, full_len;

  // First call?
  if (!state) {
    cmd_index = 0;
    len = strlen(text); // Cached for performance
    full_len = strlen(rl_line_buffer);
  }

  // Return next suggestion
  char* name;
  while ((name = commands[cmd_index].name)) {
    ++cmd_index;

    // Extract command and sub-command
    char buff[128];
    strncpy(buff, name, sizeof(buff));
    char* cmd = strtok(buff, " ");
    char* sub = strtok(NULL, " ");

    // Make sure the command *has* a sub command
    // and that we have the right command.
    if (cmd && sub && strncmp(rl_line_buffer, name, full_len) == 0) {
      // Check if the sub command is applicable
      if (strncmp(sub, text, len) == 0) {
        char* r = malloc(strlen(sub) + 1);
        strcpy(r, sub);
        return r;
      }
    }
  }

  // No (more) matches
  return (char*) NULL;
}

/**
 * Called to generate completion suggestions.
 * state == 0 on first call.
 */
char* completion_spec_generator(const char* text, int state) {

  // Parent context is initialized on the first call
  static instance_t* parent_inst;
  static const char* parent_end;
  static int parent_end_len;

  // Instace iter is advanced on each repeated call
  static instance_list_t* inst_iter;

  // First call?
  if (!state) {

    // Set the parent context
    if (parse_partial_spec_path(text, &parent_end, &parent_inst) != 0)
      return NULL; // on error

    parent_end_len = strlen(parent_end);

    // The instance iter points to the first fields
    inst_iter = parent_inst->fields;
  }

  while (inst_iter) {
    instance_t* inst = inst_iter->instance;
    inst_iter = inst_iter->next_;

    // Anonymous filler field
    if (inst->field->name == NULL)
      continue;

    char* fname = inst->field->name;
    int fname_len = strlen(fname);

    // Check if the field is applicable - right prefix
    if (fname_len >= parent_end_len &&
        strncmp(fname, parent_end, parent_end_len) == 0) {

      int parent_len = parent_end - text;
      char* str = malloc(parent_len + fname_len + 1);

      // The parent part ending with '.'
      strncpy(str, text, parent_len);

      // The field
      strncpy(str + parent_len, fname, fname_len);

      // Null termination
      *(str + parent_len + fname_len) = '\0';

      return str;
    }
  }

  // No (more) matches
  return NULL;
}
