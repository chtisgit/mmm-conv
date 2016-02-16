
#include "mmm-conv.h"
#include "question.h"

#include <string.h>

struct QuestionList qulist = {
	.buf = NULL, 
	.len = 0,
	.capacity = 0
};


static void questionlist_extend(size_t n)
{
	const size_t INC = 200;

	if(qulist.buf == NULL){
		const size_t startsize = sizeof(*qulist.buf)*INC;
		qulist.buf = malloc(startsize);
		qulist.capacity = INC;
		if(qulist.buf == NULL)
			not_enough_mem(startsize, __LINE__);
		return;
	}

	if(qulist.len + n < qulist.capacity)
		return;
	
	const size_t newsize = sizeof(*qulist.buf)*(qulist.capacity + INC);
	void *nptr = realloc(qulist.buf, newsize);
	if(nptr == NULL)
		not_enough_mem(newsize, __LINE__);
	
	qulist.capacity += INC;
	qulist.buf = nptr;
}

static int question_comparator(const void *vq1, const void *vq2)
{
	const struct Question *q1 = (const struct Question*) vq1;
	const struct Question *q2 = (const struct Question*) vq2;

	return q1->nr - q2->nr;
}

static void question_writejsobj(FILE *outfile, const struct Question *q)
{
	fprintf(outfile, "\t{\n" 
		"\t'nr': %d,\n"
		"\t'text': '%s',\n"
		"\t'points': %d,\n"
		"\t'category': %d,\n"
		"\t'followup': %d,\n"
		"\t'answer': [\n",
		q->nr,
		q->text,
		q->points,
		q->category,
		q->followup);
	
	for(int i = 0; i < 4; i++){
		fprintf(outfile, "\t\t{ 'text': '%s', 'correct': %s }",
			q->answer[i],
			q->correctness[i] ? "true" : "false");
		if(i < 3)
			fprintf(outfile, ",\n");
	}

	fprintf(outfile, "\n\t\t]\n\t}");
}

void questionlist_writejs(FILE *outfile)
{
	int i;
	fprintf(outfile, "questions = [\n");
	for(i = 0; i < qulist.len; i++){
		question_writejsobj(outfile, qulist.buf[i]);
		if(i < qulist.len - 1)
			fprintf(outfile, ",\n");
	}
	fprintf(outfile, " ];");
}

static void question_free(struct Question *q)
{
	for(int i = 0; i < 4; i++){
		free(q->answer[i]);
	}
	free(q->text);
	free(q);
}

void questionlist_free(void)
{
	for(int i = 0; i < qulist.len; i++){
		question_free(qulist.buf[i]);
	}
	free(qulist.buf);
	qulist.buf = NULL;
	qulist.capacity = qulist.len = 0;
}

void questionlist_sort(void)
{
	qsort((void*) qulist.buf, qulist.len, sizeof(*qulist.buf), question_comparator);
}

void question_add(struct Question *q)
{
	questionlist_extend(1);
	struct Question *nq = malloc(sizeof(*nq));
	if(nq == NULL)
		not_enough_mem(sizeof(*nq), __LINE__);
	
	*nq = *q;
	qulist.buf[qulist.len++] = nq;
}
