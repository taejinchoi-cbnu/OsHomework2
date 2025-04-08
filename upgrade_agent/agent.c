// OS Homework2 Team 208
// Agent binary code (used for both AgentX and AgentY)

// Libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

// Define constants and Variables
#define COLS 7
#define ROWS 6

static int this_player;
static int board[ROWS][COLS]; // Use index 0 to ROWS-1, 0 to COLS-1
static int top[COLS]; // Use index 0 to COLS-1, init to 0 (bottom of stack)

int const UPWARD = 1;
int const DOWNWARD = -1;
int const LEFTWARD = -1;
int const RIGHTWARD = 1;

// Count stones in a direction
int count_adjacent_stones(int stack, int level, int hdir, int vdir, int player) {
    int count = 0;
    while (0 <= stack && stack < COLS && 0 <= level && level < ROWS) {
        if (board[level][stack] == player) {
            count++;
            stack += hdir;
            level += vdir;
        } else {
            break;
        }
    }
    return count;
}

int count_down(int stack, int level, int player) {
    return count_adjacent_stones(stack, level + DOWNWARD, 0, DOWNWARD, player);
}

int count_left(int stack, int level, int player) {
    return count_adjacent_stones(stack + LEFTWARD, level, LEFTWARD, 0, player);
}

int count_right(int stack, int level, int player) {
    return count_adjacent_stones(stack + RIGHTWARD, level, RIGHTWARD, 0, player);
}

int count_down_left(int stack, int level, int player) {
    return count_adjacent_stones(stack + LEFTWARD, level + DOWNWARD, LEFTWARD, DOWNWARD, player);
}

int count_down_right(int stack, int level, int player) {
    return count_adjacent_stones(stack + RIGHTWARD, level + DOWNWARD, RIGHTWARD, DOWNWARD, player);
}

int count_up_left(int stack, int level, int player) {
    return count_adjacent_stones(stack + LEFTWARD, level + UPWARD, LEFTWARD, UPWARD, player);
}

int count_up_right(int stack, int level, int player) {
    return count_adjacent_stones(stack + RIGHTWARD, level + UPWARD, RIGHTWARD, UPWARD, player);
}

// Find a move that wins immediately (4 stones in a row)
int find_winning_move(int player) {
    for (int stack = 0; stack < COLS; stack++) {
        if (top[stack] >= ROWS) continue;
        if (count_down(stack, top[stack], player) >= 3) return stack;
        if (count_left(stack, top[stack], player) + count_right(stack, top[stack], player) >= 3) return stack;
        if (count_down_left(stack, top[stack], player) + count_up_right(stack, top[stack], player) >= 3) return stack;
        if (count_up_left(stack, top[stack], player) + count_down_right(stack, top[stack], player) >= 3) return stack;
    }
    return -1; // No winning move
}

// Find a move that blocks opponent's win (4 stones in a row)
int find_blocking_move(int player) {
    int other_player = 3 - player;
    for (int stack = 0; stack < COLS; stack++) {
        if (top[stack] >= ROWS) continue;
        if (count_down(stack, top[stack], other_player) >= 3) return stack;
        if (count_left(stack, top[stack], other_player) + count_right(stack, top[stack], other_player) >= 3) return stack;
        if (count_down_left(stack, top[stack], other_player) + count_up_right(stack, top[stack], other_player) >= 3) return stack;
        if (count_up_left(stack, top[stack], other_player) + count_down_right(stack, top[stack], other_player) >= 3) return stack;
    }
    return -1; // No blocking move
}

// Evaluate a move by calculating a score (greedy strategy)
int evaluate_move(int stack, int player, int other_player) {
    if (top[stack] >= ROWS) return -1; // Invalid move

    int level = top[stack];
    int score = 0;

    // Attack score: How good this move is for connecting my stones
    int down = count_down(stack, level, player);
    int horizontal = count_left(stack, level, player) + count_right(stack, level, player);
    int diag1 = count_down_left(stack, level, player) + count_up_right(stack, level, player);
    int diag2 = count_up_left(stack, level, player) + count_down_right(stack, level, player);

    if (down >= 3) score += 100; // Winning move
    else if (down == 2) score += 10; // Can connect 3 next turn
    else if (down == 1) score += 1; // Some potential

    if (horizontal >= 3) score += 100;
    else if (horizontal == 2) score += 10;
    else if (horizontal == 1) score += 1;

    if (diag1 >= 3) score += 100;
    else if (diag1 == 2) score += 10;
    else if (diag1 == 1) score += 1;

    if (diag2 >= 3) score += 100;
    else if (diag2 == 2) score += 10;
    else if (diag2 == 1) score += 1;

    // Defense score: How good this move is for blocking opponent
    down = count_down(stack, level, other_player);
    horizontal = count_left(stack, level, other_player) + count_right(stack, level, other_player);
    diag1 = count_down_left(stack, level, other_player) + count_up_right(stack, level, other_player);
    diag2 = count_up_left(stack, level, other_player) + count_down_right(stack, level, other_player);

    if (down >= 3) score += 50; // Block opponent's win
    else if (down == 2) score += 5; // Prevent opponent connecting 3
    else if (down == 1) score += 1;

    if (horizontal >= 3) score += 50;
    else if (horizontal == 2) score += 5;
    else if (horizontal == 1) score += 1;

    if (diag1 >= 3) score += 50;
    else if (diag1 == 2) score += 5;
    else if (diag1 == 1) score += 1;

    if (diag2 >= 3) score += 50;
    else if (diag2 == 2) score += 5;
    else if (diag2 == 1) score += 1;

    // Positional bonus: Center column is statistically better
    if (stack == 3) score += 1; // Column D (index 3)

    return score;
}

int main() {
    // Read player number
    if (scanf("%d", &this_player) != 1) {
        fprintf(stderr, "Error: Failed to read player number\n");
        return EXIT_FAILURE;
    }
    if (this_player != 1 && this_player != 2) {
        fprintf(stderr, "Error: Invalid player number %d\n", this_player);
        return EXIT_FAILURE;
    }

    // Initialize top array
    for (int j = 0; j < COLS; j++) {
        top[j] = 0;
    }

    // Read board
    for (int i = ROWS - 1; i >= 0; i--) {
        for (int j = 0; j < COLS; j++) {
            int value;
            if (scanf("%d", &value) != 1) {
                fprintf(stderr, "Error: Failed to read board at position [%d][%d]\n", i, j);
                return EXIT_FAILURE;
            }
            board[i][j] = value;
            if (board[i][j] != 0 && top[j] == 0) {
                top[j] = i + 1;
            }
        }
    }

    // Find winning move
    int choice = find_winning_move(this_player);
    if (choice >= 0) {
        printf("%c", 'A' + choice);
        return EXIT_SUCCESS;
    }

    // Find blocking move
    choice = find_blocking_move(this_player);
    if (choice >= 0) {
        printf("%c", 'A' + choice);
        return EXIT_SUCCESS;
    }

    // Greedy strategy: Evaluate each column and pick the best
    int other_player = 3 - this_player;
    int best_score = -1;
    int best_stack = -1;

    for (int stack = 0; stack < COLS; stack++) {
        int score = evaluate_move(stack, this_player, other_player);
        if (score > best_score) {
            best_score = score;
            best_stack = stack;
        } else if (score == best_score) {
            // If scores are equal, prefer the center column (index 3, 'D')
            if (stack == 3) {
                best_stack = stack;
            }
        }
    }

    if (best_stack >= 0) {
        printf("%c", 'A' + best_stack);
    } else {
        // Fallback: should not happen, but just in case
        fprintf(stderr, "Error: No valid move found\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
