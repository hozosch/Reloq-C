#include "reloq_eci.h"
#include "reloq_frame_adapter.h"
#include "reloq_session.h"
#include "reloq_character.h"
#include "reloq_german_best_effort_text.h"
#include "reloq61.h"
#include "reloq_inline_tags.h"
#include <stdlib.h>
#include <stdio.h>

#include <stdint.h>
#include <string.h>

#define CHECK(test) do { if (!(test)) return __LINE__; } while (0)

struct capture {
    ECIHand engine;
    int16_t pcm[512];
    size_t samples;
    int calls;
    int stop_on_call;
    int reject_on_call;
    int saw_speaking;
};

static int ECICALL callback(ECIHand engine, int message, int count, void *data)
{
    struct capture *c = data;
    if (engine != c->engine || message != RELOQ_ECI_WAVEFORM_BUFFER ||
        count < 1 || count > 7 ||
        (size_t)count > 512 - c->samples) return 2;
    ++c->calls;
    if (eciSpeaking(engine)) c->saw_speaking = 1;
    if (c->reject_on_call == c->calls) return 0;
    /* The ECI buffer belongs to the caller and stays stable for this call. */
    extern short eci_test_output[7];
    memcpy(c->pcm + c->samples, eci_test_output,
           (size_t)count * sizeof *c->pcm);
    c->samples += (size_t)count;
    if (c->stop_on_call == c->calls) eciStop(engine);
    return 1;
}

short eci_test_output[7];

struct voice_capture {
    ECIHand engine;
    size_t samples;
    int calls, voiced_samples, index_count;
    int indices[8];
    size_t index_positions[8];
    int stop_on_call, reject_index, reject_index_value;
    int stop_on_index_value;
    int saw_speaking;
    int16_t *pcm;
    size_t pcm_capacity;
};

static int ECICALL voice_callback(ECIHand engine, int message, int param,
                                   void *data)
{
    struct voice_capture *c = data;
    if (engine != c->engine) return 2;
    if (eciSpeaking(engine)) c->saw_speaking = 1;
    if (message == RELOQ_ECI_INDEX_REPLY) {
        if (c->index_count == 8) return 2;
        c->indices[c->index_count] = param;
        c->index_positions[c->index_count++] = c->samples;
        if (c->stop_on_index_value == param) eciStop(engine);
        return c->reject_index || (c->reject_index_value &&
                                    param == c->reject_index_value) ? 2 : 1;
    }
    if (message != RELOQ_ECI_WAVEFORM_BUFFER ||
        param < 1 || param > 7) return 2;
    ++c->calls;
    if (c->pcm) {
        if ((size_t)param > c->pcm_capacity - c->samples) return 2;
        memcpy(c->pcm + c->samples, eci_test_output,
               (size_t)param * sizeof *c->pcm);
    }
    for (int i = 0; i < param; ++i)
        c->voiced_samples += eci_test_output[i] != 0;
    c->samples += (size_t)param;
    if (c->stop_on_call == c->calls) eciStop(engine);
    return 1;
}

static int direct_pcm(void *context, const int16_t *pcm, size_t samples)
{
    struct capture *c = context;
    if (samples > 512 - c->samples) return -1;
    memcpy(c->pcm + c->samples, pcm, samples * sizeof *pcm);
    c->samples += samples;
    return 0;
}

static void defaults(float frame[62])
{
    memset(frame, 0, 62 * sizeof *frame);
    frame[0] = 2.5f;
    frame[1] = 1200;
    frame[2] = 50;
    frame[3] = 50;
    frame[4] = 7;
    const size_t centers[8] = {9, 13, 15, 17, 19, 21, 23, 25};
    for (size_t i = 0; i < 8; ++i) {
        frame[centers[i]] = (float)(500 + i * 400);
        frame[centers[i] + 1] = (float)(70 + i * 10);
        frame[35 + i] = (float)(25 - i);
        frame[44 + i] = (float)(80 + i * 5);
    }
    const float nasal[8] = {260, 75, 360, 90, 2100, 140, 2000, 150};
    memcpy(frame + 27, nasal, sizeof nasal);
    frame[43] = 35.5f;
}

static size_t accent_signature(const unsigned char *text,size_t length,unsigned char out[128])
{
    size_t words=0;unsigned char pending=0;
    for(size_t at=0;at<length;) {
        struct reloq_inline_tag tag=reloq_inline_tag_read(text+at,length-at,1);
        if(tag.kind==RELOQ_TAG_PROMINENCE)
            pending=(unsigned char)(tag.value+1+(tag.length==3&&text[at+1]=='0'?10:0));
        if(tag.kind==RELOQ_TAG_SPR) {
            if(words==128)return SIZE_MAX;
            out[words++]=pending;pending=0;
        }
        at+=tag.length?tag.length:1;
    }
    return words;
}
struct phoneme_output {char *data;size_t length,capacity,calls;};
static int collect_phonemes(void *context,const char *data,size_t length)
{
    struct phoneme_output *out=context;
    if(length>SIZE_MAX-out->length-1)return -1;
    size_t need=out->length+length+1;
    if(need>out->capacity) {
        size_t capacity=out->capacity?out->capacity:256;
        while(capacity<need){if(capacity>SIZE_MAX/2)return -1;capacity*=2;}
        void *next=realloc(out->data,capacity);if(!next)return -1;
        out->data=next;out->capacity=capacity;
    }
    memcpy(out->data+out->length,data,length);out->length+=length;
    out->data[out->length]=0;++out->calls;return 0;
}
static int reject_phonemes(void *context,const char *data,size_t length)
{(void)context;(void)data;(void)length;return 1;}
static int native_phoneme_stream(void)
{
    static const int languages[]={0x40000,0x10000,0x10001,0x20000,0x20001,
        0x30000,0x30001,0x50000,0x70000,0x90000,0x60000,0x80000,0xa0000,0x120000};
    char *fixed=malloc(1048576);CHECK(fixed);
    for(size_t i=0;i<sizeof languages/sizeof *languages;++i) {
        struct reloq61_request q={.language=languages[i],.annotations=1,.phonemes_only=1,
            .voice={0,50,65,30,0,0,50,92},.text="NVDA 504. test, 2 abc?",
            .phonemes=fixed,.phonemes_capacity=1048576};
        struct reloq61_result old,result;CHECK(!reloq61_generate(&q,&old));
        struct phoneme_output out={0};char untouched[8]="keep";
        q.phonemes=untouched;q.phonemes_capacity=sizeof untouched;
        CHECK(!reloq61_generate_phonemes(&q,&result,collect_phonemes,&out));
        CHECK(!strcmp(untouched,"keep")&&out.calls&&result.phonemes_length==old.phonemes_length);
        CHECK(out.length==old.phonemes_length&&!memcmp(out.data,fixed,out.length));
        free(out.data);
        CHECK(reloq61_generate_phonemes(&q,&result,reject_phonemes,NULL)==-1&&result.error==1);
        q.phonemes=fixed;q.phonemes_capacity=1048576;
        CHECK(!reloq61_generate(&q,&result)&&result.phonemes_length==old.phonemes_length);
    }
    char long_text[10241];
    for(size_t i=0;i<2048;++i)memcpy(long_text+5*i,"Wort ",5);
    long_text[10240]=0;
    struct reloq61_request q={.language=0x40000,.annotations=1,.phonemes_only=1,
        .voice={0,50,65,30,0,0,50,92},.text=long_text,
        .phonemes=fixed,.phonemes_capacity=1048576};
    struct reloq61_result old,result;CHECK(!reloq61_generate(&q,&old));
    struct phoneme_output out={0};
    CHECK(!reloq61_generate_phonemes(&q,&result,collect_phonemes,&out));
    CHECK(out.calls>1&&out.length==old.phonemes_length&&!memcmp(out.data,fixed,out.length));
    CHECK(out.capacity<strlen(long_text)*4);
    free(out.data);free(fixed);return 0;
}

static int native_german_analysis(void)
{
    static const char *const texts[]={
        "Das dauert 2 Minuten und 24 Sekunden.",
        "Das dauert zwei Minuten und vierundzwanzig Sekunden.",
        "Nach 24 Sekunden geht es weiter.",
        "Das dauert 3 Stunden und 12 Minuten.",
        "Das dauert 2 Tage und 3 Stunden.",
        "Der Weg dauert 5 Minuten und 30 Sekunden.",
        "Die Zeit liegt bei 12 Minuten und 48 Sekunden.",
        "Wir warten 2 Wochen und 3 Tage.",
        "Die Zeit ist 24 Sekunden und dann geht es weiter.",
        "Das ist u. s. w. gut.","Das ist d. h. gut.","Das ist z. Z. gut.",
        "Das ist z. B. gut.","Das ist d.h. gut.","Das ist usw. gut.",
        "Das ist 2 km gut.","Das ist 2 kg gut.","Das ist 2 min gut.",
        "Das ist Dr. Meier.","Das ist Prof. Meier.","Das ist Nr. 2.",
        "NVDA-Einstellungen: Sprachausgabe (Standard-Konfiguration)",
        "NVDA-Einstellungen: Sprachausgabe", "Sprachausgabe (Standard-Konfiguration)",
        "USB-Einstellungen: Sprachausgabe (Standard-Konfiguration)",
        "MIDI-Ausgabe: Sprachausgabe (Standard-Konfiguration)",
        "PDF-Vorschau: Sprachausgabe (Standard-Konfiguration)",
        "Eric Röder, \"„Sagt Euren Abgeordneten, dass das Nachschlagen von Namen keine Straftat ist”, ruft Quad9 Franzosen auf, zu handeln. „Das Internet muss offen und für jedermann frei zugänglich bleiben\" Zensur-Urteil: DNS-Dienst Quad9 verlässt Frankreich | heise online https://www.heise.de/news/Zensur-Urteil-DNS-Dienst-Quad9-verlaesst-Frankreich-11497622.html, 1 week ago",
        "Das ist https://example.org/Datei-42.html, bitte lesen.",
        "Bitte https://example.org/Frankreich-11497622.html lesen.",
        "www.example.org/Text-503.htm, danach lesen.",
        "http://example.net/DNS-Dienst-Quad9-123abc.html.",
        "Ein- und Ausgang sind hier."
    };
    for(int dictionary=0;dictionary<2;++dictionary)for(int prediction=0;prediction<2;++prediction)
        for(size_t i=0;i<sizeof texts/sizeof *texts;++i) {
            struct reloq61_text_options previous=reloq61_text_options_scope(
                (struct reloq61_text_options){dictionary,prediction});
            unsigned char *own=NULL;size_t length=0,approximate=0;
            char native[16384];unsigned char encoded[2048];size_t encoded_length=0;
            CHECK(!reloq_character_encode_text((const unsigned char *)texts[i],strlen(texts[i]),
                encoded,NULL,sizeof encoded,&encoded_length,NULL,NULL));
            struct reloq61_result result;
            struct reloq61_request request={.language=0x40000,.annotations=1,.phonemes_only=1,
                .voice={0,50,65,30,0,0,50,92},.text=(char *)encoded,.phonemes=native,.phonemes_capacity=sizeof native};
            int observed=reloq61_generate(&request,&result);
            int status=reloq_german_best_effort_annotate((const unsigned char *)texts[i],strlen(texts[i]),
                NULL,&own,&length,&approximate,NULL);
            reloq61_text_options_scope(previous);
            CHECK(!observed&&!status&&own);
            unsigned char a[128],b[128];
            size_t an=accent_signature(own,length,a),bn=accent_signature((unsigned char *)native,result.phonemes_length,b);
            if(an!=bn || an>=128 || memcmp(a,b,an)) {
                fprintf(stderr,"Native German alignment differs: da=%d pp=%d %s\nOwn: %.*s\nNative: %.*s\n",
                    dictionary,prediction,texts[i],(int)length,own,(int)result.phonemes_length,native);
                free(own);return __LINE__;
            }
            if(i==0)CHECK(strstr((char *)own,"`[.1fi.0rUnt.2TvAn.0TIX]"));
            if(strstr(texts[i],"Sprachausgabe"))
                CHECK(strstr((char *)own,"`[.1Sprax.2'aw's.2ga.0b@]"));
            free(own);
        }
    static const struct {const char *word,*spr;} compounds[]={
        {"Sprachauswahl.","`[.1Sprax.2'aw's.2val]"},
        {"Suchauswahl.","`[.1zux.2'aw's.2val]"},
        {"Sprachauswahlen.","`[.1Sprax.2'aw's.2va.0l@n]"},
        {"Suchauswahlen.","`[.1zux.2'aw's.2va.0l@n]"}
    };
    for(size_t i=0;i<sizeof compounds/sizeof *compounds;++i) {
        unsigned char *own=NULL;size_t length=0,approximate=0;
        CHECK(!reloq_german_best_effort_annotate((const unsigned char *)compounds[i].word,
            strlen(compounds[i].word),NULL,&own,&length,&approximate,NULL));
        CHECK(own&&strstr((char *)own,compounds[i].spr));free(own);
    }
    return 0;
}

static int phrase_prediction_frontend(void)
{
    static const unsigned char text[]="Das ist ein Test der eine l\303\244ngere Wortgruppe ohne Satzzeichen enth\303\244lt und danach noch viel mehr W\303\266rter hinzuf\303\274gt damit die automatische Satzvorhersage Grenzen finden kann.";
    struct reloq61_text_options previous=reloq61_text_options_scope((struct reloq61_text_options){0,1});
    unsigned char *annotation=NULL;size_t length=0,approximate=0;
    int status=reloq_german_best_effort_annotate(text,sizeof text-1,NULL,&annotation,&length,&approximate,NULL);
    reloq61_text_options_scope(previous);
    CHECK(!status&&annotation);
    size_t words=0,boundaries=0;
    for(size_t i=0;i<length;++i) {
        if(annotation[i]=='`'&&i+1<length&&annotation[i+1]=='[')++words;
        if(annotation[i]==','){CHECK(words==11);++boundaries;}
    }
    free(annotation);
    CHECK(words==25&&boundaries==1);
    struct reloq_character_reading name;
    CHECK(reloq_character_read_native(0xb0,0,&name)==1&&!strcmp(name.original_spr,"`[.1grat]"));
    CHECK(reloq_character_read_native('_',0,&name)==1&&!strcmp(name.original_spr,"`[.1Un.0tR.2StrIX]"));
    CHECK(reloq_character_read_native(0xc5,0,&name)==1&&strstr(name.original_spr,".1rIG.0@l"));
    return 0;
}

int main(void)
{
    int prediction=phrase_prediction_frontend();if(prediction)return prediction;
    int native=native_german_analysis();if(native)return native;
    int stream=native_phoneme_stream();if(stream)return stream;
    char version[20] = {0};
    eciVersion(version);
    CHECK(!strcmp(version, "0.2.196.0"));
    int languages = 0;
    CHECK(eciGetAvailableLanguages(NULL, &languages) == 0 &&
          languages == 0);
    unsigned int code = 123;
    languages = 1;
    CHECK(eciGetAvailableLanguages(&code, &languages) == 0 &&
          languages == 0 && code == 123);
    ECIHand english = eciNewEx(RELOQ_ECI_ENGLISH);
    CHECK(english != NULL);
    eciDelete(english);
    const int additional_languages[] = {
        0x10001, 0x20000, 0x20001, 0x30000, 0x30001,
        0x50000, 0x70000, 0x90000, 0x60000, 0x60001,
        0x80000, 0xa0000, 0x120000
    };
    for (size_t i = 0; i < sizeof additional_languages /
                                  sizeof additional_languages[0]; ++i) {
        ECIHand other = eciNewEx(additional_languages[i]);
        CHECK(other != NULL);
        eciDelete(other);
    }
    CHECK(eciNewEx(0xb0000) == NULL);
    ECIHand e = eciNewEx(RELOQ_ECI_GERMAN);
    CHECK(e && eciGetParam(e, RELOQ_ECI_SAMPLE_RATE) == 1);
    CHECK(eciGetParam(e, RELOQ_ECI_DICTIONARY) == 0);
    CHECK(eciGetParam(e, RELOQ_ECI_PHRASE_PREDICTION) == 0);
    CHECK(eciSetParam(e, RELOQ_ECI_DICTIONARY, 1) == 0);
    CHECK(eciSetParam(e, RELOQ_ECI_DICTIONARY, 2) == -1);
    CHECK(eciSetParam(e, RELOQ_ECI_DICTIONARY, 0) == 1);
    CHECK(eciSetParam(e, RELOQ_ECI_PHRASE_PREDICTION, 1) == 0);
    CHECK(eciSetParam(e, RELOQ_ECI_PHRASE_PREDICTION, -1) == -1);
    CHECK(eciSetParam(e, RELOQ_ECI_PHRASE_PREDICTION, 0) == 1);
    CHECK(eciSetParam(e, RELOQ_ECI_SYNTH_MODE, 1) == 0 &&
          eciGetParam(e, RELOQ_ECI_SYNTH_MODE) == 1);
    CHECK(eciSetParam(e, RELOQ_ECI_INPUT_TYPE, 1) == -1 &&
          eciGetParam(e, RELOQ_ECI_INPUT_TYPE) == 0);
    CHECK(eciSetParam(e, RELOQ_ECI_SAMPLE_RATE, 3) == 1 &&
          eciGetParam(e, RELOQ_ECI_SAMPLE_RATE) == 3);
    CHECK(eciSetParam(e, RELOQ_ECI_SAMPLE_RATE, 123) == -1 &&
          eciGetParam(e, RELOQ_ECI_SAMPLE_RATE) == 3);
    CHECK(eciSetParam(e, RELOQ_ECI_LANGUAGE_DIALECT, 0x10000) == -1);
    CHECK(eciAddText(e, "Maus") == 1 &&
          eciInsertIndex(e, 7) == 1);
    CHECK(eciClearInput(e) && eciSynthesize(e));
    /* No callback is registered yet; an empty queue is a genuine no-op. */
    CHECK(reloq_eci_last_error(e) == RELOQ_ECI_OK);
    CHECK(eciSynchronize(e) == 1 && eciSpeaking(e) == 0);

    float controls[3][62];
    for (size_t i = 0; i < 3; ++i) defaults(controls[i]);
    controls[1][9] = 600;
    controls[1][2] = 45;
    controls[2][2] = 0;
    controls[2][8] = 35;

    uint32_t completed = 123;
    CHECK(reloq_eci_render_controls(e, controls[0], 1, 8, &completed) ==
          RELOQ_ECI_OUTPUT && completed == 0);
    CHECK(!eciSetOutputBuffer(e, 0, eci_test_output));
    CHECK(eciSetOutputBuffer(e, 7, eci_test_output));
    struct capture actual = {.engine = e};
    eciRegisterCallback(e, callback, &actual);
    CHECK(reloq_eci_render_controls(e, &controls[0][0], 3, 8,
                                     &completed) == RELOQ_ECI_OK);
    CHECK(completed == 3 && actual.samples == 120 && actual.calls > 3 &&
          actual.saw_speaking && !eciSpeaking(e));

    /* Cross the native ECI buffer boundary and compare exact PCM against a
     * separate numeric session at the same rate and seed. */
    const struct reloq_session_config config = {
        .sample_rate = 16000, .duration_scale = 1, .output_gain = 1,
        .initial_seed = 100, .max_frame_samples = 4096
    };
    struct reloq_session *reference = reloq_session_create(&config);
    CHECK(reference);
    struct reloq_core_voiced_frame frames[3];
    for (size_t i = 0; i < 3; ++i)
        CHECK(!reloq_frame_adapter_map(controls[i], 8, &frames[i]));
    struct capture expected = {0};
    size_t rendered = 0;
    CHECK(reloq_session_render_frames(reference, frames, 3, direct_pcm,
                                      &expected, &rendered) ==
          RELOQ_SESSION_OK);
    CHECK(rendered == 3 && expected.samples == actual.samples &&
          memcmp(expected.pcm, actual.pcm,
                 actual.samples * sizeof *actual.pcm) == 0);
    reloq_session_destroy(reference);

    struct capture rejected = {.engine = e, .reject_on_call = 1};
    eciRegisterCallback(e, callback, &rejected);
    CHECK(reloq_eci_render_controls(e, controls[0], 1, 8, &completed) ==
          RELOQ_ECI_CALLBACK && !completed && !rejected.samples);
    CHECK(reloq_eci_last_error(e) == RELOQ_ECI_CALLBACK);
    CHECK(eciReset(e));

    struct capture stopped = {.engine = e, .stop_on_call = 1};
    eciRegisterCallback(e, callback, &stopped);
    CHECK(reloq_eci_render_controls(e, &controls[0][0], 3, 8,
                                     &completed) == RELOQ_ECI_CANCELLED);
    CHECK(completed == 0 && stopped.samples <= 7 && stopped.samples > 0);
    CHECK(eciReset(e));
    eciRegisterCallback(e, callback, &actual);
    CHECK(eciSetOutputBuffer(e, 0, NULL) &&
          reloq_eci_render_controls(e, controls[0], 1, 8, &completed) ==
              RELOQ_ECI_OUTPUT);
    CHECK(eciSetOutputBuffer(e, 7, eci_test_output));
    CHECK(eciSetParam(e, RELOQ_ECI_SAMPLE_RATE, 1) == 3);
    struct voice_capture voice = {.engine = e};
    eciRegisterCallback(e, voice_callback, &voice);

    /* Isolated German Hieb. is a covered h+i+p onset, planned and rendered
     * by the native C voice; PCM must precede the index placed after it. */
    CHECK(eciAddText(e, "Hieb.") && eciInsertIndex(e, 41) &&
          eciSynthesize(e) && eciSynchronize(e));
    CHECK(voice.calls > 0 && voice.samples > 0 && voice.voiced_samples > 0 &&
          voice.saw_speaking && voice.index_count == 1 &&
          voice.indices[0] == 41 &&
          voice.index_positions[0] == voice.samples);

    /* The newly composed initial /p/ class must reach the same public ECI
     * text/PCM path, with no private numeric fixture or WAV replay. */
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    CHECK(eciAddText(e, "Pack.") && eciInsertIndex(e, 45) &&
          eciSynthesize(e) && eciSynchronize(e));
    CHECK(voice.samples > 0 && voice.voiced_samples > 0 &&
          voice.index_count == 1 && voice.indices[0] == 45 &&
          voice.index_positions[0] == voice.samples);

    /* The compositional two-syllable C model is exposed through the same
     * ECI queue and PCM callback. Split AddText inside a syllable so that
     * speech cannot be synthesized before the complete SPR word arrives. */
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    CHECK(eciAddText(e, "`[.0pAp.") &&
          eciAddText(e, "1tIp].") && eciInsertIndex(e, 61) &&
          eciSynthesize(e) && eciSynchronize(e));
    CHECK(voice.samples == 11187 && voice.voiced_samples > 1000 &&
          voice.index_count == 1 && voice.indices[0] == 61 &&
          voice.index_positions[0] == voice.samples);

    /* Natural weak-schwa words use the new open-primary CV composer. A
     * chunk boundary inside the spelling cannot truncate the second
     * syllable, and EOF indices follow the complete schwa or nasal tail. */
    static const char *schwa_words[] = {"Pappe.", "Packen.", "Tappen."};
    static const size_t schwa_samples[] = {8789,9570,9691};
    for (size_t word=0;word<3;++word) {
        memset(&voice,0,sizeof voice);
        voice.engine=e;
        CHECK(eciAddText(e, schwa_words[word]) &&
              eciInsertIndex(e, 70+(int)word) &&
              eciSynthesize(e) && eciSynchronize(e));
        CHECK(voice.samples==schwa_samples[word] &&
              voice.voiced_samples>1000 &&
              voice.index_count==1 &&
              voice.indices[0]==70+(int)word &&
              voice.index_positions[0]==voice.samples);
    }
    memset(&voice,0,sizeof voice);
    voice.engine=e;
    CHECK(eciAddText(e,"Pap") && eciAddText(e,"pe.") &&
          eciInsertIndex(e,73) && eciSynthesize(e) &&
          eciSynchronize(e));
    CHECK(voice.samples==8789 && voice.index_count==1 &&
          voice.indices[0]==73 && voice.index_positions[0]==voice.samples);
    memset(&voice,0,sizeof voice);
    voice.engine=e;
    CHECK(eciAddText(e,"Pappe. Packen. Tappen.") &&
          eciInsertIndex(e,74) && eciSynthesize(e) &&
          eciSynchronize(e));
    CHECK(voice.samples==28050 && voice.voiced_samples>1000 &&
          voice.index_count==1 && voice.indices[0]==74 &&
          voice.index_positions[0]==voice.samples);

    /* The measured stressed CVC + reduced-schwa classes use the same
     * native ECI queue. Exercise onsetless /@/ and /t@n/ with a split SPR
     * callback and end index; no spelling-specific audio is involved. */
    static const char *closed_reduced[] = {
        "`[.1pAp.0@].", "`[.1pAk.0t@n]."
    };
    static const size_t closed_reduced_samples[] = {8679,10230};
    for (size_t form=0;form<2;++form) {
        memset(&voice,0,sizeof voice);
        voice.engine=e;
        CHECK(eciAddText(e,closed_reduced[form]) &&
              eciInsertIndex(e,75+(int)form) &&
              eciSynthesize(e) && eciSynchronize(e));
        CHECK(voice.samples==closed_reduced_samples[form] &&
              voice.index_count==1 && voice.indices[0]==75+(int)form &&
              voice.index_positions[0]==voice.samples);
    }
    memset(&voice,0,sizeof voice);
    voice.engine=e;
    CHECK(eciAddText(e,"`[.1pAk.0t") && eciAddText(e,"@n].") &&
          eciInsertIndex(e,77) && eciSynthesize(e) && eciSynchronize(e));
    CHECK(voice.samples==10230 && voice.index_count==1 &&
          voice.indices[0]==77 && voice.index_positions[0]==voice.samples);
    static const char *natural_past[] = {
        "Packte.", "Packten.", "Tippte.", "Kippte."
    };
    static const size_t past_samples[] = {9592,10230,9944,9757};
    for (size_t form=0;form<4;++form) {
        memset(&voice,0,sizeof voice);
        voice.engine=e;
        CHECK(eciAddText(e,natural_past[form]) &&
              eciInsertIndex(e,78+(int)form) &&
              eciSynthesize(e) && eciSynchronize(e));
        CHECK(voice.samples==past_samples[form] &&
              voice.voiced_samples>1000 && voice.index_count==1 &&
              voice.indices[0]==78+(int)form &&
              voice.index_positions[0]==voice.samples);
    }
    memset(&voice,0,sizeof voice);
    voice.engine=e;
    CHECK(eciAddText(e,"Packte. Packten. Tippte. Kippte.") &&
          eciInsertIndex(e,82) && eciSynthesize(e) &&
          eciSynchronize(e));
    CHECK(voice.samples==39523 && voice.index_count==1 &&
          voice.indices[0]==82 && voice.index_positions[0]==voice.samples);

    /* The public reference treats an unmarked ECI text end like a period.
     * The natural nasal/voiced onset words must work as typed in NVDA, and
     * an EOF index follows the synthesized (implicit) final contour. */
    static int16_t marked_word[10000], unmarked_word[10000];
    static const char *words[] = {"Mett", "Bett"};
    static const size_t expected_samples[] = {8338, 8426};
    for (size_t word = 0; word < 2; ++word) {
        char marked[16];
        size_t length = strlen(words[word]);
        CHECK(length + 2 <= sizeof marked);
        memcpy(marked, words[word], length);
        marked[length] = '.';
        marked[length + 1] = 0;
        memset(&voice, 0, sizeof voice);
        voice.engine = e;
        voice.pcm = marked_word;
        voice.pcm_capacity = sizeof marked_word / sizeof *marked_word;
        CHECK(eciAddText(e, marked) && eciSynthesize(e) &&
              voice.samples == expected_samples[word]);
        memset(&voice, 0, sizeof voice);
        voice.engine = e;
        voice.pcm = unmarked_word;
        voice.pcm_capacity = sizeof unmarked_word / sizeof *unmarked_word;
        CHECK(eciAddText(e, words[word]) && eciInsertIndex(e, 60 + (int)word) &&
              eciSynthesize(e) && eciSynchronize(e) &&
              voice.samples == expected_samples[word] &&
              voice.index_count == 1 &&
              voice.indices[0] == 60 + (int)word &&
              voice.index_positions[0] == voice.samples &&
              !memcmp(marked_word, unmarked_word,
                      voice.samples * sizeof *marked_word));
    }

    /* The ECI text path forwards the observed final-mark contour for an
     * s/f coda, even when the caller splits a word across addText calls. */
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    CHECK(eciAddText(e, "Ha") && eciAddText(e, "ss?") &&
          eciInsertIndex(e, 46) && eciSynthesize(e));
    CHECK(voice.samples > 0 && voice.voiced_samples > 0 &&
          voice.index_count == 1 && voice.indices[0] == 46 &&
          voice.index_positions[0] == voice.samples);

    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    CHECK(eciAddText(e, "Hiss,") && eciInsertIndex(e, 47) &&
          eciSynthesize(e));
    CHECK(voice.samples > 0 && voice.voiced_samples > 0 &&
          voice.index_count == 1 && voice.indices[0] == 47 &&
          voice.index_positions[0] == voice.samples);

    /* A true question boundary keeps the Klatt session alive and places
     * an index after the first sentence, before the next audio begins. */
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    CHECK(eciAddText(e, "Hass?") && eciInsertIndex(e, 48) &&
          eciAddText(e, " Tipp.") && eciInsertIndex(e, 49) &&
          eciSynthesize(e));
    CHECK(voice.index_count == 2 && voice.indices[0] == 48 &&
          voice.indices[1] == 49 &&
          voice.index_positions[0] > 0 &&
          voice.index_positions[0] < voice.samples &&
          voice.index_positions[1] == voice.samples);

    /* Connected text shares one acoustic timeline, including its word
     * boundary. An ECI index after the whole phrase has an exact endpoint. */
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    CHECK(eciAddText(e, "Pack ") && eciAddText(e, "Tipp.") &&
          eciInsertIndex(e, 54) && eciSynthesize(e));
    CHECK(voice.samples == 11220 && voice.voiced_samples > 0 &&
          voice.index_count == 1 && voice.indices[0] == 54 &&
          voice.index_positions[0] == voice.samples);

    /* The connected pair's exact first-word acoustic boundary is 198
     * frames plus one PCM sample. Splitting an 11-sample source frame and
     * a 7-sample caller buffer must not change any generated PCM. */
    static int16_t connected_plain[20000], connected_indexed[20000];
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    voice.pcm = connected_plain;
    voice.pcm_capacity = sizeof connected_plain / sizeof *connected_plain;
    CHECK(eciAddText(e, "Pack Tipp.") && eciSynthesize(e) &&
          voice.samples == 11220);
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    voice.pcm = connected_indexed;
    voice.pcm_capacity = sizeof connected_indexed / sizeof *connected_indexed;
    CHECK(eciAddText(e, "Pack ") && eciInsertIndex(e, 55) &&
          eciAddText(e, "Tipp.") && eciInsertIndex(e, 56) &&
          eciSynthesize(e));
    CHECK(voice.samples == 11220 && voice.index_count == 2 &&
          voice.indices[0] == 55 && voice.indices[1] == 56 &&
          voice.index_positions[0] == 2179 &&
          voice.index_positions[1] == voice.samples &&
          !memcmp(connected_plain, connected_indexed,
                  voice.samples * sizeof *connected_plain));

    /* Both byte offsets around the single separating space resolve to the
     * same measured callback point, preserving queued index order. */
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    CHECK(eciAddText(e, "Pack") && eciInsertIndex(e, 57) &&
          eciAddText(e, " ") && eciInsertIndex(e, 58) &&
          eciAddText(e, "Tipp.") && eciSynthesize(e));
    CHECK(voice.samples == 11220 && voice.index_count == 2 &&
          voice.indices[0] == 57 && voice.indices[1] == 58 &&
          voice.index_positions[0] == 2179 &&
          voice.index_positions[1] == 2179);

    /* A different first word and a newly modeled k->p second onset use
     * their own composition landmarks, rather than a fixed timestamp. */
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    CHECK(eciAddText(e, "Hack ") && eciInsertIndex(e, 59) &&
          eciAddText(e, "Pack.") && eciSynthesize(e));
    CHECK(voice.samples == 11462 && voice.index_count == 1 &&
          voice.indices[0] == 59 && voice.index_positions[0] == 2443);

    /* The public reference has first-coda offsets of one, two or three
     * samples after the C schedule endpoint, depending on the verified
     * first-word class. Each pair retains its own complete PCM timeline. */
    static const struct {
        const char *first, *second;
        size_t samples, index_sample;
    } word_indices[] = {
        {"Pek ", "Pak.", 12441, 2719},
        {"Tipp ", "Tipp.", 12122, 2972},
        {"Papp ", "Tipp.", 11858, 2709},
        {"App ", "Tipp.", 11242, 2093},
        {"App ", "Tab.", 11869, 2093}
    };
    for (size_t i = 0; i < sizeof word_indices / sizeof *word_indices;
         ++i) {
        memset(&voice, 0, sizeof voice);
        voice.engine = e;
        CHECK(eciAddText(e, word_indices[i].first) &&
              eciInsertIndex(e, 59) &&
              eciAddText(e, word_indices[i].second) &&
              eciSynthesize(e));
        CHECK(voice.samples == word_indices[i].samples &&
              voice.index_count == 1 && voice.indices[0] == 59 &&
              voice.index_positions[0] == word_indices[i].index_sample);
    }

    /* A missing later sentence or an unverified inner-word offset must
     * reject the complete queue before any callback. */
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    CHECK(eciAddText(e, "Pack ") && eciInsertIndex(e, 55) &&
          eciAddText(e, "Tipp. Maus.") && !eciSynthesize(e));
    CHECK(reloq_eci_last_error(e) == RELOQ_ECI_UNSUPPORTED_TEXT &&
          !voice.samples && !voice.index_count);
    CHECK(eciAddText(e, "Pa") && eciInsertIndex(e, 55) &&
          eciAddText(e, "ck Tipp.") && !eciSynthesize(e));
    CHECK(reloq_eci_last_error(e) == RELOQ_ECI_UNSUPPORTED_TEXT &&
          !voice.samples && !voice.index_count);
    CHECK(eciAddText(e, "Pack T") && eciInsertIndex(e, 55) &&
          eciAddText(e, "ipp.") && !eciSynthesize(e));
    CHECK(reloq_eci_last_error(e) == RELOQ_ECI_UNSUPPORTED_TEXT &&
          !voice.samples && !voice.index_count);
    CHECK(eciAddText(e, " `[.1pAk]") && eciInsertIndex(e, 55) &&
          eciAddText(e, " `2 `[.1tIp].") && !eciSynthesize(e));
    CHECK(reloq_eci_last_error(e) == RELOQ_ECI_UNSUPPORTED_TEXT &&
          !voice.samples && !voice.index_count);

    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    voice.reject_index_value = 60;
    CHECK(eciAddText(e, "Pack ") && eciInsertIndex(e, 60) &&
          eciAddText(e, "Tipp.") && !eciSynthesize(e));
    CHECK(reloq_eci_last_error(e) == RELOQ_ECI_CALLBACK &&
          voice.samples == 2179 && voice.index_count == 1 &&
          voice.index_positions[0] == 2179);
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    voice.stop_on_index_value = 60;
    CHECK(eciAddText(e, "Pack ") && eciInsertIndex(e, 60) &&
          eciAddText(e, "Tipp.") && !eciSynthesize(e));
    CHECK(reloq_eci_last_error(e) == RELOQ_ECI_CANCELLED &&
          voice.samples == 2179 && voice.index_count == 1 &&
          voice.index_positions[0] == 2179 && !eciSpeaking(e));
    CHECK(eciReset(e));
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    CHECK(eciAddText(e, "Pack ") && eciInsertIndex(e, 60) &&
          eciAddText(e, "Tipp.") && eciSynthesize(e));
    CHECK(voice.samples == 11220 && voice.index_count == 1 &&
          voice.index_positions[0] == 2179);

    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    CHECK(eciInsertIndex(e, 40) && eciAddText(e, "Hi") &&
          eciAddText(e, "eb.") && eciInsertIndex(e, 43) &&
          eciSynthesize(e));
    CHECK(voice.index_count == 2 && voice.indices[0] == 40 &&
          voice.indices[1] == 43 && voice.index_positions[0] == 0 &&
          voice.index_positions[1] == voice.samples && voice.samples > 0);

    /* The queue combines arbitrary text fragments, while sentence spans
     * provide the real PCM boundary for indices on either side of the space.
     * No Klatt reset or synthesized silence is inserted between sentences. */
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    CHECK(eciInsertIndex(e, 50) && eciAddText(e, "Pa") &&
          eciAddText(e, "ck.") && eciInsertIndex(e, 51) &&
          eciAddText(e, " ") && eciInsertIndex(e, 52) &&
          eciAddText(e, "Tip") && eciAddText(e, "p.") &&
          eciInsertIndex(e, 53) && eciSynthesize(e));
    CHECK(voice.index_count == 4 && voice.indices[0] == 50 &&
          voice.indices[1] == 51 && voice.indices[2] == 52 &&
          voice.indices[3] == 53 &&
          voice.index_positions[0] == 0 &&
          voice.index_positions[1] > 0 &&
          voice.index_positions[1] == voice.index_positions[2] &&
          voice.index_positions[2] < voice.samples &&
          voice.index_positions[3] == voice.samples);
    size_t pack_samples = voice.index_positions[1];

    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    voice.reject_index_value = 61;
    CHECK(eciAddText(e, "Pack.") && eciInsertIndex(e, 61) &&
          eciAddText(e, " Tipp.") && !eciSynthesize(e));
    CHECK(reloq_eci_last_error(e) == RELOQ_ECI_CALLBACK &&
          voice.samples == pack_samples && voice.index_count == 1 &&
          voice.indices[0] == 61);

    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    CHECK(eciAddText(e, "Hack. Pack. ") && eciInsertIndex(e, 62) &&
          eciAddText(e, "Tipp.") && eciInsertIndex(e, 63) &&
          eciSynthesize(e));
    CHECK(voice.index_count == 2 && voice.indices[0] == 62 &&
          voice.indices[1] == 63 &&
          voice.index_positions[0] > pack_samples &&
          voice.index_positions[0] < voice.samples &&
          voice.index_positions[1] == voice.samples);

    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    CHECK(eciAddText(e, "Hi") && eciInsertIndex(e, 42) &&
          eciAddText(e, "eb.") && !eciSynthesize(e));
    CHECK(reloq_eci_last_error(e) == RELOQ_ECI_UNSUPPORTED_TEXT &&
          !voice.samples && !voice.index_count);

    /* A later uncovered segment fails in preflight before the first PCM or
     * index; queued entries are consumed on failure. */
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    CHECK(eciAddText(e, "Pack. ") && eciInsertIndex(e, 42) &&
          eciAddText(e, "Maus.") && !eciSynthesize(e));
    CHECK(reloq_eci_last_error(e) == RELOQ_ECI_UNSUPPORTED_TEXT &&
          !voice.samples && !voice.index_count);
    CHECK(eciClearInput(e) && eciSynchronize(e));

    /* Queue growth has no fixed eight-item or short-text cutoff. All sixty
     * four supported spans are checked before the uncovered final span, so
     * even the first index/PCM callback must remain absent. */
    for (int i = 0; i < 64; ++i)
        CHECK(eciAddText(e, "Pack. ") && eciInsertIndex(e, i + 100));
    CHECK(eciAddText(e, "Maus.") && !eciSynthesize(e));
    CHECK(reloq_eci_last_error(e) == RELOQ_ECI_UNSUPPORTED_TEXT &&
          !voice.samples && !voice.index_count);
    CHECK(eciClearInput(e));

    /* Undefined Windows-1252 byte is rejected; valid upper-byte spelling is
     * decoded before coverage checking and does not leak partial PCM. */
    CHECK(!eciAddText(e, "\x81") &&
          reloq_eci_last_error(e) == RELOQ_ECI_INVALID);
    CHECK(eciClearInput(e));
    CHECK(eciAddText(e, "M\xe4rz.") && !eciSynthesize(e));
    CHECK(reloq_eci_last_error(e) == RELOQ_ECI_UNSUPPORTED_TEXT &&
          !voice.samples);

    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    voice.stop_on_call = 1;
    CHECK(eciAddText(e, "Hieb.") && eciInsertIndex(e, 43) &&
          !eciSynthesize(e));
    CHECK(reloq_eci_last_error(e) == RELOQ_ECI_CANCELLED &&
          voice.calls == 1 && !voice.index_count && !eciSpeaking(e));
    CHECK(eciReset(e));

    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    voice.reject_index = 1;
    CHECK(eciInsertIndex(e, 44) && !eciSynthesize(e));
    CHECK(reloq_eci_last_error(e) == RELOQ_ECI_CALLBACK &&
          voice.index_count == 1);
    CHECK(eciReset(e));

    /* Numeric path retains rate agility while text remains 11,025 Hz only. */
    CHECK(eciSetParam(e, RELOQ_ECI_SAMPLE_RATE, 2) == 1);
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    CHECK(eciAddText(e, "Hieb.") && !eciSynthesize(e) &&
          reloq_eci_last_error(e) == RELOQ_ECI_UNSUPPORTED_TEXT &&
          !voice.samples);

    /* Generic text and indices share one utterance plan. A mark before a
     * word, on either side of its separator and at EOF must not change a
     * single synthesized sample or silently move the interior callbacks
     * to the end. ECI input remains byte-oriented Windows-1252. */
    CHECK(eciSetParam(e, RELOQ_ECI_SAMPLE_RATE, 1) == 2);
    CHECK(eciSetParam(e, RELOQ_ECI_VOICE_MODE,
                      RELOQ_ECI_VOICE_GENERIC) == RELOQ_ECI_VOICE_EXACT);
    /* Ordinary document symbols used to reject the whole phrase before
     * its first PCM callback. Confirm native CP1252 admission and recovery
     * on the same ECI handle, not only annotation generation in isolation. */
    static const char *const symbol_sentences[] = {
        "Hallo \xa7 1.", "Das Bild ist \xa9 2026.",
        "Die Temperatur ist 21 \xb0" "C.",
        "Die Formel ist 3 \xd7 4.",
        "Die Formel enth\xe4lt x\xb2 und Schriftzeichen.",
        "Der Pfad C:\\Users\\Karl.",
        "Sein Name ist S\xf8ren.",
        "Was steht in {geschweiften} Klammern?"
    };
    for (size_t s = 0;
         s < sizeof symbol_sentences / sizeof *symbol_sentences; ++s) {
        memset(&voice, 0, sizeof voice);
        voice.engine = e;
        CHECK(eciAddText(e, symbol_sentences[s]) && eciSynthesize(e) &&
              eciSynchronize(e) && voice.samples > 3000 &&
              voice.voiced_samples > 200 &&
              reloq_eci_last_error(e) == RELOQ_ECI_OK);
    }
    /* A productive compound whose two members are analyzable must not
     * reject a complete ECI utterance. Check it alone, between ordinary
     * words, with an end index, and then reuse the very same ECI handle. */
    static const char *const compound_sentences[] = {
        "Tonqualit\xe4t", "Vorher Tonqualit\xe4t Nachher.",
        "Klangqualit\xe4t und Bildqualit\xe4t.",
        "Die Lebensqualit\xe4t steigt."
    };
    for (size_t s = 0;
         s < sizeof compound_sentences / sizeof *compound_sentences; ++s) {
        memset(&voice, 0, sizeof voice);
        voice.engine = e;
        CHECK(eciAddText(e, compound_sentences[s]) &&
              eciInsertIndex(e, 87) && eciSynthesize(e) &&
              eciSynchronize(e) && voice.samples > 3000 &&
              voice.voiced_samples > 200 && voice.index_count == 1 &&
              voice.indices[0] == 87 &&
              voice.index_positions[0] == voice.samples &&
              reloq_eci_last_error(e) == RELOQ_ECI_OK);
    }
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    CHECK(eciAddText(e, "Mett.") && eciSynthesize(e) &&
          voice.samples > 3000 && voice.voiced_samples > 200 &&
          reloq_eci_last_error(e) == RELOQ_ECI_OK);
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    CHECK(eciAddText(e, "Hallo \xa7 ") && eciInsertIndex(e, 89) &&
          eciAddText(e, "1.") && eciSynthesize(e) &&
          voice.index_count == 1 && voice.indices[0] == 89 &&
          voice.index_positions[0] > 0 &&
          voice.index_positions[0] < voice.samples);
    static int16_t generic_plain[70000], generic_indexed[70000];
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    voice.pcm = generic_plain;
    voice.pcm_capacity = sizeof generic_plain / sizeof *generic_plain;
    CHECK(eciAddText(e, "F\xfcnf Pakete.") && eciSynthesize(e));
    size_t generic_samples = voice.samples;
    CHECK(generic_samples > 1000 && voice.index_count == 0);
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    voice.pcm = generic_indexed;
    voice.pcm_capacity = sizeof generic_indexed / sizeof *generic_indexed;
    CHECK(eciInsertIndex(e, 90) && eciAddText(e, "F\xfcnf") &&
          eciInsertIndex(e, 91) && eciAddText(e, " ") &&
          eciInsertIndex(e, 92) && eciAddText(e, "Pakete.") &&
          eciInsertIndex(e, 93) && eciSynthesize(e));
    CHECK(voice.samples == generic_samples && voice.index_count == 4 &&
          voice.indices[0] == 90 && voice.indices[1] == 91 &&
          voice.indices[2] == 92 && voice.indices[3] == 93 &&
          voice.index_positions[0] == 0 &&
          voice.index_positions[1] > 0 &&
          voice.index_positions[1] == voice.index_positions[2] &&
          voice.index_positions[2] < generic_samples &&
          voice.index_positions[3] == generic_samples &&
          !memcmp(generic_plain, generic_indexed,
                  generic_samples * sizeof *generic_plain));
    size_t generic_boundary = voice.index_positions[1];

    /* A native pitch mark changes F0 within the same joined sentence.
     * It must not add a synthesis job, move acoustic word indexes, change
     * punctuation pauses or persist past the current ECI queue. */
    CHECK(eciSetParam(e, RELOQ_ECI_NATIVE_PITCH, 50) == 65);
    static int16_t pitch_plain[70000], pitch_marked[70000],
                   pitch_indexed[70000], pitch_repeat[70000];
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    voice.pcm = pitch_plain;
    voice.pcm_capacity = sizeof pitch_plain / sizeof *pitch_plain;
    CHECK(eciInsertIndex(e, 120) && eciAddText(e, "Hieb ") &&
          eciInsertIndex(e, 121) && eciAddText(e, "Hieb ") &&
          eciInsertIndex(e, 122) && eciAddText(e, "Hieb.") &&
          eciInsertIndex(e, 123) && eciSynthesize(e));
    size_t pitch_samples = voice.samples;
    size_t pitch_first = voice.index_positions[1];
    size_t pitch_third = voice.index_positions[2];
    CHECK(pitch_samples > 1000 && voice.index_count == 4 &&
          pitch_first > 0 && pitch_third > pitch_first &&
          pitch_third < pitch_samples);

    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    voice.pcm = pitch_marked;
    voice.pcm_capacity = sizeof pitch_marked / sizeof *pitch_marked;
    CHECK(eciInsertIndex(e, 120) && eciAddText(e, "Hieb ") &&
          eciInsertPitch(e, 85) && eciInsertIndex(e, 121) &&
          eciAddText(e, "Hieb ") && eciInsertPitch(e, 50) &&
          eciInsertIndex(e, 122) && eciAddText(e, "Hieb.") &&
          eciInsertIndex(e, 123) && eciSynthesize(e));
    CHECK(voice.samples == pitch_samples && voice.index_count == 4 &&
          voice.index_positions[0] == 0 &&
          voice.index_positions[1] == pitch_first &&
          voice.index_positions[2] == pitch_third &&
          voice.index_positions[3] == pitch_samples &&
          memcmp(pitch_plain + pitch_first, pitch_marked + pitch_first,
                 (pitch_third - pitch_first) * sizeof *pitch_plain) != 0);
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    voice.pcm = pitch_indexed;
    voice.pcm_capacity = sizeof pitch_indexed / sizeof *pitch_indexed;
    CHECK(eciAddText(e, "Hieb ") && eciInsertPitch(e, 85) &&
          eciAddText(e, "Hieb ") && eciInsertPitch(e, 50) &&
          eciAddText(e, "Hieb.") && eciSynthesize(e));
    CHECK(voice.samples == pitch_samples && !voice.index_count &&
          !memcmp(pitch_marked, pitch_indexed,
                  pitch_samples * sizeof *pitch_marked));
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    voice.pcm = pitch_repeat;
    voice.pcm_capacity = sizeof pitch_repeat / sizeof *pitch_repeat;
    CHECK(eciAddText(e, "Hieb Hieb Hieb.") && eciSynthesize(e) &&
          voice.samples == pitch_samples &&
          !memcmp(pitch_plain, pitch_repeat,
                  pitch_samples * sizeof *pitch_plain));
    CHECK(eciGetParam(e, RELOQ_ECI_NATIVE_PITCH) == 50);
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    voice.pcm = pitch_repeat;
    voice.pcm_capacity = sizeof pitch_repeat / sizeof *pitch_repeat;
    CHECK(eciInsertPitch(e, 85) && eciInsertPitch(e, 50) &&
          eciAddText(e, "Hieb Hieb Hieb.") && eciInsertPitch(e, 90) &&
          eciSynthesize(e) && voice.samples == pitch_samples &&
          !memcmp(pitch_plain, pitch_repeat,
                  pitch_samples * sizeof *pitch_plain));
    CHECK(eciInsertPitch(e, 85) && eciClearInput(e));
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    voice.pcm = pitch_repeat;
    voice.pcm_capacity = sizeof pitch_repeat / sizeof *pitch_repeat;
    CHECK(eciAddText(e, "Hieb Hieb Hieb.") && eciSynthesize(e) &&
          voice.samples == pitch_samples &&
          !memcmp(pitch_plain, pitch_repeat,
                  pitch_samples * sizeof *pitch_plain));

    /* Stop can arrive after text was queued but before synthesis starts.
     * It signals an active renderer; ClearInput owns disposal of that queue,
     * including temporary pitch marks, before a later utterance is added. */
    CHECK(eciAddText(e, "Hieb ") && eciInsertPitch(e, 90) &&
          eciStop(e) && eciClearInput(e));
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    voice.pcm = pitch_repeat;
    voice.pcm_capacity = sizeof pitch_repeat / sizeof *pitch_repeat;
    CHECK(eciAddText(e, "Hieb Hieb Hieb.") && eciSynthesize(e) &&
          voice.samples == pitch_samples &&
          !memcmp(pitch_plain, pitch_repeat,
                  pitch_samples * sizeof *pitch_plain));

    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    voice.stop_on_call = 1;
    CHECK(eciAddText(e, "Hieb ") && eciInsertPitch(e, 90) &&
          eciAddText(e, "Hieb.") && !eciSynthesize(e) &&
          reloq_eci_last_error(e) == RELOQ_ECI_CANCELLED);
    CHECK(eciReset(e));
    CHECK(eciSetParam(e, RELOQ_ECI_NATIVE_PITCH, 50) == 50);
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    voice.pcm = pitch_repeat;
    voice.pcm_capacity = sizeof pitch_repeat / sizeof *pitch_repeat;
    CHECK(eciAddText(e, "Hieb Hieb Hieb.") && eciSynthesize(e) &&
          voice.samples == pitch_samples &&
          !memcmp(pitch_plain, pitch_repeat,
                  pitch_samples * sizeof *pitch_plain));

    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    voice.pcm = pitch_plain;
    voice.pcm_capacity = sizeof pitch_plain / sizeof *pitch_plain;
    CHECK(eciAddText(e, "Hieb, Hieb.") && eciSynthesize(e));
    size_t comma_samples = voice.samples;
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    voice.pcm = pitch_marked;
    voice.pcm_capacity = sizeof pitch_marked / sizeof *pitch_marked;
    CHECK(eciAddText(e, "Hieb, ") && eciInsertPitch(e, 85) &&
          eciAddText(e, "Hieb.") && eciSynthesize(e) &&
          voice.samples == comma_samples &&
          memcmp(pitch_plain, pitch_marked,
                 comma_samples * sizeof *pitch_plain));
    CHECK(!eciInsertPitch(e, 101) &&
          reloq_eci_last_error(e) == RELOQ_ECI_INVALID);
    CHECK(eciClearInput(e));

    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    voice.reject_index_value = 91;
    CHECK(eciAddText(e, "F\xfcnf") && eciInsertIndex(e, 91) &&
          eciAddText(e, " Pakete.") && !eciSynthesize(e));
    CHECK(reloq_eci_last_error(e) == RELOQ_ECI_CALLBACK &&
          voice.samples == generic_boundary && voice.index_count == 1 &&
          voice.index_positions[0] == generic_boundary);
    CHECK(eciReset(e));
    memset(&voice, 0, sizeof voice);
    voice.engine = e;
    voice.stop_on_index_value = 91;
    CHECK(eciAddText(e, "F\xfcnf") && eciInsertIndex(e, 91) &&
          eciAddText(e, " Pakete.") && !eciSynthesize(e));
    CHECK(reloq_eci_last_error(e) == RELOQ_ECI_CANCELLED &&
          voice.samples == generic_boundary && voice.index_count == 1 &&
          voice.index_positions[0] == generic_boundary);
    CHECK(eciDelete(e) == NULL);
    return 0;
}
