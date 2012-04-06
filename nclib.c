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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "nclib.h"

static char nc_errormsg[256] = {'\0'};
static bool b_verbose = false;
static bool b_multiplot = false;

/*
   nc_initialize
   Allocate any data structure that might be needed and initialize variables.
   Returns true on success, false otherwise. Error message can be obtained
   trought nc_strerror() function.
*/
bool nc_initialize(void)
{
	nc_errormsg[0] = '\0';
	return true;
}

bool aux_readline(FILE *f,char *buffer_ptr,size_t buffer_size,unsigned int *line_idx)
{
	if( fgets(buffer_ptr,buffer_size,f) == NULL )
		return false;
		
	if( strchr(buffer_ptr,'\n') )
		*(strchr(buffer_ptr,'\n')) = '\0';
	
	if( strchr(buffer_ptr,'\r') )
		*(strchr(buffer_ptr,'\n')) = '\0';
		
	(*line_idx)++;
	return true;
}
/*
   nc_process
*/
bool nc_process(FILE *input_file,nc_data **pncd)
{
	static char buffer[1024];
	nc_data *ncd = NULL, *ncd_head = NULL, *ncd_new = NULL;
	unsigned int line_idx;
	char sz_title[] = "Title: ";
	char sz_novars[] = "No. Variables: ";
	char sz_nopts[] = "No. Points: ";
	char sz_variables[] = "Variables:";
	char sz_points[] = "Values:";
	bool b_atvars = false;
	unsigned int headers_left = 0;
	size_t alloc_size;
	nc_header_entry *aux_phdr = NULL;
	unsigned int plot_number = 0;
	 
	/* allocate a new data structure */
	ncd_head = NULL;
	
	/*
		the file starts with the following header:
		
		Title: (no title)
		Date: 5:52:14 AM, Fri Apr 6, 2012
	*	Plotname: DC Analysis `VGS_SWEEP-000_VDS_SWEEP': VDN = (0 -> 3.3)
	*	Flags: real
	*	No. Variables:       80    
	*	No. Points:      101   
	*	Variables:  0   VDN sweep 
	*				...
	
	(*) Headers associated for each plot.
	
	*/
	rewind(input_file);
	
next_plot:

	ncd_new = (nc_data*)malloc(sizeof(nc_data));
	if( !ncd_new ) {
		strcpy(nc_errormsg,"Unable to allocate memory for ncdata.");
		return false;
	}
	memset(ncd_new,'\0',sizeof(nc_data));
	
	if( ncd_head == NULL )
		ncd_head = ncd = ncd_new;

	b_atvars = false;
	while( aux_readline(input_file,buffer,sizeof(buffer),&line_idx) )
	{
		if( buffer[0] == '\0' )
			continue;
			
		if( !memcmp(buffer,sz_title,sizeof(sz_title)-1) ) {
			strncpy(ncd_new->title,buffer+sizeof(sz_title)-1,sizeof(ncd_new->title));
		}
		
		if( !memcmp(buffer,sz_novars,sizeof(sz_novars)-1) )
		{
			if( !sscanf(buffer+sizeof(sz_novars)-1,"%u",&(ncd_new->var_number)) ) {
				strcpy(nc_errormsg,"Unable to process \"No. Variables\" token.");
			}
		}
		
		if( !memcmp(buffer,sz_nopts,sizeof(sz_nopts)-1) )
		{
			if( !sscanf(buffer+sizeof(sz_nopts)-1,"%u",&(ncd_new->point_number)) ) {
				strcpy(nc_errormsg,"Unable to process \"No. Points\" token.");
			}
		}
		
		if( !memcmp(buffer,sz_variables,sizeof(sz_variables)-1) ) {
			b_atvars = true;
			break;
		}
	}
	
	if( !b_atvars )
	{	
		if( ncd_new != ncd ) {
			/* No more plots to process. */
			free(ncd_new);
		}
		*pncd = ncd_head;
		ncd_head->plot_number = plot_number;
		return true;
		
//		strcpy(nc_errormsg,"Unable to find variables definition.");
//		goto fail_free_ncd;
	}
	
	if( !ncd->var_number ) {
		strcpy(nc_errormsg,"The results file doesnt contain any variable.");
		goto fail_free_ncd;
	}
	
	/* link this new ncd to the plots if it is multi-plot result */
	if( ncd_new != ncd ) {
		ncd->next_plot = ncd_new;
		ncd = ncd_new;
	}
	
	/* we've the header informations, allocate the required memory for headers */
	alloc_size = sizeof(nc_header_entry)*ncd->var_number;
	ncd->headers = (nc_header_list)malloc(alloc_size);
	if( !ncd->headers ) {
		sprintf(nc_errormsg,"Unable to allocate headers data structure "
			"(malloc failed for %u bytes).",(unsigned int)alloc_size);
		goto fail_free_ncd;
	}
	memset(ncd->headers,0,alloc_size);
	headers_left = ncd->var_number;
	
	/* Copy header's informations, here we assume the file pointer is at "Variables: ". */
	
	assert(b_atvars); // just to be sure..
	aux_phdr = &(ncd->headers[0]);
	if( !(sscanf(buffer+sizeof(sz_variables)-1,"%u %s",&(aux_phdr->index),aux_phdr->name) == 2) ) {
		sprintf(nc_errormsg,"Unable to read first variable at line %u.",line_idx);
		goto fail_free_headers;
	}
	headers_left--; //zero based countdown
	aux_phdr++;
	
	while( headers_left && aux_readline(input_file,buffer,sizeof(buffer),&line_idx) )
	{
		if( !(sscanf(buffer,"%u %s %s",&(aux_phdr->index),aux_phdr->name,aux_phdr->unit) == 3) ) {
			sprintf(nc_errormsg,"Unable to read header information (line %u).",line_idx);
			goto fail_free_headers;
		}
		headers_left--;
		aux_phdr++;
	}
	
	/* Allocate memory to store the values. */
	if( !ncd->point_number ) {
		sprintf(nc_errormsg,"Point number is zero, not going to read any point data.");
		return true;
	}
	alloc_size = sizeof(nc_point)*ncd->point_number;
	ncd->points = (nc_point_list)malloc(alloc_size);
	if( !ncd->headers ) {
		sprintf(nc_errormsg,"Unable to allocate points data structure "
			"(malloc failed for %u bytes).",(unsigned int)alloc_size);
		goto fail_free_headers;
	}
	memset(ncd->points,0,alloc_size);
	
	// Locate the "Points:" line.
	while( aux_readline(input_file,buffer,sizeof(buffer),&line_idx) ) {
		if( !memcmp(buffer,sz_points,sizeof(sz_points)-1) )
			goto found_points_line;
	}
	sprintf(nc_errormsg,"Unable to find \"Points:\" line in results.");
	goto fail_free_points;

found_points_line:
	;
	unsigned int point_idx = 0;
	unsigned int value_idx = 0;
	unsigned int points_left = ncd->point_number; //zero based countdown
	unsigned int vars_left = 0;
	nc_point *aux_ppoint;
	
	/* Read all points */	
	while( points_left )
	{
		//fprintf(stderr,"Reading point %u (left to read %u)...\n",point_idx,points_left);
		aux_ppoint = &(ncd->points[point_idx]);
		aux_ppoint->values = (double*)malloc(sizeof(double)*ncd->var_number);
		memset(aux_ppoint->values,0,sizeof(double)*ncd->var_number);
		value_idx = 0;
		vars_left = ncd->var_number + 1; //zero based countdown
		
		/* Read point' value. */
		while( vars_left && fscanf(input_file,"%lf",&aux_ppoint->values[value_idx]) ) {
			vars_left--;
			value_idx++;
		}
		
		point_idx++;
		points_left--;
	}
	
	plot_number++;
	goto next_plot;

fail_free_points:
	free(ncd->points);
	
fail_free_headers:
	free(ncd->headers);
	
fail_free_ncd:
	free(ncd);
	return false;
}

bool nc_uninitialize(void)
{
	return true;
}

const char *nc_strerror(void)
{
	return (const char*)nc_errormsg;
}

bool nc_output_csv(nc_data *pncd,FILE *fout)
{
	/* Output file in CSV format, default. */
	fprintf(fout,"; ");
	for(int i = 0 ; i < pncd->var_number ; i++)
		fprintf(fout,"%s%s",pncd->headers[i].name,(i < (pncd->var_number-1) ? "," : "\n"));
	
plot_next:	
	for( int pidx = 0 ; pidx < pncd->point_number ; pidx++ )
	{
		for( int vidx = 0 ; vidx < pncd->var_number ; vidx++ ) {
			fprintf(fout,"%0.6e%s",pncd->points[pidx].values[vidx],(vidx < (pncd->var_number-1) ? "\t" : "\n"));
		}
	}
	
	if( pncd->next_plot ) {
		pncd = pncd->next_plot;
		goto plot_next;
	}
	
	return true;
}

bool nc_output_matlab(nc_data *pncd,FILE *fout)
{
	return false;
}
