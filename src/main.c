#include "main.h"

/*******************************************************************************/
//global command
char IP_default[20] = "10.103.241.251";
int Port_default = 8888;
int K_default = 500;
int T_default = 128;
int Lost_default = 0;
int node_id = 1;
int mode = 0; //中继节点工作模式
/*******************************************************************************/

/*******************************************************************************/
//global var
int sendfd = 0;
double rtt = 0; //单跳往返时延
double time_dec = 0; //译码时延
double rate_loss = 0; //视频丢包率
double rate_fail = 0; //译码失败率

double time_enc_temp = 0;
double time_dec_temp = 0;
/*******************************************************************************/

static int run_args(int argc, char **argv) {
	int i;
	if(argc % 2 == 0) {
		fprintf(stderr, ">>>>> Wrong number of arguments!\n");
		return -1;
	}

	for(i = 1; i < argc; i += 2) {
		switch(argv[i][1]) {
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
				if(Lost_default <0 &&Lost_default>100) {
					fprintf(stderr, ">>>>> Wrong Lost_default! \n");
					return -1;
				}
				break;
			case 'n':
				node_id = atoi(argv[i+1]);
				break;
			case 'm':
				mode = atoi(argv[i+1]);
				break;
			default:
				fprintf(stderr, ">>>>> Wrong arguments!\n");
				return -1;
		}
	}

	printf("\t**********************************************************\n");
	printf("\t\tIP_default   = %15s\t -i\n",  IP_default);
	printf("\t\tport_default = %15d\t -p\n",  Port_default);
	printf("\t\tK_default    = %15d\t -k\n",  K_default);
	printf("\t\tT_default    = %15d\t -t\n",  T_default);
	printf("\t\tLost_default = %15d\t -l\n",  Lost_default);
	printf("\t\tNode_id      = %15d\t -n\n",  node_id);
	printf("\t**********************************************************\n");

	sleep(1);
	return 0;
}

int main(int argc, char *argv[]) {
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
	if(heart_func() < 0) return -1;
	
	while(1) {
		printf("***************************************************\n");
		printf("\tnode_id   = %6d\n", node_id);
		printf("\tmode      = %6d\n", mode);
		printf("\trate_loss = %6f\n", rate_loss);
		printf("\trtt       = %6f\n", rtt);
		printf("\trate_fail = %6f\n", rate_fail);
		printf("\ttime_dec  = %6f\n", time_dec);
		printf("***************************************************\n");
		printf("\n\n\n");

		sleep(5);
	}
	return 0;
}

