#ifndef QUESTION_H
#define QUESTION_H

#include <stdio.h>

struct Question{
	int nr;
	char *text; // Question text
	int points;
	int followup; // next question's number
	int category;

	/* Answers */
	char *answer[4]; // Answer text
	int correctness[4]; // Answer correct? yes = nonzero
};

extern struct QuestionList{
	struct Question **buf;
	size_t len, capacity;
} qulist;


void question_add(struct Question *q);

void questionlist_sort(void);
void questionlist_writejs(FILE *outfile);
void questionlist_free(void);

#endif
