package org.reloq.tts;

import android.Manifest;
import android.app.*;
import android.app.job.*;
import android.content.*;
import android.content.pm.PackageManager;
import android.os.Build;
import android.provider.Settings;

/** Periodic, network-constrained checks; no APK installation permission. */
final class UpdateScheduler {
    static final int PERIODIC=0x524c01, FIRST=0x524c02;
    static final String PREFS="updates", CHANNEL="reloq_app_updates";
    private static final long DAY=24L*60*60*1000;

    static boolean enabled(Context context) {
        return context.getSharedPreferences(PREFS,Context.MODE_PRIVATE).getBoolean("automatic",true);
    }
    static void schedule(Context context) {
        if(!BootPreferences.unlocked(context))return;
        JobScheduler jobs=(JobScheduler)context.getSystemService(Context.JOB_SCHEDULER_SERVICE);
        if(!enabled(context)) {jobs.cancel(PERIODIC);jobs.cancel(FIRST);return;}
        ComponentName component=new ComponentName(context,UpdateJob.class);
        if(jobs.getPendingJob(PERIODIC)==null) {
            jobs.schedule(new JobInfo.Builder(PERIODIC,component).setRequiredNetworkType(JobInfo.NETWORK_TYPE_ANY)
                .setPeriodic(DAY).setPersisted(true).build());
        }
        android.content.SharedPreferences p=context.getSharedPreferences(PREFS,Context.MODE_PRIVATE);
        long now=System.currentTimeMillis(),last=p.getLong("firstScheduled",0);
        if((last>now||now-last>=DAY)&&jobs.getPendingJob(FIRST)==null) {
            JobInfo first=new JobInfo.Builder(FIRST,component).setRequiredNetworkType(JobInfo.NETWORK_TYPE_ANY)
                .setMinimumLatency(60000).build();
            if(jobs.schedule(first)==JobScheduler.RESULT_SUCCESS)p.edit().putLong("firstScheduled",now).apply();
        }
    }
    static boolean notificationsEnabled(Context context) {
        NotificationManager manager=(NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE);
        return manager.areNotificationsEnabled() && (Build.VERSION.SDK_INT<33 ||
            context.checkSelfPermission(Manifest.permission.POST_NOTIFICATIONS)==PackageManager.PERMISSION_GRANTED);
    }
    static void requestNotificationsOnce(Activity activity) {
        if(Build.VERSION.SDK_INT<33||notificationsEnabled(activity))return;
        android.content.SharedPreferences p=activity.getSharedPreferences(PREFS,Context.MODE_PRIVATE);
        if(p.getBoolean("notificationPrompted",false))return;
        p.edit().putBoolean("notificationPrompted",true).apply();
        new AlertDialog.Builder(activity).setTitle(R.string.app_updates)
            .setMessage(R.string.notification_explanation)
            .setPositiveButton(R.string.allow_notifications,(dialog,which) ->
                activity.requestPermissions(new String[]{Manifest.permission.POST_NOTIFICATIONS},701))
            .setNegativeButton(R.string.not_now,null).show();
    }
    static void notificationSettings(Activity activity) {
        if(Build.VERSION.SDK_INT>=33 && activity.checkSelfPermission(Manifest.permission.POST_NOTIFICATIONS)
                !=PackageManager.PERMISSION_GRANTED && activity.shouldShowRequestPermissionRationale(Manifest.permission.POST_NOTIFICATIONS)) {
            activity.requestPermissions(new String[]{Manifest.permission.POST_NOTIFICATIONS},701);return;
        }
        Intent intent=new Intent(Settings.ACTION_APP_NOTIFICATION_SETTINGS)
            .putExtra(Settings.EXTRA_APP_PACKAGE,activity.getPackageName());
        activity.startActivity(intent);
    }
    static synchronized void notifyUpdate(Context context,UpdateData.Package update) {
        if(!enabled(context)||!notificationsEnabled(context))return;
        android.content.SharedPreferences p=context.getSharedPreferences(PREFS,Context.MODE_PRIVATE);
        if(update.version.equals(p.getString("notifiedVersion","")))return;
        NotificationManager manager=(NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE);
        manager.createNotificationChannel(new NotificationChannel(CHANNEL,
            context.getString(R.string.update_notification_title),NotificationManager.IMPORTANCE_DEFAULT));
        Intent open=new Intent(context,UpdateActivity.class).putExtra("autoCheck",true);
        PendingIntent tap=PendingIntent.getActivity(context,0,open,
            PendingIntent.FLAG_UPDATE_CURRENT|PendingIntent.FLAG_IMMUTABLE);
        Notification notification=new Notification.Builder(context,CHANNEL)
            .setSmallIcon(android.R.drawable.stat_sys_download_done)
            .setContentTitle(context.getString(R.string.update_notification_title))
            .setContentText(context.getString(R.string.update_notification_body,update.version))
            .setContentIntent(tap).setAutoCancel(true).build();
        manager.notify(PERIODIC,notification);
        p.edit().putString("notifiedVersion",update.version).apply();
    }
}
