package org.reloq.tts.preview;

import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.util.concurrent.*;

/** Exercise the real JNI bridge/core in RAM on a host JVM. */
public final class NativeTransportTest {
    private static final ExecutorService worker=Executors.newSingleThreadExecutor();
    private static long context;
    private static String dictionary;
    private static void render(int dialect,int profile,String text) throws Exception {
        render(dialect,new VoiceSettings(profile),text);
    }
    private static void render(int dialect,VoiceSettings settings,String text) throws Exception {
        PcmQueue queue=new PcmQueue(4);
        int[] controls=settings.controls(100,100);
        Future<?> producer=worker.submit(() -> {
            boolean ok=NativeEngine.synthesize(context,dialect,controls,
                text.getBytes(StandardCharsets.UTF_8),queue);
            queue.finish(ok);
        });
        MessageDigest digest=MessageDigest.getInstance("SHA-256");
        byte[] block;int bytes=0;
        while((block=queue.read())!=null) { digest.update(block);bytes+=block.length; }
        producer.get(10,TimeUnit.SECONDS);
        if(bytes==0 || queue.hasFailed()) throw new AssertionError("Empty or failed JNI output: "+dialect);
        StringBuilder hash=new StringBuilder();for(byte b:digest.digest()) hash.append(String.format("%02x",b&255));
        StringBuilder parameters=new StringBuilder();for(int n:controls){if(parameters.length()!=0)parameters.append(',');parameters.append(n);}
        System.out.println(dialect+"\t"+parameters+"\t"+bytes+"\t"+hash+"\t"+(dictionary==null?"-":dictionary)+"\t"+text);
    }
    private static void dictionary(String path) throws Exception {
        worker.submit(() -> NativeEngine.dictionary(context,path)).get(10,TimeUnit.SECONDS);dictionary=path;
    }
    public static void main(String[] arguments) throws Exception {
        try {
            context=worker.submit(() -> NativeEngine.create()).get(10,TimeUnit.SECONDS);
            if(context==0) throw new AssertionError("Context creation failed");
            for(Catalog.Language language:Catalog.LANGUAGES) render(language.id,0,language.sample);
            for(int profile=0;profile<8;++profile) render(0x10000,profile,"This is Eloquence and NVDA, with 504 and 4004.");
            for(int dialect:new int[]{0x60000,0xa0000,0x80000})
                render(dialect,0,"Müller café naïve. 📞");
            PcmQueue cancelled=new PcmQueue(1);
            StringBuilder longText=new StringBuilder();
            for(int i=0;i<80;++i) longText.append("This is a long speech test with numbers 504 and 4004. ");
            Future<Boolean> producer=worker.submit(() -> NativeEngine.synthesize(context,0x10000,
                Catalog.controls(0,100,100,1,0),longText.toString().getBytes(StandardCharsets.UTF_8),cancelled));
            if(cancelled.read()==null) throw new AssertionError("Missing pre-cancel output");
            // An old request's delayed stop must not cancel a newer queue.
            NativeEngine.stop(context,new PcmQueue(1));
            for(int i=0;i<4;++i)
                if(cancelled.read()==null) throw new AssertionError("Stale stop cancelled current output");
            cancelled.cancel();NativeEngine.stop(context,cancelled);
            if(producer.get(5,TimeUnit.SECONDS)) throw new AssertionError("Cancelled output succeeded");
            render(0x10000,0,"Speech after cancellation.");
            VoiceSettings custom=new VoiceSettings(0);
            custom.androidRate=false;custom.manualWpm=280;custom.androidPitch=false;custom.manualPitch=43;
            custom.inflection=65;custom.volume=80;custom.head=22;custom.gender=1;custom.roughness=12;custom.breathiness=60;
            custom.rate=6;custom.pauses=2;custom.tags=true;
            render(0x10000,custom,"This is `vb70 a test `vb43 with 504 and 4004.");
            render(0x40000,custom,"Dies ist eine Sprachprobe mit Eloquence.");
            render(0x10000,0,"Defaults after customized speech.");
            custom.abbreviations=false;custom.prediction=true;
            render(0x10000,custom,"Dr. Smith works at St. James with Mrs. Jones.");
            render(0x40000,custom,"Der Doktor sagt dass wir heute mit dem Zug nach Berlin fahren.");
            dictionary(arguments[0]);render(0x10000,0,"This is cancellationtoken.");
            render(0x40000,0,"Ich bin TokenX dir.");
            dictionary(arguments[1]);render(0x10000,0,"This is cancellationtoken.");
            dictionary(null);render(0x10000,0,"This is cancellationtoken.");
        } finally {
            worker.submit(() -> NativeEngine.destroy(context)).get(10,TimeUnit.SECONDS);
            worker.shutdown();
        }
    }
}
