package org.reloq.tts.preview;

import java.util.ArrayDeque;

/** Bounded handoff. The Android synthesis thread alone owns its callback. */
final class PcmQueue {
    private final int capacity;
    private final ArrayDeque<byte[]> blocks = new ArrayDeque<>();
    private boolean cancelled, finished, failed;

    PcmQueue(int capacity) {
        if (capacity < 1) throw new IllegalArgumentException("capacity");
        this.capacity = capacity;
    }
    synchronized boolean write(byte[] block) {
        while (blocks.size() >= capacity && !cancelled && !finished) {
            try { wait(); }
            catch (InterruptedException e) { Thread.currentThread().interrupt(); cancel(); }
        }
        if (cancelled || finished) return false;
        blocks.add(block);
        notifyAll();
        return true;
    }
    synchronized byte[] read() throws InterruptedException {
        while (blocks.isEmpty() && !cancelled && !finished) wait();
        byte[] result = cancelled ? null : blocks.poll();
        notifyAll();
        return result;
    }
    synchronized void finish(boolean success) {
        failed = !success;
        finished = true;
        notifyAll();
    }
    synchronized void cancel() {
        cancelled = true;
        blocks.clear();
        notifyAll();
    }
    synchronized boolean isCancelled() { return cancelled; }
    synchronized boolean hasFailed() { return failed; }
}
