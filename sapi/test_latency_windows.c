/* Compare SAPI delivery to an in-memory stream; no audio device. */
#include "settings.h"
#include <stdio.h>
#include <stdint.h>
#include <wchar.h>
static double seconds(void){LARGE_INTEGER t,f;QueryPerformanceCounter(&t);QueryPerformanceFrequency(&f);return (double)t.QuadPart/(double)f.QuadPart;}
static HRESULT clear(IStream *p){LARGE_INTEGER a={0};ULARGE_INTEGER b={0};HRESULT r=IStream_Seek(p,a,STREAM_SEEK_SET,NULL);return SUCCEEDED(r)?IStream_SetSize(p,b):r;}
int wmain(int argc,wchar_t **argv)
{
 if(argc<2 || argc>4)return 2;
 const int xml=argc>=3 && !wcscmp(argv[2],L"xml");
 const int greek=argc==4 && !wcscmp(argv[3],L"el");
 setvbuf(stdout,NULL,_IONBF,0);
 HRESULT h=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(h))return 3;
 ISpVoice *voice=NULL;ISpObjectToken *token=NULL;ISpStream *out=NULL;IStream *mem=NULL;
 h=CoCreateInstance(&CLSID_SpVoice,NULL,CLSCTX_INPROC_SERVER,&IID_ISpVoice,(void **)&voice);
 if(SUCCEEDED(h))h=CoCreateInstance(&CLSID_SpObjectToken,NULL,CLSCTX_INPROC_SERVER,&IID_ISpObjectToken,(void **)&token);
 if(SUCCEEDED(h))h=ISpObjectToken_SetId(token,NULL,argv[1],FALSE);
 if(SUCCEEDED(h))h=ISpVoice_SetVoice(voice,token);
 if(SUCCEEDED(h))h=CreateStreamOnHGlobal(NULL,TRUE,&mem);
 if(SUCCEEDED(h))h=CoCreateInstance(&CLSID_SpStream,NULL,CLSCTX_INPROC_SERVER,&IID_ISpStream,(void **)&out);
 WAVEFORMATEX wf={WAVE_FORMAT_PCM,1,22050,44100,2,16,0};
 if(SUCCEEDED(h))h=ISpStream_SetBaseStream(out,mem,&SPDFID_WaveFormatEx,&wf);
 if(SUCCEEDED(h))h=ISpVoice_SetOutput(voice,(IUnknown *)out,FALSE);
 if(SUCCEEDED(h)){ULONGLONG interest=SPFEI(SPEI_START_INPUT_STREAM)|SPFEI(SPEI_END_INPUT_STREAM)|SPFEI(SPEI_TTS_BOOKMARK);h=ISpVoice_SetInterest(voice,interest,interest);}
 if(SUCCEEDED(h))h=ISpVoice_SetRate(voice,5);
 if(SUCCEEDED(h))h=ISpVoice_Speak(voice,greek?L"Μια δοκιμή για την προετοιμασία.":L"Eine kurze Ansage zur Vorbereitung.",SPF_IS_NOT_XML,NULL);
 for(int i=0;i<15&&SUCCEEDED(h);++i){
  h=clear(mem);double start=seconds(),first=0,play=0;ULONGLONG firstbytes=0;
  LPCWSTR message=greek?L"Μια εκτενής δοκιμαστική πρόταση για την καθυστέρηση της ελληνικής ομιλίας.":xml?L"<bookmark mark=\"1\"/>Ein Beispiel für eine kurze Ansage beim Navigieren durch NVDA. Weitere Wörter machen die Unterbrechung gut sichtbar.":L"Ein Beispiel für eine kurze Ansage beim Navigieren durch NVDA. Weitere Wörter machen die Unterbrechung gut sichtbar.";
  if(SUCCEEDED(h))h=ISpVoice_Speak(voice,message,SPF_ASYNC|(xml?SPF_IS_XML:SPF_IS_NOT_XML),NULL);
  while(SUCCEEDED(h)&&!play&&seconds()-start<2.0){STATSTG s={0};h=IStream_Stat(mem,&s,STATFLAG_NONAME);double elapsed=seconds()-start;
   if(s.cbSize.QuadPart&&!first){first=elapsed;firstbytes=s.cbSize.QuadPart;}
   if(s.cbSize.QuadPart>=2206)play=elapsed;
   if(!play)SwitchToThread();
  }
  if(!play)h=E_FAIL;
  if(SUCCEEDED(h))h=ISpVoice_Speak(voice,NULL,SPF_PURGEBEFORESPEAK,NULL);
  HGLOBAL global=NULL;STATSTG s={0};double onset=-1;
  if(SUCCEEDED(h))h=IStream_Stat(mem,&s,STATFLAG_NONAME);
  if(SUCCEEDED(h))h=GetHGlobalFromStream(mem,&global);
  if(SUCCEEDED(h)){short *p=GlobalLock(global);if(!p)h=E_FAIL;else {for(size_t j=0;j<(size_t)s.cbSize.QuadPart/2;++j)if(p[j]>64||p[j]<-64){onset=1000.0*j/22050;break;}GlobalUnlock(global);}}
  wprintf(L"iteration=%d first_ms=%.3f playable_ms=%.3f first_bytes=%llu onset_ms=%.3f result=%08lX\n",i,first*1000,play*1000,(unsigned long long)firstbytes,onset,(unsigned long)h);
 }
 if(voice)ISpVoice_Release(voice);if(out)ISpStream_Release(out);if(mem)IStream_Release(mem);if(token)ISpObjectToken_Release(token);
 CoUninitialize();return FAILED(h)?1:0;
}
