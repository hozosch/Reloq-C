/* Per-token control state, committed directly. The position parameter is
 * unused: this operation neither reads nor changes the token projection.
 * Layout names below belong only to the compatibility boundary. */
#include "reloq61_scalar_state.h"
#define DG_s111   (DG_BASE +  1282)
#define DG_s112   (DG_BASE +  1294)
#define DG_s130   (DG_BASE +  1366)
#define DG_s131   (DG_BASE +  1370)
#define DG_s132   (DG_BASE +  1374)
#define DG_s145   (DG_BASE +  1426)
#define DG_s152   (DG_BASE +  1454)
#define DG_s175   (DG_BASE +  1546)
#define DG_s179   (DG_BASE +  1562)
#define DG_s182   (DG_BASE +  1574)
#define DG_s184   (DG_BASE +  1582)
#define DG_s203   (DG_BASE +  1658)
#define DG_s205   (DG_BASE +  1666)
#define DG_s206   (DG_BASE +  1670)
#define DG_s207   (DG_BASE +  1674)

#define S(name) (*(int16_t *)((unsigned char *)state+DG_##name))

int32_t reloq_dede61_reset_token_vars(void *state,int32_t p0,int32_t p1)
{
    (void)p0;(void)p1;
    if(!reloq_scalar_begin(state))return 94;
    /* Carry the active token flag through one token when no new mode is
     * pending. A supplied mode becomes the previous mode before clearing. */
    S(s207)=S(s206)==1 ||
        (S(s207)==1 && S(s131)==0);
    if(S(s131)!=0)S(s130)=S(s131);
    S(s205)=0;
    S(s206)=0;
    S(s131)=0;
    S(s145)=0;
    S(s152)=0;
    S(s203)=0;
    S(s184)=0;
    S(s112)=0;
    S(s175)=0;
    S(s132)=0;
    S(s111)=0;
    S(s179)=0;
    S(s182)=0;
    return reloq_scalar_finish(state,0);
}
