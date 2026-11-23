package org.reloq.tts;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public final class VolumeGainTest {
    public static void main(String[] arguments) {
        short[] source={100,30000,-30000,0};
        byte[] block=new byte[source.length*2];
        ByteBuffer.wrap(block).order(ByteOrder.LITTLE_ENDIAN).asShortBuffer().put(source);
        VolumeGain.apply(block);
        short[] result=new short[source.length];
        ByteBuffer.wrap(block).order(ByteOrder.LITTLE_ENDIAN).asShortBuffer().get(result);
        short[] expected={130,32767,-32768,0};
        for(int i=0;i<expected.length;++i)if(result[i]!=expected[i])throw new AssertionError(i+" "+result[i]);
        System.out.println("VolumeGain: OK");
    }
}
