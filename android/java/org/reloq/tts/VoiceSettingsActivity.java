package org.reloq.tts;

import android.app.Activity;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.text.InputType;
import android.view.View;
import android.widget.*;

/** Separate page for the voice-related controls. */
public final class VoiceSettingsActivity extends Activity {
    private LinearLayout content;
    private TextView tempoHelp;
    private Spinner profiles,rates,pauses,models,numberModes;
    private CheckBox androidRate,androidPitch,boost,tags,volumeGain,abbreviations,prediction,originalQuestions,mellowMode,originalAcousticClock,wordSegmentation,camelCase;
    private RadioButton speedEciMode,speedWpmMode,pitchEciMode,pitchHertzMode;
    private RadioGroup speedModes,pitchModes;
    private Numeric speed,pitch,inflection,volume,head,roughness,breathiness;
    private EditText pitchHertzInput,wpmInput;
    private TextView pitchHertzLabel,wpmLabel;
    private SharedPreferences preferences;
    private int loadedProfile,clockUiLanguage,acousticClockMask;
    private int pitchHertz=150,manualWpm=190;
    private boolean initializing=true;

    private void changed() {if(!initializing)saveVoice();}

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
        final LinearLayout box;
        final TextView label;
        final SeekBar control;
        Button decrease,increase;
        final int title;
        Numeric(int title,int value) {
            this(title,value,0,100);
        }
        Numeric(int title,int value,int minimum,int maximum) {
            this.title=title;label=new TextView(VoiceSettingsActivity.this);
            control=new SeekBar(VoiceSettingsActivity.this);control.setId(View.generateViewId());
            control.setMin(minimum);control.setMax(maximum);label.setLabelFor(control.getId());
            control.setKeyProgressIncrement(1);
            control.setContentDescription(getString(title));
            box=new LinearLayout(VoiceSettingsActivity.this);box.setOrientation(LinearLayout.VERTICAL);
            content.addView(box);box.addView(label);
            if(title==R.string.speed_eci_value||title==R.string.pitch||title==R.string.volume||title==R.string.inflection) {
                LinearLayout row=new LinearLayout(VoiceSettingsActivity.this);
                decrease=stepButton(-1);increase=stepButton(1);
                row.addView(decrease);row.addView(control,new LinearLayout.LayoutParams(0,LinearLayout.LayoutParams.WRAP_CONTENT,1));
                row.addView(increase);box.addView(row);
            } else box.addView(control);
            control.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                public void onStartTrackingTouch(SeekBar bar) {}
                public void onStopTrackingTouch(SeekBar bar) {}
                public void onProgressChanged(SeekBar bar,int progress,boolean user) {
                    describe();if(user)changed();
                }
            });set(value);
        }
        void describe() {
            label.setText(getString(R.string.parameter_value,getString(title),control.getProgress()));
            if(android.os.Build.VERSION.SDK_INT>=30)control.setStateDescription(Integer.toString(control.getProgress()));
            if(decrease!=null) {
                decrease.setEnabled(control.isEnabled()&&get()>control.getMin());
                increase.setEnabled(control.isEnabled()&&get()<control.getMax());
            }
        }
        Button stepButton(int delta) {
            Button button=new Button(VoiceSettingsActivity.this);button.setText(delta>0?"+1":"-1");
            button.setContentDescription(delta>0?"+1":"-1");
            button.setOnClickListener(v -> {
                int next=VoiceSettings.bounded(get()+delta,control.getMin(),control.getMax());
                if(next==get())return;
                set(next);changed();
                button.announceForAccessibility(getString(R.string.parameter_value,getString(title),get()));
            });return button;
        }
        void set(int value) {control.setProgress(VoiceSettings.bounded(value,control.getMin(),control.getMax()));describe();}
        int get() {return control.getProgress();}
        void enabled(boolean value) {control.setEnabled(value);label.setEnabled(value);describe();}
        void visible(boolean value) {box.setVisibility(value?View.VISIBLE:View.GONE);}
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
        setTitle(R.string.voice_settings_page);
        ScrollView page=page();content=(LinearLayout)page.getChildAt(0);setContentView(page);
        LinearLayout header=new LinearLayout(this);header.setOrientation(LinearLayout.HORIZONTAL);
        header.setGravity(android.view.Gravity.CENTER_VERTICAL);
        Button back=new Button(this);back.setText("←");
        back.setContentDescription(getString(R.string.voice_settings_back));
        back.setOnClickListener(v -> finish());
        header.addView(back,new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.WRAP_CONTENT,LinearLayout.LayoutParams.WRAP_CONTENT));
        TextView title=new TextView(this);title.setText(R.string.voice_settings_page);
        title.setTextSize(20);title.setPadding(24,0,0,0);
        if(android.os.Build.VERSION.SDK_INT>=28)title.setAccessibilityHeading(true);
        header.addView(title,new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.WRAP_CONTENT,LinearLayout.LayoutParams.WRAP_CONTENT));
        content.addView(header);
        profiles=select(R.string.profile,Catalog.PROFILES,preferences.getInt("profile",0));
        loadedProfile=profiles.getSelectedItemPosition();
        VoiceSettings settings=Preferences.read(preferences,loadedProfile);
        acousticClockMask=settings.acousticClockMask;
        Catalog.Language system=Catalog.find(java.util.Locale.getDefault().getLanguage(),java.util.Locale.getDefault().getCountry());
        int first=3;for(int i=0;i<Catalog.LANGUAGES.length;++i)if(Catalog.LANGUAGES[i]==system)first=i;
        int language=preferences.getInt("language",first);
        clockUiLanguage=VoiceSettings.bounded(language==AutoLanguage.INDEX
            ?preferences.getInt("autoBase",first):language,0,Catalog.LANGUAGES.length-1);
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
        tempoHelp=new TextView(this);tempoHelp.setText(R.string.speed_help);content.addView(tempoHelp);
        boost=check(R.string.rate_boost,settings.rateBoost);
        originalAcousticClock=check(R.string.original_acoustic_clock,settings.originalClock(Catalog.LANGUAGES[clockUiLanguage].id));
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
        volumeGain=check(R.string.volume_gain,settings.volumeGain);
        models=select(R.string.anatomy,getResources().getStringArray(R.array.models),settings.gender);
        head=new Numeric(R.string.head_size,settings.head);
        roughness=new Numeric(R.string.roughness,settings.roughness);
        breathiness=new Numeric(R.string.breathiness,settings.breathiness);
        Button reset=new Button(this);reset.setText(R.string.reset_profile);content.addView(reset);
        reset.setOnClickListener(v -> {
            VoiceSettings current=Preferences.read(preferences,loadedProfile);
            VoiceSettings defaults=new VoiceSettings(loadedProfile);
            if(current.manualPitch==defaults.manualPitch && current.pitchHertz==defaults.pitchHertz &&
               current.pitchHertzMode==defaults.pitchHertzMode && current.inflection==defaults.inflection &&
               current.volume==defaults.volume && current.gender==defaults.gender && current.head==defaults.head &&
               current.roughness==defaults.roughness && current.breathiness==defaults.breathiness) {
                Toast.makeText(this,R.string.reset_not_needed,Toast.LENGTH_SHORT).show();
                return;
            }
            loadVoice(defaults);changed();
            Toast.makeText(this,R.string.reset_done,Toast.LENGTH_SHORT).show();
        });
        profiles.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            public void onNothingSelected(AdapterView<?> parent) {}
            public void onItemSelected(AdapterView<?> parent,View view,int position,long id) {
                if(position==loadedProfile)return;
                saveVoice();loadedProfile=position;loadVoice(Preferences.read(preferences,position));
                preferences.edit().putInt("profile",position).apply();
                BootPreferences.refresh(VoiceSettingsActivity.this);
            }
        });
        heading(R.string.output_settings);
        rates=select(R.string.sample_rate,new String[]{"8 kHz","11.025 kHz","16 kHz","22.05 kHz",
            "32 kHz","44.1 kHz","48 kHz"},settings.rate);
        mellowMode=check(R.string.mellow_mode,settings.mellowMode);
        pauses=select(R.string.pause_mode,getResources().getStringArray(R.array.pause_modes),settings.pauses);
        numberModes=select(R.string.number_mode,getResources().getStringArray(R.array.number_modes),
            preferences.getInt("numberMode",NumberReading.AUTO));
        abbreviations=check(R.string.abbreviation_dictionary,settings.abbreviations);
        prediction=check(R.string.phrase_prediction,settings.prediction);
        originalQuestions=check(R.string.original_questions,settings.originalQuestions);
        wordSegmentation=check(R.string.word_segmentation,settings.wordSegmentation);
        camelCase=check(R.string.camel_case,settings.camelCase);
        tags=check(R.string.eci_tags,settings.tags);
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
        VoiceSettings s=Preferences.read(preferences,loadedProfile);
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
        s.wordSegmentation=wordSegmentation==null?preferences.getBoolean("wordSegmentation",false):wordSegmentation.isChecked();
        s.camelCase=camelCase==null?preferences.getBoolean("camelCase",false):camelCase.isChecked();
        if(originalAcousticClock!=null) {
            int bit=1<<clockUiLanguage;
            acousticClockMask=originalAcousticClock.isChecked()?acousticClockMask|bit:acousticClockMask&~bit;
        }
        s.acousticClockMask=acousticClockMask;
        s.tags=tags==null?preferences.getBoolean("tags",false):tags.isChecked();
        s.volumeGain=volumeGain==null?preferences.getBoolean("volumeGain",false):volumeGain.isChecked();
        Preferences.save(preferences,loadedProfile,s);
        preferences.edit().putInt("profile",loadedProfile)
            .putInt("numberMode",numberModes==null?NumberReading.AUTO:numberModes.getSelectedItemPosition()).apply();
        BootPreferences.refresh(this);
    }
    private void updatePitchControls() {
        boolean manual=!androidPitch.isChecked(),hz=manual&&pitchHertzMode.isChecked();
        pitch.enabled(manual&&!hz);pitchEciMode.setEnabled(manual);pitchHertzMode.setEnabled(manual);
        pitchHertzInput.setEnabled(hz);pitchHertzLabel.setEnabled(hz);
        pitch.visible(manual&&!hz);
        pitchHertzInput.setVisibility(hz?View.VISIBLE:View.GONE);pitchHertzLabel.setVisibility(hz?View.VISIBLE:View.GONE);
    }
    private void updateSpeedControls() {
        boolean manual=!androidRate.isChecked(),wpm=manual&&speedWpmMode.isChecked();
        speedEciMode.setEnabled(manual);speedWpmMode.setEnabled(manual);speed.enabled(manual&&!wpm);
        wpmInput.setEnabled(wpm);wpmLabel.setEnabled(wpm);boost.setEnabled(!manual);
        speed.visible(manual&&!wpm);
        wpmInput.setVisibility(wpm?View.VISIBLE:View.GONE);wpmLabel.setVisibility(wpm?View.VISIBLE:View.GONE);
        tempoHelp.setVisibility(wpm?View.VISIBLE:View.GONE);
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
}
