#include "heart_thread.h"

static void *heart_thread(void *);

int heart_func(void) {
	int err = 0;
	pthread_t tid;
	err = pthread_create(&tid, NULL, heart_thread, NULL);
	if (err) {
		fprintf(stderr, "\tcan't create heart_thread: %s\n", strerror(err));
		return -2;
	}

	usleep(1000*200);
	return 0;
}

static void *heart_thread(void *arg) {
	struct timeval t_start, t_end;
	long cost_t_sec, cost_t_usec;
	double rtt_temp;
	char ack[10];
	int n;
	while(1) {
		gettimeofday(&t_start, NULL);
		send(sendfd, "###", 3, 0);
		if(recv(sendfd, ack, 10, 0) <= 0) {
			rtt_temp = 0.3;
		} else {
			gettimeofday(&t_end, NULL);
			cost_t_sec = t_end.tv_sec - t_start.tv_sec;
			cost_t_usec = t_end.tv_usec - t_start.tv_usec;
			rtt_temp = cost_t_sec + 0.000001 * cost_t_usec;
		}

		rtt = 0.875 * rtt + 0.125 * rtt_temp;
		
		sleep(5);
	}

	pthread_exit((void *)0);
}


