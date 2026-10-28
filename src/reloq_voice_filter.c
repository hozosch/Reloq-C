#include "reloq_voice_filter.h"

static int32_t percent(int32_t value,int scale)
{return (int32_t)((int64_t)value*scale/100);}
static int32_t ceiling(int32_t value,int maximum)
{return value>maximum?maximum:value;}

/* Endpoint anatomy and the female range map recovered from the Apple 6.1
 * rules. Keep the order: head scaling, female map, narrow-band limits.
 * Applying this after interpolation changes crossings of the range edges. */
int32_t reloq_voice_filter_endpoint(const struct reloq_voice_filter *m,
    unsigned parameter,int32_t value)
{
    if(m->head_size!=50) {
        if(parameter>=1 && parameter<=5)value=percent(value,m->scale_percent);
        else if(parameter==10 && !m->female && m->head_size<=40)value*=8;
        else if(parameter==9 && m->female) {
            if(m->head_size<=40)value=percent(value,160);
            value=ceiling(value,3000);
        }
    }
    if(m->female) {
        switch(parameter) {
        case 1:value=percent(value,value<=350?107:value<=500?110:value<=650?118:125);break;
        case 2:value=percent(value,value<=1100?107:value<=1300?115:value<=1700?117:121);break;
        case 3:value=percent(value,119);break;
        case 4:
            if(m->explicit_poles&(1u<<3))
                value=value>3650?4200:percent(value,value<=3000?120:115);
            break;
        case 5:
            if(m->explicit_poles&(1u<<4))
                value=value>=4000?4600:percent(value,value<=2700?120:115);
            break;
        }
    }
    if(m->sample_rate==8000) {
        switch(parameter) {
        case 3:value=ceiling(value,3600);break;
        case 4:value=ceiling(value,3900);break;
        case 9:value=m->female?1000:ceiling(percent(value,150),3000);break;
        case 8:
            if(m->female)value=m->head_size<30?ceiling(value*8,3000):
                m->head_size<=40?ceiling(value*4,1000):ceiling(value*2,500);
            break;
        }
    }
    return value;
}
