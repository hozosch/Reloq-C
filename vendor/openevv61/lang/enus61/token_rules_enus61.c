/* Per-token control state, committed directly. The position parameter is
 * unused: this operation neither reads nor changes the token projection.
 * Layout names below belong only to the compatibility boundary. */
#include "reloq61_scalar_state.h"
#define DG_s111   (DG_BASE +  1378)
#define DG_s112   (DG_BASE +  1390)
#define DG_s130   (DG_BASE +  1462)
#define DG_s131   (DG_BASE +  1466)
#define DG_s132   (DG_BASE +  1470)
#define DG_s145   (DG_BASE +  1522)
#define DG_s152   (DG_BASE +  1550)
#define DG_s175   (DG_BASE +  1642)
#define DG_s179   (DG_BASE +  1658)
#define DG_s183   (DG_BASE +  1674)
#define DG_s202   (DG_BASE +  1750)
#define DG_s204   (DG_BASE +  1758)
#define DG_s205   (DG_BASE +  1762)
#define DG_s206   (DG_BASE +  1766)

#define S(name) (*(int16_t *)((unsigned char *)state+DG_##name))

int32_t reloq_enus61_reset_token_vars(void *state,int32_t p0,int32_t p1)
{
    (void)p0;(void)p1;
    if(!reloq_scalar_begin(state))return 94;
    /* Carry the active token flag through one token when no new mode is
     * pending. A supplied mode becomes the previous mode before clearing. */
    S(s206)=S(s205)==1 ||
        (S(s206)==1 && S(s131)==0);
    if(S(s131)!=0)S(s130)=S(s131);
    S(s204)=0;
    S(s205)=0;
    S(s131)=0;
    S(s145)=0;
    S(s152)=0;
    S(s202)=0;
    S(s183)=0;
    S(s112)=0;
    S(s175)=0;
    S(s132)=0;
    S(s111)=0;
    S(s179)=0;
    return reloq_scalar_finish(state,0);
}
