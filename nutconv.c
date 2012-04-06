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


#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>

#include "nclib.h"

void version(void)
{
	printf(	"nutconv v0.1\n"
			"Copyright (c) 2012 - Jean-Francois Mousinho\n");
}

void help(void)
{
	version();
	printf("\nUsage: nutconv <-i input> [-o output] [-f format]\n"); 
}

int main(int argc,char *argv[])
{
	char c;
	char input_file[512] = {'\0'};
	char output_file[512] = {'\0'};
	char output_format[32] = {'\0'};
	FILE *f_in,*f_out = stdout;

	while((c = getopt(argc,argv,"i:o:f:vh")) != -1)
	{
		switch(c)
		{
			case 'i':
				strncpy(input_file,optarg,sizeof(input_file));
				break;
			case 'o':
				strncpy(output_file,optarg,sizeof(output_file));
				break;
			case 'f':
				strncpy(output_format,optarg,sizeof(output_format));
				break;
			case 'v':
				version();
				return EXIT_SUCCESS;
			default:
			case 'h':
				help();
				return EXIT_SUCCESS;
		}
	}

	if( !nc_initialize() ) {
		fprintf(stderr,"Error: Failed to initialize nutconv library.\n");
		exit(EXIT_FAILURE);
	}

	if( !strlen(input_file) ) {
		fprintf(stderr,"Error: Missing input file argument.\n");
		exit(EXIT_FAILURE);
	}
	
	if( strlen(output_file) )
	{
		f_out = fopen(output_file,"w");
		if( !f_out ) {
			fprintf(stderr,"Error: Unable to open the output file for write <%s>.\n",output_file);
			exit(EXIT_FAILURE);
		}
	}
	
	/* validate output format before processing the file */
	if( (output_format[0] == '\0') || (!strcmp(output_format,"csv")) ) {
		// OK
	} else if( !strcmp(output_format,"matlab") ) {
		// OK
	} else {
		fprintf(stderr,"Error: Invalid or unsupported output format given (%s)\n",output_format);
		exit(EXIT_FAILURE);
	}


	/* open input file, start processing */
	f_in = fopen(input_file,"r");
	if( !f_in ) {
		fprintf(stderr,"Error: Unable to open input file <%s>.\n",input_file);
		exit(EXIT_FAILURE);
	}
	
	nc_data *pncd;
	if( !nc_process(f_in,&pncd) ) {
		fprintf(stderr,"\nError: nclib failed to process results file (error description: %s)\n\n",nc_strerror());
		exit(EXIT_FAILURE);
	}
	printf("Processed %u plots.\n",pncd->plot_number);

	fclose(f_in);
	
	if( (output_format[0] == '\0') || (!strcmp(output_format,"csv")) ) {
		nc_output_csv(pncd,f_out);
	} else if( !strcmp(output_format,"matlab") ) {
		nc_output_matlab(pncd,f_out);
	}
	
	if( f_out != stdout ) {
		fclose(f_out);
	}
	
	return EXIT_SUCCESS;
}

