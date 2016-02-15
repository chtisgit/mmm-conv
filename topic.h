#ifndef TOPIC_H
#define TOPIC_H

extern struct Topic{
	int id;
	const char *name;
	struct Topic *next;
} *topics;

void topic_add(int id, char *s);
void topic_rdestroy(struct Topic *d);
const char* topic_name(int id);

#endif
