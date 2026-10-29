package org.reloq.tts.preview;

import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.util.concurrent.*;

/** Exercise the real JNI bridge/core in RAM on a host JVM. */
public final class NativeTransportTest {
    private static final ExecutorService worker=Executors.newSingleThreadExecutor();
    private static long context;
    private static void render(int dialect,int profile,String text) throws Exception {
        PcmQueue queue=new PcmQueue(4);
        Future<?> producer=worker.submit(() -> {
            boolean ok=NativeEngine.synthesize(context,dialect,Catalog.controls(profile,100,100,1,0),
                text.getBytes(StandardCharsets.UTF_8),queue);
            queue.finish(ok);
        });
        MessageDigest digest=MessageDigest.getInstance("SHA-256");
        byte[] block;int bytes=0;
        while((block=queue.read())!=null) { digest.update(block);bytes+=block.length; }
        producer.get(10,TimeUnit.SECONDS);
        if(bytes==0 || queue.hasFailed()) throw new AssertionError("Empty or failed JNI output: "+dialect);
        StringBuilder hash=new StringBuilder();for(byte b:digest.digest()) hash.append(String.format("%02x",b&255));
        System.out.println(dialect+"\t"+profile+"\t"+bytes+"\t"+hash+"\t"+text);
    }
    public static void main(String[] arguments) throws Exception {
        try {
            context=worker.submit(() -> NativeEngine.create(arguments[0])).get(10,TimeUnit.SECONDS);
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
        } finally {
            worker.submit(() -> NativeEngine.destroy(context)).get(10,TimeUnit.SECONDS);
            worker.shutdown();
        }
    }
}
