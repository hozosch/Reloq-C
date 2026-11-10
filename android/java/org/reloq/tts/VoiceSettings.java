package org.reloq.tts;

/** Settings copied for one request; usable in host tests without Android. */
final class VoiceSettings {
    boolean androidRate=true,androidPitch=true,rateBoost=false,tags=false,abbreviations=true,prediction=false,originalQuestions=false,mellowMode=false;
    boolean pitchHertzMode=false,speedWpmMode=true;
    boolean eqEnabled=false;
    int eqPreset=0,eqLow=0,eqMid=0,eqHigh=0;
    static final int[][] EQ_PRESETS={{0,0,0},{4,0,-1},{-1,0,4},{0,-1,-4}};
    static final int[] EQ_FILTER_FREQUENCIES={125,500,1200,3000,6000};
    final int[][] eqFilters=new int[5][4];
    int manualWpm=190,manualSpeed=55,manualPitch,pitchHertz=150,gender,head,pitch,inflection,roughness,breathiness,volume;
    int rate=1,pauses=0;

    VoiceSettings(int profile) {
        int[] p=Catalog.PARAMETERS[Catalog.profile(null,profile)];
        gender=p[0];head=p[1];pitch=p[2];manualPitch=p[2];inflection=p[3];
        roughness=p[4];breathiness=p[5];volume=p[6];
        for(int i=0;i<eqFilters.length;++i){eqFilters[i][1]=EQ_FILTER_FREQUENCIES[i];eqFilters[i][3]=100;}
    }
    static int bounded(int value,int low,int high) { return Math.max(low,Math.min(high,value)); }
    static double timeScale(int speed) {
        speed=bounded(speed,40,250);
        for(int i=1;i<RateCurve.SPEEDS.length;++i) if(speed<=RateCurve.SPEEDS[i]) {
            double t=(double)(speed-RateCurve.SPEEDS[i-1])/(RateCurve.SPEEDS[i]-RateCurve.SPEEDS[i-1]);
            return Math.exp(Math.log(RateCurve.TIMES[i-1])*(1-t)+Math.log(RateCurve.TIMES[i])*t);
        }
        return RateCurve.TIMES[RateCurve.TIMES.length-1];
    }
    static int androidSpeed(int percent) {
        return targetSpeed(Math.max(1,percent)/100.0);
    }
    static int targetSpeed(double ratio) {
        double wanted=timeScale(55)/ratio;
        int low=40,high=250;
        while(low<high) { int mid=(low+high)/2;if(timeScale(mid)>wanted)low=mid+1;else high=mid; }
        if(low>40 && Math.abs(Math.log(timeScale(low-1)/wanted))<Math.abs(Math.log(timeScale(low)/wanted)))--low;
        return low;
    }
    static double pitchHz(int value) { return 35.11023877+4.889761232*Math.pow(2,.06306456*value); }
    static int androidPitch(int base,int percent) {
        double desired=pitchHz(base)*Math.max(1,percent)/100;
        if(desired<=pitchHz(0))return 0;
        return bounded((int)Math.round(Math.log((desired-35.11023877)/4.889761232)/Math.log(2)/.06306456),0,100);
    }
    int[] controls(int requestedRate,int requestedPitch) {
        double ratio=Math.max(1,requestedRate)/100.0;
        int speed=androidRate?targetSpeed(ratio*(rateBoost?1.6:1)):bounded(manualSpeed,0,250);
        int preset=bounded(eqPreset,0,4);
        int[] eq=!eqEnabled?EQ_PRESETS[0]:preset==4?new int[]{bounded(eqLow,-12,12),bounded(eqMid,-12,12),bounded(eqHigh,-12,12)}:EQ_PRESETS[preset];
        int[] base=new int[]{bounded(gender,0,1),bounded(head,0,100),
            androidPitch?androidPitch(pitch,requestedPitch):
                pitchHertzMode?1000+bounded(pitchHertz,1,24000):bounded(manualPitch,0,100),
            bounded(inflection,0,100),bounded(roughness,0,100),bounded(breathiness,0,100),
            bounded(volume,0,100),speed,Catalog.RATE_IDS[bounded(rate,0,6)],bounded(pauses,0,2),tags?1:0,abbreviations?0:1,prediction?1:0,originalQuestions?1:0,mellowMode?1:0,
            !androidRate&&speedWpmMode?bounded(manualWpm,1,6000):0,
            eq[0]+12,eq[1]+12,eq[2]+12};
        int[] result=java.util.Arrays.copyOf(base,base.length+4*eqFilters.length);
        for(int i=0;i<eqFilters.length;++i){int[] f=eqFilters[i];int offset=base.length+4*i;
            result[offset]=eqEnabled&&f[0]!=0?1:0;result[offset+1]=bounded(f[1],1,24000);
            result[offset+2]=bounded(f[2],-12,12)+12;result[offset+3]=bounded(f[3],10,400);}
        return result;
    }
}
