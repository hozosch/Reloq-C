package org.reloq.tts.preview;

final class NativeEngine {
    static { System.loadLibrary("reloq_android"); }
    private NativeEngine() {}
    static native long create();
    static native boolean synthesize(long context, int dialect, int[] controls,
                                     byte[] utf8, PcmQueue sink);
    static native void stop(long context, PcmQueue sink);
    static native void dictionary(long context,String path);
    static native void destroy(long context);
}
