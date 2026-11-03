/* Broad, low-gain versions of A's upper oral resonances. They act on the
 * existing periodic source before AH joins it, so extra richness cannot
 * manufacture aspiration or another independently phased voice. */
void reloq_rate_transfer_prepare_body(struct reloq_rate_transfer *s,
    const struct reloq_core_voiced_formants *f,float rate)
{
    if(s->body_amount<=0||rate<22050)return;
    float key[7]={s->body_amount};
    for(unsigned j=0;j<3;++j) {
        const unsigned at=j+5;
        if((long)at<f->highest_formant) {
            key[1+2*j]=f->oral[at].frequency_hz;
            key[2+2*j]=f->oral[at].bandwidth_hz;
        }
    }
    if(s->body_ready&&!memcmp(key,s->body_key,sizeof key))return;
    memcpy(s->body_key,key,sizeof key);
    for(unsigned j=0;j<3;++j) {
        double *c=s->body_target[j];
        memset(c,0,5*sizeof *c);c[0]=1;
        if(key[1+2*j]>0) {
            double hz=fmin(rate*.35,fmax(3500,key[1+2*j]));
            double width=fmax(hz/2.5,key[2+2*j]);
            double w=2*pi*hz/rate,alpha=sin(w)*width/(2*hz);
            double a=pow(10,3*s->body_amount/40),den=1+alpha/a;
            c[0]=(1+alpha*a)/den;c[1]=-2*cos(w)/den;
            c[2]=(1-alpha*a)/den;c[3]=c[1];c[4]=(1-alpha/a)/den;
        }
        if(!s->body_ready) {
            memset(s->body_coefficient[j],0,5*sizeof *c);
            s->body_coefficient[j][0]=1;
        }
    }
    s->body_remaining=(unsigned)(rate*.005f);
    if(!s->body_remaining)s->body_remaining=1;
    for(unsigned j=0;j<3;++j)for(unsigned k=0;k<5;++k)
        s->body_step[j][k]=(s->body_target[j][k]-s->body_coefficient[j][k])/s->body_remaining;
    s->body_ready=1;
}
static double gradual_body_sample(struct reloq_rate_transfer *s,double y)
{
    if(s->body_remaining) {
        for(unsigned j=0;j<3;++j)for(unsigned k=0;k<5;++k)
            s->body_coefficient[j][k]+=s->body_step[j][k];
        if(!--s->body_remaining)
            memcpy(s->body_coefficient,s->body_target,sizeof s->body_target);
    }
    for(unsigned j=0;j<3;++j) {
        const double *c=s->body_coefficient[j];double *d=s->body_delay[j];
        double x=y;y=c[0]*x+d[0];
        d[0]=c[1]*x-c[3]*y+d[1];d[1]=c[2]*x-c[4]*y;
    }
    return y;
}
