#include "main.h"

static int run_args(int argc, char **argv)
{
	int i;
	if(argc % 2 == 0)
	{
		fprintf(stderr, ">>>>> Wrong number of arguments!\n");
		return -1;
	}
	for(i = 1; i < argc; i += 2)
	{
		switch(argv[i][1])
		{
			case 'i':
				break;
			case 'p':
				break;
			default:
				fprintf(stderr, ">>>>> Wrong  arguments!\n");
				return -1;
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	if(run_args(argc, argv) < 0)
		return -1;

	DecodeBuffer_t *decodeBuf = NULL;
	EncodeBuffer_t *encodeBuf = NULL;
	SendBuffer_t *sendBuf = NULL;

	decodeBuf = (DecodeBuffer_t *)decode_malloc(MAXBUFSIZE, MAXLINE);
	encodeBuf = (EncodeBuffer_t *)encode_malloc(MAXBUFSIZE, 20000);
	sendBuf = (SendBuffer_t *)send_malloc(MAXBUFSIZE, 20000);

	int recvPort = 8888;

	char *sendIP = "10.103.241.251";
	int sendPort = 8889;


	if(recv_func(NULL, recvPort, decodeBuf) < 0) return -1;
	if(decode_func(decodeBuf, encodeBuf) < 0) return -1;
	if(encode_func(encodeBuf, sendBuf) < 0) return -1;
	if(send_func(sendIP, sendPort, sendBuf) < 0) return -1;

	while(1)
	{
		sleep(10);
	}
	return 0;
}
