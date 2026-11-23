package org.reloq.tts;

/** Inspect text preparation and language routing without modifying app settings. */
public final class NumberReadingProbe {
    public static void main(String[] args) {
        int base=Integer.parseInt(args[0],16);
        for(int i=1;i<args.length;++i) {
            String prepared=NumberReading.prepare(args[i],NumberReading.AUTO,base);
            System.out.println("INPUT: "+args[i]);
            System.out.println("PREPARED: "+prepared);
            for(AutoLanguage.Segment segment:AutoLanguage.segments(prepared,AutoLanguage.latinDialect(base),base))
                System.out.println("ROUTE "+Integer.toHexString(segment.dialect)+": "+segment.text);
        }
    }
}
