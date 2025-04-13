# Makefile for OS Homework2 Team 208

# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Targets
all: gamatch

# Build gamatch
gamatch: gamatch.c
	$(CC) $(CFLAGS) -o gamatch gamatch.c

# Clean up
clean:
	rm -f gamatch

# Phony targets
.PHONY: all clean
