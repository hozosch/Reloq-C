package org.reloq.tts.preview;

import android.app.Activity;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.provider.OpenableColumns;
import android.view.View;
import android.widget.*;
import org.json.*;

/** Native dictionary data selection; downloads/merges never run on the UI. */
public final class DictionaryActivity extends Activity {
    private static final int OPEN=41;
    private CheckBox community;
    private Spinner language,volume;
    private ListView list;
    private TextView status;
    private LinearLayout content;
    private JSONArray sources;
    private boolean busy,loading;
    private final java.util.ArrayList<View> actions=new java.util.ArrayList<>();
    private Spinner select(int title,Object[] values,int at) {
        TextView label=new TextView(this);label.setText(title);content.addView(label);
        Spinner control=new Spinner(this);control.setId(View.generateViewId());label.setLabelFor(control.getId());
        ArrayAdapter<Object> adapter=new ArrayAdapter<>(this,android.R.layout.simple_spinner_item,values);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);control.setAdapter(adapter);
        control.setSelection(VoiceSettings.bounded(at,0,values.length-1));content.addView(control);actions.add(control);return control;
    }
    private void button(int text,Runnable action) {
        Button button=new Button(this);button.setText(text);content.addView(button);actions.add(button);
        button.setOnClickListener(v -> {if(!busy)action.run();});
    }
    @Override public void onCreate(Bundle state) {
        super.onCreate(state);ScrollView scroll=new ScrollView(this);
        content=new LinearLayout(this);content.setOrientation(LinearLayout.VERTICAL);
        int padding=(int)(16*getResources().getDisplayMetrics().density);
        content.setPadding(padding,padding,padding,padding);scroll.addView(content);setContentView(scroll);
        scroll.setOnApplyWindowInsetsListener((view,insets) -> {
            content.setPadding(padding+insets.getSystemWindowInsetLeft(),padding+insets.getSystemWindowInsetTop(),
                padding+insets.getSystemWindowInsetRight(),padding+insets.getSystemWindowInsetBottom());
            return insets.consumeSystemWindowInsets();
        });scroll.requestApplyInsets();
        TextView title=new TextView(this);title.setText(R.string.dictionaries);title.setTextSize(22);content.addView(title);
        TextView info=new TextView(this);info.setText(R.string.dictionary_help);content.addView(info);
        community=new CheckBox(this);community.setText(R.string.community_dictionaries);content.addView(community);actions.add(community);
        community.setOnCheckedChangeListener((view,enabled) -> {
            if(!loading&&!busy){JSONArray next=copy(sources);work(() -> DictionaryStore.rebuild(this,enabled,next));}
        });
        button(R.string.update_dictionaries,() -> work(() -> DictionaryStore.updateCommunity(this)));
        list=new ListView(this);list.setChoiceMode(ListView.CHOICE_MODE_SINGLE);
        list.setContentDescription(getString(R.string.own_dictionaries));
        content.addView(list,new LinearLayout.LayoutParams(-1,(int)(200*getResources().getDisplayMetrics().density)));
        int first=DictionaryStore.prefs(this).getInt("language",3);
        language=select(R.string.dictionary_language,Catalog.LANGUAGES,first);
        volume=select(R.string.dictionary_volume,getResources().getStringArray(R.array.dictionary_volumes),0);
        button(R.string.add_dictionary,() -> {
            Intent intent=new Intent(Intent.ACTION_OPEN_DOCUMENT).setType("*/*").addCategory(Intent.CATEGORY_OPENABLE)
                .addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION|Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION);
            startActivityForResult(intent,OPEN);
        });
        button(R.string.remove_dictionary,() -> change(-1));
        button(R.string.move_dictionary_up,() -> change(-2));
        button(R.string.move_dictionary_down,() -> change(1));
        button(R.string.reload_dictionaries,() -> {
            boolean enabled=community.isChecked();JSONArray next=copy(sources);
            work(() -> DictionaryStore.rebuild(this,enabled,next));
        });
        button(R.string.back,() -> finish());
        status=new TextView(this);status.setAccessibilityLiveRegion(View.ACCESSIBILITY_LIVE_REGION_POLITE);content.addView(status);
        reload();
    }
    private static JSONArray copy(JSONArray array) {
        try{return new JSONArray(array.toString());}catch(JSONException e){throw new IllegalArgumentException(e);}
    }
    private void reload() {
        loading=true;sources=DictionaryStore.sources(this);
        community.setChecked(DictionaryStore.prefs(this).getBoolean("communityDictionaries",false));
        java.util.ArrayList<String> names=new java.util.ArrayList<>();
        for(int i=0;i<sources.length();++i)try {
            JSONObject row=sources.getJSONObject(i);Catalog.Language lang=null;
            for(Catalog.Language l:Catalog.LANGUAGES)if(l.id==row.getInt("language"))lang=l;
            names.add((i+1)+". "+row.optString("name",row.getString("uri"))+" — "+
                (lang==null?"":lang.name)+", "+getResources().getStringArray(R.array.dictionary_volumes)[row.getInt("volume")]);
        }catch(JSONException e){names.add(getString(R.string.dictionary_unavailable));}
        list.setAdapter(new ArrayAdapter<>(this,android.R.layout.simple_list_item_single_choice,names));loading=false;
    }
    private interface Action {void run() throws Exception;}
    private void work(Action action) {
        if(busy)return;busy=true;for(View view:actions)view.setEnabled(false);status.setText(R.string.dictionary_working);
        DictionaryStore.WORK.execute(() -> {
            Exception failure=null;try{action.run();}catch(Exception e){failure=e;}
            final Exception error=failure;
            runOnUiThread(() -> {
                if(isFinishing()||isDestroyed())return;
                busy=false;for(View view:actions)view.setEnabled(true);reload();
                status.setText(error==null?getString(R.string.dictionary_ready):getString(R.string.dictionary_error,error.getMessage()));
            });
        });
    }
    private void change(int direction) {
        int at=list.getCheckedItemPosition();if(at<0||at>=sources.length())return;
        JSONArray next=copy(sources);
        if(direction==-1)next.remove(at);
        else {
            int other=at+(direction==-2?-1:1);if(other<0||other>=next.length())return;
            try{Object row=next.get(at);next.put(at,next.get(other));next.put(other,row);}
            catch(JSONException e){throw new IllegalArgumentException(e);}
        }
        boolean enabled=community.isChecked();work(() -> DictionaryStore.rebuild(this,enabled,next));
    }
    @Override protected void onActivityResult(int request,int result,Intent data) {
        super.onActivityResult(request,result,data);
        if(request!=OPEN||result!=RESULT_OK||data==null||data.getData()==null)return;
        Uri uri=data.getData();String name=uri.getLastPathSegment();
        try{getContentResolver().takePersistableUriPermission(uri,Intent.FLAG_GRANT_READ_URI_PERMISSION);}
        catch(SecurityException error){status.setText(R.string.dictionary_permission);return;}
        try(Cursor cursor=getContentResolver().query(uri,new String[]{OpenableColumns.DISPLAY_NAME},null,null,null)) {
            if(cursor!=null&&cursor.moveToFirst())name=cursor.getString(0);
        }catch(Exception ignored){}
        try {
            JSONArray next=copy(sources);
            next.put(new JSONObject().put("uri",uri.toString()).put("name",name)
                .put("language",Catalog.LANGUAGES[language.getSelectedItemPosition()].id).put("volume",volume.getSelectedItemPosition()));
            boolean enabled=community.isChecked();work(() -> DictionaryStore.rebuild(this,enabled,next));
        }catch(JSONException error){status.setText(getString(R.string.dictionary_error,error.getMessage()));}
    }
}
