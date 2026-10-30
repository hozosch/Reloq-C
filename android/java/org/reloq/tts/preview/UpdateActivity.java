package org.reloq.tts.preview;

import android.app.Activity;
import android.content.*;
import android.content.pm.*;
import android.net.Uri;
import android.os.*;
import android.provider.Settings;
import android.widget.*;
import org.json.*;
import java.io.*;
import java.net.*;
import java.util.*;

/** User-requested releases, streamed download, then Android's installer. */
public final class UpdateActivity extends Activity {
    private TextView status;
    private Button action;
    private UpdateData.Package available;
    private boolean downloaded;
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
        status=new TextView(this);status.setAccessibilityLiveRegion(android.view.View.ACCESSIBILITY_LIVE_REGION_POLITE);
        column.addView(status);
        action=new Button(this);action.setText(R.string.check_updates);column.addView(action);
        try {
            PackageInfo info=getPackageManager().getPackageInfo(getPackageName(),0);installed=code(info);
            status.setText(getString(R.string.installed_version,info.versionName));
        } catch(Exception e) {status.setText(e.toString());action.setEnabled(false);}
        action.setOnClickListener(v -> {if(downloaded)install();else work();});
    }
    private static long code(PackageInfo info) {return Build.VERSION.SDK_INT>=28?info.getLongVersionCode():info.versionCode;}
    private URLConnection connection(String address) throws IOException {
        URLConnection c=new URL(address).openConnection();c.setConnectTimeout(15000);c.setReadTimeout(15000);
        c.setRequestProperty("User-Agent","ReLoq-C-updater");return c;
    }
    private UpdateData.Package check() throws Exception {
        ByteArrayOutputStream bytes=new ByteArrayOutputStream();
        try(InputStream input=connection(UpdateData.RELEASES).getInputStream()) {
            byte[] b=new byte[8192];int n;
            while((n=input.read(b))!=-1) {if(bytes.size()+n>2*1024*1024)throw new IOException("Release list too large");bytes.write(b,0,n);}
        }
        JSONArray releases=new JSONArray(bytes.toString("UTF-8"));UpdateData.Package best=null;
        for(int i=0;i<releases.length();++i) {
            JSONObject release=releases.getJSONObject(i);if(release.optBoolean("draft"))continue;
            JSONArray assets=release.optJSONArray("assets");if(assets==null)continue;
            for(int a=0;a<assets.length();++a) {
                JSONObject asset=assets.getJSONObject(a);
                UpdateData.Package next=UpdateData.candidate(asset.optString("name"),
                    asset.optString("browser_download_url"),asset.optLong("size"),asset.optString("digest"),installed);
                if(next!=null&&(best==null||next.code>best.code))best=next;
            }
        }
        return best;
    }
    private void verifyPackage(File file,UpdateData.Package data) throws Exception {
        int flags=Build.VERSION.SDK_INT>=28?PackageManager.GET_SIGNING_CERTIFICATES:PackageManager.GET_SIGNATURES;
        PackageManager pm=getPackageManager();PackageInfo next=pm.getPackageArchiveInfo(file.getPath(),flags);
        PackageInfo current=pm.getPackageInfo(getPackageName(),flags);
        if(next==null||!getPackageName().equals(next.packageName)||code(next)!=data.code||
           code(next)<=installed||!data.version.equals(next.versionName)||!signers(next).equals(signers(current)))
            throw new IOException("Update package identity, version or signing certificate mismatch");
    }
    private Set<String> signers(PackageInfo info) throws Exception {
        Signature[] signatures=Build.VERSION.SDK_INT>=28?info.signingInfo.getApkContentsSigners():info.signatures;
        if(signatures==null||signatures.length==0)throw new IOException("Missing signing certificate");
        Set<String> result=new HashSet<>();
        for(Signature signature:signatures)result.add(signature.toCharsString());return result;
    }
    private void work() {
        action.setEnabled(false);status.setText(available==null?R.string.checking_updates:R.string.downloading_update);
        final UpdateData.Package selected=available;
        worker=new Thread(() -> {
            try {
                UpdateData.Package found=selected==null?check():selected;
                if(selected!=null) {
                    File file=UpdateProvider.packageFile(this);File folder=file.getParentFile();
                    if(!folder.isDirectory()&&!folder.mkdirs())throw new IOException("Cannot create update folder");
                    try(InputStream input=connection(found.url).getInputStream()) {UpdateData.copyVerified(input,file,found);}
                    try {verifyPackage(file,found);}catch(Exception e){file.delete();throw e;}
                }
                runOnUiThread(() -> {
                    if(closed)return;available=found;downloaded=selected!=null;action.setEnabled(true);
                    action.setText(downloaded?R.string.install_update:found==null?R.string.check_updates:R.string.download_update);
                    status.setText(downloaded?getString(R.string.update_ready):found==null?
                        getString(R.string.no_published_update):getString(R.string.version_available,found.version));
                });
            } catch(Exception e) {runOnUiThread(() -> {if(!closed){action.setEnabled(true);status.setText(getString(R.string.update_failed,e.getMessage()));}});}
        },"ReLoq package update");worker.start();
    }
    private void install() {
        if(!getPackageManager().canRequestPackageInstalls()) {
            status.setText(R.string.allow_updates);
            startActivity(new Intent(Settings.ACTION_MANAGE_UNKNOWN_APP_SOURCES,Uri.parse("package:"+getPackageName())));return;
        }
        try {
            verifyPackage(UpdateProvider.packageFile(this),available);
            Uri uri=UpdateProvider.uri(this);
            Intent intent=new Intent(Intent.ACTION_INSTALL_PACKAGE).setDataAndType(uri,"application/vnd.android.package-archive");
            intent.setClipData(ClipData.newRawUri("ReLoq update",uri));
            intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);startActivity(intent);
        } catch(Exception e) {status.setText(getString(R.string.update_failed,e.getMessage()));}
    }
    @Override public void onDestroy() {closed=true;if(worker!=null)worker.interrupt();super.onDestroy();}
}
