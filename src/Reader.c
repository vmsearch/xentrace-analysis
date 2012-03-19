#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include "EventHandler.h"
#include "Macros.h"
#include "Reader.h"


void reader_init(Reader *reader, const char *filename)
{
	int i = 0;
	if((reader->fp = fopen(filename, "rb")) == NULL)
	{
		strerror(errno);
		printf("Usage: ./a.out <filename>\n");
		exit(0);
	}

	/* Init Hash table */
	reader->handler_array = (EventHandler *)malloc(sizeof(struct EventHandler) * MAX_EVENTS); 

	for(i = 0; i < MAX_EVENTS; i++)
	{
		/* Init evh struct */
		reader->handler_array[i].event_id = 0;
		reader->handler_array[i].data = NULL;
		/* INIT_LIST_HEAD called during handler registartion */
		reader->handler_array[i].init = NULL;
		reader->handler_array[i].process_event = NULL;
		reader->handler_array[i].finalize = NULL;
	}
	/* Need to free memory later */
}

void reader_exit(Reader *reader)
{
	fclose(reader->fp);

	free(reader->handler_array);
	return;
}

int reader_loop(Reader *reader)
{
	int ret = 0;
	Event event;

	clear_event(&event);

	evh_call_initializers(reader);

	do
	{
		ret = parse_next_event(&event, reader->fp);
		if (ret == FAIL)
			break;
		evh_call_handlers(reader, &event);

	} while(!feof(reader->fp));

	evh_call_finalizers(reader);
	
	return SUCCESS;
}