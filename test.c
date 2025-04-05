#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <poll.h>

int pipes[2];
int turn = 0;

void timer_handler(int sig) {
	if (sig == SIGALRM) {
		turn++;
	}
}

void exit_handler(int sig) {
	if (sig == SIGINT) {
		printf("\n");
		printf("program will be exit\n");
		exit(0);
	}
}

int main() {
	pid_t playerX, playerY;
	struct itimerval timer;
	struct pollfd fds[1];

	if (pipe(pipes) != 0) {
		perror("Error");
		exit(1);
	}

	signal(SIGALRM, timer_handler);
	signal(SIGINT, exit_handler);

	timer.it_value.tv_sec = 1;
	timer.it_interval.tv_usec = 100000;
	timer.it_interval = timer.it_value;

	setitimer(ITIMER_REAL, &timer, NULL);

	playerX = fork();

	if (playerX == 0) {
		// playerX
		close(pipes[0]);

		while(1) {
			if (turn % 2 == 0) {
			write(pipes[1], "playerX turn\n", strlen("playerX turn\n") + 1);
			sleep(1);
			}
		}
	} else {
		// playerY
		playerY = fork();

		if (playerY == 0) {
			close(pipes[0]);

			while (1) {
				if (turn % 2 == 1) {
					write(pipes[1], "playerY turn\n", strlen("playerY turn\n") + 1);
					sleep(1);
				}
			}
		} else {
			// parent process
			close(pipes[1]);
			fds[0].fd = pipes[0];
			fds[0].events = POLLIN;
			
			while(1) {
				if (poll(fds, 1, -1) > 0) {
					char buffer[32];
					read(pipes[0], buffer, sizeof(buffer));
					printf("Recived: %s\n", buffer);
				}
			}
		}
	}
	return 0;
}

