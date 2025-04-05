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

void print_usage() {
    printf("Usage: ./gamatch -X <agent-binary> -Y <agent-binary>\n");
}

void run_game(char *agent_x, char *agent_y);
void print_board(char board[MAX_HEIGHT][MAX_STACK]);
int check_winner(char board[MAX_HEIGHT][MAX_STACK]);

pid_t child_pid_x = 0;
pid_t child_pid_y = 0;

void signal_handler(int signo) {
    if (signo == SIGINT || signo == SIGALRM) {
        if (child_pid_x > 0) kill(child_pid_x, SIGKILL);
        if (child_pid_y > 0) kill(child_pid_y, SIGKILL);
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 5 || strcmp(argv[1], "-X") != 0 || strcmp(argv[3], "-Y") != 0) {
        print_usage();
        return 1;
    }

    char *agent_x = argv[2];
    char *agent_y = argv[4];

    signal(SIGINT, signal_handler);
    signal(SIGALRM, signal_handler);
    run_game(agent_x, agent_y);

    return 0;
}

void run_game(char *agent_x, char *agent_y) {
    char board[MAX_HEIGHT][MAX_STACK];
    int current_player = 1; // 1: X, 2: Y
    int winner = 0;
    int moves = 0;

    // 보드 초기화
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_STACK; j++) {
            board[i][j] = '0';
        }
    }

    while (moves < MAX_STACK * MAX_HEIGHT && winner == 0) {
        int pipe_to_agent[2], pipe_from_agent[2];
        pid_t pid;
        char move;
        char player_char = (current_player == 1) ? '1' : '2';
        int stack_index;
        char input_buffer[10];

        // 파이프 생성
        if (pipe(pipe_to_agent) != 0 || pipe(pipe_from_agent) != 0) {
            perror("Pipe Error");
            exit(1);
        }

        pid = fork();
        if (pid == -1) {
            perror("fork failed");
            exit(1);
        }

        if (pid == 0) { // 자식 프로세스
            close(pipe_to_agent[1]);
            close(pipe_from_agent[0]);
            dup2(pipe_to_agent[0], STDIN_FILENO);
            dup2(pipe_from_agent[1], STDOUT_FILENO);
            close(pipe_to_agent[0]);
            close(pipe_from_agent[1]);

            if (current_player == 1) {
                execl(agent_x, agent_x, NULL);
            } else {
                execl(agent_y, agent_y, NULL);
            }
            perror("execl failed");
            exit(1);
        }

        if (current_player == 1) child_pid_x = pid;
        else child_pid_y = pid;

        // 부모 프로세스: 파이프로 데이터 전송
        close(pipe_to_agent[0]);
        close(pipe_from_agent[1]);

        dprintf(pipe_to_agent[1], "%d\n", current_player);
        for (int i = 0; i < MAX_HEIGHT; i++) {
            for (int j = 0; j < MAX_STACK; j++) {
                dprintf(pipe_to_agent[1], "%d", board[i][j] - '0');
                if (j < MAX_STACK - 1) dprintf(pipe_to_agent[1], " ");
            }
            dprintf(pipe_to_agent[1], "\n");
        }
        close(pipe_to_agent[1]);

        // 타임아웃 설정
        alarm(TIMEOUT);
        read(pipe_from_agent[0], input_buffer, sizeof(input_buffer));
        alarm(0); // 타임아웃 해제
        move = input_buffer[0];
        close(pipe_from_agent[0]);

        printf("\n%c\n", player_char);
        print_board(board);

        if (move < 'A' || move > 'G') {
            printf("\nInvalid input! %c wins.\n", (current_player == 1) ? '2' : '1');
            winner = (current_player == 1) ? 2 : 1;
            break;
        }

        stack_index = move - 'A';
        if (board[0][stack_index] != '0') {
            printf("\nColumn is full! %c wins.\n", (current_player == 1) ? '2' : '1');
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
        winner = check_winner(board);
        if (winner != 0) break;

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

    if (child_pid_x > 0) kill(child_pid_x, SIGKILL);
    if (child_pid_y > 0) kill(child_pid_y, SIGKILL);
    wait(NULL);
    wait(NULL);
}

void print_board(char board[MAX_HEIGHT][MAX_STACK]) {
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_STACK; j++) {
            printf("%c ", board[i][j]);
        }
        printf("\n");
    }
    printf("---------------\n");
}

int check_winner(char board[MAX_HEIGHT][MAX_STACK]) {
    // 수평 체크
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j <= MAX_STACK - 4; j++) {
            if (board[i][j] != '0' && board[i][j] == board[i][j + 1] &&
                board[i][j] == board[i][j + 2] && board[i][j] == board[i][j + 3]) {
                return (board[i][j] == '1') ? 1 : 2;
            }
        }
    }

    // 수직 체크
    for (int i = 0; i <= MAX_HEIGHT - 4; i++) {
        for (int j = 0; j < MAX_STACK; j++) {
            if (board[i][j] != '0' && board[i][j] == board[i + 1][j] &&
                board[i][j] == board[i + 2][j] && board[i][j] == board[i + 3][j]) {
                return (board[i][j] == '1') ? 1 : 2;
            }
        }
    }

    // 대각선 (오른쪽 아래)
    for (int i = 0; i <= MAX_HEIGHT - 4; i++) {
        for (int j = 0; j <= MAX_STACK - 4; j++) {
            if (board[i][j] != '0' && board[i][j] == board[i + 1][j + 1] &&
                board[i][j] == board[i + 2][j + 2] && board[i][j] == board[i + 3][j + 3]) {
                return (board[i][j] == '1') ? 1 : 2;
            }
        }
    }

    // 대각선 (왼쪽 아래)
    for (int i = 0; i <= MAX_HEIGHT - 4; i++) {
        for (int j = MAX_STACK - 1; j >= 3; j--) {
            if (board[i][j] != '0' && board[i][j] == board[i + 1][j - 1] &&
                board[i][j] == board[i + 2][j - 2] && board[i][j] == board[i + 3][j - 3]) {
                return (board[i][j] == '1') ? 1 : 2;
            }
        }
    }

    // 무승부 체크
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_STACK; j++) {
            if (board[i][j] == '0') return 0;
        }
    }
    return 3; // Draw
}
