#include "mmm-conv.h"
#include "topic.h"

#include <assert.h>
#include <stdlib.h>

struct Topic *topics = NULL;

void topic_add(int id, char *s)
{
	struct Topic *d = topics;

	if(topics == NULL){
		d = topics = malloc( sizeof(*topics) );
	}else{
		d = topics;
		while(d->next != NULL)
			d = d->next;

		d->next = malloc( sizeof(*topics) );
		d = d->next;
	}
	if(d == NULL)
		not_enough_mem(sizeof(*topics), __LINE__);

	d->id = id;
	d->name = s;
	d->next = NULL;
}

static void topic_destroy(struct Topic *d)
{
	free((void*) d->name);
	free(d);
}

void topic_rdestroy(struct Topic *d)
{
	if(d == NULL)
		return;
	
	topic_rdestroy(d->next);
	topic_destroy(d);
}

const char* topic_name(int id)
{
	struct Topic *d = topics;
	while(d != NULL){
		if(d->id == id)
			return d->name;
		
		d = d->next;
	}
	return NULL;
}
