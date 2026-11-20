package org.reloq.tts;

final class NativeEngine {
    static { System.loadLibrary("reloq_android"); }
    private NativeEngine() {}
    static native long create();
    // The JNI transport calls write(byte[]) and isCancelled() on the sink by
    // name, so any object with those methods works; PcmQueue is one option.
    private static native boolean synthesizeImpl(long context, int dialect, int[] controls,
                                                  byte[] utf8, Object sink, boolean guarded);
    static boolean synthesize(long context,int dialect,int[] controls,byte[] utf8,Object sink) {
        return synthesizeImpl(context,dialect,controls,utf8,sink,false);
    }
    static boolean synthesize(long context,int dialect,int[] controls,byte[] utf8,Object sink,boolean guarded) {
        return synthesizeImpl(context,dialect,controls,utf8,sink,guarded);
    }
    static native void stop(long context, PcmQueue sink);
    static native void dictionary(long context,String path);
    static native void destroy(long context);
}