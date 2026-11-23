package org.reloq.tts;

import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.util.List;

/** Run with app_process and the APK's JNI library; capture PCM in RAM only. */
public final class AtRoutingProbe {
    public static final class Sink {
        final MessageDigest digest;
        int bytes;
        Sink() throws Exception {digest=MessageDigest.getInstance("SHA-256");}
        public boolean write(byte[] block) {digest.update(block);bytes+=block.length;return true;}
        public boolean isCancelled() {return false;}
        String result() {
            StringBuilder hash=new StringBuilder();
            for(byte b:digest.digest())hash.append(String.format("%02x",b&255));
            return bytes+":"+hash;
        }
    }
    private static String render(int dialect,String text) throws Exception {
        long context=NativeEngine.create();
        if(context==0)throw new AssertionError("Native context unavailable");
        try {
            Sink sink=new Sink();
            int[] controls=new VoiceSettings(0).controls(100,100,dialect);
            if(!NativeEngine.synthesize(context,dialect,controls,
                    text.getBytes(StandardCharsets.UTF_8),sink)||sink.bytes==0)
                throw new AssertionError("Synthesis failed for "+Integer.toHexString(dialect));
            return sink.result();
        } finally {NativeEngine.destroy(context);}
    }
    public static void main(String[] args) throws Exception {
        String prepared=NumberReading.prepare("\u4f60\u597d@\u4e16\u754c",NumberReading.AUTO,0x60000);
        List<AutoLanguage.Segment> segments=AutoLanguage.segments(prepared,0x10000,0x60000);
        if(segments.size()!=3||segments.get(0).dialect!=0x60000||
                segments.get(1).dialect!=0x10000||segments.get(2).dialect!=0x60000||
                !segments.get(1).text.trim().equals("at"))
            throw new AssertionError("At-sign language routing failed");
        AutoLanguage.Segment at=segments.get(1);
        String routed=render(at.dialect,at.text);
        String english=render(0x10000,"at ");
        String chinese=render(0x60000,"at ");
        if(!routed.equals(english)||routed.equals(chinese))
            throw new AssertionError("Routed at does not match English PCM");
        System.out.println("ROUTE: 60000 -> 10000(at) -> 60000");
        System.out.println("ROUTED_ENGLISH_PCM: "+routed);
        System.out.println("CHINESE_PCM: "+chinese);
        System.out.println("AtRoutingProbe: English PCM match passed; no audio playback");
    }
}
