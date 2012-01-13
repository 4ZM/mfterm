# Copyright (C) 2011 Anders Sundman <anders@4zm.org>
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

.PHONY: clean all

all: mfterm

clean:
	rm -f mfterm *.o *~ *.bak

CFLAGS	= -g -Wall -std=c99
LDFLAGS	= -g -lreadline -lnfc
CC      = gcc

MFTERM_SRCS =   \
	mfterm.c      \
	term_cmd.c    \
	util.c        \
	tag.c         \
	mifare.c      \
	mifare_ctrl.c

MFTERM_OBJS = $(MFTERM_SRCS:.c=.o)

mfterm: $(MFTERM_OBJS)
	${CC} ${LDFLAGS} -o $@ $^ 

# Generic compilation rule - make file plumbing
%.o : %.c Makefile
	${CC} ${CFLAGS} -c $<

# makedepend section - set up include dependencies
DEPFILE		= .depends
DEPTOKEN	= '\# MAKEDEPENDS'
DEPFLAGS	= -Y -f $(DEPFILE) -s $(DEPTOKEN)

depend:
	rm -f $(DEPFILE)
	make $(DEPFILE)

$(DEPFILE):
	@echo $(DEPTOKEN) > $(DEPFILE)
	makedepend $(DEPFLAGS) -- $(CFLAGS) -- *.c 2> /dev/null

sinclude $(DEPFILE)
