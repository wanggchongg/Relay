#include "main.h"

/*******************************************************************************/
//global variable
char IP_default[20] = "10.103.241.251";
int Port_default = 8888;
int K_default = 500;
int T_default = 128;
int Lost_default = 0;
/*******************************************************************************/

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
				strcpy(IP_default, argv[i+1]);
				break;
			case 'p':
				Port_default = atoi(argv[i+1]);
				break;
			case 't':
				T_default = atoi(argv[i+1]);
				break;
			case 'k':
				K_default = atoi(argv[i+1]);
				break;
			case 'l':
				Lost_default = atoi(argv[i+1]);
				if(Lost_default <0 &&Lost_default>100)
				{
					fprintf(stderr, ">>>>> Wrong Lost_default! \n");
					return -1;
				}
				break;
			default:
				fprintf(stderr, ">>>>> Wrong  arguments!\n");
				return -1;
		}
	}

	printf("\t**********************************************************\n");
	printf("\t\tIP_default   = %15s\t -i \n",  IP_default);
	printf("\t\tport_default = %15d\t -p\n",  Port_default);
	printf("\t\tK_default    = %15d\t -k\n",  K_default);
	printf("\t\tT_default    = %15d\t -t\n",  T_default);
	printf("\t\tLost_default = %15d\t -l\n",  Lost_default);
	printf("\t**********************************************************\n");

	sleep(1);
	return 0;
}

int main(int argc, char *argv[])
{
	if(run_args(argc, argv) < 0)
		return -1;

	DecodeBuffer_t *decodeBuf = NULL;
	EncodeBuffer_t *encodeBuf = NULL;
	SendBuffer_t *sendBuf = NULL;

	decodeBuf = (DecodeBuffer_t *)decode_malloc(CODE_BUF_NUM, MAX_PAC_LEN);
	encodeBuf = (EncodeBuffer_t *)encode_malloc(CODE_BUF_NUM, MAX_FRM_LEN);
	sendBuf = (SendBuffer_t *)send_malloc(SEND_BUF_NUM, sizeof(FrameHeader_t)+T_default);


	if(recv_func(NULL, Port_default, decodeBuf) < 0) return -1;
	if(decode_func(decodeBuf, encodeBuf) < 0) return -1;
	if(encode_func(encodeBuf, sendBuf) < 0) return -1;
	if(send_func(IP_default, Port_default, sendBuf) < 0) return -1;

	while(1)
	{
		sleep(10);
	}
	return 0;
}
