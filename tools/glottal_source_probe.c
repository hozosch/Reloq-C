/* Raw float source only, for the numerical anti-alias reference check. */
#include "reloq_core.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif
int main(int argc,char **argv)
{
    assert(argc==5);
    float rate=(float)atof(argv[1]),pitch=(float)atof(argv[3]),open=(float)atof(argv[4]);
    struct reloq_core_voiced_state state={0};
    assert(!reloq_core_voiced_configure(&state,rate,pitch,open,55));
    state.bandlimited_closure=atoi(argv[2]);
    size_t count=(size_t)(rate*2);float *samples=calloc(count,sizeof *samples);assert(samples);
    assert(!reloq_core_voiced_fill(&state,samples,count));
#ifdef _WIN32
    _setmode(_fileno(stdout),_O_BINARY);
#endif
    assert(fwrite(samples,sizeof *samples,count,stdout)==count);
    free(samples);return 0;
}
