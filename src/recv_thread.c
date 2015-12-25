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

	////////////////////////////////////////////////
	int feed = 0;
	uint32 rand_num;
	////////////////////////////////////////////////



	//////////////////////////////////////////////////////////////////
	FrameHeader_t *frameHeader = (FrameHeader_t *)malloc(sizeof(FrameHeader_t));
	int data_len = 0;
	int cur_frame_no = -1;
	double rate_loss_temp = 0;
	uint32 slice_N = 1, slice_recv = 1;
	//////////////////////////////////////////////////////////////////


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

		//////////////////////////////////////////////////////////////
		// 模拟丢包
		if (Lost_default) {
			feed = (feed + 1) % 1000;
			srand(feed);
			rand_num = rand() % 100;
			if (rand_num <= Lost_default)
				continue;
		}
		//////////////////////////////////////////////////////////////


		////////////////////////////////////////////////////////////////////////////
		// 计算丢包率
		memcpy(frameHeader, message, sizeof(FrameHeader_t));
		frameHeader->T = ntohl(frameHeader->T);
		memcpy(&data_len, message+frameHeader->T+sizeof(FrameHeader_t), 4);
		data_len = ntohl(data_len);
		frameHeader->frame_no = ntohl(frameHeader->frame_no);
		if (cur_frame_no < frameHeader->frame_no) {
			rate_loss_temp = 1 - (double) slice_recv / slice_N;
			rate_loss = 0.75 * rate_loss + 0.25 * rate_loss_temp;
			frameHeader->K = ntohl(frameHeader->K);
			frameHeader->R = ntohl(frameHeader->R);
			slice_N = frameHeader->K + frameHeader->R;
			slice_recv = data_len + 1;
			cur_frame_no = frameHeader->frame_no;
		} else {
			slice_recv += data_len + 1;
		}
		/////////////////////////////////////////////////////////////////////////////
		
		
		if(mode) {
			sem_wait(&decodeBuf->sem_empty);
			memcpy(decodeBuf->buffer[decodeBuf->sig_put], message, mesglen);
			sem_post(&decodeBuf->sem_full);
			decodeBuf->sig_put = (decodeBuf->sig_put + 1) % CODE_BUF_NUM;
			//printf("decodeBuf->sig_put=%d, recv_len=%d\n", decodeBuf->sig_put, mesglen);
		} else {
			send(sendfd, message, mesglen, 0);	
		}
	}

	free(message);
	pthread_exit((void *)0);
}
