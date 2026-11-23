package org.reloq.tts;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/** Matches the Aisound output-side gain: scale 16-bit PCM and saturate. */
final class VolumeGain {
    private static final double FACTOR = 1.3;
    private VolumeGain() {}
    static void apply(byte[] block) {
        if(block==null||block.length<2)return;
        ByteBuffer bytes=ByteBuffer.wrap(block).order(ByteOrder.LITTLE_ENDIAN);
        java.nio.ShortBuffer samples=bytes.asShortBuffer();
        for(int i=0;i<samples.capacity();++i) {
            int value=(int)Math.round(samples.get(i)*FACTOR);
            samples.put(i,(short)Math.max(Short.MIN_VALUE,Math.min(Short.MAX_VALUE,value)));
        }
    }
}
