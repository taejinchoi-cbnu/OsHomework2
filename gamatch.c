#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_STACK 7
#define MAX_HEIGHT 6
#define TIMEOUT 3

void print_usage();
void run_game(char *agent_x, char *agent_y);
void print_board(char board[MAX_HEIGHT][MAX_STACK]);
int check_winner(char board[MAX_HEIGHT][MAX_STACK]);


pid_t child_pid_x = 0;
pid_t child_pid_y = 0;

void signal_handler(int signo) {
    if (signo == SIGINT) {
        if (child_pid_x > 0) kill(child_pid_x, SIGKILL);
        if (child_pid_y > 0) kill(child_pid_y, SIGKILL);
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    
    if (argc != 3) {
        return 1;
    }

    char *agent_x = argv[1];
    char *agent_y = argv[2];

    signal(SIGINT, signal_handler);
    run_game(agent_x, agent_y);

    return 0;
}

void run_game(char *agent_x, char *agent_y) {
    int pipe_x_to_gamatch[2];
    int pipe_gamatch_to_x[2];
    int pipe_y_to_gamatch[2];
    int pipe_gamatch_to_y[2];
    pid_t pid_x, pid_y;
    char board[MAX_HEIGHT][MAX_STACK];
    int current_player = 1; // 1: X, 2: Y
    int winner = 0; // 0: In progress, 1: X wins, 2: Y wins, 3: Draw
    int moves = 0;

    // Initialize the game board
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_STACK; j++) {
            board[i][j] = '0';
        }
    }

    // Create pipes
    if (pipe(pipe_x_to_gamatch) != 0 || pipe(pipe_gamatch_to_x) != 0 ||
        pipe(pipe_y_to_gamatch) != 0 || pipe(pipe_gamatch_to_y) != 0) {
        perror("Pipe Error");
        exit(1);
    }

    // Create X Agent process
    pid_x = fork();
    if (pid_x == -1) {
        perror("fork failed");
        exit(1);
    }

    if (pid_x == 0) { // Child process (X Agent)
        close(pipe_gamatch_to_x[1]);
        close(pipe_x_to_gamatch[0]);

        dup2(pipe_gamatch_to_x[0], STDIN_FILENO);
        dup2(pipe_x_to_gamatch[1], STDOUT_FILENO);

        close(pipe_gamatch_to_x[0]);
        close(pipe_x_to_gamatch[1]);

        execl(agent_x, agent_x, NULL);
        perror("execl failed");
        exit(1);
    }

    child_pid_x = pid_x;

    close(pipe_gamatch_to_x[0]);
    close(pipe_x_to_gamatch[1]);

    // Create Y Agent process
    pid_y = fork();
    if (pid_y == -1) {
        perror("fork failed");
        exit(1);
    }

    if (pid_y == 0) { // Child process (Y Agent)
        close(pipe_gamatch_to_y[1]);
        close(pipe_y_to_gamatch[0]);

        dup2(pipe_gamatch_to_y[0], STDIN_FILENO);
        dup2(pipe_y_to_gamatch[1], STDOUT_FILENO);

        close(pipe_gamatch_to_y[0]);
        close(pipe_y_to_gamatch[1]);

        execl(agent_y, agent_y, NULL);
        perror("execl error");
        exit(1);
    }

    child_pid_y = pid_y;

    close(pipe_gamatch_to_y[0]);
    close(pipe_y_to_gamatch[1]);

     while (moves < MAX_STACK * MAX_HEIGHT && winner == 0) {
        char move;
        char player_char = (current_player == 1) ? '1' : '2';
        int player_num = current_player;
        int stack_index;
        char input_buffer[10];

        // Print current player
        printf("\n%c\n", player_char);
        print_board(board);

        if (current_player == 1) {
            dprintf(pipe_gamatch_to_x[1], "%d\n", player_num);
            for (int i = 0; i < MAX_HEIGHT; i++) {
                for (int j = 0; j < MAX_STACK; j++) {
                    dprintf(pipe_gamatch_to_x[1], "%c", board[i][j]);
                    if (j < MAX_STACK - 1) dprintf(pipe_gamatch_to_x[1], " ");
                }
                dprintf(pipe_gamatch_to_x[1], "\n");
            }

            close(pipe_gamatch_to_x[1]);
            pipe_gamatch_to_x[1] = -1;
            read(pipe_x_to_gamatch[0], input_buffer, sizeof(input_buffer));
            move = input_buffer[0];
            close(pipe_x_to_gamatch[0]);
            pipe_x_to_gamatch[0] = -1;
        } else {
            dprintf(pipe_gamatch_to_y[1], "%d\n", player_num);
            for (int i = 0; i < MAX_HEIGHT; i++) {
                for (int j = 0; j < MAX_STACK; j++) {
                    dprintf(pipe_gamatch_to_y[1], "%c", board[i][j]);
                    if (j < MAX_STACK - 1) dprintf(pipe_gamatch_to_y[1], " ");
                }
                dprintf(pipe_gamatch_to_y[1], "\n");
            }
           
            close(pipe_gamatch_to_y[1]);
            pipe_gamatch_to_y[1] = -1;
            read(pipe_y_to_gamatch[0], input_buffer, sizeof(input_buffer));
            move = input_buffer[0];
            close(pipe_y_to_gamatch[0]);
            pipe_y_to_gamatch[0] = -1;
        }

        if (move < 'A' || move > 'G') {
	    printf("\n");
            printf("Invalid input! %c wins.\n", (current_player == 1) ? '2' : '1');
            winner = (current_player == 1) ? 2 : 1;
            break;
        }

        stack_index = move - 'A';

        if (board[0][stack_index] != '0') {
	    printf("\n");
            printf("Column is full! %c wins.\n", (current_player == 1) ? '2' : '1');
            winner = (current_player == 1) ? 2 : 1;
            break;
        }

        for (int i = MAX_HEIGHT - 1; i >= 0; i--) {
            if (board[i][stack_index] == '0') {
                board[i][stack_index] = player_char;
                break;
            }
        }

        moves++;
	
	// Check winner
        winner = check_winner(board);
        if (winner != 0) break;

        // Change turn
        current_player = (current_player == 1) ? 2 : 1;
	
        sleep(1);
    }

    if (winner == 0) {
        printf("Draw.\n");
    } else if (winner == 1) {
        printf("Player X wins!\n");
    } else {
        printf("Player Y wins!\n");
    }

    // Cleanup child processes
    if (child_pid_x > 0) kill(child_pid_x, SIGKILL);
    if (child_pid_y > 0) kill(child_pid_y, SIGKILL);
    wait(NULL);
    wait(NULL);
}

// Print game board function
void print_board(char board[MAX_HEIGHT][MAX_STACK]) {
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_STACK; j++) {
            printf("%c ", board[i][j]);
        }
        printf("\n");
    }
    printf("---------------\n");
}

// Check for winner function
int check_winner(char board[MAX_HEIGHT][MAX_STACK]) {
    // Check horizontal
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j <= MAX_STACK - 4; j++) {
            if (board[i][j] != '0' && board[i][j] == board[i][j + 1] &&
                board[i][j] == board[i][j + 2] && board[i][j] == board[i][j + 3]) {
                return (board[i][j] == '1') ? 1 : 2;
            }
        }
    }

    // Check vertical
    for (int i = 0; i <= MAX_HEIGHT - 4; i++) {
        for (int j = 0; j < MAX_STACK; j++) {
            if (board[i][j] != '0' && board[i][j] == board[i + 1][j] &&
                board[i][j] == board[i + 2][j] && board[i][j] == board[i + 3][j]) {
                return (board[i][j] == '1') ? 1 : 2;
            }
        }
    }

    // Check diagonal (bottom right)
    for (int i = 0; i <= MAX_HEIGHT - 4; i++) {
        for (int j = 0; j <= MAX_STACK - 4; j++) {
            if (board[i][j] != '0' && board[i][j] == board[i + 1][j + 1] &&
                board[i][j] == board[i + 2][j + 2] && board[i][j] == board[i + 3][j + 3]) {
                return (board[i][j] == '1') ? 1 : 2;
            }
        }
    }

    // Check diagonal (bottom left)
    for (int i = 0; i <= MAX_HEIGHT - 4; i++) {
        for (int j = MAX_STACK - 1; j >= 3; j--) {
            if (board[i][j] != '0' && board[i][j] == board[i + 1][j - 1] &&
                board[i][j] == board[i + 2][j - 2] && board[i][j] == board[i + 3][j - 3]) {
                return (board[i][j] == '1') ? 1 : 2;
            }
        }
    }

    // Check for draw
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_STACK; j++) {
            if (board[i][j] == '0') {
                return 0; // Empty slot still exists
            }
        }
    }

    return 3; // Draw
}
