package org.reloq.tts;
import java.util.List;
/** Script segmentation checks for the automatic language detection. */
public final class AutoLanguageTest {
    private static int checks;
    private static void require(boolean value,String name)
        {++checks;if(!value)throw new AssertionError("check "+checks+": "+name);}
    private static String show(List<AutoLanguage.Segment> segments) {
        StringBuilder text=new StringBuilder();
        for(AutoLanguage.Segment segment:segments)
            text.append('[').append(Integer.toHexString(segment.dialect)).append(']')
                .append(segment.text).append('|');
        return text.toString();
    }
    public static void main(String[] arguments) {
        // Trailing rule: neutral characters stay with the previous run.
        require("[60000]你好，|[10000]Hello world.|".equals(show(
            AutoLanguage.segments("你好，Hello world.",0x10000,0x60000))),"zh+en");
        require("[10000]Say hello, |[60000]你好, |[10000]please.|".equals(show(
            AutoLanguage.segments("Say hello, 你好, please.",0x10000,0x10000))),"en+zh");
        require("[10000]This is a test.|".equals(show(
            AutoLanguage.segments("This is a test.",0x10000,0x10000))),"pure english");
        require("[60000]这是一段测试文本。|".equals(show(
            AutoLanguage.segments("这是一段测试文本。",0x10000,0x60000))),"pure chinese");
        require("[60000]123 456。|".equals(show(
            AutoLanguage.segments("123 456。",0x10000,0x60000))),"pure numbers");
        require("[40000]Hallo Welt, Hello, wie gehts?|".equals(show(
            AutoLanguage.segments("Hallo Welt, Hello, wie gehts?",0x40000,0x40000))),"german base");
        require("[80000]これはテストです。|[10000]Windows |[80000]と コンピュータ。|".equals(show(
            AutoLanguage.segments("これはテストです。Windows と コンピュータ。",0x10000,0x80000))),"japanese");
        require("[a0000]음성 테스트입니다. |[10000]Windows computer.|".equals(show(
            AutoLanguage.segments("음성 테스트입니다. Windows computer.",0x10000,0xa0000))),"korean");
        require("[120000]Αυτή είναι μια δοκιμή. |[10000]Test. |[60000]完了。|".equals(show(
            AutoLanguage.segments("Αυτή είναι μια δοκιμή. Test. 完了。",0x10000,0x120000))),"greek");
        require("[60000]你好。|[10000]OK|".equals(show(
            AutoLanguage.segments("你好。OK",0x10000,0x60000))),"cjk punctuation");
        require("".equals(show(AutoLanguage.segments("",0x10000,0x60000))),"empty");
        require("[60000]!?...|".equals(show(
            AutoLanguage.segments("!?...",0x10000,0x60000))),"symbols only");
        require("[10000]Hello Welt Hello|".equals(show(
            AutoLanguage.segments("Hello Welt Hello",0x10000,0x10000))),"merge same");
        require("[10000]Version 12 is here.|[60000]好了|".equals(show(
            AutoLanguage.segments("Version 12 is here.好了",0x10000,0x10000))),"digits inline");
        require("[10000]Operit, |[60000]10 分钟前, |[10000]Operit |[60000]正在运行|".equals(show(
            AutoLanguage.segments(NumberReading.prepare("Operit, • 10 分钟前, Operit 正在运行",
                NumberReading.AUTO,0x60000),0x10000,0x60000))),"notification bullet route");
        // Numbers always read in the base language, even next to English.
        require("[10000]Android|[60000]13系统|".equals(show(
            AutoLanguage.segments("Android13系统",0x10000,0x60000))),"digits base language");
        for (Catalog.Language language:Catalog.LANGUAGES) {
            if(language.id==0x60000||language.id==0x60001)continue;
            for(String digits:new String[]{"2","200","2000"}) {
                String prepared=NumberReading.prepare(digits,NumberReading.AUTO,language.id);
                require(digits.equals(prepared),"non-Chinese auto number "+digits+" in "+language.name);
                require(("["+Integer.toHexString(language.id)+"]"+digits+"|").equals(
                    show(AutoLanguage.segments(prepared,AutoLanguage.latinDialect(language.id),language.id))),
                    "non-Chinese auto number route "+digits+" in "+language.name);
            }
        }
        require("[60000]2026年|[10000]Google |[60000]发布|".equals(show(
            AutoLanguage.segments("2026年Google 发布",0x10000,0x60000))),"year chinese");
        // Decimal points inside numbers become the spoken word for CJK.
        require("[60000]17点11|[10000]MB|".equals(show(
            AutoLanguage.segments("17.11MB",0x10000,0x60000))),"decimal chinese");
        require("[60000]17点11|[10000]MB|".equals(show(
            AutoLanguage.segments("17．11MB",0x10000,0x60000))),"decimal fullwidth");
        require("[10000]17.11MB|".equals(show(
            AutoLanguage.segments("17.11MB",0x10000,0x10000))),"decimal english");
        require(AutoLanguage.latinDialect(0x60000)==0x10000,"latin fallback");
        require(AutoLanguage.latinDialect(0x40000)==0x40000,"latin keep base");
        require(AutoLanguage.latinDialect(0x130000)==0x130000,"Aragonese keeps its Latin dialect");
        require("[130000]Esta ye una preba de voz.|".equals(show(
            AutoLanguage.segments("Esta ye una preba de voz.",AutoLanguage.latinDialect(0x130000),0x130000))),"Aragonese base");
        require("[80000]日本語を読む。|[10000]Windows|".equals(show(
            AutoLanguage.segments("日本語を読む。Windows",0x10000,0x80000))),"Japanese kana and kanji stay together");
        require("[80000]漢字|".equals(show(
            AutoLanguage.segments("漢字",0x10000,0x80000))),"Japanese base keeps standalone kanji");
        require("[80000]日本語を読む |[10000]Windows|".equals(show(
            AutoLanguage.segments("日本語を読む Windows",0x10000,0x60000))),"Kana context routes kanji to Japanese");
        require("[60001]繁體中文 |[10000]Windows|".equals(show(
            AutoLanguage.segments("繁體中文 Windows",0x10000,0x60001))),"Taiwanese Chinese base retains dialect");
        require("auto-Reed".equals(AutoLanguage.voice(0)),"voice name");
        require(AutoLanguage.isVoice("auto-Bobby"),"isVoice yes");
        require(!AutoLanguage.isVoice("en-US-Reed"),"isVoice no");
        require(!AutoLanguage.isVoice(null),"isVoice null");
        require("[60000]你好 |[10000]at |[60000]世界|".equals(show(
            AutoLanguage.segments(NumberReading.prepare("你好@世界",NumberReading.AUTO,0x60000),
                0x10000,0x60000))),"expanded at uses English dialect");
        require("[60000] |[10000]at |".equals(show(
            AutoLanguage.segments(NumberReading.prepare("@",NumberReading.AUTO,0x60000),
                0x10000,0x60000))),"standalone at uses English dialect");
        System.out.println("AutoLanguageTest: "+checks+" checks passed");
    }
}
