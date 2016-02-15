# Makefile for mmm-conv

CC = gcc
CFLAGS = -std=c99 -Wall -pedantic 
LIBS = 
OBJS = mmm-conv.o topic.o question.o
MAIN = mmm-conv

all: release

debug: CFLAGS += -g -O0
debug: $(MAIN)

release: CFLAGS += -s -O3 -DNDEBUG
release: $(MAIN)

$(MAIN): $(OBJS) 
	$(CC) -o $(MAIN) $(CFLAGS) $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o 

.PHONY: all debug release clean

