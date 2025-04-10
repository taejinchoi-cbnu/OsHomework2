// OS Homework2 Team 208
// AgentX binary code

// Libraries
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Define constants and Variables
#define COLS 7
#define ROWS 6

static int this_player; // Player number (1 is AgentX, 2 is AgentY)
static int board[ROWS + 1][COLS + 1]; // Game board
static int top[COLS + 1] = {0, 1, 1, 1, 1, 1, 1, 1}; // Top position of each column

// Direction constant for movement
int const UPWARD = 1;
int const DOWNWARD = -1;
int const LEFTWARD = -1;
int const RIGHTWARD = 1;

// Count adjacent stones
int count_adjacent_stones(int stack, int level, int hdir, int vdir, int player) {
    int count = 0;
    while (1 <= stack && stack <= COLS && 1 <= level && level <= ROWS) {
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

// Count stones downward
int count_down(int stack, int level, int player) {
    return count_adjacent_stones(stack, level + DOWNWARD, 0, DOWNWARD, player);
}

// Count stones to the left
int count_left(int stack, int level, int player) {
    return count_adjacent_stones(stack + LEFTWARD, level, LEFTWARD, 0, player);
}

// Count stones to the right
int count_right(int stack, int level, int player) {
    return count_adjacent_stones(stack + RIGHTWARD, level, RIGHTWARD, 0, player);
}

// Count stones diagonally down-left (/)
int count_down_left(int stack, int level, int player) {
    return count_adjacent_stones(stack + LEFTWARD, level + DOWNWARD, LEFTWARD, DOWNWARD, player);
}

// Count stones diagonally down-right (\)
int count_down_right(int stack, int level, int player) {
    return count_adjacent_stones(stack + RIGHTWARD, level + DOWNWARD, RIGHTWARD, DOWNWARD, player);
}

// Count stones diagonally up-left (\)
int count_up_left(int stack, int level, int player) {
    return count_adjacent_stones(stack + LEFTWARD, level + UPWARD, LEFTWARD, UPWARD, player);
}

// Count stones diagonally up-right(/)
int count_up_right(int stack, int level, int player) {
    return count_adjacent_stones(stack + RIGHTWARD, level + UPWARD, RIGHTWARD, UPWARD, player);
}

// Find a winning move for the player
int find_winning_move(int player) {
    for (int stack = 1; stack <= COLS; stack++) {
        if (top[stack] > ROWS) continue;
        if (count_down(stack, top[stack], player) == 3) return stack;
        if (count_left(stack, top[stack], player) + count_right(stack, top[stack], player) == 3) return stack;
        if (count_down_left(stack, top[stack], player) + count_up_right(stack, top[stack], player) == 3) return stack;
        if (count_up_left(stack, top[stack], player) + count_down_right(stack, top[stack], player) == 3) return stack;
    }
    return 0;
}
// Find a blocking move to prevent opponent's win
int find_blocking_move(int player) {
    int other_player = 3 - player;
    for (int stack = 1; stack <= COLS; stack++) {
        if (top[stack] > ROWS) continue;
        if (count_down(stack, top[stack], other_player) == 3) return stack;
        if (count_left(stack, top[stack], other_player) + count_right(stack, top[stack], other_player) == 3) return stack;
        if (count_down_left(stack, top[stack], other_player) + count_up_right(stack, top[stack], other_player) == 3) return stack;
        if (count_up_left(stack, top[stack], other_player) + count_down_right(stack, top[stack], other_player) == 3) return stack;
    }
    return 0;
}

int main() {
    // Read player number
    if (scanf("%d", &this_player) != 1) {
        perror("scanf failed");
        return EXIT_FAILURE;
    }

    if (this_player != 1 && this_player != 2) return EXIT_FAILURE;

    for (int i = ROWS; i > 0; i--) {
        for (int j = 1; j <= COLS; j++) {
            if (scanf("%d", &board[i][j]) != 1) return EXIT_FAILURE;
            if (board[i][j] != 0 && top[j] == 1) top[j] = i + 1;
        }
    }

    // Finding winning move
    int choice = find_winning_move(this_player); // 0 is no winning move
    if (choice) {
        printf("%c", 'A' + choice - 1);
        return EXIT_SUCCESS;
    }

    choice = find_blocking_move(this_player); // 0 is no blocking move
    if (choice) {
        printf("%c", 'A' + choice - 1);
        return EXIT_SUCCESS;
    }

    // Random move if no winning or blocking move
    srand(time(NULL));
    do {
        choice = rand() % COLS + 1;
    } while (top[choice] > ROWS);
    printf("%c", 'A' + choice - 1);

    return EXIT_SUCCESS;
}
