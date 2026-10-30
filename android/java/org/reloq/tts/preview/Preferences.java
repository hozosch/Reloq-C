package org.reloq.tts.preview;

import android.content.SharedPreferences;

final class Preferences {
    static VoiceSettings read(SharedPreferences p,int profile) {
        VoiceSettings s=new VoiceSettings(profile);
        String key="voice."+profile+".";
        s.androidRate=p.getBoolean("androidRate",true);s.androidPitch=p.getBoolean("androidPitch",true);
        s.manualWpm=p.getInt("manualWpm",190);s.rateBoost=p.getBoolean("rateBoost",false);
        s.abbreviations=p.getBoolean("abbreviations",true);s.prediction=p.getBoolean("prediction",false);
        s.tags=p.getBoolean("tags",false);s.rate=p.getInt("rate",1);s.pauses=p.getInt("pauses",0);
        s.manualPitch=p.getInt(key+"pitch",s.manualPitch);
        s.gender=p.getInt(key+"gender",s.gender);s.head=p.getInt(key+"head",s.head);
        s.inflection=p.getInt(key+"inflection",s.inflection);s.roughness=p.getInt(key+"roughness",s.roughness);
        s.breathiness=p.getInt(key+"breathiness",s.breathiness);s.volume=p.getInt(key+"volume",s.volume);
        return s;
    }
    static void save(SharedPreferences p,int profile,VoiceSettings s) {
        String key="voice."+profile+".";
        p.edit().putBoolean("androidRate",s.androidRate).putBoolean("androidPitch",s.androidPitch)
            .putInt("manualWpm",s.manualWpm).putBoolean("rateBoost",s.rateBoost)
            .putBoolean("abbreviations",s.abbreviations).putBoolean("prediction",s.prediction)
            .putBoolean("tags",s.tags).putInt("rate",s.rate).putInt("pauses",s.pauses)
            .putInt(key+"pitch",s.manualPitch).putInt(key+"gender",s.gender).putInt(key+"head",s.head)
            .putInt(key+"inflection",s.inflection).putInt(key+"roughness",s.roughness)
            .putInt(key+"breathiness",s.breathiness).putInt(key+"volume",s.volume).apply();
    }
}
