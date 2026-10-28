/* One romanizer instance, and what it forwards to.
 *
 * IBM's RomInstance is a COM-shaped object of 0x18 bytes holding two things:
 * a RomInstParam it makes with the path it was given, and a Romanizer it makes
 * with that RomInstParam. Every one of its thirty-one methods then forwards --
 * the parameter calls to the first, everything about text to the second -- and
 * that is the whole of the object. So this is that forwarding, over the table
 * in src/eci/lang/eci_rom.h rather than over a vtable found in a loaded library.
 *
 * romedll_link.obj is what answers the manager's question in a build where the
 * romanizer is part of the program rather than a DLL, and jp_rom_new is that:
 * the manager takes it from the table in src/eci/lang/eci_romedll.c and calls it for an
 * instance.
 */

#include <string.h>
#include "jprom.h"
#include "romanizer.h"

/* The romanizer proper sits inside the instance rather than beside it: the
   engine's own EvvRom is the head, and everything past it is the record
   rom/jajp/romanizer.h maps, with the pointers parked past its end. */
typedef struct JpRom {
    EvvRom       base;
    RomInstParam param;
    uint8_t      rom[RZ_ROOM];
} JpRom;

static void jp_release(EvvRom *r)
{
    JpRom *j = (JpRom *)r;

    jrz_dtor(j->rom);
    ci_closeBase(j->rom);
    rp_dtor(&j->param);
    cpp_delete(j);
}

static int32_t jp_addText(EvvRom *r, const char *text, int32_t len,
                          int32_t flag)
{
    JpRom *j=(JpRom *)r;
    /* The ReLoq host supplies UTF-8. Recode once into the romanizer's own
       native Shift-JIS alphabet, retaining its word and accent analysis. */
    if(len<0)return 0;
    uint16_t *wide=cpp_new(((uint32_t)len+1)*2);
    if(!wide)return 0;
    uint32_t count=0;int valid=1;
    for(int32_t i=0;i<len;) {
        unsigned c=(unsigned char)text[i++],need=c<128?0:c>=0xc2&&c<0xe0?1:c>=0xe0&&c<0xf0?2:c>=0xf0&&c<0xf5?3:99;
        if(need==99||need>(unsigned)(len-i)){valid=0;break;}
        uint32_t cp=need?c&((1u<<(6-need))-1):c;
        for(unsigned k=0;k<need;++k){unsigned b=(unsigned char)text[i++];if((b&0xc0)!=0x80)valid=0;cp=(cp<<6)|(b&63);}
        if(cp>=0x10000){wide[count++]=' ';continue;}
        wide[count++]=(uint16_t)cp;
    }
    wide[count]=0;
    if(!valid){cpp_delete(wide);return ci_addText(j->rom,text,len,flag);}
    /* InputManager retains the encoded text until processSentence reads it.
       Use the instance's converter so its buffer lives across that call. */
    char *mapped=NULL;
    int error=ci_UCS2ToMBCS(j->rom,wide,&mapped,0);
    int result=error?0:ci_addText(j->rom,mapped,(int32_t)strlen(mapped),flag);
    cpp_delete(wide);return result;
}

static int32_t jp_insertIndex(EvvRom *r)
{
    return ci_insertIndex(((JpRom *)r)->rom);
}

static int32_t jp_processSentence(EvvRom *r, char **out, int32_t annotated)
{
    return jrz_processSentence(((JpRom *)r)->rom, out, annotated);
}

static int32_t jp_stop(EvvRom *r)
{
    return ci_stop(((JpRom *)r)->rom);
}

static int32_t jp_resume(EvvRom *r)
{
    return ci_resume(((JpRom *)r)->rom);
}

static int32_t jp_UCS2ToMBCS(EvvRom *r, const uint16_t *in, char **out,
                             int32_t n)
{
    return ci_UCS2ToMBCS(((JpRom *)r)->rom, in, out, n);
}

static int32_t jp_setParam(EvvRom *r, int32_t which, int32_t value)
{
    return rp_setParam(&((JpRom *)r)->param, which, value);
}

static int32_t jp_getParam(EvvRom *r, int32_t which)
{
    return rp_getParam(&((JpRom *)r)->param, which);
}

static void jp_clearErrors(EvvRom *r)
{
    rp_clearErrors(&((JpRom *)r)->param);
}

static uint32_t jp_progStatus(EvvRom *r)
{
    return rp_getErrors(&((JpRom *)r)->param);
}

static void jp_errorMessage(EvvRom *r, char *out)
{
    rp_getErrorMessage(&((JpRom *)r)->param, out);
}

static int32_t jp_addParam(EvvRom *r, const char *text, int32_t len)
{
    return ci_addParam(((JpRom *)r)->rom, text, len);
}

/* ---- and the user dictionary ------------------------------------------ */

/* IBM's RomInstance forwards all of these to the ConverterInterface under it,
   and the two conversions through the converter's own table rather than by
   name. That is what these do. */

static void *jp_newDict(EvvRom *r)
{
    return ci_newDict(((JpRom *)r)->rom);
}

static void jp_deleteDict(EvvRom *r, void *dict)
{
    ci_deleteDict(((JpRom *)r)->rom, dict);
}

static void jp_setDict(EvvRom *r, void *dict)
{
    ci_setDict(((JpRom *)r)->rom, dict);
}

static int32_t jp_loadDict(EvvRom *r, void *dict, int32_t which,
                           const char *name)
{
    return ci_loadDict(((JpRom *)r)->rom, dict, which, name);
}

static int32_t jp_saveDict(EvvRom *r, void *dict, int32_t which,
                           const char *name)
{
    return ci_saveDict(((JpRom *)r)->rom, dict, which, name);
}

static int32_t jp_lookupDictExt(EvvRom *r, void *dict, int32_t which,
                                void *word, int32_t wordLen, void **value,
                                int32_t *valueLen, int32_t *pos,
                                int32_t codeset)
{
    return ci_lookupDictExt(((JpRom *)r)->rom, dict, which, (uint8_t *)word,
                            wordLen, value, valueLen, pos, codeset);
}

static int32_t jp_updateDictExt(EvvRom *r, void *dict, int32_t which,
                                void *word, int32_t wordLen, void *value,
                                int32_t valueLen, int32_t pos,
                                int32_t codeset)
{
    return ci_updateDictExt(((JpRom *)r)->rom, dict, which, (uint8_t *)word,
                            wordLen, (char *)value, valueLen, pos, codeset);
}

static int32_t jp_findFirstDictExt(EvvRom *r, void *dict, int32_t which,
                                   void **key, int32_t *keyLen, void **value,
                                   int32_t *valueLen, int32_t *pos,
                                   int32_t codeset)
{
    return ci_findFirstDictEntryExt(((JpRom *)r)->rom, dict, which, key,
                                    keyLen, value, valueLen, pos, codeset);
}

static int32_t jp_findNextDictExt(EvvRom *r, void *dict, int32_t which,
                                  void **key, int32_t *keyLen, void **value,
                                  int32_t *valueLen, int32_t *pos,
                                  int32_t codeset)
{
    return ci_findNextDictEntryExt(((JpRom *)r)->rom, dict, which, key,
                                   keyLen, value, valueLen, pos, codeset);
}

static int32_t jp_mbcs2Rom(EvvRom *r, const char *in, char **out)
{
    void *c = ((JpRom *)r)->rom;

    return CI_VT(c)->mbcs2Rom((Converter *)c, in, out);
}

static int32_t jp_rom2Mbcs(EvvRom *r, const char *in, char **out)
{
    void *c = ((JpRom *)r)->rom;

    return CI_VT(c)->rom2Mbcs((Converter *)c, in, out);
}

static const EvvRomOps JP_OPS = {
    jp_release,
    jp_addText,
    jp_insertIndex,
    jp_processSentence,
    jp_stop,
    jp_resume,
    jp_UCS2ToMBCS,
    jp_setParam,
    jp_getParam,
    jp_clearErrors,
    jp_progStatus,
    jp_errorMessage,
    jp_addParam,
    jp_newDict,
    jp_deleteDict,
    jp_setDict,
    jp_loadDict,
    jp_saveDict,
    jp_lookupDictExt,
    jp_updateDictExt,
    jp_findFirstDictExt,
    jp_findNextDictExt,
    jp_mbcs2Rom,
    jp_rom2Mbcs,
};

/* What the manager calls for an instance. The original checks a licence
   first, makes the object, and asks it for the one interface the manager
   wants; if that comes back empty the object is thrown away again. */
EvvRom *jp_rom_new(const char *dir)
{
    JpRom *j = (JpRom *)cpp_new((uint32_t)sizeof *j);

    if (j == 0)
        return 0;
    memset(j, 0, sizeof *j);
    j->base.ops = &JP_OPS;
    rp_ctor(&j->param, dir);
    if (rp_getErrors(&j->param) & ROM_ERR_MEMORY) {
        rp_dtor(&j->param);
        cpp_delete(j);
        return 0;
    }
    jrz_ctor(j->rom, &j->param);
    if (rp_getErrors(&j->param) & ROM_ERR_MEMORY) {
        jp_release(&j->base);
        return 0;
    }
    return &j->base;
}
