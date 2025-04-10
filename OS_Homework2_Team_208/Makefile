# Makefile for OS Homework2 Team 208

# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Targets
all: gamatch agentX agentY

# Build gamatch
gamatch: gamatch.c
	$(CC) $(CFLAGS) -o gamatch gamatch.c

# Build agentX
agentX: agentX.c
	$(CC) $(CFLAGS) -o agentX agentX.c

# Build agentY
agentY: agentY.c
	$(CC) $(CFLAGS) -o agentY agentY.c

# Clean up
clean:
	rm -f gamatch agentX agentY

# Phony targets
.PHONY: all clean
