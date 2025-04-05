#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>

void handler(int sig) {
	/*if (sig == SIGINT) {
		printf("do you want to quit?\n");
		if (getchar() == 'y');
			exit(1);
	}*/

	if (sig == SIGALRM) {
		printf("Ring~\n");
	}
}
 
int main() {
	struct itimerval t;

	//signal(SIGINT, handler);
	signal(SIGALRM, handler);
	
	t.it_value.tv_sec = 2;
	t.it_interval = t.it_value;

	setitimer(ITIMER_REAL, &t, 0x0);

	while(1);
}
