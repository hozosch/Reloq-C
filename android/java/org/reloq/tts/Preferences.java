package org.reloq.tts;

import android.content.SharedPreferences;

final class Preferences {
    static int clockMask(SharedPreferences p) {
        int max=(1<<Catalog.LANGUAGES.length)-1,mask=p.getInt("acousticClockMask",-1);
        if(mask>=0&&mask<=max)return mask;
        return p.getBoolean("originalAcousticClock",false)?max:VoiceSettings.defaultClockMask();
    }
    static VoiceSettings read(SharedPreferences p,int profile) {
        VoiceSettings s=new VoiceSettings(profile);
        String key="voice."+profile+".";
        s.androidRate=p.getBoolean("androidRate",true);s.androidPitch=p.getBoolean("androidPitch",true);
        s.manualWpm=p.getInt("manualWpm",190);s.rateBoost=p.getBoolean("rateBoost",false);
        /* Older manual WPM was an approximate ECI target. Preserve the WPM
         * number and use the native WPM timing by default. Keep the old ECI
         * target available if the user explicitly selects that scale. */
        int oldSpeed=VoiceSettings.targetSpeed(VoiceSettings.bounded(s.manualWpm,140,1400)/190.0*(s.rateBoost?1.6:1));
        s.manualSpeed=p.getInt("manualSpeed",oldSpeed);s.speedWpmMode=p.getBoolean("speedWpmMode",true);
        s.abbreviations=p.getBoolean("abbreviations",true);s.prediction=p.getBoolean("prediction",false);
        s.originalQuestions=p.getBoolean("originalQuestions",false);
        s.mellowMode=p.getBoolean("mellowMode",false);
        s.wordSegmentation=p.getBoolean("wordSegmentation",false);
        s.camelCase=p.getBoolean("camelCase",false);
        s.acousticClockMask=clockMask(p);
        s.eqEnabled=p.getBoolean("eqEnabled",false);s.eqPreset=p.getInt("eqPreset",0);
        s.eqLow=p.getInt("eqLow",0);s.eqMid=p.getInt("eqMid",0);s.eqHigh=p.getInt("eqHigh",0);
        for(int i=0;i<s.eqFilters.length;++i){String prefix="eqFilter."+i+".";
            s.eqFilters[i][0]=p.getBoolean(prefix+"enabled",false)?1:0;
            s.eqFilters[i][1]=p.getInt(prefix+"frequency",s.eqFilters[i][1]);
            s.eqFilters[i][2]=p.getInt(prefix+"gain",0);s.eqFilters[i][3]=p.getInt(prefix+"width",100);}
        s.tags=p.getBoolean("tags",false);s.rate=p.getInt("rate",1);s.pauses=p.getInt("pauses",0);
        s.manualPitch=p.getInt(key+"pitch",s.manualPitch);
        s.pitchHertz=p.getInt(key+"pitchHertz",s.pitchHertz);
        s.pitchHertzMode=p.getBoolean(key+"pitchHertzMode",false);
        s.gender=p.getInt(key+"gender",s.gender);s.head=p.getInt(key+"head",s.head);
        s.inflection=p.getInt(key+"inflection",s.inflection);s.roughness=p.getInt(key+"roughness",s.roughness);
        s.breathiness=p.getInt(key+"breathiness",s.breathiness);s.volume=p.getInt(key+"volume",s.volume);
        return s;
    }
    static void save(SharedPreferences p,int profile,VoiceSettings s) {
        String key="voice."+profile+".";
        SharedPreferences.Editor editor=p.edit();
        for(int i=0;i<s.eqFilters.length;++i){String prefix="eqFilter."+i+".";int[] f=s.eqFilters[i];
            editor.putBoolean(prefix+"enabled",f[0]!=0).putInt(prefix+"frequency",f[1])
                .putInt(prefix+"gain",f[2]).putInt(prefix+"width",f[3]);}
        editor.putBoolean("androidRate",s.androidRate).putBoolean("androidPitch",s.androidPitch)
            .putInt("manualWpm",s.manualWpm).putInt("manualSpeed",s.manualSpeed)
            .putBoolean("speedWpmMode",s.speedWpmMode).putBoolean("rateBoost",s.rateBoost)
            .putBoolean("abbreviations",s.abbreviations).putBoolean("prediction",s.prediction)
            .putBoolean("originalQuestions",s.originalQuestions)
            .putBoolean("mellowMode",s.mellowMode)
            .putBoolean("wordSegmentation",s.wordSegmentation).putBoolean("camelCase",s.camelCase)
            .putInt("acousticClockMask",s.acousticClockMask&((1<<Catalog.LANGUAGES.length)-1))
            .putBoolean("eqEnabled",s.eqEnabled).putInt("eqPreset",s.eqPreset)
            .putInt("eqLow",s.eqLow).putInt("eqMid",s.eqMid).putInt("eqHigh",s.eqHigh)
            .putBoolean("tags",s.tags).putInt("rate",s.rate).putInt("pauses",s.pauses)
            .putInt(key+"pitch",s.manualPitch).putInt(key+"pitchHertz",s.pitchHertz)
            .putBoolean(key+"pitchHertzMode",s.pitchHertzMode).putInt(key+"gender",s.gender).putInt(key+"head",s.head)
            .putInt(key+"inflection",s.inflection).putInt(key+"roughness",s.roughness)
            .putInt(key+"breathiness",s.breathiness).putInt(key+"volume",s.volume).apply();
    }
}
