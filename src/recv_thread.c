#include "recv_thread.h"

static void *recv_thread(void *);

int recv_func(char *IP, int Port, DecodeBuffer_t *decodeBuf)
{
	int err = 0;
	pthread_t tid;
	int  listenfd;
	struct sockaddr_in servaddr;

	memset(&servaddr, '\0', sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(Port);
	if(IP)
	{
		if(inet_pton(AF_INET, IP, &servaddr.sin_addr) <= 0)
		{
			fprintf(stderr, "\trecv_func: inet_pton error for %s\n", IP);
			return -2;
		}
	}
	else
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if((listenfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("\trecv_func: UDP socket error");
		return -2;
	}

	int on=1;
	if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)  //设置套接字选项避免地址使用错误:address already in use
	{
		perror("\trecv_func: UDP socket setsockopt failed");
		close(listenfd);
		return -2;
	}

	if(bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		perror("\trecv_func: UDP bind error");
		close(listenfd);
		return -2;
	}

	DoubleArg_t arg = {decodeBuf, &listenfd};

	err = pthread_create(&tid, NULL, recv_thread, &arg);
	if(err)
	{
		fprintf(stderr, "\tcan't create recv_thread: %s\n", strerror(err));
		return -2;
	}

	usleep(1000*200);
	return 0;
}

static void *recv_thread(void *arg)
{
	DecodeBuffer_t *decodeBuf = (DecodeBuffer_t *)((DoubleArg_t *)arg)->arg1;
	int listenfd = *(int *)((DoubleArg_t *)arg)->arg2;
	pthread_detach(pthread_self()); //使线程处理分离状态

	socklen_t clilen;
	struct sockaddr_in  cliaddr;

	uint8_t *message = NULL;
	ssize_t  mesglen = 0;

	message = (uint8 *)malloc(MAX_PAC_LEN);
	printf("recv_thread started...\n");
	while(1)
	{
		//printf("waiting for the UDP packet...\n");

		if((mesglen = recvfrom(listenfd, message, MAX_PAC_LEN, 0, (struct sockaddr*)&cliaddr, &clilen)) < 0)
		{
			fprintf(stderr, "\trecv_thread: UDP recvfrom error\n");
			continue;
		}
		else if('#' == message[0])
		{
			sendto(listenfd, "###", 3, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
			continue;
		}
		else if(mesglen == 0)
		{
			continue;
		}

		sem_wait(&decodeBuf->sem_empty);
		memcpy(decodeBuf->buffer[decodeBuf->sig_put], message, mesglen);
		sem_post(&decodeBuf->sem_full);
		decodeBuf->sig_put = (decodeBuf->sig_put + 1) % CODE_BUF_NUM;
		//printf("decodeBuf->sig_put=%d, recv_len=%d\n", decodeBuf->sig_put, mesglen);
	}

	free(message);
	pthread_exit((void *)0);
}
