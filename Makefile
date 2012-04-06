#
# nutconv v0.1
# Copyright (C) 2012 Jean-François Mousinho
#
# This file is part of nutconv.
#
# nutconv is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# nutconv is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with nutconv.  If not, see <http://www.gnu.org/licenses/>.
#

CC		:= gcc
#CFLAGS	:= -g -O -Wall -pedantic -std=c99 -Wuninitialized -Wsign-compare -Wno-pointer-sign -Wformat-security
CFLAGS	:= -g -Wall -pedantic -std=c99
LFLAGS	:=
LIBS	:=
OBJS	:= nutconv.o nclib.o

all: nutconv

nutconv: nutconv.c nclib.h nclib.c
	$(CC) $(CFLAGS) -o nutconv nutconv.c nclib.c

clean:
	rm *.o nutconv 2>&1 > /dev/null | echo > /dev/null

