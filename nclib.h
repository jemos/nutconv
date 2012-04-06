/*
	Copyright (c) 2012 Jean-François Mousinho

	Permission is hereby granted, free of charge, to any person obtaining
	a copy of this software and associated documentation files (the
	"Software"), to deal in the Software without restriction, including
	without limitation the rights to use, copy, modify, merge, publish,
	distribute, sublicense, and/or sell copies of the Software, and to
	permit persons to whom the Software is furnished to do so, subject to
	the following conditions:

	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
	LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
	OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
	WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _NCLIB_H_
#define _NCLIB_H_

#include <stdio.h>
#include <stdbool.h>

#define NC_MAX_TITLE_SIZE 64
#define NC_MAX_HEAD_ENTRY_SIZE 64
#define NC_MAX_HEAD_UNIT_SIZE 16

typedef struct _nc_header_entry {
	unsigned int index;
	char name[NC_MAX_HEAD_ENTRY_SIZE];
	char unit[NC_MAX_HEAD_UNIT_SIZE];
} nc_header_entry, *nc_header_list;

typedef struct _nc_point {
	unsigned int index;
	double *values;
} nc_point, *nc_point_list;

typedef struct _nc_data {
	char filename[FILENAME_MAX];
	char title[NC_MAX_TITLE_SIZE];
	nc_header_list headers;
	nc_point_list points;
	unsigned int var_number;
	unsigned int point_number;
} nc_data;

bool nc_initialize(void);
bool nc_process(FILE *input_file,nc_data **pncd);
bool nc_uninitialize(void);

#endif
