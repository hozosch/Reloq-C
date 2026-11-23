package org.reloq.tts;

import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.util.Locale;
import java.util.List;

/** Device-side onset metrics from the actual JNI output, without playback. */
public final class SpeechStartProbe {
    public static final class Sink {
        int samples, firstSignal=-1;
        long started=System.nanoTime(), firstBlock;
        final MessageDigest digest;
        int windowSquares, quietStart=-1;
        final StringBuilder gaps=new StringBuilder();
        Sink() throws Exception {digest=MessageDigest.getInstance("SHA-256");}
        public boolean write(byte[] pcm) {
            if (samples==0) firstBlock=System.nanoTime();
            digest.update(pcm);
            for (int i=0;i<pcm.length;i+=2) {
                int sample=(short)((pcm[i]&255)|(pcm[i+1]<<8));
                if (firstSignal<0&&Math.abs(sample)>=128) firstSignal=samples;
                windowSquares+=(sample*sample)/256;
                ++samples;
                if(samples%240==0) {
                    if(windowSquares<240*64) {
                        if(quietStart<0)quietStart=samples-240;
                    } else closeGap(samples-240);
                    windowSquares=0;
                }
            }
            return true;
        }
        public boolean isCancelled() {return false;}
        void closeGap(int end) {
            if(quietStart>=0&&end-quietStart>=2400)
                gaps.append(quietStart/48).append('-').append(end/48).append("ms ");
            quietStart=-1;
        }
        String hash() {
            StringBuilder out=new StringBuilder();
            for(byte value:digest.digest())out.append(String.format(Locale.ROOT,"%02x",value&255));
            return out.toString();
        }
    }

    private static void compareRoutes(VoiceSettings settings,int base,String text) throws Exception {
        settings.speedWpmMode=false;
        settings.tags=true;
        String prepared=NumberReading.prepare(text,NumberReading.AUTO,base);
        List<AutoLanguage.Segment> segments=AutoLanguage.segments(prepared,AutoLanguage.latinDialect(base),base);
        for(int preview=1;preview>=0;--preview) {
            long context=NativeEngine.create();
            if(context==0)throw new AssertionError("No JNI context");
            try {
              for(int pass=0;pass<2;++pass) {
                java.util.Set<Integer> warmed=new java.util.HashSet<>();
                for(int i=0;i<segments.size();++i) {
                    AutoLanguage.Segment segment=segments.get(i);
                    if(segment.text.trim().isEmpty())continue;
                    int[] controls=settings.controls(100,100,segment.dialect);
                    long before=System.nanoTime();
                    if(preview!=0&&segment.dialect!=base&&warmed.add(segment.dialect)) {
                        String sample=segment.text.substring(0,Math.min(40,segment.text.length()));
                        NativeEngine.synthesize(context,segment.dialect,controls,sample.getBytes(StandardCharsets.UTF_8),new Sink());
                    }
                    if(i+1<segments.size()&&controls[9]==0&&!NumberReading.endsWithPausePunctuation(segment.text))controls[9]=1;
                    Sink sink=new Sink();
                    if(!NativeEngine.synthesize(context,segment.dialect,controls,segment.text.getBytes(StandardCharsets.UTF_8),sink))
                        throw new AssertionError("Synthesis failed");
                    sink.closeGap(sink.samples);
                    System.out.printf(Locale.ROOT,"base=%x preview=%d pass=%d dialect=%x onset=%.1fms firstBlock=%.1fms duration=%.1fms gaps=%s sha256=%s text=%s%n",
                        base,preview,pass,segment.dialect,sink.firstSignal/48.0,(sink.firstBlock-before)/1e6,
                        sink.samples/48.0,sink.gaps,sink.hash(),segment.text);
                }
              }
            } finally {NativeEngine.destroy(context);}
        }
    }

    public static void main(String[] args) throws Exception {
        VoiceSettings settings=new VoiceSettings(0);
        settings.androidRate=false;
        settings.androidPitch=false;
        settings.rate=6;
        settings.acousticClockMask=24608;
        if(args[0].equals("--route")) {
            for(int i=1;i<args.length;++i) {
                compareRoutes(settings,0x10001,args[i]);
                compareRoutes(settings,0x60000,args[i]);
            }
            return;
        }
        int dialect=Integer.parseInt(args[0],16);
        long context=NativeEngine.create();
        if(context==0)throw new AssertionError("No JNI context");
        try {
            for (boolean wpm:new boolean[]{true,false}) for (boolean tags:new boolean[]{true,false}) {
                settings.speedWpmMode=wpm;
                settings.tags=tags;
                int[] controls=settings.controls(100,100,dialect);
                for (int i=1;i<args.length;++i) {
                    Sink sink=new Sink();
                    if(!NativeEngine.synthesize(context,dialect,controls,
                            args[i].getBytes(StandardCharsets.UTF_8),sink)||sink.firstSignal<0)
                        throw new AssertionError("No signal for "+args[i]);
                    System.out.printf(Locale.ROOT,"%s wpm=%s tags=%s onset=%.1fms firstBlock=%.1fms total=%.1fms synth=%.1fms sha256=%s%n",
                        args[i],wpm,tags,sink.firstSignal/48.0,(sink.firstBlock-sink.started)/1e6,
                        sink.samples/48.0,(System.nanoTime()-sink.started)/1e6,sink.hash());
                }
            }
        } finally {NativeEngine.destroy(context);}
    }
}
