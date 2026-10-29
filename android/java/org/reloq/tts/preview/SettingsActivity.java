package org.reloq.tts.preview;

import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.speech.tts.TextToSpeech;
import android.speech.tts.Voice;
import android.view.View;
import android.widget.*;

/** Standard Android controls with explicit labels for TalkBack. */
public final class SettingsActivity extends Activity {
    private TextToSpeech tts;
    private boolean ready;
    private LinearLayout content;
    private Spinner languages,profiles,rates,pauses;
    private EditText text;
    private TextView status;
    private SharedPreferences preferences;

    private Spinner select(int label,Object[] values,int selected) {
        TextView caption=new TextView(this); caption.setText(label);
        Spinner control=new Spinner(this); control.setId(View.generateViewId());
        caption.setLabelFor(control.getId()); content.addView(caption);
        ArrayAdapter<Object> adapter=new ArrayAdapter<>(this,android.R.layout.simple_spinner_item,values);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        control.setAdapter(adapter); control.setSelection(Math.max(0,Math.min(values.length-1,selected)));
        content.addView(control);
        return control;
    }
    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        preferences=getSharedPreferences("settings",MODE_PRIVATE);
        ScrollView scroll=new ScrollView(this);
        content=new LinearLayout(this); content.setOrientation(LinearLayout.VERTICAL);
        int padding=(int)(16*getResources().getDisplayMetrics().density);
        content.setPadding(padding,padding,padding,padding); scroll.addView(content); setContentView(scroll);
        scroll.setOnApplyWindowInsetsListener((view,insets) -> {
            content.setPadding(padding+insets.getSystemWindowInsetLeft(),
                padding+insets.getSystemWindowInsetTop(),padding+insets.getSystemWindowInsetRight(),
                padding+insets.getSystemWindowInsetBottom());
            return insets.consumeSystemWindowInsets();
        });
        scroll.requestApplyInsets();
        TextView intro=new TextView(this); intro.setText(R.string.preview_help); content.addView(intro);
        Catalog.Language system=Catalog.find(java.util.Locale.getDefault().getLanguage(),
            java.util.Locale.getDefault().getCountry());
        int first=3; for(int i=0;i<Catalog.LANGUAGES.length;++i) if(Catalog.LANGUAGES[i]==system) first=i;
        languages=select(R.string.language,Catalog.LANGUAGES,preferences.getInt("language",first));
        profiles=select(R.string.profile,Catalog.PROFILES,preferences.getInt("profile",0));
        rates=select(R.string.sample_rate,new String[]{"8 kHz","11.025 kHz","16 kHz","22.05 kHz",
            "32 kHz","44.1 kHz","48 kHz"},preferences.getInt("rate",1));
        pauses=select(R.string.pause_mode,getResources().getStringArray(R.array.pause_modes),
            preferences.getInt("pauses",0));
        TextView caption=new TextView(this); caption.setText(R.string.sample_text);
        text=new EditText(this); text.setId(View.generateViewId()); caption.setLabelFor(text.getId());
        text.setMinLines(3); text.setGravity(android.view.Gravity.TOP);
        text.setText(Catalog.LANGUAGES[languages.getSelectedItemPosition()].sample);
        content.addView(caption); content.addView(text);
        languages.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            public void onNothingSelected(AdapterView<?> parent) {}
            public void onItemSelected(AdapterView<?> parent,View view,int position,long id) {
                text.setText(Catalog.LANGUAGES[position].sample);
            }
        });
        Button save=new Button(this); save.setText(R.string.save); content.addView(save);
        save.setOnClickListener(v -> { save(); status.setText(R.string.saved); });
        Button speak=new Button(this); speak.setText(R.string.speak); content.addView(speak);
        speak.setOnClickListener(v -> {
            save();
            if(!ready) { status.setText(R.string.initializing); return; }
            Catalog.Language item=Catalog.LANGUAGES[languages.getSelectedItemPosition()];
            String name=item.voice(profiles.getSelectedItemPosition());
            for(Voice voice:tts.getVoices()) if(name.equals(voice.getName())) { tts.setVoice(voice); break; }
            tts.speak(text.getText(),TextToSpeech.QUEUE_FLUSH,null,"reloq-preview");
        });
        Button stop=new Button(this); stop.setText(R.string.stop); content.addView(stop);
        stop.setOnClickListener(v -> { if(tts!=null) tts.stop(); });
        Button settings=new Button(this); settings.setText(R.string.system_settings); content.addView(settings);
        settings.setOnClickListener(v -> {
            try { startActivity(new Intent("com.android.settings.TTS_SETTINGS")); }
            catch(android.content.ActivityNotFoundException error) { startActivity(new Intent(android.provider.Settings.ACTION_ACCESSIBILITY_SETTINGS)); }
        });
        status=new TextView(this); status.setAccessibilityLiveRegion(View.ACCESSIBILITY_LIVE_REGION_POLITE);
        status.setText(R.string.initializing); content.addView(status);
        tts=new TextToSpeech(this,result -> runOnUiThread(() -> {
            ready=result==TextToSpeech.SUCCESS;
            status.setText(ready?R.string.ready:R.string.unavailable);
        }),getPackageName());
    }
    private void save() {
        preferences.edit().putInt("language",languages.getSelectedItemPosition())
            .putInt("profile",profiles.getSelectedItemPosition()).putInt("rate",rates.getSelectedItemPosition())
            .putInt("pauses",pauses.getSelectedItemPosition()).apply();
    }
    @Override public void onDestroy() { if(tts!=null) { tts.stop(); tts.shutdown(); } super.onDestroy(); }
}
