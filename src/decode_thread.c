#include "decode_thread.h"

static void *decode_thread(void *);

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

	FrameHeader_t *frameHeader = NULL;
    uint8 *inputBuf = NULL;
    uint8 *outputBuf = NULL;
    long inputBufSize = 0;
    long outputBufSize = 0;

    int K_old = 0, R_old = 0;

    int cur_frame_no = -1;
    int last_frame_no = 0;
    int old_frame_no = -1;

    uint32 raptor_K_recieve = 0;
    uint32 raptor_R_recieve = 0;
    uint32 raptor_N_recieve = 0;
    uint32 raptor_K_temp = 0;
    uint32 raptor_R_temp = 0;
    uint32 raptor_N_temp = 0;

    uint16 *list_cur = NULL;
    uint16 list_temp = 0;

    int i, j;
    while(1)
    {
    	uint8 *tmp_buf_total = (uint8 *)malloc(MAXLINE);
    	memset(tmp_buf_total, 0, MAXLINE);
		sem_wait(&decodeBuf->sem_full);
        memcpy(tmp_buf_total, decodeBuf->buffer[decodeBuf->sig_get], MAXLINE);
        sem_post(&decodeBuf->sem_empty);
        decodeBuf->sig_get = (decodeBuf->sig_get + 1) % MAXBUFSIZE;

        frameHeader = (FrameHeader_t *)malloc(sizeof(FrameHeader_t));
        memcpy(frameHeader, tmp_buf_total, sizeof(FrameHeader_t));

        frameHeader->T = ntohl(frameHeader->T);
        int T_cur = frameHeader->T;
        frameHeader->slice_no = ntohl(frameHeader->slice_no);

        int data_len;
        memcpy(&data_len, tmp_buf_total+T_cur+sizeof(FrameHeader_t), 4);
        data_len = ntohl(data_len);

        for(i=0; i<data_len+1; i++)
	    {
	        uint8 *frame_data = (uint8 *)malloc(T_cur+1);
	        memset(frame_data, 0, T_cur+1);

	        if(i == 0){
	            frameHeader->frame_no = ntohl(frameHeader->frame_no);
	            frameHeader->frame_type = ntohl(frameHeader->frame_type);
	            frameHeader->F = ntohl(frameHeader->F);
	            frameHeader->K = ntohl(frameHeader->K);
	            frameHeader->R = ntohl(frameHeader->R);
	            frameHeader->esi = ntohl(frameHeader->esi);

	            memcpy(frame_data, tmp_buf_total+sizeof(FrameHeader_t), frameHeader->T);

	            if(i == data_len){
	                free(tmp_buf_total);
	                tmp_buf_total = NULL;
	            }
	        }
	        else{
	            (frameHeader->slice_no)++;
	            (frameHeader->esi)++;

	            memcpy(frame_data, tmp_buf_total+i*T_cur+sizeof(FrameHeader_t)+sizeof(int), frameHeader->T);

	            if(i == data_len){
	                free(tmp_buf_total);
	                tmp_buf_total = NULL;
	            }
	        }

	        if(frameHeader->frame_no != cur_frame_no)//当收到的分片的帧号与预计收到的不同时 cur_frame_no 是当前应该收到的帧号
	        {//两种特殊情况：1.收到超前的分片 2.接收到已经过期的分片
	            if(frameHeader->frame_no > cur_frame_no)//情况1
	            {
	                last_frame_no = cur_frame_no;

	                if(frameHeader->frame_type == 1)//raptor
	                {
	                    cur_frame_no = frameHeader->frame_no;//重新开始计数

	                    //保存上一帧的相关数据计数信息
	                    raptor_K_recieve = raptor_K_temp;
	                    raptor_R_recieve = raptor_R_temp;
	                    raptor_N_recieve = raptor_N_temp;

	                    //重新计数
	                    raptor_K_temp = 0;
	                    raptor_R_temp = 0;

	                    if(frameHeader->esi < frameHeader->K)
	                        raptor_K_temp = 1;
	                    else
	                        raptor_R_temp = 1;
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
	                        raptor_R_recieve = raptor_R_temp;
	                        raptor_N_recieve = raptor_N_temp;

	                        //重新计数
	                        raptor_K_temp = 0;
	                        raptor_R_temp = 0;

	                        if(frameHeader->esi < frameHeader->K)
	                            raptor_K_temp = 1;
	                        else
	                            raptor_R_temp = 1;
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
	                else
	                    raptor_R_temp++;
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

	                free(frame_data);
	                frame_data = NULL;

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

	                free(frame_data);
	                frame_data = NULL;

	                continue;//继续接收分片
	            }
	        }


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

	                        result_dec = raptor_decode(para, temp, output, T_cur);
	                        if(result_dec)
	                        {
	                            //cout<<"use raptor_decode && success"<<endl;
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
	                            //cout<<"raptor_decode error!!!"<<endl;
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
	                        //cout<<"lack of raptor_decode's slice"<<endl;
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
	                }
	            }

	            free(inputBuf);
	            inputBuf = NULL;
	            inputBufSize = 0;
	            K_old = 0;
	            R_old = 0;
	        }//raptor解码

	        ///////////////////视频存储帧号可能不从1开始，因而导致视与源视频帧不对齐
	        if(outputBufSize)
	        {
	            //qDebug()<<"frame= "<<last_frame_no<<" out_buf_size = "<<outputBufSize<<endl;
	            sem_wait(&encodeBuf->sem_empty);
	            encodeBuf->frame[encodeBuf->sig_put]->frameNo = last_frame_no;
	            encodeBuf->frame[encodeBuf->sig_put]->size = outputBufSize;
				memcpy(encodeBuf->frame[encodeBuf->sig_put]->buffer, outputBuf, outputBufSize);
	            sem_post(&encodeBuf->sem_full);
	            encodeBuf->sig_put = (encodeBuf->sig_put + 1) % MAXBUFSIZE;

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
	            inputBufSize = T_cur;

	            free(frame_data);
	            frame_data = NULL;
	        }
	        else
	        {
	            /************************/
	            int buf_size = frameHeader->K*T_cur;//cannot frameHeader->F!!!
	            outputBuf = (uint8 *)malloc(buf_size+1);
	            /************************/

	            outputBufSize = frameHeader->K*T_cur;
	            memcpy(outputBuf+frameHeader->esi*T_cur, frame_data, T_cur);

	            free(frame_data);
	            frame_data = NULL;
	        }
	    }//for

	    free(frameHeader);
	    frameHeader = NULL;
    }
    pthread_exit((void *)0);
}
