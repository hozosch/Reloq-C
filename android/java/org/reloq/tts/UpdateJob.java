package org.reloq.tts;

import android.app.job.*;
import android.content.pm.PackageInfo;
import android.os.Build;

/** JobScheduler runs this independently of the speech synthesis worker. */
public final class UpdateJob extends JobService {
    private final java.util.concurrent.atomic.AtomicLong serial=new java.util.concurrent.atomic.AtomicLong();
    private final java.util.concurrent.ConcurrentHashMap<Integer,Long> active=new java.util.concurrent.ConcurrentHashMap<>();
    @Override public boolean onStartJob(JobParameters job) {
        final int id=job.getJobId();final long current=serial.incrementAndGet();active.put(id,current);
        new Thread(() -> {
            boolean retry=false;
            try {
                if(UpdateScheduler.enabled(this)) {
                    PackageInfo info=getPackageManager().getPackageInfo(getPackageName(),0);
                    long installed=Build.VERSION.SDK_INT>=28?info.getLongVersionCode():info.versionCode;
                    UpdateData.Package found=UpdateClient.check(installed);
                    if(found!=null&&Long.valueOf(current).equals(active.get(id)))UpdateScheduler.notifyUpdate(this,found);
                }
            } catch(Exception error) {
                android.util.Log.w("ReLoq","Background update check failed",error);retry=true;
            }
            if(active.remove(id,current))jobFinished(job,retry);
        },"ReLoq update check").start();
        return true;
    }
    @Override public boolean onStopJob(JobParameters job) {active.remove(job.getJobId());return true;}
}
