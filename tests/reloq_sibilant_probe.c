#include "reloq_core.h"
#include "reloq_core_internal.h"
#include "reloq_german_voice_defaults.h"
#include "reloq_frame_adapter.h"
#include <string.h>
/* Test-only bridge to the production numeric renderer; audio stays in RAM. */
int reloq_test_sibilant_probe(int rate,int profile,int kind,int seed,float centre,float *out,unsigned capacity)
{
 float c[RELOQ_PARAMETER_TRACKS];unsigned char map[RELOQ_TARGET_STREAM_COUNT];
 struct reloq_session_config cfg;
 if(reloq_german_voice_defaults(c,map,&cfg))return -1;
 c[0]=5;c[2]=0;c[7]=0;c[8]=60;c[19]=centre;
 c[39]=(kind==0||kind==4)?50:0;c[38]=kind==1?50:kind==4?24:0;c[43]=kind==2?40:0;
 if(rate==8000&&(kind==0||kind==4)){c[38]=50;c[39]=0;c[17]=3300;}
 if(kind==3){c[2]=60;c[8]=0;}
 if(kind==4)c[2]=60;
 struct reloq_core_voiced_frame f;
 if(reloq_frame_adapter_map(c,rate==8000?4:rate<16000?5:rate<32000?6:8,&f))return -2;
 f.noise.s_frication=(kind==0||kind==4)?profile:0;
 struct reloq_core_voiced_frame_state state;
 if(reloq_core_voiced_frame_initialize(&state,(float)rate,seed,4,6,0))return -3;
 if(rate>=16000){state.glottal_reference_rate=11025;state.frication_reference_rate=16000;state.reference_formants=5;}
 unsigned used=0;
 for(unsigned at=0;at<200;++at) {
  float part[512],af[512],scratch[512];unsigned char mask[512];size_t written=0;
  if(reloq_core_voiced_frame_render_owned(&state,&f,part,af,scratch,mask,0,512,&written))return -4;
  if(used+written>capacity)return -5;
  memcpy(out+used,part,written*sizeof *out);used+=(unsigned)written;
 }
 return (int)used;
}
