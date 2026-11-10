package org.reloq.tts;

import android.app.Activity;
import android.content.*;
import android.content.pm.*;
import android.net.Uri;
import android.os.*;
import android.widget.*;

/** User-requested release checks; the browser owns downloads and installation. */
public final class UpdateActivity extends Activity {
    private TextView status;
    private Button action;
    private UpdateData.Package available;
    private long installed;
    private volatile boolean closed;
    private Thread worker;

    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        LinearLayout column=new LinearLayout(this);column.setOrientation(LinearLayout.VERTICAL);
        int p=(int)(16*getResources().getDisplayMetrics().density);column.setPadding(p,p,p,p);
        ScrollView scroll=new ScrollView(this);scroll.addView(column);setContentView(scroll);
        scroll.setOnApplyWindowInsetsListener((view,insets) -> {
            column.setPadding(p+insets.getSystemWindowInsetLeft(),p+insets.getSystemWindowInsetTop(),
                p+insets.getSystemWindowInsetRight(),p+insets.getSystemWindowInsetBottom());
            return insets.consumeSystemWindowInsets();
        });scroll.requestApplyInsets();
        TextView heading=new TextView(this);heading.setText(R.string.app_updates);heading.setTextSize(22);
        if(Build.VERSION.SDK_INT>=28)heading.setAccessibilityHeading(true);column.addView(heading);
        CheckBox automatic=new CheckBox(this);automatic.setText(R.string.automatic_updates);
        automatic.setChecked(UpdateScheduler.enabled(this));column.addView(automatic);
        automatic.setOnCheckedChangeListener((button,enabled) -> {
            getSharedPreferences(UpdateScheduler.PREFS,MODE_PRIVATE).edit().putBoolean("automatic",enabled).apply();
            UpdateScheduler.schedule(this);
            if(enabled)UpdateScheduler.requestNotificationsOnce(this);
        });
        Button notifications=new Button(this);notifications.setText(R.string.notification_settings);
        notifications.setOnClickListener(v -> UpdateScheduler.notificationSettings(this));column.addView(notifications);
        status=new TextView(this);status.setAccessibilityLiveRegion(android.view.View.ACCESSIBILITY_LIVE_REGION_POLITE);
        column.addView(status);
        action=new Button(this);action.setText(R.string.check_updates);column.addView(action);
        try {
            PackageInfo info=getPackageManager().getPackageInfo(getPackageName(),0);installed=code(info);
            status.setText(getString(R.string.installed_version,info.versionName));
        } catch(Exception e) {status.setText(e.toString());action.setEnabled(false);}
        action.setOnClickListener(v -> {if(available==null)work();else openDownload();});
        UpdateScheduler.schedule(this);
        if(getIntent().getBooleanExtra("autoCheck",false))work();
    }
    private static long code(PackageInfo info) {return Build.VERSION.SDK_INT>=28?info.getLongVersionCode():info.versionCode;}
    private void work() {
        action.setEnabled(false);status.setText(R.string.checking_updates);
        worker=new Thread(() -> {
            try {
                UpdateData.Package found=UpdateClient.check(installed);
                runOnUiThread(() -> {
                    if(closed)return;available=found;action.setEnabled(true);
                    action.setText(found==null?R.string.check_updates:R.string.download_update);
                    status.setText(found==null?getString(R.string.no_published_update):getString(R.string.version_available,found.version));
                });
            } catch(Exception e) {runOnUiThread(() -> {if(!closed){action.setEnabled(true);status.setText(getString(R.string.update_failed,e.getMessage()));}});}
        },"ReLoq package update");worker.start();
    }
    private void openDownload() {
        try {
            Intent intent=new Intent(Intent.ACTION_VIEW,Uri.parse(available.url));
            intent.addCategory(Intent.CATEGORY_BROWSABLE);startActivity(intent);
            status.setText(R.string.browser_update);
        } catch(Exception e) {status.setText(getString(R.string.update_failed,e.getMessage()));}
    }
    @Override public void onDestroy() {closed=true;if(worker!=null)worker.interrupt();super.onDestroy();}
}
