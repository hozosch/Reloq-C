/* Development-only comparison with the native rule arithmetic. The shipped
 * clock uses ordinary scalar C and 80 bytes of slow-rate data. */
#include "reloq_english_nucleus_duration.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
extern int32_t fixed_exp(void *,int32_t);
int main(void)
{
    for(int speed=0;speed<=250;++speed){
        int32_t value=fixed_exp(NULL,-400*((speed+50)/2));
        int expected=(100*value+676)/1353;
        assert(reloq_english_nucleus_rate_percent(speed)==expected);
    }
    puts("Native speed clock: 251 duration percentages match the rule arithmetic");return 0;
}
