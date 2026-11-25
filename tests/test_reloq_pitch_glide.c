#include "reloq_pitch_glide.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

static int near(double a,double b) { return fabs(a-b)<1e-9; }

int main(void)
{
    struct reloq_pitch_glide g={0};
    assert(near(reloq_pitch_glide_value(&g,1.0,40),1.0));
    /* Start an octave rise; at halfway log-frequency gives half an octave. */
    assert(near(reloq_pitch_glide_value(&g,2.0,40),1.0));
    reloq_pitch_glide_advance(&g,20,40);
    assert(near(reloq_pitch_glide_value(&g,2.0,40),sqrt(2.0)));
    /* An overlapping change starts at the reached value, without a jump. */
    assert(near(reloq_pitch_glide_value(&g,0.5,40),sqrt(2.0)));
    reloq_pitch_glide_advance(&g,40,40);
    assert(near(reloq_pitch_glide_value(&g,0.5,40),0.5));
    reloq_pitch_glide_advance(&g,100,40);
    assert(near(reloq_pitch_glide_value(&g,0.5,40),0.5));
    puts("Pitch glide: exact endpoints, logarithmic midpoint and interrupted ramp passed");
    return 0;
}
