package org.reloq.tts;

import android.content.Context;
import android.widget.Toast;

/** Shared accessible text for manual results and transient automatic updates. */
final class DictionaryUpdateFeedback {
    static String message(Context context,DictionaryData.UpdateSummary summary) {
        boolean first=summary.known&&!summary.compared&&(summary.total>0||summary.totalFiles>0);
        String message=!summary.known?context.getString(R.string.dictionary_update_unknown):
            first?context.getString(R.string.dictionary_update_first,summary.total):
            context.getString(R.string.dictionary_update_new,summary.added);
        return message+"\n"+(!summary.known?context.getString(R.string.dictionary_update_files_unknown):
            first?context.getString(R.string.dictionary_update_files_total,summary.totalFiles):
            context.getString(R.string.dictionary_update_files_new,summary.addedFiles));
    }
    static void toast(Context context,DictionaryData.UpdateSummary summary) {
        if(summary!=null)Toast.makeText(context,message(context,summary),Toast.LENGTH_LONG).show();
    }
}
