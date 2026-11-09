#ifndef RELOQ_INLINE_WORD_CONTROLS_H
#define RELOQ_INLINE_WORD_CONTROLS_H

/* The Western/Greek frontends receive complete lexical words. A voice
 * command between letters belongs to the containing word, just as in the
 * German admission path. CJK keeps its native acoustic-word segmentation. */
static int word_voice_control(struct reloq_inline_tag t)
{
    return (t.kind>=RELOQ_TAG_PITCH&&t.kind<=RELOQ_TAG_PRESET&&t.kind!=RELOQ_TAG_PAUSE)||
        t.kind==RELOQ_TAG_WPM;
}

static size_t voice_word_letter(const unsigned char *p,size_t n,int unicode)
{
    if(!n)return 0;
    unsigned c=p[0];
    if((c>='a'&&c<='z')||(c>='A'&&c<='Z'))return 1;
    if(!unicode)return ((c>=0xc0&&c<=0xd6)||(c>=0xd8&&c<=0xf6)||c>=0xf8||
        c==0x8a||c==0x8c||c==0x8e||c==0x9a||c==0x9c||c==0x9e||c==0x9f)?1:0;
    size_t at=0;uint32_t cp;
    if(reloq_utf8_next(p,n,&at,&cp)!=1)return 0;
    return ((cp>=0xc0&&cp<=0x2af&&cp!=0xd7&&cp!=0xf7)||
        (cp>=0x300&&cp<=0x3ff)||(cp>=0x1f00&&cp<=0x1fff))?at:0;
}

static int normalize_word_voice_controls(unsigned char **input,size_t **source,
    size_t *length,int unicode)
{
    unsigned char *a=*input,*bytes=NULL;size_t *map=*source,*positions=NULL,n=*length;
    if(n>SIZE_MAX/sizeof *positions)return -1;
    int spelling=0;
    for(size_t at=0;at<n;) {
        struct reloq_inline_tag t=reloq_inline_tag_read(a+at,n-at,1);
        if(t.length){if(t.kind==RELOQ_TAG_SPELL)spelling=t.value;at+=t.length;continue;}
        size_t letter=voice_word_letter(a+at,n-at,unicode);
        if(spelling||!letter){++at;continue;}
        size_t start=at,end=at,controls=0;
        while(end<n) {
            letter=voice_word_letter(a+end,n-end,unicode);
            if(letter){end+=letter;continue;}
            size_t after=end;
            while(after<n) {
                t=reloq_inline_tag_read(a+after,n-after,1);
                if(!word_voice_control(t))break;
                after+=t.length;
            }
            if(after==end||!voice_word_letter(a+after,n-after,unicode))break;
            controls+=after-end;end=after;
        }
        if(controls) {
            if(!bytes) {
                bytes=malloc(n);positions=malloc(n*sizeof *positions);
                if(!bytes||!positions){free(bytes);free(positions);return -1;}
            }
            size_t command=0,word=controls;
            for(size_t i=start;i<end;) {
                t=reloq_inline_tag_read(a+i,end-i,1);size_t count=t.length?t.length:1;
                size_t out=t.length?command:word;
                memcpy(bytes+out,a+i,count);
                if(t.length) {
                    for(size_t j=0;j<count;++j)positions[out+j]=map[start];
                    command+=count;
                } else {positions[out]=map[i];word+=count;}
                i+=count;
            }
            memcpy(a+start,bytes,end-start);
            memcpy(map+start,positions,(end-start)*sizeof *map);
        }
        at=end;
    }
    free(bytes);free(positions);
    /* Give native preset parsing an explicit delimiter too. This avoids
     * interpreting the following Italian letters as part of a command.
     * Synthetic whitespace carries the command's original source offset. */
    size_t extra=0;
    for(size_t at=0;at<n;) {
        struct reloq_inline_tag t=reloq_inline_tag_read(a+at,n-at,1);
        size_t count=t.length?t.length:1;
        if(word_voice_control(t)&&voice_word_letter(a+at+count,n-at-count,unicode))++extra;
        at+=count;
    }
    if(!extra)return 0;
    if(n>SIZE_MAX-extra-1||(n+extra+1)>SIZE_MAX/sizeof *map)return -1;
    bytes=malloc(n+extra+1);positions=malloc((n+extra+1)*sizeof *positions);
    if(!bytes||!positions){free(bytes);free(positions);return -1;}
    size_t out=0;
    for(size_t at=0;at<n;) {
        struct reloq_inline_tag t=reloq_inline_tag_read(a+at,n-at,1);
        size_t count=t.length?t.length:1;
        memcpy(bytes+out,a+at,count);memcpy(positions+out,map+at,count*sizeof *map);out+=count;
        if(word_voice_control(t)&&voice_word_letter(a+at+count,n-at-count,unicode)) {
            bytes[out]=' ';positions[out++]=map[at];
        }
        at+=count;
    }
    bytes[out]=0;positions[out]=map[n];free(a);free(map);
    *input=bytes;*source=positions;*length=out;return 0;
}

/* In a phrase using native VWPM, native VS changes must reach the same
 * duration planner. Presets are still applied via word-scoped controls, so
 * project their speed into that planner at the preset's original position. */
static int native_preset_speed_tags(unsigned char **input,size_t **source,size_t *length)
{
    unsigned char *a=*input;size_t *map=*source,n=*length,count=0;
    for(size_t i=0;i<n;) {
        struct reloq_inline_tag t=reloq_inline_tag_read(a+i,n-i,1);
        if(t.kind==RELOQ_TAG_PRESET)++count;
        i+=t.length?t.length:1;
    }
    if(!count)return 0;
    if(count>(SIZE_MAX-n-1)/10 || n+count*10+1>SIZE_MAX/sizeof *map)return -1;
    unsigned char *bytes=malloc(n+count*10+1);
    size_t *positions=malloc((n+count*10+1)*sizeof *positions);
    if(!bytes||!positions){free(bytes);free(positions);return -1;}
    size_t out=0;
    for(size_t i=0;i<n;) {
        struct reloq_inline_tag t=reloq_inline_tag_read(a+i,n-i,1);
        size_t size=t.length?t.length:1;
        memcpy(bytes+out,a+i,size);memcpy(positions+out,map+i,size*sizeof *map);out+=size;
        if(t.kind==RELOQ_TAG_PRESET) {
            struct reloq_inline_word voice={0};reloq_inline_preset(&voice,t.value);
            char speed[10];int written=snprintf(speed,sizeof speed,"`vs%d ",voice.speed);
            if(written<0||(size_t)written>=sizeof speed){free(bytes);free(positions);return -1;}
            memcpy(bytes+out,speed,(size_t)written);
            for(int j=0;j<written;++j)positions[out++]=map[i];
        }
        i+=size;
    }
    bytes[out]=0;positions[out]=map[n];free(a);free(map);
    *input=bytes;*source=positions;*length=out;return 0;
}
#endif
