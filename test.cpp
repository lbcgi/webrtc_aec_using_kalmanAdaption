#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "echo_cancellation.h"

#define MAX_SAMPLERATE   (16000)
#define MAX_SAMPLES_10MS ((MAX_SAMPLERATE*10)/1000)
#define KALMAN_ADAPTION

using namespace webrtc;

int main(int argc, char **argv)
{
    FILE *fref = NULL;
    FILE *fmic = NULL;
    FILE *faec = NULL;
    void *haec = NULL;
    short sref[MAX_SAMPLES_10MS];
    short smic[MAX_SAMPLES_10MS];
    short saec[MAX_SAMPLES_10MS];
    float dref[MAX_SAMPLES_10MS];
    float dmic[MAX_SAMPLES_10MS];
    float daec[MAX_SAMPLES_10MS];
    int framesamples = 0;
    int framedelays = 0;
    int frametimes = 10;//为什么是这样的
    int samplerate = 0;
    int framecnt = 0;
    int status = 0;
    int delay = 0;

   /* if (argc != 6) 
    {
        printf("usage: dse mic.pcm ref.pcm aec.pcm samplerate delay(ms)\n");
        return -1;
    }
	*/
   // samplerate = atoi(argv[4]);
	samplerate = 8000;
    /*if (samplerate != 8000  && 
        samplerate != 16000)
    {
        printf("samplerate %d unsupported(%s)\n",samplerate, argv[4]);
        return -1;
    }*/
    framesamples = (samplerate * frametimes) / 1000;

//  delay = atoi(argv[5]);
	delay = 0;
    framedelays = (delay + (frametimes - 1)) / frametimes;

	haec = WebRtcAec_Create();
    if (NULL == haec)
    {
        printf("create aec failed\n");
        return -1;
    }

    status = WebRtcAec_Init(haec, samplerate, samplerate);
    if (status)
    {
        printf("init aec failed, return %d\n", status);
        goto exitproc;
    }

    fmic = fopen("rec.wav", "rb");
    if (NULL == fmic)
    {
        printf("fopen %s failed\n", argv[1]);
        goto exitproc;
    }

    fref = fopen("ref.wav", "rb");
    if (NULL == fref)
    {
        printf("fopen %s failed\n", argv[2]);
        goto exitproc;
    }

    faec = fopen("aec.wav", "wb");
    if (NULL == faec)
    {
        printf("fopen %s failed\n", argv[3]);
        goto exitproc;
    }

    while(1)
    {
	    int i = 0;
		//获取参考数据(远端数据)
        if (fread(sref, sizeof(short), framesamples, fref) != framesamples)
        {
            printf("process all data, exiting...\n");
            goto exitproc;
        }
		//获取近端数据
        if (framecnt >= framedelays) 
        {
            if (fread(smic, sizeof(short), framesamples, fmic) != framesamples)
            {
                printf("process all data, exiting...\n");
                goto exitproc;
            }
        }
        else 
        {
            memset(smic, 0, sizeof(short)*framesamples);
        }

	    for (i = 0; i < framesamples; i++)
	    {
            dref[i] = (float)sref[i];
            dmic[i] = (float)smic[i];
	    }
		//缓存远端
        WebRtcAec_BufferFarend(haec, dref, framesamples);

        float* const pfmic = &(dmic[0]);
	    const float* const* ppfmic = &pfmic;
 
        float* const pfaec = &(daec[0]);
	    float* const* ppfaec = &pfaec;
		//处理近端回声
        WebRtcAec_Process(haec, ppfmic, 1, ppfaec, framesamples, delay, 0);

        for (i = 0; i < framesamples; i++)
        {
            saec[i] = (short)daec[i];
        }
		//保存回声消除的结果
        fwrite(saec, sizeof(short), framesamples, faec);

        framecnt++;
    }

exitproc:
    if (faec)
    {
        fclose(faec);
    }
    if (fref)
    {
        fclose(fref);
    }
    if (fmic)
    {
        fclose(fmic);
    }
    if (haec)
    {
        WebRtcAec_Free(haec);
    }

    return 0;
}
 

