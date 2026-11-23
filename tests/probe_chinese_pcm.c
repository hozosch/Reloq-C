#include "reloq_eci.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static short pcm[220500];
static short captured[220500];
static size_t samples;

static int ECICALL callback(ECIHand engine,int message,int param,void *data)
{
    (void)engine;(void)data;
    if(message!=RELOQ_ECI_WAVEFORM_BUFFER)return 1;
    if(param<1||(size_t)param+samples>sizeof pcm/sizeof *pcm)return 2;
    memcpy(captured+samples,pcm,sizeof(short)*(size_t)param);
    samples+=(size_t)param;
    return 1;
}

int main(int argc,char **argv)
{
    if(argc!=3)return 2;
    ECIHand e=eciNewEx(RELOQ_ECI_CHINESE_CHINA);
    if(!e)return 3;
    eciRegisterCallback(e,callback,0);
    if(!eciSetParam(e,RELOQ_ECI_SAMPLE_RATE,1)||
       !eciSetOutputBuffer(e,220500,pcm))return 4;
    if(!eciAddTextUtf8(e,argv[1],strlen(argv[1]),0))return 5;
    if(!eciSynthesize(e))return 6;
    FILE *out=fopen(argv[2],"wb");
    if(!out)return 7;
    if(samples&&fwrite(captured,sizeof *pcm,samples,out)!=samples) {
        fclose(out);return 8;
    }
    fclose(out);
    printf("%zu\n",samples);
    eciDelete(e);
    return 0;
}
