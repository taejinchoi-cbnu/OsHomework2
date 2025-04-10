# OS Homework2 Team 208 - (Hyeong-Jin-Lee, Tae-Jin-Choi)

## Overview
This project implements a 4-in-a-row game where two agents compete against each other. The game is managed by `gamatch`, which handles the game loop, board state, and communication with the agents via pipes. The agents are pre-existing on the server (e.g., `agent_blue`, `agent_red`), and this submission provides the `gamatch` program to run the game with those agents.

## Prerequisites
Before building and running the project, ensure you have the following:
- **Operating System**: Linux/Unix-based system (e.g., Ubuntu)
- **Compiler**: GCC (GNU Compiler Collection)
- **Build Tool**: Make
- **Dependencies**: Standard C libraries (`stdio.h`, `stdlib.h`, `unistd.h`, etc.), Linux libraries (`sys/wait.h`, `sys/types.h`)

## Directory Structure
- `gamatch.c`: Main game manager that runs the 4-in-a-row game.
- `Makefile`: Build script to compile the project.
- `README.md`: This file.

## Build Instructions
Follow these steps to build the project:

1. **Clone or download the project**:
   Ensure the source file (`gamatch.c`) and `Makefile` are in the same directory as the pre-existing agent binaries (e.g., `agent_blue`, `agent_red`).

2. **Build the project**:
Open a terminal in the project directory and run:
```bash
make
```
This will generate the executable file: gamatch.

## Run Instructions
To run the game, use the following command:
`./gamatch -X ./agent_blue -Y ./agent_red`

- `./gamatch`: The main game executable
- `-X ./agent_blue`: Specifies the binary for Player X (player 1).
- `-Y ./agent_red`: Specifies the binary for Player Y (player 2).

## Expected Output
The game will display the current player (1 or 2) and the board state after each move.
Example output after a few moves:
```text
1
0 0 0 0 0 0 0
0 0 0 0 0 0 0
0 0 0 0 0 0 0
0 0 0 0 0 0 0
0 0 0 0 0 0 0
0 1 0 0 0 0 0
---------------
.
.
.
```
The game ends with a result: "Player X wins!", "Player Y wins!", or "Draw."

## Notes
- The first turn is always given to Player 1 (X).
- The game uses pipes for communication between `gamatch` and the agents.
- When the user presses `Ctrl+C`, gamatch immediately terminates its execution.
A timeout of 3 seconds is set for each agent's move. If an agent exceeds this, it will be terminated.
- The board is 6 rows x 7 columns, and a player wins by connecting 4 stones horizontally, vertically, or diagonally, or if the opponent places a stone in a full column or non-existing line.

## Testing
To test the game:
1. Build the project using make.
2. Run the game with:
```bash
./gamatch -X ./agent_blue -Y ./agent_red
```
3. Observe the board state after each move.
4. The game should end with a win or draw within 42 moves (6x7 board).

## Trobleshooting
- Permission denied: Ensure the executable has execute permissions. Run:
```bash
chmod +x gamatch
```
- Compilation errors: Ensure GCC and Make are installed. On Ubuntu, install them with:
```bash
sudo apt update
sudo apt install build-essential
```
- Agent not responding: Check if the agent binaries (e.g., `agent_blue`, `agent_red`) are in the same directory and have execute permissions. The agent may have exceeded the 3-second timeout, causing `gamatch` to terminate it.
## Authors
Team 208
**Hyeong-Jin-Lee**,
**Tae-Jin-Choi**