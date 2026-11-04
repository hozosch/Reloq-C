#define main other_backend_tests
#include "test_backend.c"
#undef main
int main(int argc,char **argv)
{
 assert(argc==2);sapi_test_library=argv[1];struct sapi_settings s=settings();size_t cases=0;
 for(size_t i=0;i<sizeof sapi_languages/sizeof *sapi_languages;++i)
 for(unsigned rateindex=0;rateindex<2;++rateindex){
  struct sapi_backend *b=sapi_backend_new(sapi_languages[i].dialect);assert(b);
  WAVEFORMATEX out=format(rateindex?48000:11025);SPVTEXTFRAG f=text(sapi_languages[i].sample,0);
  struct site reference=site_new();assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&f,&reference.iface)));
  size_t stops[]={64,512,(size_t)out.nSamplesPerSec/3};
  for(size_t j=0;j<sizeof stops/sizeof *stops;++j){
   if(reference.count<=stops[j])continue;
   for(int option=0;option<2;++option){
    SPVTEXTFRAG variation=f;
    if(option){variation.State.PitchAdj.MiddleAdj=3;variation.State.RateAdj=2;}
    struct site prior=site_new();prior.abort_after=stops[j];
    assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&variation,&prior.iface)));assert(prior.count>=stops[j]);free(prior.pcm);
    struct site recovered=site_new();assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&f,&recovered.iface)));
    if(recovered.count!=reference.count||memcmp(recovered.pcm,reference.pcm,reference.count*2)){
      fprintf(stderr,"PCM recovery mismatch lang=%ls rate=%u point=%zu variation=%d baseline=%zu got=%zu\n",sapi_languages[i].name,out.nSamplesPerSec,stops[j],option,reference.count,recovered.count);return 1;}
    if(recovered.event_count!=reference.event_count){fprintf(stderr,"event count mismatch: lang=%ls rate=%u point=%zu\n",sapi_languages[i].name,out.nSamplesPerSec,stops[j]);return 1;}
    for(size_t e=0;e<reference.event_count;++e){const SPEVENT *a=&reference.events[e],*z=&recovered.events[e];
     if(a->eEventId!=z->eEventId||a->wParam!=z->wParam||a->lParam!=z->lParam||a->ullAudioStreamOffset!=z->ullAudioStreamOffset){
      fprintf(stderr,"event mismatch lang=%ls rate=%u point=%zu event=%zu\n",sapi_languages[i].name,out.nSamplesPerSec,stops[j],e);return 1;}}
    free(recovered.pcm);++cases;
   }
  }
  free(reference.pcm);sapi_backend_delete(b);
 }
 printf("%zu cancel/recovery PCM and event cases passed\n",cases);return 0;
}
