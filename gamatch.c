#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <fcntl.h>

#define ROWS 6
#define COLS 7

int pipes[2];
int turn = 0;
char board[ROWS][COLS];

void init_board() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            board[i][j] = ' ';
        }
    }
}

void print_board() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            printf("|%c", board[i][j]);
        }
        printf("|\n");
    }
    printf("-----------------------------\n");
}

int place_piece(int col, char piece) {
    for (int i = ROWS - 1; i >= 0; i--) {
        if (board[i][col - 1] == ' ') {
            board[i][col - 1] = piece;
            return 1;
        }
    }
    return 0;
}

int check_win(char piece) {

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j <= COLS - 4; j++) {
            if (board[i][j] == piece && board[i][j + 1] == piece &&
                board[i][j + 2] == piece && board[i][j + 3] == piece) {
                return 1;
            }
        }
    }

    for (int i = 0; i <= ROWS - 4; i++) {
        for (int j = 0; j < COLS; j++) {
            if (board[i][j] == piece && board[i + 1][j] == piece &&
                board[i + 2][j] == piece && board[i + 3][j] == piece) {
                return 1;
            }
        }
    }

    for (int i = 0; i <= ROWS - 4; i++) {
        for (int j = 0; j <= COLS - 4; j++) {
            if (board[i][j] == piece && board[i + 1][j + 1] == piece &&
                board[i + 2][j + 2] == piece && board[i + 3][j + 3] == piece) {
                return 1;
            }
        }
    }

    for (int i = 0; i <= ROWS - 4; i++) {
        for (int j = 3; j < COLS; j++) {
            if (board[i][j] == piece && board[i + 1][j - 1] == piece &&
                board[i + 2][j - 2] == piece && board[i + 3][j - 3] == piece) {
                return 1;
            }
        }
    }

    return 0;
}

int check_draw() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (board[i][j] == ' ') {
                return 0;
            }
        }
    }
    return 1;
}

void timer_handler(int signum) {
    if (signum == SIGALRM) {
        turn++;
    }
}

int main(int argc, char *argv[]) {
    pid_t playerX, playerY;
    struct itimerval timer;

    if (argc != 5 || strcmp(argv[1], "-X") != 0 || strcmp(argv[3], "-Y") != 0) {
        fprintf(stderr, "Usage: %s -X <agent-binary> -Y <agent-binary>\n", argv[0]);
        exit(1);
    }

    if (pipe(pipes) == -1) {
        perror("pipe");
        exit(1);
    }

    signal(SIGALRM, timer_handler);

    timer.it_value.tv_sec = 1;
    timer.it_value.tv_usec = 100000;
    timer.it_interval = timer.it_value;

    setitimer(ITIMER_REAL, &timer, NULL);

    init_board();

    playerX = fork();

    if (playerX == -1) {
        perror("fork");
        exit(1);
    }

    if (playerX == 0) { // playerX (child)
        close(pipes[0]);

        while (1) {
            if (turn % 2 == 0) {
                write(pipes[1], "X:1", strlen("X:1") + 1);
                execl(argv[4], argv[4], (char *)NULL);
                perror("execl playerY failed");
                exit(1);
            }
        }
    } else {
        playerY = fork();

        if (playerY == -1) {
            perror("fork");
            exit(1);
        }

        if (playerY == 0) { 
            close(pipes[0]);

            while (1) {
                if (turn % 2 == 1) {
                    write(pipes[1], "Y:2", strlen("Y:2") + 1);
                    execl(argv[2], argv[2], (char *)NULL);
                    perror("execl playerX failed");
                    exit(1);
                }
            }
        } else {
            close(pipes[1]);

            while (1) {
                char buffer[10];
                read(pipes[0], buffer, sizeof(buffer));
                printf("Received: %s\n", buffer);

                int col = atoi(&buffer[2]);
                char piece = buffer[0];

                if (place_piece(col, piece)) {
                    print_board();
                    if (check_win(piece)) {
                        printf("%c wins!\n", piece);
                        break;
                    } else if (check_draw()) {
                        printf("Draw!\n");
                        break;
                    }
                }
            }

            close(pipes[0]);
            waitpid(playerX, NULL, 0);
            waitpid(playerY, NULL, 0);
        }
    }

    return 0;
}
