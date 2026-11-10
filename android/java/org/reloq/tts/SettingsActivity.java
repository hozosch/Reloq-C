package org.reloq.tts;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.speech.tts.TextToSpeech;
import android.speech.tts.Voice;
import android.text.Editable;
import android.text.TextWatcher;
import android.text.InputType;
import android.view.View;
import android.widget.*;

/** Labelled standard controls; manual overrides are explicit and reversible. */
public final class SettingsActivity extends Activity {
    private TextToSpeech tts;
    private boolean ready;
    private LinearLayout content;
    private Spinner languages,profiles,rates,pauses,models,eqPreset;
    private CheckBox androidVoice,androidRate,androidPitch,boost,tags,abbreviations,prediction,originalQuestions,mellowMode,eqEnabled;
    private RadioButton speedEciMode,speedWpmMode,pitchEciMode,pitchHertzMode;
    private RadioGroup speedModes,pitchModes;
    private Numeric speed,pitch,inflection,volume,head,roughness,breathiness,eqLow,eqMid,eqHigh;
    private EditText text,pitchHertzInput,wpmInput;
    private TextView pitchHertzLabel,wpmLabel;
    private TextView status;
    private SharedPreferences preferences;
    private int loadedProfile;
    private int pitchHertz=150,manualWpm=190;
    private int eqCustomLow,eqCustomMid,eqCustomHigh;
    private int[][] eqFilters;
    private boolean initializing=true;

    private void changed() {if(!initializing)save();}

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
        content.addView(control);return control;
    }
    private CheckBox check(int label,boolean value) {
        CheckBox control=new CheckBox(this);control.setText(label);control.setChecked(value);
        control.setOnCheckedChangeListener((button,checked) -> changed());
        content.addView(control);return control;
    }
    private RadioGroup modes(int label,RadioButton first,RadioButton second,boolean useSecond) {
        TextView caption=new TextView(this);caption.setText(label);content.addView(caption);
        RadioGroup group=new RadioGroup(this);group.setOrientation(RadioGroup.VERTICAL);
        first.setId(View.generateViewId());second.setId(View.generateViewId());
        group.addView(first);group.addView(second);group.check(useSecond?second.getId():first.getId());
        content.addView(group);return group;
    }
    private RadioButton radio(int label) {
        RadioButton button=new RadioButton(this);button.setText(label);return button;
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
        Runnable onUserChange;
        Numeric(int title,int value) {
            this(title,value,0,100);
        }
        Numeric(int title,int value,int minimum,int maximum) {
            this.title=title;label=new TextView(SettingsActivity.this);
            control=new SeekBar(SettingsActivity.this);control.setId(View.generateViewId());
            control.setMin(minimum);control.setMax(maximum);label.setLabelFor(control.getId());
            control.setKeyProgressIncrement(1);
            control.setContentDescription(getString(title));
            content.addView(label);content.addView(control);
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
        void enabled(boolean value) {control.setEnabled(value);label.setEnabled(value);}
    }
    @Override public void onCreate(Bundle state) {
        super.onCreate(state);preferences=getSharedPreferences("settings",MODE_PRIVATE);
        UpdateScheduler.schedule(this);
        if(UpdateScheduler.enabled(this))new android.os.Handler(getMainLooper()).post(() -> {
            if(!isFinishing()&&!isDestroyed())UpdateScheduler.requestNotificationsOnce(this);
        });
        BootPreferences.refresh(this);
        DictionaryStore.queueSyncBoot(this);
        ScrollView scroll=new ScrollView(this);
        content=new LinearLayout(this);content.setOrientation(LinearLayout.VERTICAL);
        int padding=(int)(16*getResources().getDisplayMetrics().density);
        content.setPadding(padding,padding,padding,padding);scroll.addView(content);setContentView(scroll);
        scroll.setOnApplyWindowInsetsListener((view,insets) -> {
            content.setPadding(padding+insets.getSystemWindowInsetLeft(),padding+insets.getSystemWindowInsetTop(),
                padding+insets.getSystemWindowInsetRight(),padding+insets.getSystemWindowInsetBottom());
            return insets.consumeSystemWindowInsets();
        });scroll.requestApplyInsets();
        TextView intro=new TextView(this);intro.setText(R.string.engine_help);content.addView(intro);
        heading(R.string.voice_settings);
        androidVoice=check(R.string.android_voice,preferences.getBoolean("androidVoice",true));
        Catalog.Language system=Catalog.find(java.util.Locale.getDefault().getLanguage(),java.util.Locale.getDefault().getCountry());
        int first=3;for(int i=0;i<Catalog.LANGUAGES.length;++i)if(Catalog.LANGUAGES[i]==system)first=i;
        languages=select(R.string.language,Catalog.LANGUAGES,preferences.getInt("language",first));
        profiles=select(R.string.profile,Catalog.PROFILES,preferences.getInt("profile",0));
        loadedProfile=profiles.getSelectedItemPosition();
        VoiceSettings settings=Preferences.read(preferences,loadedProfile);
        androidRate=check(R.string.android_rate,settings.androidRate);
        speedEciMode=radio(R.string.speed_eci_mode);speedWpmMode=radio(R.string.speed_wpm_mode);
        speedModes=modes(R.string.speed_scale,speedEciMode,speedWpmMode,settings.speedWpmMode);
        speed=new Numeric(R.string.speed_eci_value,settings.manualSpeed,0,250);
        wpmLabel=new TextView(this);wpmLabel.setText(R.string.speed);
        wpmInput=new EditText(this);wpmInput.setId(View.generateViewId());wpmLabel.setLabelFor(wpmInput.getId());
        content.addView(wpmLabel);wpmInput.setInputType(InputType.TYPE_CLASS_NUMBER);wpmInput.setSingleLine(true);
        manualWpm=VoiceSettings.bounded(settings.manualWpm,1,6000);
        wpmInput.setText(Integer.toString(manualWpm));content.addView(wpmInput);
        wpmInput.addTextChangedListener(new TextWatcher() {
            public void beforeTextChanged(CharSequence s,int start,int count,int after) {}
            public void onTextChanged(CharSequence s,int start,int before,int count) {}
            public void afterTextChanged(Editable value) {
                if(!initializing)wpmInput.setError(validNumber(value.toString(),6000)?null:getString(R.string.speed_wpm_error));
            }
        });
        wpmInput.setOnFocusChangeListener((view,focused) -> {if(!focused)commitWpm();});
        TextView tempoHelp=new TextView(this);tempoHelp.setText(R.string.speed_help);content.addView(tempoHelp);
        boost=check(R.string.rate_boost,settings.rateBoost);
        androidRate.setOnCheckedChangeListener((button,checked) -> {updateSpeedControls();changed();});
        speedModes.setOnCheckedChangeListener((group,id) -> {updateSpeedControls();changed();});
        updateSpeedControls();
        androidPitch=check(R.string.android_pitch,settings.androidPitch);
        pitchEciMode=radio(R.string.pitch_eci_mode);pitchHertzMode=radio(R.string.pitch_hz_mode);
        pitchModes=modes(R.string.pitch_scale,pitchEciMode,pitchHertzMode,settings.pitchHertzMode);
        pitch=new Numeric(R.string.pitch,settings.manualPitch);
        pitchHertzLabel=new TextView(this);pitchHertzLabel.setText(R.string.pitch_hz_value);
        pitchHertzInput=new EditText(this);pitchHertzInput.setId(View.generateViewId());
        pitchHertzLabel.setLabelFor(pitchHertzInput.getId());content.addView(pitchHertzLabel);
        pitchHertzInput.setInputType(InputType.TYPE_CLASS_NUMBER);pitchHertzInput.setSingleLine(true);
        pitchHertz=VoiceSettings.bounded(settings.pitchHertz,1,24000);
        pitchHertzInput.setText(Integer.toString(pitchHertz));content.addView(pitchHertzInput);
        pitchHertzInput.addTextChangedListener(new TextWatcher() {
            public void beforeTextChanged(CharSequence s,int start,int count,int after) {}
            public void onTextChanged(CharSequence s,int start,int before,int count) {}
            public void afterTextChanged(Editable value) {
                if(!initializing)pitchHertzInput.setError(validNumber(value.toString(),24000)?null:getString(R.string.pitch_hz_error));
            }
        });
        pitchHertzInput.setOnFocusChangeListener((view,focused) -> {if(!focused)commitPitchHertz();});
        androidPitch.setOnCheckedChangeListener((button,checked) -> {updatePitchControls();changed();});
        pitchModes.setOnCheckedChangeListener((group,id) -> {updatePitchControls();changed();});
        updatePitchControls();
        inflection=new Numeric(R.string.inflection,settings.inflection);
        volume=new Numeric(R.string.volume,settings.volume);
        heading(R.string.anatomy);
        models=select(R.string.model,getResources().getStringArray(R.array.models),settings.gender);
        head=new Numeric(R.string.head_size,settings.head);
        roughness=new Numeric(R.string.roughness,settings.roughness);
        breathiness=new Numeric(R.string.breathiness,settings.breathiness);
        Button reset=new Button(this);reset.setText(R.string.reset_profile);content.addView(reset);
        reset.setOnClickListener(v -> {loadVoice(new VoiceSettings(loadedProfile));changed();});
        profiles.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            public void onNothingSelected(AdapterView<?> parent) {}
            public void onItemSelected(AdapterView<?> parent,View view,int position,long id) {
                if(position==loadedProfile)return;
                saveVoice();loadedProfile=position;loadVoice(Preferences.read(preferences,position));changed();
            }
        });
        heading(R.string.output_settings);
        rates=select(R.string.sample_rate,new String[]{"8 kHz","11.025 kHz","16 kHz","22.05 kHz",
            "32 kHz","44.1 kHz","48 kHz"},settings.rate);
        mellowMode=check(R.string.mellow_mode,settings.mellowMode);
        heading(R.string.equalizer);
        eqEnabled=check(R.string.equalizer_enabled,settings.eqEnabled);
        eqCustomLow=VoiceSettings.bounded(settings.eqLow,-12,12);
        eqCustomMid=VoiceSettings.bounded(settings.eqMid,-12,12);
        eqCustomHigh=VoiceSettings.bounded(settings.eqHigh,-12,12);
        eqFilters=new int[settings.eqFilters.length][];
        for(int i=0;i<eqFilters.length;++i)eqFilters[i]=settings.eqFilters[i].clone();
        int eqSelected=VoiceSettings.bounded(settings.eqPreset,0,4);
        int[] eqValues=eqSelected==4?new int[]{eqCustomLow,eqCustomMid,eqCustomHigh}:VoiceSettings.EQ_PRESETS[eqSelected];
        eqPreset=select(R.string.equalizer_preset,getResources().getStringArray(R.array.equalizer_presets),eqSelected);
        eqLow=new Numeric(R.string.equalizer_low,eqValues[0],-12,12);
        eqMid=new Numeric(R.string.equalizer_mid,eqValues[1],-12,12);
        eqHigh=new Numeric(R.string.equalizer_high,eqValues[2],-12,12);
        Button advanced=new Button(this);advanced.setText(R.string.equalizer_advanced);content.addView(advanced);
        advanced.setOnClickListener(v -> showEqualizerFilters());
        eqLow.onUserChange=eqMid.onUserChange=eqHigh.onUserChange=this::changeEqualizerBand;
        eqEnabled.setOnCheckedChangeListener((button,enabled) -> {updateEqualizerControls();changed();});
        eqPreset.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            public void onNothingSelected(AdapterView<?> parent) {}
            public void onItemSelected(AdapterView<?> parent,View view,int position,long id) {
                if(!initializing){showEqualizerValues(position);changed();}
            }
        });
        updateEqualizerControls();
        pauses=select(R.string.pause_mode,getResources().getStringArray(R.array.pause_modes),settings.pauses);
        abbreviations=check(R.string.abbreviation_dictionary,settings.abbreviations);
        prediction=check(R.string.phrase_prediction,settings.prediction);
        originalQuestions=check(R.string.original_questions,settings.originalQuestions);
        tags=check(R.string.eci_tags,settings.tags);
        Button dictionaries=new Button(this);dictionaries.setText(R.string.dictionaries);content.addView(dictionaries);
        dictionaries.setOnClickListener(v -> startActivity(new Intent(this,DictionaryActivity.class)));
        Button updates=new Button(this);updates.setText(R.string.app_updates);content.addView(updates);
        updates.setOnClickListener(v -> startActivity(new Intent(this,UpdateActivity.class)));
        heading(R.string.test_speech);
        TextView caption=new TextView(this);caption.setText(R.string.sample_text);
        text=new EditText(this);text.setId(View.generateViewId());caption.setLabelFor(text.getId());
        text.setMinLines(3);text.setGravity(android.view.Gravity.TOP);
        text.setText(Catalog.LANGUAGES[languages.getSelectedItemPosition()].sample);
        content.addView(caption);content.addView(text);
        languages.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            public void onNothingSelected(AdapterView<?> parent) {}
            public void onItemSelected(AdapterView<?> parent,View view,int position,long id) {text.setText(Catalog.LANGUAGES[position].sample);changed();}
        });
        Button speak=new Button(this);speak.setText(R.string.speak);content.addView(speak);
        speak.setOnClickListener(v -> {
            commitWpm();commitPitchHertz();save();if(!ready){status.setText(R.string.initializing);return;}
            Catalog.Language item=Catalog.LANGUAGES[languages.getSelectedItemPosition()];
            String name=item.voice(profiles.getSelectedItemPosition());
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
        status=new TextView(this);status.setAccessibilityLiveRegion(View.ACCESSIBILITY_LIVE_REGION_POLITE);
        status.setText(R.string.initializing);content.addView(status);
        tts=new TextToSpeech(this,result -> runOnUiThread(() -> {
            ready=result==TextToSpeech.SUCCESS;status.setText(ready?R.string.ready:R.string.unavailable);
        }),getPackageName());
        initializing=false;
    }
    private void loadVoice(VoiceSettings s) {
        boolean previous=initializing;initializing=true;
        try {
            pitch.set(s.manualPitch);pitchHertz=VoiceSettings.bounded(s.pitchHertz,1,24000);
            pitchHertzInput.setText(Integer.toString(pitchHertz));
            pitchModes.check(s.pitchHertzMode?pitchHertzMode.getId():pitchEciMode.getId());
            updatePitchControls();inflection.set(s.inflection);volume.set(s.volume);
            models.setSelection(VoiceSettings.bounded(s.gender,0,1));head.set(s.head);
            roughness.set(s.roughness);breathiness.set(s.breathiness);
        } finally {initializing=previous;}
    }
    private void saveVoice() {
        VoiceSettings s=new VoiceSettings(loadedProfile);
        s.androidRate=androidRate.isChecked();s.androidPitch=androidPitch.isChecked();
        s.manualWpm=manualWpm;s.manualSpeed=speed.get();s.speedWpmMode=speedWpmMode.isChecked();
        s.rateBoost=boost.isChecked();s.manualPitch=pitch.get();
        s.pitchHertz=pitchHertz;s.pitchHertzMode=pitchHertzMode.isChecked();
        s.inflection=inflection.get();s.volume=volume.get();s.gender=models.getSelectedItemPosition();
        s.head=head.get();s.roughness=roughness.get();s.breathiness=breathiness.get();
        s.rate=rates==null?preferences.getInt("rate",1):rates.getSelectedItemPosition();
        s.pauses=pauses==null?preferences.getInt("pauses",0):pauses.getSelectedItemPosition();
        s.abbreviations=abbreviations==null?preferences.getBoolean("abbreviations",true):abbreviations.isChecked();
        s.prediction=prediction==null?preferences.getBoolean("prediction",false):prediction.isChecked();
        s.originalQuestions=originalQuestions==null?preferences.getBoolean("originalQuestions",false):originalQuestions.isChecked();
        s.mellowMode=mellowMode==null?preferences.getBoolean("mellowMode",false):mellowMode.isChecked();
        s.eqEnabled=eqEnabled==null?preferences.getBoolean("eqEnabled",false):eqEnabled.isChecked();
        s.eqPreset=eqPreset==null?preferences.getInt("eqPreset",0):eqPreset.getSelectedItemPosition();
        s.eqLow=eqCustomLow;s.eqMid=eqCustomMid;s.eqHigh=eqCustomHigh;
        if(eqFilters!=null)for(int i=0;i<s.eqFilters.length;++i)System.arraycopy(eqFilters[i],0,s.eqFilters[i],0,4);
        s.tags=tags==null?preferences.getBoolean("tags",false):tags.isChecked();
        Preferences.save(preferences,loadedProfile,s);
    }
    private void save() {
        saveVoice();preferences.edit().putInt("language",languages.getSelectedItemPosition())
            .putInt("profile",profiles.getSelectedItemPosition()).putBoolean("androidVoice",androidVoice.isChecked()).apply();
        BootPreferences.refresh(this);
    }
    private void updatePitchControls() {
        boolean manual=!androidPitch.isChecked(),hz=manual&&pitchHertzMode.isChecked();
        pitch.enabled(manual&&!hz);pitchEciMode.setEnabled(manual);pitchHertzMode.setEnabled(manual);
        pitchHertzInput.setEnabled(hz);pitchHertzLabel.setEnabled(hz);
    }
    private void updateEqualizerControls() {
        boolean enabled=eqEnabled.isChecked();eqPreset.setEnabled(enabled);
        eqLow.enabled(enabled);eqMid.enabled(enabled);eqHigh.enabled(enabled);
    }
    private void showEqualizerValues(int selected) {
        int preset=VoiceSettings.bounded(selected,0,4);
        int[] values=preset==4?new int[]{eqCustomLow,eqCustomMid,eqCustomHigh}:VoiceSettings.EQ_PRESETS[preset];
        eqLow.set(values[0]);eqMid.set(values[1]);eqHigh.set(values[2]);
    }
    private void changeEqualizerBand() {
        if(initializing)return;
        eqCustomLow=eqLow.get();eqCustomMid=eqMid.get();eqCustomHigh=eqHigh.get();
        eqPreset.setSelection(4);changed();
    }
    private EditText filterInput(LinearLayout box,int title,int minimum,int maximum) {
        TextView label=new TextView(this);
        label.setText(getString(title)+" ("+minimum+"–"+maximum+")");
        EditText input=new EditText(this);input.setId(View.generateViewId());label.setLabelFor(input.getId());
        input.setInputType(InputType.TYPE_CLASS_NUMBER|(minimum<0?InputType.TYPE_NUMBER_FLAG_SIGNED:0));
        input.setSingleLine(true);box.addView(label);box.addView(input);return input;
    }
    private void watchFilterInput(EditText input,int field,int minimum,int maximum,boolean[] loading,int[] selected) {
        input.addTextChangedListener(new TextWatcher() {
            public void beforeTextChanged(CharSequence s,int start,int count,int after) {}
            public void onTextChanged(CharSequence s,int start,int before,int count) {}
            public void afterTextChanged(Editable text) {
                if(loading[0])return;
                try {int value=Integer.parseInt(text.toString());
                    if(value<minimum||value>maximum)throw new NumberFormatException();
                    input.setError(null);
                    if(eqFilters[selected[0]][field]!=value){eqFilters[selected[0]][field]=value;changed();}
                } catch(NumberFormatException error){input.setError(minimum+"…"+maximum);}
            }
        });
    }
    private void showEqualizerFilters() {
        LinearLayout box=new LinearLayout(this);box.setOrientation(LinearLayout.VERTICAL);box.setPadding(24,12,24,12);
        TextView label=new TextView(this);label.setText(R.string.filter_select);
        Spinner select=new Spinner(this);select.setId(View.generateViewId());label.setLabelFor(select.getId());
        String[] choices=new String[eqFilters.length];
        for(int i=0;i<choices.length;++i)choices[i]=getString(R.string.filter_name)+" "+(i+1);
        ArrayAdapter<String> adapter=new ArrayAdapter<>(this,android.R.layout.simple_spinner_item,choices);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);select.setAdapter(adapter);
        box.addView(label);box.addView(select);
        CheckBox enabled=new CheckBox(this);enabled.setText(R.string.filter_enabled);box.addView(enabled);
        EditText frequency=filterInput(box,R.string.filter_frequency,1,24000);
        EditText gain=filterInput(box,R.string.filter_gain,-12,12);
        EditText width=filterInput(box,R.string.filter_width,10,400);
        TextView hint=new TextView(this);hint.setText(R.string.filter_nyquist);box.addView(hint);
        ScrollView scroll=new ScrollView(this);scroll.addView(box);
        boolean[] loading={false};int[] selected={0};
        Runnable display=() -> {loading[0]=true;try {int[] f=eqFilters[selected[0]];
            enabled.setChecked(f[0]!=0);frequency.setText(Integer.toString(f[1]));
            gain.setText(Integer.toString(f[2]));width.setText(Integer.toString(f[3]));
            frequency.setError(null);gain.setError(null);width.setError(null);
        } finally {loading[0]=false;}};
        watchFilterInput(frequency,1,1,24000,loading,selected);
        watchFilterInput(gain,2,-12,12,loading,selected);
        watchFilterInput(width,3,10,400,loading,selected);
        enabled.setOnCheckedChangeListener((button,checked) -> {if(!loading[0]){
            eqFilters[selected[0]][0]=checked?1:0;changed();}});
        select.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            public void onNothingSelected(AdapterView<?> parent) {}
            public void onItemSelected(AdapterView<?> parent,View view,int position,long id) {
                selected[0]=position;display.run();
            }
        });
        display.run();
        new AlertDialog.Builder(this).setTitle(R.string.filter_title).setView(scroll)
            .setPositiveButton(android.R.string.ok,(dialog,which) -> {}).show();
    }
    private void updateSpeedControls() {
        boolean manual=!androidRate.isChecked(),wpm=manual&&speedWpmMode.isChecked();
        speedEciMode.setEnabled(manual);speedWpmMode.setEnabled(manual);speed.enabled(manual&&!wpm);
        wpmInput.setEnabled(wpm);wpmLabel.setEnabled(wpm);boost.setEnabled(!manual);
    }
    private static boolean validNumber(String text,int maximum) {
        try {int value=Integer.parseInt(text);return value>=1&&value<=maximum;}
        catch(NumberFormatException error){return false;}
    }
    private void commitWpm() {
        if(initializing||wpmInput==null)return;
        String value=wpmInput.getText().toString();
        if(!validNumber(value,6000)){wpmInput.setError(getString(R.string.speed_wpm_error));return;}
        int next=Integer.parseInt(value);wpmInput.setError(null);
        if(next!=manualWpm){manualWpm=next;changed();}
    }
    private void commitPitchHertz() {
        if(initializing||pitchHertzInput==null)return;
        String value=pitchHertzInput.getText().toString();
        if(!validNumber(value,24000)){pitchHertzInput.setError(getString(R.string.pitch_hz_error));return;}
        int next=Integer.parseInt(value);pitchHertzInput.setError(null);
        if(next!=pitchHertz){pitchHertz=next;changed();}
    }
    @Override protected void onPause(){commitWpm();commitPitchHertz();super.onPause();}
    @Override public void onDestroy(){if(tts!=null){tts.stop();tts.shutdown();}super.onDestroy();}
}
