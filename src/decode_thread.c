#include "decode_thread.h"

static void *decode_thread(void *);

void *decode_malloc(int buf_num, int buf_len)
{
	DecodeBuffer_t *buf = NULL;
	if((buf = (DecodeBuffer_t *)malloc(sizeof(DecodeBuffer_t))) == NULL)
		return NULL;
	if((buf->buffer = (uint8 **)malloc(buf_num*sizeof(uint8 *))) == NULL)
		return NULL;

	int i = 0;
	for(i=0; i<buf_num; i++)
	{
		if((buf->buffer[i] = (uint8 *)malloc(buf_len)) == NULL)
			return NULL;
		memset(buf->buffer[i], 0, buf_len);
	}

	sem_init(&buf->sem_empty, 0, buf_num);
	sem_init(&buf->sem_full, 0, 0);
	buf->sig_put = 0;
	buf->sig_get = 0;

	return (void *)buf;
}

int decode_func(DecodeBuffer_t *decodeBuf, EncodeBuffer_t *encodeBuf)
{
	int err;
	pthread_t tid;

	DoubleArg_t arg = {decodeBuf, encodeBuf};
	err = pthread_create(&tid, NULL, decode_thread, &arg);
	if(err)
	{
		fprintf(stderr, "\tcan't create decode_thread: %s\n", strerror(err));
		return -2;
	}
	usleep(1000 * 200);
	return 0;
}

static void *decode_thread(void *arg)
{
	DecodeBuffer_t *decodeBuf = (DecodeBuffer_t *)((DoubleArg_t *)arg)->arg1;
	EncodeBuffer_t *encodeBuf = (EncodeBuffer_t *)((DoubleArg_t *)arg)->arg2;
	pthread_detach(pthread_self());

	uint8 *inputBuf = NULL;
	uint8 *outputBuf = NULL;
	long outputBufSize = 0;

	int K_old = 0, R_old = 0;

	int cur_frame_no = -1;
	int last_frame_no = 0;

	uint32 raptor_K_recieve = 0;
	uint32 raptor_N_recieve = 0;
	uint32 raptor_K_temp = 0;
	uint32 raptor_N_temp = 0;

	uint16 *list_cur = NULL;
	uint16 list_temp = 0;

	uint8 *message = (uint8 *)malloc(MAX_PAC_LEN);
	FrameHeader_t *frameHeader = (FrameHeader_t *)malloc(sizeof(FrameHeader_t));
	uint8 *frame_data = NULL;

	int i, j;

	///////////////////////////////////////////////////////////
	long totalFrameNum = 100;
	long failFrameNum = 0;
	double rate_fail_temp = 0;

	struct timeval t_start, t_end;
	long cost_t_sec, cost_t_usec;
	////////////////////////////////////////////////////////////

	printf("decode_thread started...\n");
	while(1)
	{
		sem_wait(&decodeBuf->sem_full);
		memcpy(message, decodeBuf->buffer[decodeBuf->sig_get], MAX_PAC_LEN);
		sem_post(&decodeBuf->sem_empty);
		decodeBuf->sig_get = (decodeBuf->sig_get + 1) % CODE_BUF_NUM;
		//printf("decodeBuf->sig_get=%d\n", decodeBuf->sig_get);

		memcpy(frameHeader, message, sizeof(FrameHeader_t));

		frameHeader->T = ntohl(frameHeader->T);
		int T_cur = frameHeader->T;
		frameHeader->slice_no = ntohl(frameHeader->slice_no);

		int data_len;
		memcpy(&data_len, message+T_cur+sizeof(FrameHeader_t), 4);
		data_len = ntohl(data_len);

		//printf("data_len=%d\n", data_len);

		if(!frame_data)
			frame_data = (uint8 *)malloc(T_cur+1);
		for(i=0; i<data_len+1; i++)
		{
			if(i == 0){
				frameHeader->frame_no = ntohl(frameHeader->frame_no);
				frameHeader->frame_type = ntohl(frameHeader->frame_type);
				frameHeader->F = ntohl(frameHeader->F);
				frameHeader->K = ntohl(frameHeader->K);
				frameHeader->R = ntohl(frameHeader->R);
				frameHeader->esi = ntohl(frameHeader->esi);

				memcpy(frame_data, message+sizeof(FrameHeader_t), frameHeader->T);
			}
			else{
				(frameHeader->slice_no)++;
				(frameHeader->esi)++;

				memcpy(frame_data, message+i*T_cur+sizeof(FrameHeader_t)+sizeof(int), frameHeader->T);
			}

			//printf("frame_no=%d, cur_frame_no=%d\n", frameHeader->frame_no, cur_frame_no);
			//printf("frame_F=%d, frame_esi=%d\n", frameHeader->F, frameHeader->esi);
			if(frameHeader->frame_no != cur_frame_no)//当收到的分片的帧号与预计收到的不同时 cur_frame_no 是当前应该收到的帧号
			{//两种特殊情况：1.收到超前的分片 2.接收到已经过期的分片

				if(frameHeader->frame_no > cur_frame_no)//情况1
				{
					///////////////////////////////////////////////////////////////
					totalFrameNum = (totalFrameNum + 1) % INT_MAX;
					if (totalFrameNum > 0)
						rate_fail_temp = (double)failFrameNum / totalFrameNum;
					///////////////////////////////////////////////////////////////
					
					
					last_frame_no = cur_frame_no;

					if(frameHeader->frame_type == 1)//raptor
					{
						cur_frame_no = frameHeader->frame_no;//重新开始计数

						//保存上一帧的相关数据计数信息
						raptor_K_recieve = raptor_K_temp;
						raptor_N_recieve = raptor_N_temp;

						//重新计数
						raptor_K_temp = 0;

						if(frameHeader->esi < frameHeader->K)
							raptor_K_temp = 1;

						raptor_N_temp = 1;

						list_temp = frameHeader->esi;
					}
					else//非raptor 直接进入解码上一帧的流程
					{
						cur_frame_no = frameHeader->frame_no;
					}
				}
				else//情况2
				{
					//视频持续播放但是帧编号达到最大值并从0开始 或 视频回放过程中后退进度条
					if(frameHeader->frame_no <= 10)
					{
						last_frame_no = cur_frame_no;

						if(frameHeader->frame_type == 1)//raptor
						{
							//printf("recount\n");
							cur_frame_no = frameHeader->frame_no;//重新开始计数

							//保存上一帧的相关数据计数信息
							raptor_K_recieve = raptor_K_temp;
							raptor_N_recieve = raptor_N_temp;

							//重新计数
							raptor_K_temp = 0;

							if(frameHeader->esi < frameHeader->K)
								raptor_K_temp = 1;

							raptor_N_temp = 1;

							list_temp = frameHeader->esi;
						}
						else//非raptor 直接进入解码上一帧的流程
						{
							cur_frame_no = frameHeader->frame_no;
						}
					}
					else//直接丢弃
					{
						continue;
					}
				}//情况2
			}
			else//接收到预计帧的分片
			{
				if(frameHeader->frame_type == 1)//raptor
				{
					if(raptor_K_temp == K_old)
						continue;

					if(frameHeader->esi < frameHeader->K)
						raptor_K_temp++;

					raptor_N_temp++;

					if(!list_cur){
						list_cur = (uint16 *)malloc(sizeof(uint16)*(frameHeader->K+frameHeader->R));
					}
					list_cur[raptor_N_temp-1] = frameHeader->esi;

					if(!inputBuf)
					{
						int buf_size = (frameHeader->K+frameHeader->R)*T_cur;
						inputBuf = (uint8 *)malloc(buf_size+1);
						memset(inputBuf, 0, buf_size+1);

						K_old = frameHeader->K;
						R_old = frameHeader->R;
					}
					memcpy(inputBuf+(raptor_N_temp-1)*T_cur, frame_data, T_cur);

					continue;
				}
				else//非raptor
				{
					if(!outputBuf){
						int buf_size = (frameHeader->K)*T_cur;
						outputBuf = (uint8 *)malloc(buf_size+1);
					}

					outputBufSize = frameHeader->K*T_cur;
					memcpy(outputBuf+frameHeader->esi*T_cur, frame_data, T_cur);//将数据copy到等待加入264解码队列的输入区

					continue;//继续接收分片
				}
			}


			//printf("start decode\n");
			if(frameHeader->frame_type == 1)//raptor解码
			{
				int result_dec = 0;

				if(raptor_N_recieve > 0)
				{
					if(K_old == raptor_K_recieve && K_old)
					{
						outputBufSize = K_old*T_cur;

						if(outputBuf)
						{
							free(outputBuf);
							outputBuf = NULL;
						}
						outputBuf = (uint8 *)malloc(outputBufSize+1);
						memset(outputBuf, 0, outputBufSize+1);

						memcpy(outputBuf, inputBuf, outputBufSize);
						outputBuf[outputBufSize] = '\0';
						//printf("\tsatisfied K slice\n");
					}
					else
					{
						if(raptor_N_recieve > K_old)
						{
							RParamDec_t *para = (RParamDec_t *)malloc(sizeof(RParamDec_t));

							raptor_init(K_old, raptor_N_recieve, para, 1);

							for(j=0; j<raptor_N_recieve; j++)
							{
								para->list[j] = list_cur[j];
							}
							uint8 *temp = (uint8 *)malloc((para->S+para->H+raptor_N_recieve)*T_cur);
							memset(temp, 0, (para->S+para->H+raptor_N_recieve)*T_cur);
							memcpy(temp+(para->S+para->H)*T_cur,inputBuf, raptor_N_recieve*T_cur);

							uint8 *output = (uint8 *)malloc(K_old*T_cur);
							
							///////////////////////////////////////////////////////
							gettimeofday(&t_start, NULL);
							///////////////////////////////////////////////////////

							result_dec = raptor_decode(para, temp, output, T_cur);

							////////////////////////////////////////////////////////
							gettimeofday(&t_end, NULL);
							cost_t_sec = t_end.tv_sec - t_start.tv_sec;
							cost_t_usec = t_end.tv_usec - t_start.tv_usec;
							if (cost_t_usec < 0) {
								cost_t_usec += 1000000;
								cost_t_sec -= 1;
							}
							time_dec_temp = cost_t_sec + 0.000001 * cost_t_usec;
							////////////////////////////////////////////////////////


							if(result_dec)
							{
								//printf("use raptor_decode && success\n");
								if(outputBuf)
								{
									free(outputBuf);
									outputBuf = NULL;
								}
								outputBufSize = K_old*T_cur;
								outputBuf = (uint8 *)malloc(outputBufSize+1);
								memset(outputBuf, 0, outputBufSize+1);
								memcpy(outputBuf, output, outputBufSize);
								outputBuf[outputBufSize] = '\0';
							}
							else
							{
								//////////////////////////////////////////////////////
								failFrameNum = (failFrameNum + 1) % INT_MAX;
								rate_fail = 0.75 * rate_fail + 0.25 * rate_fail_temp;
								//////////////////////////////////////////////////////
								
								
								//printf("use raptor_decode, but error!!!\n");
								outputBufSize = (K_old+R_old)*T_cur;
								if(outputBufSize)
								{
									if(outputBuf)
									{
										free(outputBuf);
										outputBuf = NULL;
									}
									outputBuf = (uint8 *)malloc(outputBufSize+1);
									memset(outputBuf, 0, outputBufSize+1);

									for(j=0; j<raptor_N_recieve; j++)
									{
										memcpy(outputBuf+list_cur[j]*T_cur, inputBuf+j*T_cur, T_cur);
									}
								}
							}
							raptor_free(para, 1);
							free(para);
							free(output);
							free(temp);
							para = NULL;
							output = NULL;
							temp = NULL;
						}
						else //如果接收到的分片数不足，则将数据按esi存储进相应的位置
						{
							/////////////////////////////////////////////////////////
							failFrameNum = (failFrameNum + 1) % INT_MAX;
							rate_fail = 0.75 * rate_fail + 0.25 * rate_fail_temp;
							/////////////////////////////////////////////////////////

							//printf("lack of raptor_decode's slice, and error\n");
							outputBufSize = (K_old+R_old)*T_cur;
							if(outputBufSize)
							{
								if(outputBuf)
								{
									free(outputBuf);
									outputBuf = NULL;
								}
								outputBuf = (uint8 *)malloc(outputBufSize+1);
								memset(outputBuf, 0, outputBufSize+1);

								for(j=0; j<raptor_N_recieve; j++)
								{
									memcpy(outputBuf+list_cur[j]*T_cur, inputBuf+j*T_cur, T_cur);
								}
							}
						}//else
					}//if
				}

				free(inputBuf);
				inputBuf = NULL;
				K_old = 0;
				R_old = 0;
			}//raptor解码

			//printf("outputBufSize=%d\n", outputBufSize);
			///////////////////视频存储帧号可能不从1开始，因而导致视与源视频帧不对齐
			if(outputBufSize)
			{
				//qDebug()<<"frame= "<<last_frame_no<<" out_buf_size = "<<outputBufSize<<endl;
				sem_wait(&encodeBuf->sem_empty);
				encodeBuf->frame[encodeBuf->sig_put]->frameNo = last_frame_no;
				encodeBuf->frame[encodeBuf->sig_put]->size = outputBufSize;
				memcpy(encodeBuf->frame[encodeBuf->sig_put]->buffer, outputBuf, outputBufSize);
				sem_post(&encodeBuf->sem_full);
				encodeBuf->sig_put = (encodeBuf->sig_put + 1) % CODE_BUF_NUM;

				//printf("\tframe_no=%d, out_buf_size = %d\n", last_frame_no, outputBufSize);
				/*************************/
				free(outputBuf);
				outputBuf = NULL;
				/*************************/
			}

			//解码上一帧后，处理当前帧的第一个分片相关数据
			if(frameHeader->frame_type == 1)
			{
				if(list_cur){
					free(list_cur);
					list_cur = NULL;
				}
				list_cur = (uint16 *)malloc(sizeof(uint16)*(frameHeader->K+frameHeader->R));

				list_cur[0] = list_temp;

				/************************/
				int buf_size = (frameHeader->K+frameHeader->R)*T_cur;
				inputBuf = (uint8 *)malloc(buf_size+1);
				memset(inputBuf, 0, buf_size+1);

				K_old = frameHeader->K;
				R_old = frameHeader->R;
				/************************/

				memcpy(inputBuf, frame_data, T_cur);
			}
			else
			{
				/************************/
				int buf_size = frameHeader->K*T_cur;//cannot frameHeader->F!!!
				outputBuf = (uint8 *)malloc(buf_size+1);
				/************************/

				outputBufSize = frameHeader->K*T_cur;
				memcpy(outputBuf+frameHeader->esi*T_cur, frame_data, T_cur);
			}
		}//for
	}
	pthread_exit((void *)0);
}
