/* Main is a text-normalization volume. Resolve it here, before ordinary word
 * classification and sentence planning; Root remains a lexical pronunciation
 * table. A text alias must not freeze the pronunciation of an isolated word. */
struct main_text_edit {size_t begin,end,length;const unsigned char *value;};
static int main_word_byte(unsigned char c)
{return letter(loan_letter(c))||digit(c)||c=='_'||c=='\'';}
static int normalize_main_text(unsigned char **text,uint32_t **scalars,size_t *length,
    int process_tags,size_t **mapping,int (*cancelled)(void *),void *context)
{
    const unsigned char *source=*text;size_t n=*length,used=0,capacity=0,total=n;
    struct main_text_edit *edits=NULL;int spelling=0,status=-2;
    for(size_t at=0;at<n;) {
        if(cancelled&&cancelled(context)){status=2;goto fail;}
        if(process_tags&&source[at]=='`') {
            struct reloq_inline_tag tag=reloq_inline_tag_read(source+at,n-at,0);
            if(tag.kind) {
                if(tag.kind==RELOQ_TAG_SPELL)spelling=tag.value;
                at+=tag.length;continue;
            }
        }
        if(spelling||!main_word_byte(source[at])){++at;continue;}
        size_t end=at;
        while(end<n&&(main_word_byte(source[end])||
            (source[end]=='-'&&end+1<n&&main_word_byte(source[end+1]))))++end;
        size_t part=at;
        while(part<end) {
            size_t stop=end;
            const char *value=reloq61_dictionary_main_text(source+part,stop-part);
            if(!value) {
                stop=part;while(stop<end&&source[stop]!='-')++stop;
                value=reloq61_dictionary_main_text(source+part,stop-part);
            }
            if(value) {
                size_t len=strlen(value),bytes=0;
                unsigned char utf8[1537];
                if(len<=512&&!reloq_cp1252_decode((const unsigned char *)value,len,
                    utf8,sizeof utf8,&bytes,NULL)) {
                    if(used==capacity) {
                        size_t next=capacity?capacity*2:8;
                        if(next<capacity||next>SIZE_MAX/sizeof *edits)goto fail;
                        void *p=realloc(edits,next*sizeof *edits);if(!p)goto fail;
                        edits=p;capacity=next;
                    }
                    if(len>SIZE_MAX-(total-(stop-part)))goto fail;
                    total=total-(stop-part)+len;
                    edits[used++]=(struct main_text_edit){part,stop,len,(const unsigned char *)value};
                }
            }
            part=stop<end?stop+1:end;
        }
        at=end;
    }
    if(!used){free(edits);return 0;}
    if(total==SIZE_MAX||total>SIZE_MAX/sizeof **scalars-1||n>SIZE_MAX/sizeof(size_t)-1)goto fail;
    unsigned char *output=malloc(total+1);
    uint32_t *letters=malloc((total+1)*sizeof *letters);
    size_t *map=malloc((n+1)*sizeof *map);
    if(!output||!letters||!map){free(output);free(letters);free(map);goto fail;}
    size_t from=0,to=0;
    for(size_t i=0;i<=used;++i) {
        size_t stop=i<used?edits[i].begin:n;
        while(from<stop){map[from]=to;output[to]=source[from];letters[to++]=(*scalars)[from++];}
        map[from]=to;
        if(i==used)break;
        const struct main_text_edit *edit=edits+i;
        unsigned char utf8[1537];size_t bytes=0,offset=0;
        (void)reloq_cp1252_decode(edit->value,edit->length,utf8,sizeof utf8,&bytes,NULL);
        memcpy(output+to,edit->value,edit->length);
        while(offset<bytes){uint32_t cp;(void)reloq_utf8_next(utf8,bytes,&offset,&cp);letters[to++]=cp;}
        while(from<edit->end)map[++from]=to;
    }
    output[to]=0;letters[to]=0;map[n]=to;
    free(*text);free(*scalars);free(edits);
    *text=output;*scalars=letters;*length=to;*mapping=map;return 0;
fail:free(edits);return status;
}
