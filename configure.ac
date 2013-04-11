# Copyright (C) 2012 Anders Sundman <anders@4zm.org>
#
# This file is part of mfterm.
#
# mfterm is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# mfterm is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with mfterm.  If not, see <http://www.gnu.org/licenses/>.

AC_PREREQ([2.67])
AC_INIT([mfterm], [1.0.2], [anders@4zm.org])
AC_CONFIG_SRCDIR([mfterm.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE

# Checks for programs.
AC_CHECK_PROGS(YACC, 'bison -y' byacc yacc, :)
if test "x$YACC" == "x:"; then
  AC_MSG_ERROR([Bison or some other YACC compatible parser generator is required.])
fi
AC_PROG_YACC

AC_PROG_CC

AC_CHECK_PROGS(LEX, flex lex, :)
if test "x$LEX" == "x:"; then
  AC_MSG_ERROR([Lex or Flex or is required.])
fi
AM_PROG_LEX

AC_PROG_MAKE_SET
AC_PROG_RANLIB
AM_PROG_CC_C_O

# Checks for libraries.
AC_CHECK_LIB([nfc], [nfc_open], [],
             [AC_MSG_ERROR([libnfc >= 1.7 is required (http://www.libnfc.org/download)])])

AC_CHECK_LIB([readline], [rl_completion_matches], [],
             [AC_MSG_ERROR([libreadline is required])])

AC_CHECK_LIB([crypto], [DES_set_key_unchecked], [HAVE_LIBCRYPTO=yes],
             [AC_MSG_ERROR([libcrypto is required])])

# Checks for header files.
AC_CHECK_HEADERS([stddef.h stdint.h stdlib.h string.h strings.h], [],
                 [AC_MSG_ERROR([A required header file was not found.])])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset strcasecmp strchr strdup strtol strtoul], [],
               [AC_MSG_ERROR([A required function was not found.])])


AC_CONFIG_FILES([Makefile])

AC_OUTPUT
