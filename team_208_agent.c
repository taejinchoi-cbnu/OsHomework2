// OS Homework2 Team 208
// Agent binary code (used for both AgentX and AgentY)
/*
 * Based on the example codes but our agent is designed to be more aggressive.
 * It only defends when the opponent has 3 stones connected, and otherwise always focuses on attacking.
 */

// Libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

// Define constants and Variables
#define COLS 7
#define ROWS 6

static int this_player;
static int board[ROWS][COLS]; // Use index 0 to ROWS-1, 0 to COLS-1
static int top[COLS]; // Use index 0 to COLS-1, top[j] is the highest occupied row + 1

int const UPWARD = -1;  // Moving up decreases row index
int const DOWNWARD = 1; // Moving down increases row index
int const LEFTWARD = -1;
int const RIGHTWARD = 1;

// Stack name conversion
char stack_name(int i) {
    return 'A' + i;
}

// Function to get a random number using /dev/urandom
int get_random_int(int min, int max) {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Error: Failed to open /dev/urandom\n");
        return -1; // Fallback to a default value if needed
    }

    unsigned int random_value;
    ssize_t result = read(fd, &random_value, sizeof(random_value));
    if (result < 0) {
        fprintf(stderr, "Error: Failed to read from /dev/urandom\n");
        close(fd);
        return -1;
    }

    close(fd);

    // Scale the random value to the desired range [min, max]
    return min + (random_value % (max - min + 1));
}

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

[Oint count_right(int stack, int level, int player) {
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

// Find a move that blocks opponent's win (4 stones in a row) - Minimal defense
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

// Evaluate a move by calculating a score (aggressive strategy)
int evaluate_move(int stack, int player, int other_player) {
    if (top[stack] >= ROWS) return -1; // Invalid move

    int level = top[stack];
    int score = 0;

    // Attack score: Prioritize connecting my stones aggressively
    int down = count_down(stack, level, player);
    int horizontal = count_left(stack, level, player) + count_right(stack, level, player);
    int diag1 = count_down_left(stack, level, player) + count_up_right(stack, level, player);
    int diag2 = count_up_left(stack, level, player) + count_down_right(stack, level, player);

    // Aggressive scoring: Higher rewards for connecting stones
    if (down >= 3) score += 1000; // Winning move (very high priority)
    else if (down == 2) score += 50; // Can connect 3 next turn
    else if (down == 1) score += 5; // Some potential

    if (horizontal >= 3) score += 1000;
    else if (horizontal == 2) score += 50;
    else if (horizontal == 1) score += 5;

    if (diag1 >= 3) score += 1000;
    else if (diag1 == 2) score += 50;
    else if (diag1 == 1) score += 5;

    if (diag2 >= 3) score += 1000;
    else if (diag2 == 2) score += 50;
    else if (diag2 == 1) score += 5;

    // No defense score: Removed all defensive scoring to focus on offense

    // Positional bonus: Stronger preference for central columns
    if (stack == 3) score += 10; // Column D
    else if (stack == 2 || stack == 4) score += 5; // Columns C and E

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

    // Read board and validate input
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            int value;
            if (scanf("%d", &value) != 1) {
                fprintf(stderr, "Error: Failed to read board at position [%d][%d]\n", i, j);
                return EXIT_FAILURE;
            }
            if (value != 0 && value != 1 && value != 2) {
                fprintf(stderr, "Error: Invalid board value %d at position [%d][%d]\n", value, i, j);
                return EXIT_FAILURE;
            }
            board[i][j] = value;
            if (board[i][j] != 0) {
                top[j] = i + 1; // Update top to the highest occupied row
            }
        }
    }

    // Find winning move
    int choice = find_winning_move(this_player);
    if (choice >= 0) {
        printf("%c", stack_name(choice));
        return EXIT_SUCCESS;
    }

    // Minimal defense: Only block opponent's immediate win
    choice = find_blocking_move(this_player);
    if (choice >= 0) {
        printf("%c", stack_name(choice));
        return EXIT_SUCCESS;
    }

    // Aggressive strategy: Evaluate each column and pick the best
    int other_player = 3 - this_player;
    int best_score = -1;
    int best_stack = -1;
    int equal_score_stacks[COLS];
    int equal_score_count = 0;

    for (int stack = 0; stack < COLS; stack++) {
        int score = evaluate_move(stack, this_player, other_player);
        if (score > best_score) {
            best_score = score;
            best_stack = stack;
            equal_score_count = 0;
            equal_score_stacks[0] = stack;
            equal_score_count = 1;
        } else if (score == best_score && score >= 0) {
            // Collect stacks with equal scores
            equal_score_stacks[equal_score_count] = stack;
            equal_score_count++;
        }
    }

    if (best_score >= 0) {
        if (equal_score_count == 1) {
            best_stack = equal_score_stacks[0];
        } else {
            // If multiple stacks have the same score, prefer center (D), otherwise randomize
            int center_found = 0;
            for (int i = 0; i < equal_score_count; i++) {
                if (equal_score_stacks[i] == 3) { // Column D
                    best_stack = 3;
                    center_found = 1;
                    break;
                }
            }
            if (!center_found) {
                // Randomly select one of the equal-scoring stacks using /dev/urandom
                int random_index = get_random_int(0, equal_score_count - 1);
                if (random_index < 0) {
                    // Fallback to first stack in case of error
                    best_stack = equal_score_stacks[0];
                } else {
                    best_stack = equal_score_stacks[random_index];
                }
            }
        }
        printf("%c", stack_name(best_stack));
    } else {
        fprintf(stderr, "Error: No valid move found\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
