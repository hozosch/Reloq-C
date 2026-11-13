package org.reloq.tts;

import android.content.Context;
import android.widget.Toast;

public final class DictionaryFeedbackTest {
    private static void require(boolean condition) {if(!condition)throw new AssertionError();}
    public static void main(String[] args) {
        Context context=new Context();
        DictionaryData.UpdateSummary changed=new DictionaryData.UpdateSummary(true,true,3,12,1,4);
        String text=DictionaryUpdateFeedback.message(context,changed);
        require(text.equals("New entries: 3\nNew files: 1"));
        DictionaryUpdateFeedback.toast(context,changed);
        require(Toast.shown==1&&Toast.duration==Toast.LENGTH_LONG&&Toast.text.equals(text));
        DictionaryUpdateFeedback.toast(context,null);
        require(Toast.shown==1);
        DictionaryData.UpdateSummary first=new DictionaryData.UpdateSummary(true,false,9,9,2,2);
        require(DictionaryUpdateFeedback.message(context,first).equals("First entries: 9\nTotal files: 2"));
        require(DictionaryUpdateFeedback.message(context,DictionaryData.UpdateSummary.unknown())
                .equals("Unknown entries\nUnknown files"));
        System.out.println("Dictionary toast: counts, first download and unknown result passed");
    }
}
