/* Sentence alignment, independent of PCM. Compare the final phones and
 * prominence with the complete native C frontend, not isolated word data. */
#include "reloq_german_best_effort_text.h"
#include "reloq_german_input.h"
#include "reloq_inline_tags.h"
#include "reloq61.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct text {unsigned char data[32768];size_t used;};
static int collect(void *context,const char *data,size_t length)
{
    struct text *out=context;assert(length<sizeof out->data-out->used);
    memcpy(out->data+out->used,data,length);out->used+=length;return 0;
}
static struct text signature(const unsigned char *s,size_t n)
{
    struct text out={0};
    for(size_t at=0;at<n;) {
        struct reloq_inline_tag t=reloq_inline_tag_read(s+at,n-at,1);
        if(t.kind==RELOQ_TAG_SPR||t.kind==RELOQ_TAG_PROMINENCE)
            assert(!collect(&out,(const char *)s+at,t.length));
        at+=t.length?t.length:1;
    }
    return out;
}
static unsigned check(const char *text)
{
    unsigned char encoded[4096];size_t encoded_n=0;
    assert(!reloq_german_input_encode((const unsigned char *)text,strlen(text),encoded,sizeof encoded,&encoded_n,NULL));
    struct text native={0};struct reloq61_request q={.language=0x40000,.annotations=1,.phonemes_only=1,
        .voice={0,50,65,30,0,0,50,92},.text=(const char *)encoded};
    struct reloq61_result r={0};assert(!reloq61_generate_phonemes(&q,&r,collect,&native));
    unsigned char *own=NULL;size_t length=0,approximate=0,bad=0,count=0;
    struct reloq_german_word_features *features=NULL;
    assert(!reloq_german_best_effort_plan_tags_checked((const unsigned char *)text,strlen(text),NULL,1,
        &own,&length,&approximate,&bad,NULL,0,&features,&count,NULL,NULL));
    struct text a=signature(own,length),b=signature(native.data,native.used);
    if(a.used!=b.used||memcmp(a.data,b.data,a.used)) {
        fprintf(stderr,"German context differs: %s\n",text);abort();
    }
    /* Every selected case is natural text, not supplied SPR. Loss of the
     * native facts here detects a silent whole-utterance fallback as well. */
    assert(count>0);free(own);free(features);return 1;
}
int main(void)
{
    void *worker=reloq61_worker_new(0x40000);assert(worker);void *previous=reloq61_worker_scope(worker);
    unsigned checks=0;char text[4096];
    static const char *const tokens[]={"NVDA-Einstellungen","12 kg","abc@example.com","file:///C:/Test"};
    static const char *const patterns[]={
        "Die %s wurde nach der Vorbereitung geprüft.",
        "„%s“ wurde isoliert richtig gelesen. Das Ergebnis bleibt unverändert.",
        "Vor der Prüfung (%s) wurde die Sprachausgabe gestartet.",
        "Der neue Eintrag: %s. Danach wurde die Datei geöffnet."};
    for(size_t i=0;i<sizeof tokens/sizeof *tokens;++i)
        for(size_t j=0;j<sizeof patterns/sizeof *patterns;++j) {
            assert(snprintf(text,sizeof text,patterns[j],tokens[i])>0);checks+=check(text);
        }
    static const char *const quotes[]={"“","”","\"","»","›","’",")","]","}"};
    static const char *const marks[]={".",",",":",";","?","!","...","?!"};
    for(size_t i=0;i<sizeof quotes/sizeof *quotes;++i)
        for(size_t j=0;j<sizeof marks/sizeof *marks;++j) {
            assert(snprintf(text,sizeof text,"Die Datei wurde gelesen%s%s Danach wurde das Ergebnis geprüft.",marks[j],quotes[i])>0);
            checks+=check(text);
        }
    checks+=check("Sie fragte: „Ist der Termin bestätigt?“");
    static const char *const separators[]={
        "Frankfurt/Main","Frankfurt/Oder","E-Mail","NVDA-Erweiterung",
        "Karl/wechseln","Stern/Neu starten","Sprach-/Satzanalyse","Ein-/Ausgabe",
        "und/oder","Audio-/Video-Dateien","S-/U-Bahn","Bindestrich / Schrägstrich",
        "Ein- und Ausgabe","Ein- / Ausgabe","Ein-/ Ausgabe","Ein- /Ausgabe",
        "Audio- / Video-Dateien","E-Mail/Chat","Windows-11-Updates","NVDA-2026.1",
        "16-kHz-Version","22/32-kHz-Version","Frankfurt / Main","Datei-/Ordnername",
        "A/B-Test","X/B-Test","Frankfurt/Main-Region","E-Mail/Chat-Verlauf",
    };
    static const char *const separator_patterns[]={
        "Die %s wurde nach der Vorbereitung geprüft.",
        "Heute wurde %s geprüft. Danach wurde das Ergebnis vorgelesen.",
        "Bei %s kann die Satzanalyse fehlschlagen.",
    };
    for(size_t i=0;i<sizeof separators/sizeof *separators;++i)
        for(size_t j=0;j<sizeof separator_patterns/sizeof *separator_patterns;++j) {
            assert(snprintf(text,sizeof text,separator_patterns[j],separators[i])>0);
            checks+=check(text);
        }
    reloq61_worker_scope(previous);reloq61_worker_free(worker);
    printf("{\"german_context_cases\":%u,\"native_phone_and_prominence_signatures_equal\":true,\"native_features_retained\":true,\"no_audio_files\":true}\n",checks);
    return 0;
}
