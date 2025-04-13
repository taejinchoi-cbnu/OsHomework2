// OS Homework2 Team 208

// Libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

// Define constants
#define COLS 7
#define ROWS 6
#define TIMEOUT 3

// Function declarations
void print_usage(void);
void run_game(char *agent_x, char *agent_y);
void print_board(char board[ROWS][COLS]);
int check_winner(char board[ROWS][COLS]);

// Processes PID var
pid_t child_pid_x = 0;
pid_t child_pid_y = 0;

// Signal handler (SIGINT, SIGALRM)
void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGALRM) {
        if (child_pid_x > 0) kill(child_pid_x, SIGKILL);
        if (child_pid_y > 0) kill(child_pid_y, SIGKILL);
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 5 || strcmp(argv[1], "-X") != 0 || strcmp(argv[3], "-Y") != 0) {
        print_usage();
        exit(1);
    }

    char *agent_x = argv[2];
    char *agent_y = argv[4];

    signal(SIGINT, signal_handler);
    signal(SIGALRM, signal_handler);
    run_game(agent_x, agent_y);

    return 0;
}

void print_usage(void) {
    printf("Usage: ./gamatch -X <agent-binary> -Y <agent-binary>\n");
}

// Main game function
void run_game(char *agent_x, char *agent_y) {
    char board[ROWS][COLS];
    int current_player = 1; // 1 is X, 2 is Y
    int winner = 0; // 0 is progress, 1 is X win, 2 is Y win, 3 is draw
    int moves = 0; // Turn count

    // Init board
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            board[i][j] = '0';
        }
    }
    
    // Main game loop
    while (moves < COLS * ROWS && winner == 0) {
        int pipe_to_agent[2], pipe_from_agent[2];
        pid_t pid;
        char move;
        char player_char = (current_player == 1) ? '1' : '2';
        int col_idx;
        char input_buf[10];
        
        // Create pipe
        if (pipe(pipe_to_agent) != 0 || pipe(pipe_from_agent) != 0) {
            perror("Pipe Error");
            exit(1);
        }
        
        // Create child process
        pid = fork();
        if (pid == -1) {
            perror("fork failed");
            exit(1);
        }

        if (pid == 0) {
            // Child process
            close(pipe_to_agent[1]);
            close(pipe_from_agent[0]);
            
            dup2(pipe_to_agent[0], STDIN_FILENO);
            dup2(pipe_from_agent[1], STDOUT_FILENO);
            
            close(pipe_to_agent[0]);
            close(pipe_from_agent[1]);

            if (current_player == 1) {
                // AgentX turn
                execl(agent_x, agent_x, NULL);
            } else {
                // AgentY turn
                execl(agent_y, agent_y, NULL);
            }
            perror("execl failed");
            exit(1);
        }

        if (current_player == 1) child_pid_x = pid;
        else child_pid_y = pid;

        // Parent process
        close(pipe_to_agent[0]);
        close(pipe_from_agent[1]);

        // Send current player
        char player_buf[16];
        
        // Convert int to char
        player_buf[0] = '0' + current_player;
        player_buf[1] = '\n';
        
        int player_len = 2;
        if (write(pipe_to_agent[1], player_buf, player_len) == -1) {
            perror("write failed");
            exit(1);
        }

        // Send current board
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                char cell_buf[2];
                
                // Convert int to char
                cell_buf[0] = (board[i][j] - '0') + '0'; 
                cell_buf[1] = (j < COLS - 1) ? ' ' : '\n';
                if (write(pipe_to_agent[1], cell_buf, 2) == -1) {
                    perror("write failed");
                    exit(1);
                }
            }
        }
        close(pipe_to_agent[1]);

        // Set timeout
        alarm(TIMEOUT);
        ssize_t bytes_read = read(pipe_from_agent[0], input_buf, sizeof(input_buf) - 1);
        if (bytes_read == -1) {
            perror("read failed");
            exit(1);
        }
        input_buf[bytes_read] = 0x0;

        // Clear timeout
        alarm(0);
        move = input_buf[0];
        close(pipe_from_agent[0]);

        printf("\n%c\n", player_char);
	    print_board(board);

        // Check invalid input
        if (move < 'A' || move > 'G') {
            printf("\nInvalid input! %c wins.\n", (current_player == 1) ? '2' : '1');
            winner = (current_player == 1) ? 2 : 1;
            break;
        }

        // Check full column
        col_idx = move - 'A';
        if (board[0][col_idx] != '0') {
            printf("\nColumn is full! %c wins.\n", (current_player == 1) ? '2' : '1');
            winner = (current_player == 1) ? 2 : 1;
            break;
        }

        // Place stone
        for (int i = ROWS - 1; i >= 0; i--) {
            if (board[i][col_idx] == '0') {
                board[i][col_idx] = player_char;
                break;
            }
        }

        moves++;
        winner = check_winner(board);
        
    // Print the board one last time to show the winning move
	if (winner != 0) {
		printf("\n%c\n", player_char);
		print_board(board);
		break;
	}

    current_player = (current_player == 1) ? 2 : 1;
	sleep(1); // For human-readable manner
    }

    // Print result
    if (winner == 0) {
        printf("Draw.\n");
    } else if (winner == 1) {
        printf("Player X wins!\n");
    } else {
        printf("Player Y wins!\n");
    }

    // Terminate all processes
    if (child_pid_x > 0) kill(child_pid_x, SIGKILL);
    if (child_pid_y > 0) kill(child_pid_y, SIGKILL);
    wait(NULL);
    wait(NULL);
}

// Print current board
void print_board(char board[ROWS][COLS]) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            printf("%c ", board[i][j]);
        }
        printf("\n");
    }
    printf("---------------\n");
}

int check_winner(char board[ROWS][COLS]) {
    // Check horizontal
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j <= COLS - 4; j++) {
            if (board[i][j] != '0' && board[i][j] == board[i][j + 1] &&
                board[i][j] == board[i][j + 2] && board[i][j] == board[i][j + 3]) {
                return (board[i][j] == '1') ? 1 : 2;
            }
        }
    }

    // Check vertical
    for (int i = 0; i <= ROWS - 4; i++) {
        for (int j = 0; j < COLS; j++) {
            if (board[i][j] != '0' && board[i][j] == board[i + 1][j] &&
                board[i][j] == board[i + 2][j] && board[i][j] == board[i + 3][j]) {
                return (board[i][j] == '1') ? 1 : 2;
            }
        }
    }

    // Check diagonal (down right \)
    for (int i = 0; i <= ROWS - 4; i++) {
        for (int j = 0; j <= COLS - 4; j++) {
            if (board[i][j] != '0' && board[i][j] == board[i + 1][j + 1] &&
                board[i][j] == board[i + 2][j + 2] && board[i][j] == board[i + 3][j + 3]) {
                return (board[i][j] == '1') ? 1 : 2;
            }
        }
    }

    // Check diagonal (down left /)
    for (int i = 0; i <= ROWS - 4; i++) {
        for (int j = COLS - 1; j >= 3; j--) {
            if (board[i][j] != '0' && board[i][j] == board[i + 1][j - 1] &&
                board[i][j] == board[i + 2][j - 2] && board[i][j] == board[i + 3][j - 3]) {
                return (board[i][j] == '1') ? 1 : 2;
            }
        }
    }

    // Check draw
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (board[i][j] == '0') return 0;
        }
    }
    return 3; // Draw
}
