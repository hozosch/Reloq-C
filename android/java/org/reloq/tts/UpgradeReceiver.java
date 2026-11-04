package org.reloq.tts;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

/** Populate the direct-boot voice copy when an earlier installation upgrades. */
public final class UpgradeReceiver extends BroadcastReceiver {
    @Override public void onReceive(Context context,Intent intent) {
        if(!Intent.ACTION_MY_PACKAGE_REPLACED.equals(intent.getAction())||!BootPreferences.unlocked(context))return;
        BootPreferences.refresh(context);
        PendingResult pending=goAsync();
        Context app=context.getApplicationContext();
        DictionaryStore.WORK.execute(() -> {
            try{DictionaryStore.syncBoot(app);}
            catch(Exception error){android.util.Log.e("ReLoq","Direct Boot dictionary migration failed",error);}
            finally{pending.finish();}
        });
    }
}
