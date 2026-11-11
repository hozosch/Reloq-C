package org.reloq.tts;

final class NativeEngine {
    static { System.loadLibrary("reloq_android"); }
    private NativeEngine() {}
    static native long create();
    private static native boolean synthesizeImpl(long context, int dialect, int[] controls,
                                                  byte[] utf8, PcmQueue sink, boolean guarded);
    static boolean synthesize(long context,int dialect,int[] controls,byte[] utf8,PcmQueue sink) {
        return synthesizeImpl(context,dialect,controls,utf8,sink,false);
    }
    static boolean synthesize(long context,int dialect,int[] controls,byte[] utf8,PcmQueue sink,boolean guarded) {
        return synthesizeImpl(context,dialect,controls,utf8,sink,guarded);
    }
    static native void stop(long context, PcmQueue sink);
    static native void dictionary(long context,String path);
    static native void destroy(long context);
}
