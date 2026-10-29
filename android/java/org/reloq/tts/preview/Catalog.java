package org.reloq.tts.preview;

import java.util.Locale;

/** Stable names, language order and anatomy settings shared with NVDA. */
final class Catalog {
    static final String[] PROFILES = {"Reed", "Shelly", "Bobby", "Rocko",
        "Glen", "Sandy", "Grandma", "Grandpa"};
    static final int[][] PARAMETERS = {
        {0,50,65,30,0,0,92}, {1,50,81,30,0,50,100},
        {1,22,93,35,0,0,90}, {0,86,56,47,0,0,93},
        {0,50,69,34,0,0,92}, {1,56,89,35,0,40,95},
        {1,45,68,30,3,40,90}, {0,30,61,44,18,20,90}
    };
    static final int[] RATE_IDS = {0,1,3,2,4,5,6};
    static final int[] RATES = {8000,11025,16000,22050,32000,44100,48000};
    static final class Language {
        final int id;
        final String name, sample;
        final Locale locale;
        Language(int id, String tag, String name, String sample) {
            this.id=id; this.locale=Locale.forLanguageTag(tag);
            this.name=name; this.sample=sample;
        }
        String voice(int profile) { return locale.toLanguageTag()+"-"+PROFILES[profile]; }
        @Override public String toString() { return name; }
    }
    static final Language[] LANGUAGES = {
        new Language(0x60000,"zh-CN","Mandarin Chinese","你好。This is a test."),
        new Language(0x40000,"de-DE","German","Dies ist eine Sprachprobe mit Eloquence."),
        new Language(0x10001,"en-GB","British English","This is a speech test with Eloquence."),
        new Language(0x10000,"en-US","American English","This is a speech test with Eloquence."),
        new Language(0x20001,"es-MX","Latin American Spanish","Esta es una prueba de voz."),
        new Language(0x20000,"es-ES","Castilian Spanish","Esta es una prueba de voz."),
        new Language(0x90000,"fi-FI","Finnish","Tämä on puhetesti."),
        new Language(0x30000,"fr-FR","French","Ceci est un test de synthèse vocale."),
        new Language(0x30001,"fr-CA","French Canadian","Ceci est un test de synthèse vocale."),
        new Language(0x50000,"it-IT","Italian","Questa è una prova vocale."),
        new Language(0x80000,"ja-JP","Japanese","これは音声のテストです。Windowsとcomputer。"),
        new Language(0xa0000,"ko-KR","Korean","음성 테스트입니다. Windows computer."),
        new Language(0x70000,"pt-BR","Brazilian Portuguese","Este é um teste de voz."),
        new Language(0x120000,"el-GR","Greek","Αυτή είναι μια δοκιμή ομιλίας.")
    };
    static Language find(String lang, String country) {
        if (lang == null || lang.isEmpty()) return null;
        Language first = null;
        for (Language item : LANGUAGES) {
            if (!lang.equalsIgnoreCase(item.locale.getLanguage()) &&
                !lang.equalsIgnoreCase(item.locale.getISO3Language())) continue;
            if (first == null || item.id == 0x10000) first = item;
            if (country != null && !country.isEmpty() &&
                (country.equalsIgnoreCase(item.locale.getCountry()) ||
                 country.equalsIgnoreCase(item.locale.getISO3Country()))) return item;
        }
        return first;
    }
    static Language forVoice(String voice) {
        for (Language item : LANGUAGES)
            for (int p=0; p<PROFILES.length; ++p)
                if (item.voice(p).equals(voice)) return item;
        return null;
    }
    static int profile(String voice, int fallback) {
        if (voice != null) for (int p=0; p<PROFILES.length; ++p)
            if (voice.endsWith("-"+PROFILES[p])) return p;
        return Math.max(0,Math.min(PROFILES.length-1,fallback));
    }
    static int[] controls(int profile, int speechRate, int pitch, int rate, int pauses) {
        VoiceSettings settings=new VoiceSettings(profile);
        settings.rate=rate;settings.pauses=pauses;
        return settings.controls(speechRate,pitch);
    }
}
