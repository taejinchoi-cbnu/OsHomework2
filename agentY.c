// OS Homework2 Team 208
// AgentY binary code

// Libraries
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Define constants and Variables
#define N_STACKS 7
#define STACK_CAP 6

static int this_player;
static int board[STACK_CAP + 1][N_STACKS + 1];
static int top[N_STACKS + 1] = {0, 1, 1, 1, 1, 1, 1, 1};

int const UPWARD = 1;
int const DOWNWARD = -1;
int const LEFTWARD = -1;
int const RIGHTWARD = 1;

int count_adjacent_stones(int stack, int level, int hdir, int vdir, int player) {
    int count = 0;
    while (1 <= stack && stack <= N_STACKS && 1 <= level && level <= STACK_CAP) {
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

int find_winning_move(int player) {
    for (int stack = 1; stack <= N_STACKS; stack++) {
        if (top[stack] > STACK_CAP) continue;
        if (count_down(stack, top[stack], player) == 3) return stack;
        if (count_left(stack, top[stack], player) + count_right(stack, top[stack], player) == 3) return stack;
        if (count_down_left(stack, top[stack], player) + count_up_right(stack, top[stack], player) == 3) return stack;
        if (count_up_left(stack, top[stack], player) + count_down_right(stack, top[stack], player) == 3) return stack;
    }
    return 0;
}

int find_blocking_move(int player) {
    int other_player = 3 - player;
    for (int stack = 1; stack <= N_STACKS; stack++) {
        if (top[stack] > STACK_CAP) continue;
        if (count_down(stack, top[stack], other_player) == 3) return stack;
        if (count_left(stack, top[stack], other_player) + count_right(stack, top[stack], other_player) == 3) return stack;
        if (count_down_left(stack, top[stack], other_player) + count_up_right(stack, top[stack], other_player) == 3) return stack;
        if (count_up_left(stack, top[stack], other_player) + count_down_right(stack, top[stack], other_player) == 3) return stack;
    }
    return 0;
}

int main() {
    scanf("%d", &this_player);
    if (this_player != 1 && this_player != 2) return EXIT_FAILURE;

    for (int i = STACK_CAP; i > 0; i--) {
        for (int j = 1; j <= N_STACKS; j++) {
            scanf("%d", &board[i][j]);
            if (board[i][j] != 0 && top[j] == 1) top[j] = i + 1;
        }
    }

    int choice = find_winning_move(this_player);
    if (choice) {
        printf("%c", 'A' + choice - 1);
        return EXIT_SUCCESS;
    }

    choice = find_blocking_move(this_player);
    if (choice) {
        printf("%c", 'A' + choice - 1);
        return EXIT_SUCCESS;
    }

    srand(time(NULL));
    do {
        choice = rand() % N_STACKS + 1;
    } while (top[choice] > STACK_CAP);
    printf("%c", 'A' + choice - 1);

    return EXIT_SUCCESS;
}

