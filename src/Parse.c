#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Event.h"
#include "Macros.h"
#include "Parse.h"

// Need to call this method before calling anything else !!
void parse_setup(Parse *self, FILE *fp)
{
	if(self->setup_flag)
		return;
	
	// Init cpuOffset array
	memset(self->cpuOff, 0, MAX_CPUS*sizeof(struct CpuOffset));

	//self->h.init(&self->h);
	mh_init(&self->h);
	self->fp = fp;		
	self->init_cpu_offset(self);
	self->setup_flag = 1;
}

void parse_init_cpu_offset(Parse *self)
{
	Event tmpev;

	while(!feof(self->fp))
	{

		if(parse_next_event(&tmpev, self->fp) != SUCCESS)
			return;

		if(!self->cpuOff[tmpev.cpu].cpuFlag)
		{
			self->cpuOff[tmpev.cpu].cpuFlag = 1;
			self->cpuOff[tmpev.cpu].nextOffset = ftell(self->fp);	// File off_t of next event.
			self->cpuOff[tmpev.cpu].ev = tmpev;
			self->numCpus++;

			// Push ev->ns on heap
			self->h.push(&self->h, self->cpuOff[tmpev.cpu]); 
		}
	}

	// Set fp to off_t 0
	rewind(self->fp);
}

int parse_get_next_event(Parse *self, Event *ev)
{
	// retrieve ev with smallest ns
	CpuOffset coff; 
	
	if(self->h.pop(&self->h, &coff) != SUCCESS)
		return FAIL;

	memcpy(ev, &coff.ev, sizeof(Event));

	off_t offt = coff.nextOffset;
	
	fseek(self->fp, offt, SEEK_SET);

	Event nextev;
	
	// Push next ev for retrieved cpu on heap
	while(!feof(self->fp))
	{
		if(parse_next_event(&nextev, self->fp) != SUCCESS)
			break;
		// If cpu of next ev is same as popped ev
		if(coff.ev.cpu == nextev.cpu)
		{
			self->cpuOff[nextev.cpu].nextOffset = ftell(self->fp);	// File off_t of next event.
			self->cpuOff[nextev.cpu].ev = nextev;
			self->h.push(&self->h, self->cpuOff[nextev.cpu]);
			break;
		}
	}

	rewind(self->fp);

	return SUCCESS;
}

struct Parse par =
{
	.setup_flag = 0,
	.numCpus = 0,
	.setup = parse_setup,
	.init_cpu_offset = parse_init_cpu_offset,
	.get_next_event = parse_get_next_event,
};

Parse* get_parse()
{
	return &par;
}

int parse_return_next_event(Event *ev)
{
	Parse *p = get_parse();
	Event tmpev;

	if(p->get_next_event(p, &tmpev) == FAIL)
		return FAIL;

	memcpy(ev, &tmpev, sizeof(Event));

	return SUCCESS;
}

/*
int get_num_cpus(void)
{
	Parse *p = get_parse();
	
	if(!self->setup_flag)
	{
		fprintf(stderr, "Parser not initialized\n");
		return -1;
	}

	return p->numCpus;
}
*/
