package org.reloq.tts.preview;

import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.speech.tts.TextToSpeech;
import android.speech.tts.Voice;
import android.view.View;
import android.widget.*;

/** Labelled standard controls; manual overrides are explicit and reversible. */
public final class SettingsActivity extends Activity {
    private TextToSpeech tts;
    private boolean ready;
    private LinearLayout content;
    private Spinner languages,profiles,rates,pauses,models;
    private CheckBox androidVoice,androidRate,androidPitch,boost,tags;
    private Numeric speed,pitch,inflection,volume,head,roughness,breathiness;
    private EditText text;
    private TextView status;
    private SharedPreferences preferences;
    private int loadedProfile;

    private Spinner select(int label,Object[] values,int selected) {
        TextView caption=new TextView(this);caption.setText(label);
        Spinner control=new Spinner(this);control.setId(View.generateViewId());
        caption.setLabelFor(control.getId());content.addView(caption);
        ArrayAdapter<Object> adapter=new ArrayAdapter<>(this,android.R.layout.simple_spinner_item,values);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        control.setAdapter(adapter);control.setSelection(VoiceSettings.bounded(selected,0,values.length-1));
        content.addView(control);return control;
    }
    private CheckBox check(int label,boolean value) {
        CheckBox control=new CheckBox(this);control.setText(label);control.setChecked(value);
        content.addView(control);return control;
    }
    private void heading(int label) {
        TextView view=new TextView(this);view.setText(label);view.setTextSize(20);
        view.setPadding(0,24,0,8);
        if(android.os.Build.VERSION.SDK_INT>=28)view.setAccessibilityHeading(true);
        content.addView(view);
    }
    private final class Numeric {
        final TextView label;
        final SeekBar control;
        final int title;
        Numeric(int title,int value) {
            this(title,value,0,100);
        }
        Numeric(int title,int value,int minimum,int maximum) {
            this.title=title;label=new TextView(SettingsActivity.this);
            control=new SeekBar(SettingsActivity.this);control.setId(View.generateViewId());
            control.setMin(minimum);control.setMax(maximum);label.setLabelFor(control.getId());
            control.setKeyProgressIncrement(title==R.string.speed?5:1);
            control.setContentDescription(getString(title));
            content.addView(label);content.addView(control);
            control.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                public void onStartTrackingTouch(SeekBar bar) {}
                public void onStopTrackingTouch(SeekBar bar) {}
                public void onProgressChanged(SeekBar bar,int progress,boolean user) { describe(); }
            });set(value);
        }
        void describe() {
            label.setText(getString(R.string.parameter_value,getString(title),control.getProgress()));
            if(android.os.Build.VERSION.SDK_INT>=30)control.setStateDescription(title==R.string.speed?
                getString(R.string.wpm_value,control.getProgress()):Integer.toString(control.getProgress()));
        }
        void set(int value) {control.setProgress(VoiceSettings.bounded(value,control.getMin(),control.getMax()));describe();}
        int get() {return control.getProgress();}
        void enabled(boolean value) {control.setEnabled(value);label.setEnabled(value);}
    }
    @Override public void onCreate(Bundle state) {
        super.onCreate(state);preferences=getSharedPreferences("settings",MODE_PRIVATE);
        ScrollView scroll=new ScrollView(this);
        content=new LinearLayout(this);content.setOrientation(LinearLayout.VERTICAL);
        int padding=(int)(16*getResources().getDisplayMetrics().density);
        content.setPadding(padding,padding,padding,padding);scroll.addView(content);setContentView(scroll);
        scroll.setOnApplyWindowInsetsListener((view,insets) -> {
            content.setPadding(padding+insets.getSystemWindowInsetLeft(),padding+insets.getSystemWindowInsetTop(),
                padding+insets.getSystemWindowInsetRight(),padding+insets.getSystemWindowInsetBottom());
            return insets.consumeSystemWindowInsets();
        });scroll.requestApplyInsets();
        TextView intro=new TextView(this);intro.setText(R.string.preview_help);content.addView(intro);
        heading(R.string.voice_settings);
        androidVoice=check(R.string.android_voice,preferences.getBoolean("androidVoice",true));
        Catalog.Language system=Catalog.find(java.util.Locale.getDefault().getLanguage(),java.util.Locale.getDefault().getCountry());
        int first=3;for(int i=0;i<Catalog.LANGUAGES.length;++i)if(Catalog.LANGUAGES[i]==system)first=i;
        languages=select(R.string.language,Catalog.LANGUAGES,preferences.getInt("language",first));
        profiles=select(R.string.profile,Catalog.PROFILES,preferences.getInt("profile",0));
        loadedProfile=profiles.getSelectedItemPosition();
        VoiceSettings settings=Preferences.read(preferences,loadedProfile);
        androidRate=check(R.string.android_rate,settings.androidRate);
        speed=new Numeric(R.string.speed,settings.manualWpm,140,1400);speed.enabled(!settings.androidRate);
        TextView tempoHelp=new TextView(this);tempoHelp.setText(R.string.speed_help);content.addView(tempoHelp);
        androidRate.setOnCheckedChangeListener((button,checked) -> speed.enabled(!checked));
        boost=check(R.string.rate_boost,settings.rateBoost);
        androidPitch=check(R.string.android_pitch,settings.androidPitch);
        pitch=new Numeric(R.string.pitch,settings.manualPitch);pitch.enabled(!settings.androidPitch);
        androidPitch.setOnCheckedChangeListener((button,checked) -> pitch.enabled(!checked));
        inflection=new Numeric(R.string.inflection,settings.inflection);
        volume=new Numeric(R.string.volume,settings.volume);
        heading(R.string.anatomy);
        models=select(R.string.model,getResources().getStringArray(R.array.models),settings.gender);
        head=new Numeric(R.string.head_size,settings.head);
        roughness=new Numeric(R.string.roughness,settings.roughness);
        breathiness=new Numeric(R.string.breathiness,settings.breathiness);
        Button reset=new Button(this);reset.setText(R.string.reset_profile);content.addView(reset);
        reset.setOnClickListener(v -> loadVoice(new VoiceSettings(loadedProfile)));
        profiles.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            public void onNothingSelected(AdapterView<?> parent) {}
            public void onItemSelected(AdapterView<?> parent,View view,int position,long id) {
                if(position==loadedProfile)return;
                saveVoice();loadedProfile=position;loadVoice(Preferences.read(preferences,position));
            }
        });
        heading(R.string.output_settings);
        rates=select(R.string.sample_rate,new String[]{"8 kHz","11.025 kHz","16 kHz","22.05 kHz",
            "32 kHz","44.1 kHz","48 kHz"},settings.rate);
        pauses=select(R.string.pause_mode,getResources().getStringArray(R.array.pause_modes),settings.pauses);
        tags=check(R.string.eci_tags,settings.tags);
        Button dictionaries=new Button(this);dictionaries.setText(R.string.dictionaries);content.addView(dictionaries);
        dictionaries.setOnClickListener(v -> {save();startActivity(new Intent(this,DictionaryActivity.class));});
        heading(R.string.test_speech);
        TextView caption=new TextView(this);caption.setText(R.string.sample_text);
        text=new EditText(this);text.setId(View.generateViewId());caption.setLabelFor(text.getId());
        text.setMinLines(3);text.setGravity(android.view.Gravity.TOP);
        text.setText(Catalog.LANGUAGES[languages.getSelectedItemPosition()].sample);
        content.addView(caption);content.addView(text);
        languages.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            public void onNothingSelected(AdapterView<?> parent) {}
            public void onItemSelected(AdapterView<?> parent,View view,int position,long id) {text.setText(Catalog.LANGUAGES[position].sample);}
        });
        Button save=new Button(this);save.setText(R.string.save);content.addView(save);
        save.setOnClickListener(v -> {save();status.setText(R.string.saved);});
        Button speak=new Button(this);speak.setText(R.string.speak);content.addView(speak);
        speak.setOnClickListener(v -> {
            save();if(!ready){status.setText(R.string.initializing);return;}
            Catalog.Language item=Catalog.LANGUAGES[languages.getSelectedItemPosition()];
            String name=item.voice(profiles.getSelectedItemPosition());
            java.util.Set<Voice> voices=tts.getVoices();
            if(voices!=null)for(Voice voice:voices)if(name.equals(voice.getName())){tts.setVoice(voice);break;}
            tts.speak(text.getText(),TextToSpeech.QUEUE_FLUSH,null,"reloq-preview");
        });
        Button stop=new Button(this);stop.setText(R.string.stop);content.addView(stop);
        stop.setOnClickListener(v -> {if(tts!=null)tts.stop();});
        Button systemSettings=new Button(this);systemSettings.setText(R.string.system_settings);content.addView(systemSettings);
        systemSettings.setOnClickListener(v -> {
            try{startActivity(new Intent("com.android.settings.TTS_SETTINGS"));}
            catch(android.content.ActivityNotFoundException error){startActivity(new Intent(android.provider.Settings.ACTION_ACCESSIBILITY_SETTINGS));}
        });
        status=new TextView(this);status.setAccessibilityLiveRegion(View.ACCESSIBILITY_LIVE_REGION_POLITE);
        status.setText(R.string.initializing);content.addView(status);
        tts=new TextToSpeech(this,result -> runOnUiThread(() -> {
            ready=result==TextToSpeech.SUCCESS;status.setText(ready?R.string.ready:R.string.unavailable);
        }),getPackageName());
    }
    private void loadVoice(VoiceSettings s) {
        pitch.set(s.manualPitch);inflection.set(s.inflection);volume.set(s.volume);
        models.setSelection(VoiceSettings.bounded(s.gender,0,1));head.set(s.head);
        roughness.set(s.roughness);breathiness.set(s.breathiness);
    }
    private void saveVoice() {
        VoiceSettings s=new VoiceSettings(loadedProfile);
        s.androidRate=androidRate.isChecked();s.androidPitch=androidPitch.isChecked();
        s.manualWpm=speed.get();s.rateBoost=boost.isChecked();s.manualPitch=pitch.get();
        s.inflection=inflection.get();s.volume=volume.get();s.gender=models.getSelectedItemPosition();
        s.head=head.get();s.roughness=roughness.get();s.breathiness=breathiness.get();
        s.rate=rates==null?preferences.getInt("rate",1):rates.getSelectedItemPosition();
        s.pauses=pauses==null?preferences.getInt("pauses",0):pauses.getSelectedItemPosition();
        s.tags=tags==null?preferences.getBoolean("tags",false):tags.isChecked();
        Preferences.save(preferences,loadedProfile,s);
    }
    private void save() {
        saveVoice();preferences.edit().putInt("language",languages.getSelectedItemPosition())
            .putInt("profile",profiles.getSelectedItemPosition()).putBoolean("androidVoice",androidVoice.isChecked()).apply();
    }
    @Override public void onDestroy(){if(tts!=null){tts.stop();tts.shutdown();}super.onDestroy();}
}
