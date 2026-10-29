package org.reloq.tts.preview;

import java.util.Arrays;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;

/** Host JVM tests for cancellation, backpressure, order and locale routing. */
public final class PcmQueueTest {
    private static void require(boolean value) { if(!value) throw new AssertionError(); }
    public static void main(String[] arguments) throws Exception {
        PcmQueue queue=new PcmQueue(1);
        require(queue.write(new byte[]{1,2}));
        CountDownLatch entered=new CountDownLatch(1),exited=new CountDownLatch(1);
        AtomicBoolean accepted=new AtomicBoolean(true);
        Thread producer=new Thread(() -> {
            entered.countDown(); accepted.set(queue.write(new byte[]{3,4})); exited.countDown();
        });
        producer.start();require(entered.await(5,TimeUnit.SECONDS));
        require(!exited.await(50,TimeUnit.MILLISECONDS));
        queue.cancel();require(exited.await(5,TimeUnit.SECONDS));producer.join();
        require(!accepted.get() && queue.read()==null && !queue.write(new byte[]{5}));

        PcmQueue waiting=new PcmQueue(1);
        CountDownLatch readerDone=new CountDownLatch(1);
        Thread consumer=new Thread(() -> {
            try { require(waiting.read()==null);readerDone.countDown(); }
            catch(InterruptedException e) { throw new AssertionError(e); }
        });
        consumer.start();waiting.cancel();require(readerDone.await(5,TimeUnit.SECONDS));consumer.join();

        PcmQueue stream=new PcmQueue(2);
        Thread writer=new Thread(() -> {
            for(int i=0;i<1000;++i) require(stream.write(new byte[]{(byte)i,(byte)(i>>8)}));
            stream.finish(true);
        });
        writer.start();
        for(int i=0;i<1000;++i) require(Arrays.equals(stream.read(),new byte[]{(byte)i,(byte)(i>>8)}));
        require(stream.read()==null && !stream.hasFailed());writer.join();
        PcmQueue failure=new PcmQueue(1);failure.write(new byte[]{9});failure.finish(false);
        require(failure.read()[0]==9 && failure.read()==null && failure.hasFailed());
        require(!failure.write(new byte[]{8}));

        require(Catalog.find("eng","USA").id==0x10000);
        require(Catalog.find("eng","").id==0x10000);
        require(Catalog.find("eng","GBR").id==0x10001);
        require(Catalog.find("zho","TWN").id==0x60000);
        require(Catalog.find("kor","KOR").id==0xa0000);
        require(Catalog.find("xx","")==null);
        for(Catalog.Language language:Catalog.LANGUAGES) for(int p=0;p<8;++p) {
            require(Catalog.forVoice(language.voice(p))==language);
            require(Catalog.profile(language.voice(p),0)==p);
        }
        require(Catalog.controls(2,100,100,1,0)[2]==93);
        require(Catalog.controls(0,100,100,6,2)[7]==98);
        require(Catalog.controls(0,100,100,6,2)[8]==6);
        System.out.println("PCM queue: bounded streaming/cancel/finish passed; 112 voice names and locale routing passed");
    }
}
