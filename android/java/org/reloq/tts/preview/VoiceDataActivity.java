package org.reloq.tts.preview;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.speech.tts.TextToSpeech;
import java.util.ArrayList;

/** All language data is installed with the APK; no network installer. */
public final class VoiceDataActivity extends Activity {
    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        Intent result=new Intent();
        if(TextToSpeech.Engine.ACTION_GET_SAMPLE_TEXT.equals(getIntent().getAction())) {
            Catalog.Language item=Catalog.find(getIntent().getStringExtra("language"),
                getIntent().getStringExtra("country"));
            result.putExtra(TextToSpeech.Engine.EXTRA_SAMPLE_TEXT,
                (item==null?Catalog.LANGUAGES[3]:item).sample);
            setResult(TextToSpeech.LANG_AVAILABLE,result);
        } else {
            ArrayList<String> available=new ArrayList<>();
            for(Catalog.Language item:Catalog.LANGUAGES)
                available.add(item.locale.getISO3Language()+"-"+item.locale.getISO3Country());
            result.putStringArrayListExtra(TextToSpeech.Engine.EXTRA_AVAILABLE_VOICES,available);
            result.putStringArrayListExtra(TextToSpeech.Engine.EXTRA_UNAVAILABLE_VOICES,new ArrayList<>());
            setResult(TextToSpeech.Engine.CHECK_VOICE_DATA_PASS,result);
        }
        finish();
    }
}
