package org.reloq.tts;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

/** Script based language detection for the automatic language option.
 *  Mixed text is split into runs per writing system so each run is spoken
 *  by the matching Eloquence language engine within one utterance. Han
 *  characters use Mandarin, kana Japanese, hangul Korean and Greek script
 *  Greek. Latin letters use the English variant, unless the base language
 *  itself is written with Latin letters. Runs of the same language merge,
 *  and short silences separate language changes. */
final class AutoLanguage {
    /** Spinner index and stored preference value of the automatic entry. */
    static final int INDEX=Catalog.LANGUAGES.length;
    private static final int LATIN=1,HAN=2,KANA=3,HANGUL=4,GREEK=5,OTHER=6,CJK_PUNCT=7,DIGIT=8;
    private static final String PREFIX="auto-";
    private AutoLanguage() {}
    /** Android voice names of the automatic detection entries. */
    static String voice(int profile) { return PREFIX+Catalog.PROFILES[profile]; }
    static boolean isVoice(String name) {
        if(name==null||!name.startsWith(PREFIX))return false;
        for(String profile:Catalog.PROFILES)if(name.equals(PREFIX+profile))return true;
        return false;
    }
    static final class Segment {
        final int dialect;
        final String text;
        Segment(int dialect,String text){this.dialect=dialect;this.text=text;}
    }
    /** Latin runs keep a Latin script base language, otherwise American English. */
    static int latinDialect(int base) {
        switch(base) {
            case 0x10000: case 0x10001: case 0x20000: case 0x20001:
            case 0x30000: case 0x30001: case 0x40000: case 0x50000:
            case 0x70000: case 0x90000: return base;
            default: return 0x10000;
        }
    }
    private static int dialectOf(int script,int latin,int base) {
        switch(script) {
            case LATIN: return latin;
            case HAN: return 0x60000;
            case KANA: return 0x80000;
            case HANGUL: return 0xa0000;
            case GREEK: return 0x120000;
            case CJK_PUNCT: return 0x60000; // Mandarin shares Han punctuation
            case DIGIT: return base; // numbers read in the base language
            default: return base; // OTHER and symbols
        }
    }
    private static int script(int cp) {
        if(cp>=0x3040&&cp<=0x30FF||cp>=0x31F0&&cp<=0x31FF||cp==0x1B000||cp==0x1B001)return KANA;
        if(cp>=0x1100&&cp<=0x11FF||cp>=0x3130&&cp<=0x318F||cp>=0xA960&&cp<=0xA97F||cp>=0xAC00&&cp<=0xD7FF)return HANGUL;
        if(cp>=0x0370&&cp<=0x03FF||cp>=0x1F00&&cp<=0x1FFF)return GREEK;
        // CJK punctuation and fullwidth forms bind to surrounding CJK runs
        // instead of starting a separate language segment. The fullwidth
        // period FF0E and solidus FF0F stay neutral so decimal numbers
        // such as 17．11 keep the point inside the digit run.
        if(cp>=0x3000&&cp<=0x303F||cp>=0xFE10&&cp<=0xFE4F||cp>=0xFF01&&cp<=0xFF0D||
           cp>=0xFF1A&&cp<=0xFF20||cp>=0xFF3B&&cp<=0xFF40||cp>=0xFF5B&&cp<=0xFF65)return CJK_PUNCT;
        // CJK radicals, symbols, extensions and unified ideographs.
        if(cp>=0x2E80&&cp<=0x2EFF||cp>=0x31C0&&cp<=0x31EF||cp>=0x3300&&cp<=0x4DBF||
           cp>=0x4E00&&cp<=0x9FFF||cp>=0xF900&&cp<=0xFAFF||cp>=0x20000&&cp<=0x2FA1F)return HAN;
        // Digits form their own runs so numbers always follow the base
        // language instead of sticking to adjacent English words.
        if(cp>='0'&&cp<='9'||cp>=0xFF10&&cp<=0xFF19)return DIGIT;
        if(!Character.isLetter(cp))return 0; // whitespace, punctuation, symbols
        if(cp<=0x02AF||cp>=0x1E00&&cp<=0x1EFF||cp>=0x2C60&&cp<=0x2C7F||
           cp>=0xA720&&cp<=0xA7FF||cp>=0xAB30&&cp<=0xAB6F||cp>=0xFF21&&cp<=0xFF5A)return LATIN;
        return OTHER; // Cyrillic, Hebrew, Arabic, Thai and other unsupported scripts
    }
    /** Split into language runs. Text without any detected script stays one
     *  base language run, so plain numbers keep their previous behaviour.
     *  Neutral characters (spaces, digits, punctuation) stay with the run
     *  they follow: a punctuation mark is the pause of the sentence just
     *  spoken, so each language engine keeps its own sentence endings. */
    static List<Segment> segments(String text,int latin,int base) {
        List<Segment> result=new ArrayList<>();
        StringBuilder current=new StringBuilder();
        int script=0;
        int length=text.length();
        for(int i=0;i<length;) {
            int cp=text.codePointAt(i);
            i+=Character.charCount(cp);
            int next=script(cp);
            if(next==0) {
                if(script==0) { // leading neutral text: base language
                    add(result,0,new StringBuilder().appendCodePoint(cp),latin,base);
                } else current.appendCodePoint(cp);
                continue;
            }
            if(next==script) {current.appendCodePoint(cp);continue;}
            add(result,script,current,latin,base);
            current.setLength(0);
            current.appendCodePoint(cp);
            script=next;
        }
        add(result,script,current,latin,base);
        return result;
    }
    private static void add(List<Segment> result,int script,CharSequence text,int latin,int base) {
        if(text.length()==0)return;
        String value=script==DIGIT?
            decimalPoint(text.toString(),dialectOf(DIGIT,latin,base)):text.toString();
        if(script==CJK_PUNCT) {
            // CJK punctuation and fullwidth forms stick to the previous run
            // as-is; without any run yet the base language reads them.
            if(result.isEmpty()) {result.add(new Segment(base,text.toString()));return;}
            Segment previous=result.get(result.size()-1);
            result.set(result.size()-1,
                new Segment(previous.dialect,previous.text+text));
            return;
        }
        int dialect=dialectOf(script,latin,base);
        if(!result.isEmpty()) {
            Segment previous=result.get(result.size()-1);
            if(previous.dialect==dialect) {
                result.set(result.size()-1,new Segment(dialect,previous.text+value));
                return;
            }
        }
        result.add(new Segment(dialect,value));
    }
    /** CJK engines read a period inside numbers as a sentence pause, so a
     *  decimal point between digits becomes the spoken word of the language.
     *  Latin language engines pronounce the period themselves. */
    private static String decimalPoint(String text,int dialect) {
        String mark;
        switch(dialect) {
            case 0x60000: case 0x80000: mark="点";break;
            case 0x60001: mark="點";break;
            case 0xa0000: mark="점";break;
            default: return text;
        }
        return text.replaceAll("(?<=[0-9０-９])[.．](?=[0-9０-９])",mark);
    }
    /** The automatic entry label, localized without new resources. */
    static String label() {
        String language=Locale.getDefault().getLanguage();
        if("zh".equals(language))return "自动检测语言";
        if("ja".equals(language))return "自動言語検出";
        if("ko".equals(language))return "자동 언어 감지";
        if("de".equals(language))return "Automatische Spracherkennung";
        if("fr".equals(language))return "Détection automatique";
        if("es".equals(language))return "Detección automática";
        if("it".equals(language))return "Rilevamento automatico";
        if("fi".equals(language))return "Automaattinen tunnistus";
        if("pt".equals(language))return "Detecção automática";
        if("el".equals(language))return "Αυτόματη ανίχνευση γλώσσας";
        return "Automatic language detection";
    }
    static String sample() {
        if("zh".equals(Locale.getDefault().getLanguage()))
            return "自动语言测试：你好世界，Hello world。";
        return "Automatic language test: Hello world. 你好世界。";
    }
}
