"""Run the real scheduler with host Android stubs: no device or network needed."""
import argparse
from pathlib import Path
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
STUBS={
'android/Manifest.java': '''package android; public class Manifest {public static class permission {public static final String POST_NOTIFICATIONS="post";}}''',
'android/R.java': '''package android; public class R {public static class drawable {public static final int stat_sys_download_done=1;}}''',
'android/os/Build.java': '''package android.os; public class Build {public static class VERSION {public static int SDK_INT=35;}}''',
'android/content/pm/PackageManager.java': '''package android.content.pm; public class PackageManager {public static final int PERMISSION_GRANTED=0;}''',
'android/content/DialogInterface.java': '''package android.content; public interface DialogInterface {interface OnClickListener {void onClick(DialogInterface d,int w);}}''',
'android/content/ComponentName.java': '''package android.content; public class ComponentName {public ComponentName(Context c,Class<?> x){}}''',
'android/content/Intent.java': '''package android.content; public class Intent {public Intent(String s){} public Intent(Context c,Class<?> x){} public Intent putExtra(String s,boolean b){return this;} public Intent putExtra(String s,String v){return this;}}''',
'android/provider/Settings.java': '''package android.provider; public class Settings {public static final String ACTION_APP_NOTIFICATION_SETTINGS="settings",EXTRA_APP_PACKAGE="package";}''',
'android/content/SharedPreferences.java': '''package android.content; import java.util.*; public class SharedPreferences {
 private final Map<String,Object> v=new HashMap<>();
 public boolean getBoolean(String k,boolean d){return (Boolean)v.getOrDefault(k,d);}
 public long getLong(String k,long d){return (Long)v.getOrDefault(k,d);}
 public String getString(String k,String d){return (String)v.getOrDefault(k,d);}
 public Editor edit(){return new Editor();} public class Editor {
 public Editor putBoolean(String k,boolean x){v.put(k,x);return this;}
 public Editor putLong(String k,long x){v.put(k,x);return this;}
 public Editor putString(String k,String x){v.put(k,x);return this;} public void apply(){} }}''',
'android/content/Context.java': '''package android.content; import android.app.*; import android.app.job.*; public class Context {
 public static final int MODE_PRIVATE=0;public static final String JOB_SCHEDULER_SERVICE="jobs",NOTIFICATION_SERVICE="notifications";
 public boolean locked,permission;public final SharedPreferences prefs=new SharedPreferences();
 public final JobScheduler jobs=new JobScheduler();public final NotificationManager notifications=new NotificationManager();
 public Object getSystemService(String name){return name.equals(JOB_SCHEDULER_SERVICE)?jobs:notifications;}
 public SharedPreferences getSharedPreferences(String n,int m){return prefs;}
 public int checkSelfPermission(String p){return permission?0:-1;} public String getPackageName(){return "org.reloq.tts";}
 public String getString(int id,Object... args){return Integer.toString(id);}}''',
'android/app/Activity.java': '''package android.app; public class Activity extends android.content.Context {
 public void requestPermissions(String[] p,int r){} public boolean shouldShowRequestPermissionRationale(String p){return false;}
 public void startActivity(android.content.Intent i){} }''',
'android/app/AlertDialog.java': '''package android.app; public class AlertDialog {public static class Builder {
 public Builder(Activity a){} public Builder setTitle(int id){return this;}public Builder setMessage(int id){return this;}
 public Builder setPositiveButton(int id,android.content.DialogInterface.OnClickListener x){return this;}
 public Builder setNegativeButton(int id,android.content.DialogInterface.OnClickListener x){return this;}public void show(){} }}''',
'android/app/NotificationChannel.java': '''package android.app; public class NotificationChannel {public NotificationChannel(String id,String name,int importance){}}''',
'android/app/PendingIntent.java': '''package android.app; import android.content.*;public class PendingIntent {
 public static final int FLAG_UPDATE_CURRENT=1,FLAG_IMMUTABLE=2;
 public static PendingIntent getActivity(Context c,int i,Intent n,int f){return new PendingIntent();}}''',
'android/app/Notification.java': '''package android.app;import android.content.*;public class Notification {public static class Builder {
 public Builder(Context c,String id){}public Builder setSmallIcon(int v){return this;}public Builder setContentTitle(String v){return this;}
 public Builder setContentText(String v){return this;}public Builder setContentIntent(PendingIntent v){return this;}
 public Builder setAutoCancel(boolean v){return this;}public Notification build(){return new Notification();}}}''',
'android/app/NotificationManager.java': '''package android.app;public class NotificationManager {public static final int IMPORTANCE_DEFAULT=3;
 public int count;public boolean enabled=true;public boolean areNotificationsEnabled(){return enabled;}
 public void createNotificationChannel(NotificationChannel c){}public void notify(int id,Notification n){++count;}}''',
'android/app/job/JobInfo.java': '''package android.app.job;import android.content.*;public class JobInfo {
 public static final int NETWORK_TYPE_ANY=1;public int id,network;public long period,latency;public boolean persisted;
 public static class Builder {private final JobInfo job=new JobInfo();public Builder(int id,ComponentName c){job.id=id;}
 public Builder setRequiredNetworkType(int n){job.network=n;return this;}public Builder setPeriodic(long n){job.period=n;return this;}
 public Builder setPersisted(boolean b){job.persisted=b;return this;}public Builder setMinimumLatency(long n){job.latency=n;return this;}
 public JobInfo build(){return job;}}}''',
'android/app/job/JobScheduler.java': '''package android.app.job;import java.util.*;public class JobScheduler {
 public static final int RESULT_SUCCESS=1;public final Map<Integer,JobInfo> jobs=new HashMap<>();
 public int schedule(JobInfo j){jobs.put(j.id,j);return RESULT_SUCCESS;}public void cancel(int id){jobs.remove(id);}
 public JobInfo getPendingJob(int id){return jobs.get(id);}}''',
'org/reloq/tts/BootPreferences.java': '''package org.reloq.tts;final class BootPreferences {static boolean unlocked(android.content.Context c){return !c.locked;}}''',
'org/reloq/tts/UpdateJob.java': '''package org.reloq.tts; public class UpdateJob {}''',
'org/reloq/tts/UpdateActivity.java': '''package org.reloq.tts; public class UpdateActivity {}''',
'org/reloq/tts/R.java': '''package org.reloq.tts;public class R {public static class string {public static final int app_updates=1,notification_explanation=2,allow_notifications=3,not_now=4,update_notification_title=5,update_notification_body=6;}}''',
'org/reloq/tts/SchedulerTest.java': '''package org.reloq.tts; import android.content.*;import android.app.job.*;
public class SchedulerTest {static int checks;static void check(boolean v){if(!v)throw new AssertionError("check "+checks);++checks;}
 public static void main(String[] args){Context c=new Context();c.locked=true;UpdateScheduler.schedule(c);check(c.jobs.jobs.isEmpty());
 c.locked=false;UpdateScheduler.schedule(c);check(c.jobs.jobs.size()==2);
 JobInfo periodic=c.jobs.getPendingJob(UpdateScheduler.PERIODIC),first=c.jobs.getPendingJob(UpdateScheduler.FIRST);
 check(periodic.persisted&&periodic.network==JobInfo.NETWORK_TYPE_ANY&&periodic.period==86400000L);
 check(!first.persisted&&first.network==JobInfo.NETWORK_TYPE_ANY&&first.latency==60000);
 UpdateScheduler.schedule(c);check(c.jobs.jobs.size()==2);
 UpdateData.Package one=new UpdateData.Package("n","1.3.1","u","h",11030001,1);
 UpdateScheduler.notifyUpdate(c,one);check(c.notifications.count==0);check(c.prefs.getString("notifiedVersion","").isEmpty());
 c.permission=true;UpdateScheduler.notifyUpdate(c,one);check(c.notifications.count==1);
 UpdateScheduler.notifyUpdate(c,one);check(c.notifications.count==1);
 UpdateData.Package two=new UpdateData.Package("n","1.3.2","u","h",11030002,1);
 UpdateScheduler.notifyUpdate(c,two);check(c.notifications.count==2);
 c.prefs.edit().putBoolean("automatic",false).apply();UpdateScheduler.schedule(c);check(c.jobs.jobs.isEmpty());
 UpdateScheduler.notifyUpdate(c,one);check(c.notifications.count==2);
 c.prefs.edit().putBoolean("automatic",true).putLong("firstScheduled",0).apply();UpdateScheduler.schedule(c);
 check(c.jobs.jobs.size()==2);c.notifications.enabled=false;check(!UpdateScheduler.notificationsEnabled(c));
 c.notifications.enabled=true;c.permission=false;check(!UpdateScheduler.notificationsEnabled(c));
 android.os.Build.VERSION.SDK_INT=32;check(UpdateScheduler.notificationsEnabled(c));
 System.out.println("Android automatic updates: "+checks+" scheduling, boot, opt-out, permission and notification checks passed");}}'''
}

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--java-home',type=Path,required=True);args=parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='reloq-update-test-') as tmp:
        root=Path(tmp)
        for path,content in STUBS.items():
            target=root/path;target.parent.mkdir(parents=True,exist_ok=True);target.write_text(content)
        actual=ROOT/'android/java/org/reloq/tts'
        sources=list(root.rglob('*.java'))+[actual/'UpdateData.java',actual/'UpdateScheduler.java']
        subprocess.run([str(args.java_home/'bin/javac'),'-encoding','UTF-8','-d',str(root),*map(str,sources)],check=True)
        subprocess.run([str(args.java_home/'bin/java'),'-cp',str(root),'org.reloq.tts.SchedulerTest'],check=True,timeout=30)

if __name__=='__main__':main()
