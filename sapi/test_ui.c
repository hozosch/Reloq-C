#include "settings.h"
#include "ui.inc"
#include <assert.h>
#include <stdio.h>
#include <wchar.h>
int main(void)
{
    unsigned checks=0;
    const unsigned ids[]={0x409,0x407,0xc0a,0x40c,0x410,0x416,0x40b,0x408,0x411,0x412,0x804,0x404};
    for(unsigned i=0;i<sizeof ids/sizeof *ids;++i){
        const struct sapi_ui *ui=sapi_ui_for_language(ids[i]);assert(ui==sapi_ui_languages+i);
        for(unsigned k=0;k<UI_COUNT;++k){assert(ui->text[k]&&*ui->text[k]);++checks;}
    }
    assert(sapi_ui_for_language(0x809)==sapi_ui_languages);
    assert(sapi_ui_for_language(0x419)==sapi_ui_languages);
    assert(sapi_ui_for_language(0xc04)==sapi_ui_for_language(0x404));
    assert(sapi_ui_for_language(0x1404)==sapi_ui_for_language(0x404));
    assert(sapi_ui_for_language(0x1004)==sapi_ui_for_language(0x804));
    printf("{\"localized_strings\":%u,\"ui_tables\":12,\"regional_fallbacks\":5}\n",checks);return 0;
}
