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
agentX: AgentX.c
	$(CC) $(CFLAGS) -o agentX AgentX.c

# Build agentY
agentY: AgentY.c
	$(CC) $(CFLAGS) -o agentY AgentY.c

# Clean up
clean:
	rm -f gamatch agentX agentY

# Phony targets
.PHONY: all clean
