package org.reloq.tts;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.text.InputType;
import android.view.View;
import android.widget.*;

/** Separate page for equalizer controls. */
public final class EqualizerSettingsActivity extends Activity {
    private LinearLayout content;
    private Spinner eqPreset;
    private CheckBox eqEnabled;
    private Numeric eqLow,eqMid,eqHigh;
    private SharedPreferences preferences;
    private int eqCustomLow,eqCustomMid,eqCustomHigh;
    private int[][] eqFilters;
    private boolean initializing=true;

    private void changed() {if(!initializing)saveEqualizer();}

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
            this.title=title;label=new TextView(EqualizerSettingsActivity.this);
            control=new SeekBar(EqualizerSettingsActivity.this);control.setId(View.generateViewId());
            control.setMin(minimum);control.setMax(maximum);label.setLabelFor(control.getId());
            control.setKeyProgressIncrement(1);
            control.setContentDescription(getString(title));
            box=new LinearLayout(EqualizerSettingsActivity.this);box.setOrientation(LinearLayout.VERTICAL);
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
        setTitle(R.string.equalizer);
        ScrollView page=page();content=(LinearLayout)page.getChildAt(0);setContentView(page);
        LinearLayout header=new LinearLayout(this);header.setOrientation(LinearLayout.HORIZONTAL);
        header.setGravity(android.view.Gravity.CENTER_VERTICAL);
        Button back=new Button(this);back.setText("←");
        back.setContentDescription(getString(R.string.back));
        back.setOnClickListener(v -> finish());
        header.addView(back,new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.WRAP_CONTENT,LinearLayout.LayoutParams.WRAP_CONTENT));
        TextView title=new TextView(this);title.setText(R.string.equalizer);
        title.setTextSize(20);title.setPadding(24,0,0,0);
        if(android.os.Build.VERSION.SDK_INT>=28)title.setAccessibilityHeading(true);
        header.addView(title,new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.WRAP_CONTENT,LinearLayout.LayoutParams.WRAP_CONTENT));
        content.addView(header);
        VoiceSettings settings=Preferences.read(preferences,preferences.getInt("profile",0));
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
        Button resetFilters=new Button(this);resetFilters.setText(R.string.reset_equalizer);content.addView(resetFilters);
        resetFilters.setOnClickListener(v -> resetEqualizer());
        eqLow.onUserChange=eqMid.onUserChange=eqHigh.onUserChange=this::changeEqualizerBand;
        eqEnabled.setOnCheckedChangeListener((button,enabled) -> {updateEqualizerControls();changed();});
        eqPreset.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            public void onNothingSelected(AdapterView<?> parent) {}
            public void onItemSelected(AdapterView<?> parent,View view,int position,long id) {
                if(!initializing){showEqualizerValues(position);changed();}
            }
        });
        updateEqualizerControls();
        initializing=false;
    }
    private boolean equalizerIsDefault() {
        if(eqEnabled.isChecked() || eqPreset.getSelectedItemPosition()!=0 ||
           eqCustomLow!=0 || eqCustomMid!=0 || eqCustomHigh!=0)return false;
        VoiceSettings defaults=new VoiceSettings(0);
        for(int i=0;i<eqFilters.length;++i)
            for(int j=0;j<4;++j)
                if(eqFilters[i][j]!=defaults.eqFilters[i][j])return false;
        return true;
    }
    private void resetEqualizer() {
        if(equalizerIsDefault()) {
            Toast.makeText(this,R.string.reset_not_needed,Toast.LENGTH_SHORT).show();
            return;
        }
        boolean previous=initializing;initializing=true;
        try {
            eqEnabled.setChecked(false);eqPreset.setSelection(0);
            eqCustomLow=eqCustomMid=eqCustomHigh=0;
            VoiceSettings defaults=new VoiceSettings(0);
            for(int i=0;i<eqFilters.length;++i)System.arraycopy(defaults.eqFilters[i],0,eqFilters[i],0,4);
            showEqualizerValues(0);updateEqualizerControls();
        } finally {initializing=previous;}
        changed();
        Toast.makeText(this,R.string.reset_done,Toast.LENGTH_SHORT).show();
    }
    private void saveEqualizer() {
        int profile=preferences.getInt("profile",0);
        VoiceSettings s=Preferences.read(preferences,profile);
        s.eqEnabled=eqEnabled.isChecked();
        s.eqPreset=eqPreset.getSelectedItemPosition();
        s.eqLow=eqCustomLow;s.eqMid=eqCustomMid;s.eqHigh=eqCustomHigh;
        if(eqFilters!=null)for(int i=0;i<s.eqFilters.length;++i)System.arraycopy(eqFilters[i],0,s.eqFilters[i],0,4);
        Preferences.save(preferences,profile,s);
        BootPreferences.refresh(this);
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
            public void afterTextChanged(Editable value) {
                if(loading[0])return;
                try {int parsed=Integer.parseInt(value.toString());
                    if(parsed<minimum||parsed>maximum)throw new NumberFormatException();
                    input.setError(null);
                    if(eqFilters[selected[0]][field]!=parsed){eqFilters[selected[0]][field]=parsed;changed();}
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
}
