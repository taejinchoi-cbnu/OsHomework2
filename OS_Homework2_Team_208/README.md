# OS Homework2 Team 208 - (Hyeong-Jin-Lee, Tae-Jin-Choi)

## Overview
This project implements a 4-in-a-row game where two agents (`agentX` and `agentY`) compete against each other. The game is managed by `gamatch`, which handles the game loop, board state, and communication with the agents via pipes. The agents decide their moves based on a simple strategy.

## Prerequisites
Before building and running the project, ensure you have the following:
- **Operating System**: Linux/Unix-based system (e.g., Ubuntu)
- **Compiler**: GCC (GNU Compiler Collection)
- **Build Tool**: Make
- **Dependencies**: Standard C libraries (`stdio.h`, `stdlib.h`, `unistd.h`, etc.), Linux libraries (`sys/wait.h`, `sys/types.h`)

## Directory Structure
- `gamatch.c`: Main game manager that runs the 4-in-a-row game.
- `AgentX.c`: Code for Agent X (player 1).
- `AgentY.c`: Code for Agent Y (player 2).
- `Makefile`: Build script to compile the project.
- `README.md`: This file.

## Build Instructions
Follow these steps to build the project:

1. **Clone or download the project**:
   Ensure all source files (`gamatch.c`, `AgentX.c`, `AgentY.c`, `Makefile`) are in the same directory.

2. **Build the project**:
   Open a terminal in the project directory and run:
   This will generate three executable files: `gamatch`, `agentX`, and `agentY`.

3. **Clean up (optional)**: To remove the generated executables, run: `make clean`

## Run Instructions
To run the game, use the following command:
`./gamatch -X ./agentX -Y ./agentY`

- `./gamatch`: The main game executable.
- `X ./agentX`: Specifies the binary for Agent X (player 1).
- `Y ./agentY`: Specifies the binary for Agent Y (player 2).

## Expected Output
- The game will display the current player (1 or 2) and the board state after each move.

- Example output after a few moves:
```text
1
0 0 0 0 0 0 0
0 0 0 0 0 0 0
0 0 0 0 0 0 0
0 0 0 0 0 0 0
0 0 0 0 0 0 0
0 1 0 0 0 0 0
---------------
```

- The game ends with a result: "Player X wins!", "Player Y wins!", or "Draw."

## Agent Strategy
- AgentX and AgentY:
    - Both agents use the same strategy (code is identical).
    - Winning Move: Check if placing a stone in a column can connect 4 stones (horizontally, vertically, or diagonally).
    - Blocking Move: Check if the opponent can win on their next turn and block it by placing a stone in that column.
    - Random Move: If no winning or blocking move exists, select a random column that is not full.

## Notes
- The first turn always given to Player 1 (AgentX)

- The game uses pipes for communication between gamatch and the agents.

- When the user press Ctrl+C, Gamatch immediately terminates its execution.

- A timeout of 3 seconds is set for each agent's move. If an agent exceeds this, it will be terminated.

- The board is 6 rows x 7 columns, and a player wins by connecting 4 stones horizontally, vertically, or diagonally
    or opposite player place the stone on wrong place which is full columns or non-existing line.

## Testing
To test the game:
    1. Build the project using `make`.
    2. Run the game with:
    ```bash
    ./gamatch -X ./agentX -Y ./agentY
    ```
    3. Observe the board state after each move.
    4. The game should end with a win or draw within 42 moves (6x7 board).

## Troubleshooting
- Permission denied: Ensure the executables have execute permissions. Run:
```bash
chmod +x gamatch agentX agentY
```

- Compilation errors: Ensure GCC and Make are installed. On Ubuntu, install them with:
```bash
sudo apt update
sudo apt install build-essential
```

- Agent not responding: Check if `agentX` and `agentY` are correctly compiled and in the same directory.
                        Or The agent may have exceeded the 3-second timeout, causing gamatch to terminate it.

## Authors
Team 208
**Hyeong-Jin-Lee**,
**Tae-Jin-Choi**
