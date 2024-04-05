# Makefile

CC = gcc
CFLAGS = -Wall -O2

testing: testing.o
	$(CC) -o testing testing.o

bfsNew: bfsNew.o
	$(CC) -o bfsNew bfsNew.o
