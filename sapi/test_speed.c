#include "speed.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
int main(void)
{
    const int expected[21]={0,0,0,0,0,10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160};
    for(int rate=-10;rate<=10;++rate)assert(sapi_native_speed(rate,0,0)==expected[rate+10]);
    assert(sapi_native_speed(0,19,0)==250);
    assert(sapi_native_speed(10,5,0)==210);
    assert(sapi_native_speed(0,INT_MAX,0)==250);
    assert(sapi_native_speed(-10,INT_MIN,0)==0);
    assert(sapi_native_speed(0,0,1)==96);
    assert(sapi_native_speed(10,0,1)==250);
    puts("{\"reference_rate_checks\":27,\"sapi_zero_native\":60,\"native_minimum\":0,\"native_maximum\":250}");return 0;
}
