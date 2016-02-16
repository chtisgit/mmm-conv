#include "mmm-conv.h"
#include "topic.h"
#include "question.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

const char *progname = "mmm-conv";
FILE *infile = NULL;
FILE *outfile = NULL;

void help(void)
{
	fprintf(stderr, "Usage: %s <topic file> <question file>\n\n", progname);
	exit(2);
}

void not_enough_mem(size_t sz, int line)
{
	error("out of memory! (last alloc = %zu, line = %d)\n\n", sz, line);
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
		not_enough_mem(len+1, __LINE__);

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
	int i, j = -1;
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


char* create_escaped_string(const char *s)
{
	char *snew, *ptr;
	size_t len = 0, i;

	for(i = 0; s[i] != '\0'; i++){
		switch(((unsigned char*)s)[i]){
		case '\'':
			len += 2;
			break;

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
		not_enough_mem(len+1, __LINE__);	

	ptr = snew;
	for(i = 0; s[i] != '\0'; i++){
		switch(((unsigned char*)s)[i]){
		case '\'': /* ' */
			strcpy(ptr, "\\'");
			ptr += 2;
			break;

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

int parse_question(int *const last_category)
{
	struct Question q = { .answer = {0}, .correctness = {0}};
	int i, a_len;
	char answer[256];
	unsigned char header[23];

	fread(header, 1, 23, infile);

	/* text */
	char *qtext = readstring(2);
	if(qtext == NULL)
		return 0; /* exit silently */
	q.text = create_escaped_string(qtext);
	if(q.text == NULL)
		error("esc_question == NULL");
	free(qtext);

	/* category */
	i = header[4] | header[5] | header[6] | header[7];
	if(i == 0)
		q.category = *last_category;
	else
		q.category = (header[6] << 8) | header[7];

	*last_category = i;

	/* nr, points & follow-up question */
	q.nr = ((((int) header[2]) << 8) | (int) header[3]);
	q.points = (int) header[15];
	q.followup = ((((int) header[10]) << 8) | (int) header[11]);

	/* answers */
	for(i = 0; i < 4; i++){
		a_len = fgetc(infile) << 8;
		a_len |= fgetc(infile); /* big endian */
		if(fread(answer, 1, a_len, infile) != a_len)
			return 0;

		answer[a_len] = '\0';
		
		q.answer[i] = create_escaped_string(answer);
		if(q.answer[i] == NULL)
			error("q.answer == NULL");

		q.correctness[i] = ((int) header[18] & (1 << i));
	}

	debug("got question %d\n", q.nr);
	/* 'magic' dword after each question = 0 */
	uint32_t nul;
	fread(&nul, 4, 1, infile);
	if(nul != 0)
		debug("question not terminated by 0 (terminating number was %u) \n", nul);

	question_add(&q);

	return 1;
}

void parse_questionfile(void)
{
	int last_cat = 0;
	while(!feof(infile) && parse_question(&last_cat));
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
	char header[6];
	char *name;
	int id;

	fread(header, 1, 6, infile);

	name = readstring(1);
	if(name == NULL)
		return 0;

	id = (header[4] << 8) | header[5];

	char *esc_name = create_escaped_string(name);
	if(esc_name == NULL)
		not_enough_mem(strlen(name), __LINE__);

	debug("\t - %s [%d]\n", esc_name, id);

	free(name);

	topic_add(id, esc_name);
	return 1;
}

void parse_topicfile(void)
{
	debug("Table of Topics:\n");
	while(!feof(infile) && parse_topic());
}

void cleanup(void)
{
	if(outfile != NULL){
		fclose(outfile);
		outfile = NULL;
	}
	if(infile != NULL){
		fclose(infile);
		infile = NULL;
	}
	topic_rdestroy(topics);
	questionlist_free();
}

int open_output_file(void)
{
	int ok = 1;
	/* check if exists */
	outfile = fopen(OUTPUT_FILENAME, "r");
	if(outfile != NULL){
		fprintf(stderr, "A file \"" OUTPUT_FILENAME " does already exist\n" 
			"Continuing would delete this file\n");
		ok = proceed();
	}
	if(!ok) return 0;

	outfile = fopen(OUTPUT_FILENAME, "w");
	
	return outfile == NULL ? 0 : 1;
}

int main(int argc, char **argv)
{
	if(argc < 3)
		help();

	progname = argv[0];

	atexit(cleanup);
	infile = fopen(argv[1], "r");

	if(infile == NULL) 
		help();

	fprintf(stderr, "Reading File '%s'...\n", argv[1]);

	if(readheader_topicfile() == 0 && !proceed())
		return 3;

	parse_topicfile();

	fclose(infile);
	infile = fopen(argv[2], "r");

	if(infile == NULL) 
		help();

	fprintf(stderr, "Reading File '%s'...\n", argv[2]);

	if(readheader_questionfile() == 0 && !proceed())
		return 3;

	parse_questionfile();


	if(!open_output_file())
		return 2;

	debug("Writing file...\n");
	questionlist_writejs(outfile);
	debug("Finished\n");

	return 0;
}
