#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N_STACKS 7
#define STACK_CAP 6

// 보드: board[row][stack], row=1(아래)~6(위)
static int board[STACK_CAP + 1][N_STACKS + 1];
static int top[N_STACKS + 1]; 
static int this_player;

// 방향 정의
int const UPWARD    =  1;
int const DOWNWARD  = -1;
int const LEFTWARD  = -1;
int const RIGHTWARD =  1;

int count_adjacent_stones(int stack, int level, int hdir, int vdir, int player) {
    int cnt = 0;
    while (1 <= stack && stack <= N_STACKS && 1 <= level && level <= STACK_CAP) {
        if (board[level][stack] == player) {
            cnt++;
            stack += hdir;
            level += vdir;
        } else {
            break;
        }
    }
    return cnt;
}

// 방향별 체크 함수
int count_down(int st, int lv, int player){return count_adjacent_stones(st, lv + DOWNWARD, 0, DOWNWARD, player);}
int count_left(int st, int lv, int player){return count_adjacent_stones(st + LEFTWARD, lv, LEFTWARD, 0, player);}
int count_right(int st, int lv, int player){return count_adjacent_stones(st + RIGHTWARD, lv, RIGHTWARD, 0, player);}
int count_down_left(int st, int lv, int player){return count_adjacent_stones(st + LEFTWARD, lv + DOWNWARD, LEFTWARD, DOWNWARD, player);}
int count_down_right(int st, int lv, int player){return count_adjacent_stones(st + RIGHTWARD, lv + DOWNWARD, RIGHTWARD, DOWNWARD, player);}
int count_up_left(int st, int lv, int player){return count_adjacent_stones(st + LEFTWARD, lv + UPWARD, LEFTWARD, UPWARD, player);}
int count_up_right(int st, int lv, int player){return count_adjacent_stones(st + RIGHTWARD, lv + UPWARD, RIGHTWARD, UPWARD, player);}

// (a) 승리 수 찾기
int find_winning_move(int player) {
    for (int st = 1; st <= N_STACKS; st++) {
        // 이미 가득 찬 열
        if (top[st] > STACK_CAP) continue;

        int row = top[st];
        // 수직
        int down_cnt = count_down(st, row, player);
        if (1 + down_cnt >= 4) {
            fprintf(stderr, "[agentX] WIN vertical at stack=%d\n", st);
            return st;
        }
        // 수평
        int left_cnt = count_left(st, row, player);
        int right_cnt = count_right(st, row, player);
        if (1 + left_cnt + right_cnt >= 4) {
            fprintf(stderr, "[agentX] WIN horizontal at stack=%d\n", st);
            return st;
        }
        // 대각선 (\)
        int dl_cnt = count_down_left(st, row, player);
        int ur_cnt = count_up_right(st, row, player);
        if (1 + dl_cnt + ur_cnt >= 4) {
            fprintf(stderr, "[agentX] WIN diag(\\) at stack=%d\n", st);
            return st;
        }
        // 대각선 (/)
        int ul_cnt = count_up_left(st, row, player);
        int dr_cnt = count_down_right(st, row, player);
        if (1 + ul_cnt + dr_cnt >= 4) {
            fprintf(stderr, "[agentX] WIN diag(/) at stack=%d\n", st);
            return st;
        }
    }
    return 0;
}

// (b) 블로킹 수 찾기
int find_blocking_move(int player) {
    int other = (player == 1) ? 2 : 1;
    for (int st = 1; st <= N_STACKS; st++) {
        if (top[st] > STACK_CAP) continue;

        int row = top[st];
        // 수직
        int down_cnt = count_down(st, row, other);
        if (1 + down_cnt >= 4) {
            fprintf(stderr, "[agentX] BLOCK vertical at stack=%d\n", st);
            return st;
        }
        // 수평
        int left_cnt = count_left(st, row, other);
        int right_cnt = count_right(st, row, other);
        if (1 + left_cnt + right_cnt >= 4) {
            fprintf(stderr, "[agentX] BLOCK horizontal at stack=%d\n", st);
            return st;
        }
        // 대각선 (\)
        int dl_cnt = count_down_left(st, row, other);
        int ur_cnt = count_up_right(st, row, other);
        if (1 + dl_cnt + ur_cnt >= 4) {
            fprintf(stderr, "[agentX] BLOCK diag(\\) at stack=%d\n", st);
            return st;
        }
        // 대각선 (/)
        int ul_cnt = count_up_left(st, row, other);
        int dr_cnt = count_down_right(st, row, other);
        if (1 + ul_cnt + dr_cnt >= 4) {
            fprintf(stderr, "[agentX] BLOCK diag(/) at stack=%d\n", st);
            return st;
        }
    }
    return 0;
}

// [추가] 보드 디버그 출력 함수
void print_board_debug() {
    fprintf(stderr, "[agentX] Current board state (row=6 top, row=1=bottom):\n");
    for (int i = 6; i >= 1; i--) {
        fprintf(stderr, " row %d |", i);
        for (int s = 1; s <= N_STACKS; s++) {
            fprintf(stderr, " %d", board[i][s]);
        }
        fprintf(stderr, "\n");
    }
    // top[] 상태
    fprintf(stderr, "[agentX] top[] array:\n");
    for (int s = 1; s <= N_STACKS; s++) {
        fprintf(stderr, "  stack %d => %d\n", s, top[s]);
    }
    fprintf(stderr, "---------------------------------\n");
}

// [추가] 한 턴 분량의 보드 입력 + 로그
int read_board_once() {
    // 플레이어 번호
    if (scanf("%d", &this_player) != 1) {
        // EOF or error -> 종료 신호
        return 0;
    }
    fprintf(stderr, "\n[agentX] Turn start. player=%d\n", this_player);

    // board, top 초기화
    for (int s = 1; s <= N_STACKS; s++) {
        top[s] = 1;
    }
    for (int i = 1; i <= STACK_CAP; i++) {
        for (int s = 1; s <= N_STACKS; s++) {
            board[i][s] = 0;
        }
    }

    // 6행 (위) ~ 1행 (아래)
    for (int i = 6; i >= 1; i--) {
        for (int s = 1; s <= N_STACKS; s++) {
            if (scanf("%d", &board[i][s]) != 1) {
                fprintf(stderr, "[agentX] read board[%d][%d] fail.\n", i, s);
                return 0;
            }
        }
    }

    // top[] 계산
    for (int s = 1; s <= N_STACKS; s++) {
        while (top[s] <= STACK_CAP && board[top[s]][s] != 0) {
            top[s]++;
        }
    }

    // 디버그 출력
    print_board_debug();

    return 1; // 정상적으로 읽음
}

// [추가] 한 턴의 수를 결정
char decide_move() {
    // (1) 이길 수 있으면
    int w = find_winning_move(this_player);
    if (w) {
        fprintf(stderr, "[agentX] => Win move at stack=%d\n", w);
        return 'A' + (w - 1);
    }

    // (2) 상대를 막아야 한다면
    int b = find_blocking_move(this_player);
    if (b) {
        fprintf(stderr, "[agentX] => Block move at stack=%d\n", b);
        return 'A' + (b - 1);
    }

    // (3) 무작위
    srand((unsigned int)time(NULL));
    int possible[8], cnt=0;
    for (int s = 1; s <= N_STACKS; s++) {
        if (top[s] <= STACK_CAP) {
            possible[cnt++] = s;
        }
    }
    if (cnt == 0) {
        // 모두 가득
        fprintf(stderr, "[agentX] => All stacks full => default A\n");
        return 'A';
    }
    int r = rand() % cnt;
    int chosen = possible[r];
    fprintf(stderr, "[agentX] => Random choose stack=%d\n", chosen);
    return 'A' + (chosen - 1);
}

int main() {
    // 무한 루프: 턴마다 보드 입력 → 수 결정 → 출력
    while (1) {
        if (!read_board_once()) {
            // EOF or error => 에이전트 종료
            break;
        }
        // 수 결정
        char mv = decide_move();

        // 표준 출력
        fprintf(stderr, "[agentX] => Final move: %c\n", mv);
        printf("%c", mv);
        fflush(stdout);
    }

    fprintf(stderr, "[agentX] Terminating.\n");
    return 0;
}
