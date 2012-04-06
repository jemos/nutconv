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
	char token[64];
	bool vector_read = false;
	double token_d;
	int vector_idx;
	FILE *f_in,*f_out;

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

	/* open input file, start processing */
	f_in = fopen(input_file,"r");
	if( !f_in ) {
		fprintf(stderr,"Error: Unable to open input file <%s>.\n",input_file);
		exit(EXIT_FAILURE);
	}
	nc_data *pncd;
	nc_process(f_in,&pncd);
	
	printf("; ");
	for(int i = 0 ; i < pncd->var_number ; i++)
		printf("%s%s",pncd->headers[i].name,(i < (pncd->var_number-1) ? "," : "\n"));

	return EXIT_SUCCESS;
	
	f_in = fopen(input_file,"r");
	f_out = fopen(output_file,"w");
	vector_read = false;
	vector_idx = 0;
	while(!feof(f_in))
	{
		if( vector_read )
		{
			if( fscanf(f_in,"%lf",&token_d) == 1 ) {
				fprintf(f_out,"%s%e", vector_idx ? "," : "",token_d);
				vector_idx++;

				if(vector_idx == 81) {
					vector_idx = 0;
					fprintf(f_out,"\n");
					printf(".");
				}
				continue;
			}	

			vector_read = false;
			vector_idx = 0;
			fprintf(f_out,"\n");
		}

		if( (fscanf(f_in,"%s",token) == 1) && !strcmp(token,"Values:") )
		{
			/* start reading vector */
			vector_read = true;
			continue;
		}
	}
	fclose(f_in);
	fclose(f_out);

	return EXIT_SUCCESS;
}

