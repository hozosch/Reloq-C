package org.reloq.tts;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.UserManager;
import java.util.Map;

/** The device-protected copy contains only settings needed for speech. */
final class BootPreferences {
    private static final String DEVICE_FILE="speech-before-unlock";
    private static final String[] BOOLS={"androidVoice","androidRate","androidPitch",
        "rateBoost","speedWpmMode","abbreviations","prediction","originalQuestions","mellowMode","eqEnabled","tags"};
    private static final String[] INTS={"language","profile","manualWpm","manualSpeed","rate","pauses","eqPreset","eqLow","eqMid","eqHigh"};
    private static final String[] VOICE={"pitch","pitchHertz","gender","head","inflection","roughness","breathiness","volume"};
    private static final String[] VOICE_BOOLS={"pitchHertzMode"};
    private BootPreferences() {}
    static boolean unlocked(Context context) {
        UserManager users=(UserManager)context.getSystemService(Context.USER_SERVICE);
        return users!=null&&users.isUserUnlocked();
    }
    static SharedPreferences forService(Context context,boolean unlocked) {
        return unlocked?context.getSharedPreferences("settings",Context.MODE_PRIVATE):
            context.createDeviceProtectedStorageContext().getSharedPreferences(DEVICE_FILE,Context.MODE_PRIVATE);
    }
    static void refresh(Context context) {
        if(unlocked(context))mirror(context.getSharedPreferences("settings",Context.MODE_PRIVATE),
            context.createDeviceProtectedStorageContext().getSharedPreferences(DEVICE_FILE,Context.MODE_PRIVATE));
    }
    static synchronized void mirror(SharedPreferences source,SharedPreferences destination) {
        Map<String,?> values=source.getAll();
        SharedPreferences.Editor edit=destination.edit().clear();
        for(String key:BOOLS)if(values.get(key) instanceof Boolean)edit.putBoolean(key,(Boolean)values.get(key));
        for(String key:INTS)if(values.get(key) instanceof Integer)edit.putInt(key,(Integer)values.get(key));
        for(int i=0;i<5;++i){String prefix="eqFilter."+i+".";
            if(values.get(prefix+"enabled") instanceof Boolean)edit.putBoolean(prefix+"enabled",(Boolean)values.get(prefix+"enabled"));
            for(String field:new String[]{"frequency","gain","width"})
                if(values.get(prefix+field) instanceof Integer)edit.putInt(prefix+field,(Integer)values.get(prefix+field));
        }
        Object sources=values.get("dictionarySources");
        boolean own=sources instanceof String&&!((String)sources).replaceAll("\\s","").equals("[]");
        edit.putBoolean("dictionaryRequired",Boolean.TRUE.equals(values.get("communityDictionaries"))||
            own||values.get("dictionarySnapshot") instanceof String);
        for(int profile=0;profile<Catalog.PROFILES.length;++profile)for(String field:VOICE) {
            String key="voice."+profile+"."+field;
            if(values.get(key) instanceof Integer)edit.putInt(key,(Integer)values.get(key));
        }
        for(int profile=0;profile<Catalog.PROFILES.length;++profile)for(String field:VOICE_BOOLS) {
            String key="voice."+profile+"."+field;
            if(values.get(key) instanceof Boolean)edit.putBoolean(key,(Boolean)values.get(key));
        }
        edit.apply();
    }
}
