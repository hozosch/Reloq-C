/* Case normalization is one direct pass over the admitted letter run. */
#include "reloq61_text_case.h"
int32_t reloq_enus61_convert_to_lowercase(void *state,int32_t p0,int32_t p1,int32_t p2)
{(void)p0;return reloq61_text_lowercase(state,p1,p2,0);}
