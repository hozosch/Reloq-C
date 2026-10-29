package org.reloq.tts.preview;

/** Settings copied for one request; usable in host tests without Android. */
final class VoiceSettings {
    boolean androidRate=true,androidPitch=true,rateBoost=false,tags=false;
    int manualWpm=190,manualPitch,gender,head,pitch,inflection,roughness,breathiness,volume;
    int rate=1,pauses=0;

    VoiceSettings(int profile) {
        int[] p=Catalog.PARAMETERS[Catalog.profile(null,profile)];
        gender=p[0];head=p[1];pitch=p[2];manualPitch=p[2];inflection=p[3];
        roughness=p[4];breathiness=p[5];volume=p[6];
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
        double ratio=androidRate?Math.max(1,requestedRate)/100.0:bounded(manualWpm,140,1400)/190.0;
        int speed=targetSpeed(ratio*(rateBoost?1.6:1));
        return new int[]{bounded(gender,0,1),bounded(head,0,100),
            androidPitch?androidPitch(pitch,requestedPitch):bounded(manualPitch,0,100),
            bounded(inflection,0,100),bounded(roughness,0,100),bounded(breathiness,0,100),
            bounded(volume,0,100),speed,Catalog.RATE_IDS[bounded(rate,0,6)],bounded(pauses,0,2),tags?1:0};
    }
}
