/* Exhaustive fixed-layout reads, including unaligned record bases. */
#include "delta.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "test_reloq_field_offsets_expected.h"

extern delta_stmt enus61_vstmtbl[],dede61_vstmtbl[];

static unsigned check(delta_stmt *types,const uint16_t *const *expected,unsigned count)
{
    unsigned cases=0;unsigned char bytes[256];
    for(unsigned pattern=0;pattern<256;++pattern) {
        for(unsigned i=0;i<sizeof bytes;++i)bytes[i]=(unsigned char)(pattern+i*31);
        for(unsigned base=0;base<8;++base) {
            void *record=bytes+8+base;
            for(unsigned type=0;type<count;++type) {
                const delta_stmt *e=types+type;
                assert(e->length>0&&e->length<128);
                for(int32_t field=0;field<e->nfields;++field) {
                    assert(e->field_offset[field]==expected[type][field]);
                    assert(e->field_offset[field]<e->length);
                    void *a=delta_record_field(e,field,record);
                    void *b=delta_record_at(expected[type][field],record);
                    assert(a==(unsigned char *)record+expected[type][field]);
                    assert(a==b);
                    assert(*(unsigned char *)a==bytes[8+base+expected[type][field]]);
                    ++cases;
                }
            }
        }
    }
    return cases;
}

int main(void)
{
    /* Replacing a pointer to field readers by a pointer to byte positions
       preserves the native metadata layout on all supported x64 builds. */
    assert(offsetof(delta_stmt,field_offset)==2*sizeof(void *));
    unsigned n=check(enus61_vstmtbl,expected_enus61,sizeof expected_enus61/sizeof *expected_enus61)
              +check(dede61_vstmtbl,expected_dede61,sizeof expected_dede61/sizeof *expected_dede61);
    assert(n==239616);
    printf("Field offsets: %u original field/base/pattern reads exact; both language layouts passed\n",n);
    return 0;
}
