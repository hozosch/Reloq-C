package org.reloq.tts;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.net.Uri;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.PowerManager;
import android.speech.tts.TextToSpeech;
import android.speech.tts.Voice;
import android.text.Editable;
import android.text.TextWatcher;
import android.text.InputType;
import android.view.View;
import android.widget.*;

/** Main settings page: language selection, equalizer, speech test and links. */
public final class SettingsActivity extends Activity {
    private TextToSpeech tts;
    private boolean ready,returnFromVoice,initializing=true;
    private LinearLayout content;
    private Button voiceSettings;
    private Spinner languages,fallback;
    private CheckBox androidVoice;
    private EditText text;
    private Button batteryCheckButton;
    private TextView status;
    private SharedPreferences preferences;
    private Object[] languageChoices() {
        Object[] choices=new Object[AutoLanguage.INDEX+1];
        System.arraycopy(Catalog.LANGUAGES,0,choices,0,AutoLanguage.INDEX);
        choices[AutoLanguage.INDEX]=AutoLanguage.label();
        return choices;
    }
    private String sampleFor(int position) {
        return position==AutoLanguage.INDEX?AutoLanguage.sample():
            Catalog.LANGUAGES[position].sample;
    }
    private void changed() {if(!initializing)saveMain();}

    private Spinner select(int label,Object[] values,int selected) {
        TextView caption=new TextView(this);caption.setText(label);
        Spinner control=new Spinner(this);control.setId(View.generateViewId());
        caption.setLabelFor(control.getId());content.addView(caption);
        ArrayAdapter<Object> adapter=new ArrayAdapter<>(this,android.R.layout.simple_spinner_item,values);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        control.setAdapter(adapter);control.setSelection(VoiceSettings.bounded(selected,0,values.length-1));
        control.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            public void onNothingSelected(AdapterView<?> parent) {}
            public void onItemSelected(AdapterView<?> parent,View view,int position,long id) {changed();}
        });
        control.setTag(caption);content.addView(control);return control;
    }
    private CheckBox check(int label,boolean value) {
        CheckBox control=new CheckBox(this);control.setText(label);control.setChecked(value);
        control.setOnCheckedChangeListener((button,checked) -> changed());
        content.addView(control);return control;
    }
    private void heading(int label) {
        TextView view=new TextView(this);view.setText(label);view.setTextSize(20);
        view.setPadding(0,24,0,8);
        if(android.os.Build.VERSION.SDK_INT>=28)view.setAccessibilityHeading(true);
        content.addView(view);
    }
    private final class Numeric {
        final LinearLayout box;
        final TextView label;
        final SeekBar control;
        final int title;
        Runnable onUserChange;
        Numeric(int title,int value,int minimum,int maximum) {
            this.title=title;label=new TextView(SettingsActivity.this);
            control=new SeekBar(SettingsActivity.this);control.setId(View.generateViewId());
            control.setMin(minimum);control.setMax(maximum);label.setLabelFor(control.getId());
            control.setKeyProgressIncrement(1);
            control.setContentDescription(getString(title));
            box=new LinearLayout(SettingsActivity.this);box.setOrientation(LinearLayout.VERTICAL);
            content.addView(box);box.addView(label);box.addView(control);
            control.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                public void onStartTrackingTouch(SeekBar bar) {}
                public void onStopTrackingTouch(SeekBar bar) {}
                public void onProgressChanged(SeekBar bar,int progress,boolean user) {
                    describe();if(user){if(onUserChange!=null)onUserChange.run();else changed();}
                }
            });set(value);
        }
        void describe() {
            label.setText(getString(R.string.parameter_value,getString(title),control.getProgress()));
            if(android.os.Build.VERSION.SDK_INT>=30)control.setStateDescription(Integer.toString(control.getProgress()));
        }
        void set(int value) {control.setProgress(VoiceSettings.bounded(value,control.getMin(),control.getMax()));describe();}
        int get() {return control.getProgress();}
        void enabled(boolean value) {control.setEnabled(value);label.setEnabled(value);describe();}
    }
    private ScrollView page() {
        ScrollView scroll=new ScrollView(this);
        LinearLayout box=new LinearLayout(this);box.setOrientation(LinearLayout.VERTICAL);
        int padding=(int)(16*getResources().getDisplayMetrics().density);
        box.setPadding(padding,padding,padding,padding);scroll.addView(box);
        scroll.setOnApplyWindowInsetsListener((view,insets) -> {
            box.setPadding(padding+insets.getSystemWindowInsetLeft(),padding+insets.getSystemWindowInsetTop(),
                padding+insets.getSystemWindowInsetRight(),padding+insets.getSystemWindowInsetBottom());
            return insets.consumeSystemWindowInsets();
        });return scroll;
    }
    @Override public void onCreate(Bundle state) {
        super.onCreate(state);preferences=getSharedPreferences("settings",MODE_PRIVATE);
        UpdateScheduler.schedule(this);
        if(UpdateScheduler.enabled(this))new android.os.Handler(getMainLooper()).post(() -> {
            if(!isFinishing()&&!isDestroyed())UpdateScheduler.requestNotificationsOnce(this);
        });
        BootPreferences.refresh(this);
        DictionaryStore.queueSyncBoot(this);
        ScrollView page=page();content=(LinearLayout)page.getChildAt(0);setContentView(page);
        TextView intro=new TextView(this);intro.setText(R.string.engine_help);content.addView(intro);
        heading(R.string.voice_settings);
        androidVoice=check(R.string.android_voice,preferences.getBoolean("androidVoice",true));
        Catalog.Language system=Catalog.find(java.util.Locale.getDefault().getLanguage(),java.util.Locale.getDefault().getCountry());
        int first=3;for(int i=0;i<Catalog.LANGUAGES.length;++i)if(Catalog.LANGUAGES[i]==system)first=i;
        languages=select(R.string.language,languageChoices(),
            VoiceSettings.bounded(preferences.getInt("language",first),0,AutoLanguage.INDEX));
        int baseLanguage=VoiceSettings.bounded(preferences.getInt("autoBase",first),0,Catalog.LANGUAGES.length-1);
        fallback=select(R.string.auto_fallback,Catalog.LANGUAGES,baseLanguage);
        voiceSettings=new Button(this);voiceSettings.setText(R.string.voice_settings_page);content.addView(voiceSettings);
        voiceSettings.setOnClickListener(v -> {returnFromVoice=true;startActivity(new Intent(this,VoiceSettingsActivity.class));});
        Button equalizerSettings=new Button(this);equalizerSettings.setText(R.string.equalizer_settings);
        content.addView(equalizerSettings);
        equalizerSettings.setOnClickListener(v -> startActivity(new Intent(this,EqualizerSettingsActivity.class)));
        Button dictionaries=new Button(this);dictionaries.setText(R.string.dictionaries);content.addView(dictionaries);
        dictionaries.setOnClickListener(v -> startActivity(new Intent(this,DictionaryActivity.class)));
        Button updates=new Button(this);updates.setText(R.string.app_updates);content.addView(updates);
        updates.setOnClickListener(v -> startActivity(new Intent(this,UpdateActivity.class)));
        heading(R.string.test_speech);
        TextView caption=new TextView(this);caption.setText(R.string.sample_text);
        text=new EditText(this);text.setId(View.generateViewId());caption.setLabelFor(text.getId());
        text.setMinLines(3);text.setGravity(android.view.Gravity.TOP);
        text.setText(sampleFor(languages.getSelectedItemPosition()));
        content.addView(caption);content.addView(text);
        languages.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            public void onNothingSelected(AdapterView<?> parent) {}
            public void onItemSelected(AdapterView<?> parent,View view,int position,long id) {
                updateFallbackVisibility();text.setText(sampleFor(position));changed();
            }
        });
        Button speak=new Button(this);speak.setText(R.string.speak);content.addView(speak);
        speak.setOnClickListener(v -> {
            saveMain();if(!ready){status.setText(R.string.initializing);return;}
            int position=languages.getSelectedItemPosition();
            int profile=preferences.getInt("profile",0);
            String name=(position==AutoLanguage.INDEX
                ?AutoLanguage.voice(profile)
                :Catalog.LANGUAGES[position].voice(profile));
            java.util.Set<Voice> voices=tts.getVoices();
            if(voices!=null)for(Voice voice:voices)if(name.equals(voice.getName())){tts.setVoice(voice);break;}
            tts.speak(text.getText(),TextToSpeech.QUEUE_FLUSH,null,"reloq-test");
        });
        Button stop=new Button(this);stop.setText(R.string.stop);content.addView(stop);
        stop.setOnClickListener(v -> {if(tts!=null)tts.stop();});
        Button systemSettings=new Button(this);systemSettings.setText(R.string.system_settings);content.addView(systemSettings);
        systemSettings.setOnClickListener(v -> {
            try{startActivity(new Intent("com.android.settings.TTS_SETTINGS"));}
            catch(android.content.ActivityNotFoundException error){startActivity(new Intent(android.provider.Settings.ACTION_ACCESSIBILITY_SETTINGS));}
        });
        batteryCheckButton=new Button(this);content.addView(batteryCheckButton);
        updateBatteryCheck(batteryCheckButton);
        batteryCheckButton.setOnClickListener(v -> {
            PowerManager power=(PowerManager)getSystemService(POWER_SERVICE);
            if(power!=null&&power.isIgnoringBatteryOptimizations(getPackageName())){
                updateBatteryCheck(batteryCheckButton);return;
            }
            try{
                startActivity(new Intent("android.settings.REQUEST_IGNORE_BATTERY_OPTIMIZATIONS",
                    Uri.parse("package:"+getPackageName())));
            }catch(Exception error){
                try{startActivity(new Intent("android.settings.IGNORE_BATTERY_OPTIMIZATION_SETTINGS"));}
                catch(Exception ignored){}
            }
        });
        status=new TextView(this);status.setAccessibilityLiveRegion(View.ACCESSIBILITY_LIVE_REGION_POLITE);
        status.setText(R.string.initializing);content.addView(status);
        tts=new TextToSpeech(this,result -> runOnUiThread(() -> {
            ready=result==TextToSpeech.SUCCESS;status.setText(ready?R.string.ready:R.string.unavailable);
        }),getPackageName());
        updateFallbackVisibility();
        initializing=false;
    }
    private void updateBatteryCheck(Button button) {
        PowerManager power=(PowerManager)getSystemService(POWER_SERVICE);
        boolean ok=power!=null&&power.isIgnoringBatteryOptimizations(getPackageName());
        button.setText(ok?R.string.battery_check_normal:R.string.battery_check_missing);
    }
    @Override protected void onResume() {
        super.onResume();
        if(batteryCheckButton!=null)updateBatteryCheck(batteryCheckButton);
        if(!returnFromVoice)return;
        returnFromVoice=false;
        voiceSettings.post(() -> {
            voiceSettings.requestFocus();
            voiceSettings.performAccessibilityAction(android.view.accessibility.AccessibilityNodeInfo.ACTION_ACCESSIBILITY_FOCUS,null);
        });
    }
    private void updateFallbackVisibility() {
        TextView caption=(TextView)fallback.getTag();
        boolean automatic=languages.getSelectedItemPosition()==AutoLanguage.INDEX;
        caption.setVisibility(automatic?View.VISIBLE:View.GONE);
        fallback.setVisibility(automatic?View.VISIBLE:View.GONE);
    }
    private void saveMain() {
        int profile=preferences.getInt("profile",0);
        VoiceSettings s=Preferences.read(preferences,profile);
        Preferences.save(preferences,profile,s);
        preferences.edit()
            .putInt("language",languages.getSelectedItemPosition())
            .putInt("autoBase",fallback.getSelectedItemPosition())
            .putBoolean("androidVoice",androidVoice.isChecked()).apply();
        BootPreferences.refresh(this);
    }
    @Override public void onDestroy(){if(tts!=null){tts.stop();tts.shutdown();}super.onDestroy();}
}
