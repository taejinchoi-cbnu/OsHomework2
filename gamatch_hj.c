#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_STACK 7
#define MAX_HEIGHT 6
#define TIMEOUT 3 // 초 단위 시간 제한

// 함수 선언
void print_usage();
void run_game(char *agent_x, char *agent_y);
void print_board(char board[MAX_HEIGHT][MAX_STACK]);
int check_winner(char board[MAX_HEIGHT][MAX_STACK]);

// 전역 변수
pid_t child_pid_x = 0;
pid_t child_pid_y = 0;

// 시그널 핸들러
void signal_handler(int signo) {
    if (signo == SIGINT) {
        printf("\nCtrl+C를 감지했습니다. Gamatch를 종료합니다.\n");
        if (child_pid_x > 0) kill(child_pid_x, SIGKILL);
        if (child_pid_y > 0) kill(child_pid_y, SIGKILL);
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    // 인자 처리
    if (argc != 3) {
        print_usage();
        return 1;
    }

    char *agent_x = argv[1];
    char *agent_y = argv[2];

    // 시그널 핸들러 등록
    signal(SIGINT, signal_handler);

    // 게임 실행
    run_game(agent_x, agent_y);

    return 0;
}

// 사용법 출력 함수
void print_usage() {
    printf("사용법: ./gamatch <agent_x_binary> <agent_y_binary>\n");
}

// 게임 실행 함수
void run_game(char *agent_x, char *agent_y) {
    int pipe_x_to_gamatch[2];
    int pipe_gamatch_to_x[2];
    int pipe_y_to_gamatch[2];
    int pipe_gamatch_to_y[2];
    pid_t pid_x, pid_y;
    char board[MAX_HEIGHT][MAX_STACK];
    int current_player = 1; // 1: X, 2: Y
    int winner = 0; // 0: 진행 중, 1: X 승리, 2: Y 승리, 3: 무승부
    int moves = 0;

    // 게임 보드 초기화
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_STACK; j++) {
            board[i][j] = ' ';
        }
    }

    // 파이프 생성
    if (pipe(pipe_x_to_gamatch) == -1 || pipe(pipe_gamatch_to_x) == -1 ||
        pipe(pipe_y_to_gamatch) == -1 || pipe(pipe_gamatch_to_y) == -1) {
        perror("파이프 생성 오류");
        exit(1);
    }

    // X Agent 프로세스 생성
    pid_x = fork();
    if (pid_x == -1) {
        perror("fork 오류");
        exit(1);
    }

    if (pid_x == 0) { // 자식 프로세스 (X Agent)
        close(pipe_gamatch_to_x[1]);
        close(pipe_x_to_gamatch[0]);

        dup2(pipe_gamatch_to_x[0], STDIN_FILENO);
        dup2(pipe_x_to_gamatch[1], STDOUT_FILENO);

        close(pipe_gamatch_to_x[0]);
        close(pipe_x_to_gamatch[1]);

        execl(agent_x, agent_x, NULL);
        perror("execl 오류");
        exit(1);
    }

    child_pid_x = pid_x; // 전역 변수에 저장

    close(pipe_gamatch_to_x[0]);
    close(pipe_x_to_gamatch[1]);

    // Y Agent 프로세스 생성
    pid_y = fork();
    if (pid_y == -1) {
        perror("fork 오류");
        exit(1);
    }

    if (pid_y == 0) { // 자식 프로세스 (Y Agent)
        close(pipe_gamatch_to_y[1]);
        close(pipe_y_to_gamatch[0]);

        dup2(pipe_gamatch_to_y[0], STDIN_FILENO);
        dup2(pipe_y_to_gamatch[1], STDOUT_FILENO);

        close(pipe_gamatch_to_y[0]);
        close(pipe_y_to_gamatch[1]);

        execl(agent_y, agent_y, NULL);
        perror("execl 오류");
        exit(1);
    }

    child_pid_y = pid_y; // 전역 변수에 저장

    close(pipe_gamatch_to_y[0]);
    close(pipe_y_to_gamatch[1]);

    // 게임 진행
    while (moves < MAX_STACK * MAX_HEIGHT && winner == 0) {
        char move;
        char player_char = (current_player == 1) ? 'X' : 'Y';
        int player_num = current_player;
        int stack_index;
        char input_buffer[10];

        // 현재 보드 출력
        printf("\n현재 보드:\n");
        print_board(board);

        // Agent에게 입력 전달
        if (current_player == 1) {
            dprintf(pipe_gamatch_to_x[1], "%d\n", player_num);
            for (int i = 0; i < MAX_HEIGHT; i++) {
                for (int j = 0; j < MAX_STACK; j++) {
                    dprintf(pipe_gamatch_to_x[1], "%d", (board[i][j] == 'X') ? 1 : ((board[i][j] == 'Y') ? 2 : 0));
                    if (j < MAX_STACK - 1) dprintf(pipe_gamatch_to_x[1], " ");
                }
                dprintf(pipe_gamatch_to_x[1], "\n");
            }
            fsync(pipe_gamatch_to_x[1]); // 데이터 즉시 전송
 	        //close(pipe_gamatch_to_x[1]);
            //pipe_gamatch_to_x[1] = -1;
            read(pipe_x_to_gamatch[0], input_buffer, sizeof(input_buffer));
            move = input_buffer[0];
            //close(pipe_x_to_gamatch[0]);
            //pipe_x_to_gamatch[0] = -1;
        } else {
            dprintf(pipe_gamatch_to_y[1], "%d\n", player_num);
            for (int i = 0; i < MAX_HEIGHT; i++) {
                for (int j = 0; j < MAX_STACK; j++) {
                    dprintf(pipe_gamatch_to_y[1], "%d", (board[i][j] == 'X') ? 1 : ((board[i][j] == 'Y') ? 2 : 0));
                    if (j < MAX_STACK - 1) dprintf(pipe_gamatch_to_y[1], " ");
                }
                dprintf(pipe_gamatch_to_y[1], "\n");
            }
            fsync(pipe_gamatch_to_y[1]); // 데이터 즉시 전송
            //close(pipe_gamatch_to_y[1]);
            //pipe_gamatch_to_y[1] = -1;
            read(pipe_y_to_gamatch[0], input_buffer, sizeof(input_buffer));
            move = input_buffer[0];
            //close(pipe_y_to_gamatch[0]);
            //pipe_y_to_gamatch[0] = -1;
        }

        // 입력 유효성 검사
        if (move < 'A' || move > 'G') {
            printf("잘못된 입력입니다! 상대방이 승리합니다.\n");
            winner = (current_player == 1) ? 2 : 1;
            break;
        }

        stack_index = move - 'A';

        if (board[0][stack_index] != ' ') {
            printf("해당 열이 가득 찼습니다! 상대방이 승리합니다.\n");
            winner = (current_player == 1) ? 2 : 1;
            break;
        }


        // 돌 놓기
        for (int i = MAX_HEIGHT - 1; i >= 0; i--) {
            if (board[i][stack_index] == ' ') {
                board[i][stack_index] = player_char;
                break;
            }
        }

        moves++;

        // 승리 여부 확인
        winner = check_winner(board);
        if (winner != 0) break;

        // 턴 변경
        current_player = (current_player == 1) ? 2 : 1;

        // 시각화를 위한 잠시 대기 (선택 사항)
        sleep(1);
    }
    // 게임 끝난 후에만 파이프 정리:
    close(pipe_gamatch_to_x[1]);
    close(pipe_x_to_gamatch[0]);
    close(pipe_gamatch_to_y[1]);
    close(pipe_y_to_gamatch[0]);


    // 결과 출력
    printf("\n게임 종료!\n");
    print_board(board);

    if (winner == 0) {
        printf("무승부입니다.\n");
    } else if (winner == 1) {
        printf("X 플레이어 승리!\n");
    } else {
        printf("Y 플레이어 승리!\n");
    }

    // 자식 프로세스 정리
    if (child_pid_x > 0) kill(child_pid_x, SIGKILL);
    if (child_pid_y > 0) kill(child_pid_y, SIGKILL);
    wait(NULL);
    wait(NULL);
}

// 게임 보드 출력 함수
void print_board(char board[MAX_HEIGHT][MAX_STACK]) {
    printf("  A B C D E F G\n");
    for (int i = 0; i < MAX_HEIGHT; i++) {
        printf("%d ", MAX_HEIGHT - i);
        for (int j = 0; j < MAX_STACK; j++) {
            printf("%c ", board[i][j]);
        }
        printf("\n");
    }
}

// 승리 여부 확인 함수
int check_winner(char board[MAX_HEIGHT][MAX_STACK]) {
    // 가로 확인
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j <= MAX_STACK - 4; j++) {
            if (board[i][j] != ' ' && board[i][j] == board[i][j + 1] &&
                board[i][j] == board[i][j + 2] && board[i][j] == board[i][j + 3]) {
                return (board[i][j] == 'X') ? 1 : 2;
            }
        }
    }

    // 세로 확인
    for (int i = 0; i <= MAX_HEIGHT - 4; i++) {
        for (int j = 0; j < MAX_STACK; j++) {
            if (board[i][j] != ' ' && board[i][j] == board[i + 1][j] &&
                board[i][j] == board[i + 2][j] && board[i][j] == board[i + 3][j]) {
                return (board[i][j] == 'X') ? 1 : 2;
            }
        }
    }

    // 대각선 (오른쪽 아래) 확인
    for (int i = 0; i <= MAX_HEIGHT - 4; i++) {
        for (int j = 0; j <= MAX_STACK - 4; j++) {
            if (board[i][j] != ' ' && board[i][j] == board[i + 1][j + 1] &&
                board[i][j] == board[i + 2][j + 2] && board[i][j] == board[i + 3][j + 3]) {
                return (board[i][j] == 'X') ? 1 : 2;
            }
        }
    }

    // 대각선 (왼쪽 아래) 확인
    for (int i = 0; i <= MAX_HEIGHT - 4; i++) {
        for (int j = MAX_STACK - 1; j >= 3; j--) {
            if (board[i][j] != ' ' && board[i][j] == board[i + 1][j - 1] &&
                board[i][j] == board[i + 2][j - 2] && board[i][j] == board[i + 3][j - 3]) {
                return (board[i][j] == 'X') ? 1 : 2;
            }
        }
    }

    // 무승부 확인
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_STACK; j++) {
            if (board[i][j] == ' ') {
                return 0; // 아직 빈 칸이 있음
            }
        }
    }

    return 3; // 무승부
}
