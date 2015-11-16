#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define DEBUG

#ifdef DEBUG
#define debug(...) fprintf(stderr, "debug:: " __VA_ARGS__)
#else
#define debug(...) ;
#endif

#define error(...) fprintf(stderr, "Error: " __VA_ARGS__), exit(1)


FILE *infile = NULL;

void help(const char *progname)
{
	fprintf(stderr, "Usage: %s <topic file> <question file>\n\n", progname);
	exit(2);
}


void not_enough_mem(void)
{
	error("out of memory!\n\n");
}

void close_infile(void)
{
	if(infile) fclose(infile);
}

char* readstring(const int byte)
{
	char *s;
	size_t len;

	if(byte == 1){
		len = (unsigned char) fgetc(infile);
	}else if(byte == 2){
		len = ((unsigned char) fgetc(infile)) << 8;
		len |= (unsigned char) fgetc(infile); /* big endian */
	}else{
		debug("readstring: parameter wrong\n");
		return NULL;
	}

	if(len == 0){
		debug("readstring: len == 0\n");
		return NULL;
	}
	s = malloc(len+1);
	if(s == NULL)
		not_enough_mem();

	if(fread(s, 1, len, infile) != len){
		free(s);
		debug("readstring: fread failed\n");
		return NULL;
	}

	s[len] = '\0';

	return s;
}

void rtrim(char *s)
{
	int i, j;
	for(i = 0; s[i] != '\0'; i++){
		if(s[i] != ' ' && s[i] != '\r' && s[i] != '\n' && s[i] != '\t')
			j = i;
	}
	s[j+1] = '\0';
}

int readheader_questionfile(void)
{
	char *s = readstring(1);
	if(s == NULL)
		error("Error in header. Aborting.");

	rtrim(s);
	fprintf(stderr, "### %s\n", s);
	free(s);

	s = readstring(1);
	if(s == NULL)
		error("Error in header. Aborting.");

	rtrim(s);
	fprintf(stderr, "### %s\n", s);

	fgetc(infile); /* two bytes that I don't */
	fgetc(infile); /* know much about */

	if(strcmp(s, "Datei-Version 3.0") != 0){
		fprintf(stderr, "WARNING: unknown version\n");
		free(s);
		return 0;
	}else{
		free(s);
		return 300;
	}
}

int proceed(void)
{
	char c;
	fprintf(stderr, "Continue? [y/n] ");
	do
		c = tolower(getchar());
	while(c != 'y' && c != 'n' && c != EOF);

	return c == 'y' ? 1 : 0;
}

const char CAP_AUML[] = "&Auml;";
const char CAP_OUML[] = "&Ouml;";
const char CAP_UUML[] = "&Uuml;";
const char LOW_AUML[] = "&auml;";
const char LOW_OUML[] = "&ouml;";
const char LOW_UUML[] = "&uuml;";
const char SZLIG[] = "&szlig;";


char* create_escaped_string(const unsigned char *s)
{
	char *snew, *ptr;
	size_t len = 0, i;

	for(i = 0; s[i] != '\0'; i++){
		switch(s[i]){
		case 0xC4: /* &Auml; */
			len += sizeof(CAP_AUML)-1;
			break;

		case 0xE4: /* &auml; */
			len += sizeof(LOW_AUML)-1;
			break;

		case 0xD6: /* &Ouml; */
			len += sizeof(CAP_OUML)-1;
			break;

		case 0xF6: /* &ouml; */
			len += sizeof(LOW_OUML)-1;
			break;

		case 0xDC: /* &Uuml; */
			len += sizeof(CAP_UUML)-1;
			break;

		case 0xFC: /* &uuml; */
			len += sizeof(LOW_UUML)-1;
			break;

		case 0xDF: /* &szlig; */
			len += sizeof(SZLIG)-1;
			break;
		
		default:
			len++;
		}
	}

	snew = malloc(len+1);

	if(snew == NULL)
		not_enough_mem();	

	ptr = snew;
	for(i = 0; s[i] != '\0'; i++){
		switch(s[i]){
		case 0xC4: /* &Auml; */
			strcpy(ptr, CAP_AUML);
			ptr += sizeof(CAP_AUML)-1;
			break;

		case 0xE4: /* &auml; */
			strcpy(ptr, LOW_AUML);
			ptr += sizeof(LOW_AUML)-1;
			break;

		case 0xD6: /* &Ouml; */
			strcpy(ptr, CAP_OUML);
			ptr += sizeof(CAP_OUML)-1;
			break;

		case 0xF6: /* &ouml; */
			strcpy(ptr, LOW_OUML);
			ptr += sizeof(LOW_OUML)-1;
			break;

		case 0xDC: /* &Uuml; */
			strcpy(ptr, CAP_UUML);
			ptr += sizeof(CAP_UUML)-1;
			break;

		case 0xFC: /* &uuml; */
			strcpy(ptr, LOW_UUML);
			ptr += sizeof(LOW_UUML)-1;
			break;

		case 0xDF: /* &szlig; */
			strcpy(ptr, SZLIG);
			ptr += sizeof(SZLIG)-1;
			break;

		case '<':
		case '>':		
			*ptr++ = ' ';
			break;

		default:
			*ptr++ = s[i];
		}
	}
	*ptr = '\0';

	return snew;
}

struct Topics{
	int id;
	const char *name;
	struct Topics *next;
} *topics = NULL;

void topics_add(int id, char *s)
{
	struct Topics *d = topics;

	if(topics == NULL){
		d = topics = malloc( sizeof(struct Topics) );
	}else{
		d = topics;
		while(d->next != NULL)
			d = d->next;

		d->next = malloc( sizeof(struct Topics) );
		d = d->next;
	}
	if(d == NULL)
		not_enough_mem();

	d->id = id;
	d->name = s;
	d->next = NULL;
}

void destroy_topicslist(struct Topics *d)
{
	if(d == NULL)
		return;
	
	destroy_topicslist(d->next);

	free((void*)d->name);
	free(d);
}

const char* get_topic_name(int id)
{
	struct Topics *d = topics;
	while(d != NULL){
		if(d->id == id)
			return d->name;
		
		d = d->next;
	}
	return NULL;
}

int parse_question(void)
{
	static int last_category = 0;
	unsigned char header[23];
	int i, a_len;
	char *question;
	char answer[256];

	fread(header, 1, 23, infile);

	/* text */
	question = readstring(2);
	if(question == NULL)
		return 0; /* exit silently */
	char *esc_question = create_escaped_string(question);
	if(esc_question == NULL)
		error("esc_question == NULL");
	free(question);

	printf("<question>\n<text>%s</text>\n", esc_question);
	free(esc_question);

	/* category */
	i = header[4] | header[5] | header[6] | header[7];
	if(i == 0)
		i = last_category;
	else
		i = (header[6] << 8) | header[7];

	const char *topicname = get_topic_name(i);
	if(topicname == NULL)
		error("no category with id %d", i);
	
	printf("<category>%s</category>\n", topicname);
	last_category = i;

	/* number */
	i = (header[2] << 8) | header[3];
	printf("<number>%d</number>\n", i);

	/* points */
	printf("<points>%d</points>\n", header[15]);
	
	/* follow-up question */
	i = (header[10] << 8) | header[11];
	printf("<followup>%d</followup>\n", i);

	/* answers */
	for(i = 0; i < 4; i++){
		a_len = fgetc(infile) << 8;
		a_len |= fgetc(infile); /* big endian */
		if(fread(answer, 1, a_len, infile) != a_len)
			return 0;

		answer[a_len] = '\0';
		
		char *esc_answer = create_escaped_string(answer);
		if(esc_answer == NULL)
			error("esc_answer == NULL");

		printf("<answer>\n<text>%s</text>\n", esc_answer);
		printf("<correct>%c</correct>\n</answer>\n", (header[18] & (1 << i)) ? '1' : '0');
		
		free(esc_answer);
	}

	printf("</question>\n\n");

	/* 'magic' dword after each question = 0 */
	uint32_t nul;
	fread(&nul, 4, 1, infile);
	if(nul != 0)
		debug("nul = %p\n", nul);

	return 1;
}


void parse_questionfile(void)
{
	printf("<questionfile>\n");
	while(!feof(infile) && parse_question());
	printf("</questionfile>\n");
}

int readheader_topicfile(void)
{
	char *s = readstring(1);
	if(s == NULL)
		error("Error in header. Aborting.");

	rtrim(s);
	fprintf(stderr, "### %s\n", s);
	free(s);

	s = readstring(1);
	if(s == NULL)
		error("Error in header. Aborting.");

	rtrim(s);
	fprintf(stderr, "### %s\n", s);

	if(strcmp(s, "Datei-Version 2.0") != 0){
		fprintf(stderr, "WARNING: unknown version\n");
		free(s);
		return 0;
	}else{
		free(s);
		return 300;
	}
	
}

int parse_topic(void)
{
	unsigned char header[6];
	char *name;
	int id;

	fread(header, 1, 6, infile);

	name = readstring(1);
	if(name == NULL)
		return 0;

	id = (header[4] << 8) | header[5];

	char *esc_name = create_escaped_string(name);
	if(esc_name == NULL)
		not_enough_mem();

	debug("\t - %s [%d]\n", esc_name, id);

	free(name);

	topics_add(id, esc_name);
	return 1;
}

void parse_topicfile(void)
{
	debug("Table of Topics:\n");
	while(!feof(infile) && parse_topic());
}

int main(int argc, char **argv)
{
	if(argc < 3)
		help(argv[0]);

	atexit(close_infile);
	infile = fopen(argv[1], "r");

	if(infile == NULL) 
		help(argv[0]);

	fprintf(stderr, "Reading File '%s'...\n", argv[1]);

	if(readheader_topicfile() == 0 && !proceed())
		exit(3);
	parse_topicfile();

	fclose(infile);
	infile = fopen(argv[2], "r");

	if(infile == NULL) 
		help(argv[0]);

	fprintf(stderr, "Reading File '%s'...\n", argv[2]);

	if(readheader_questionfile() == 0 && !proceed())
		exit(3);

	parse_questionfile();

	destroy_topicslist(topics);

	return 0;
}
