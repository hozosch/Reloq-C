package org.reloq.tts.preview;

import android.media.AudioFormat;
import android.speech.tts.SynthesisCallback;
import android.speech.tts.SynthesisRequest;
import android.speech.tts.TextToSpeech;
import android.speech.tts.TextToSpeechService;
import android.speech.tts.Voice;
import android.util.Log;
import java.io.File;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.RejectedExecutionException;

public final class ReLoqService extends TextToSpeechService {
    private final Object contextGuard = new Object();
    private final ExecutorService worker = Executors.newSingleThreadExecutor(r ->
        new Thread(r,"ReLoq C engine"));
    private long context;
    private boolean destroyed;
    private volatile PcmQueue active;
    private volatile Catalog.Language selected = Catalog.LANGUAGES[3];

    @Override public void onCreate(){super.onCreate();DictionaryStore.start(this);}

    @Override protected String[] onGetLanguage() {
        Catalog.Language item = selected;
        return new String[]{item.locale.getISO3Language(),item.locale.getISO3Country(),""};
    }
    @Override protected int onIsLanguageAvailable(String lang, String country, String variant) {
        Catalog.Language item=Catalog.find(lang,country);
        if (item==null) return TextToSpeech.LANG_NOT_SUPPORTED;
        return country!=null && (country.equalsIgnoreCase(item.locale.getCountry()) ||
            country.equalsIgnoreCase(item.locale.getISO3Country()))
            ? TextToSpeech.LANG_COUNTRY_AVAILABLE : TextToSpeech.LANG_AVAILABLE;
    }
    @Override protected int onLoadLanguage(String lang, String country, String variant) {
        Catalog.Language item=Catalog.find(lang,country);
        if (item!=null) selected=item;
        return onIsLanguageAvailable(lang,country,variant);
    }
    @Override public List<Voice> onGetVoices() {
        List<Voice> result=new ArrayList<>();
        for (Catalog.Language item:Catalog.LANGUAGES)
            for (int p=0;p<Catalog.PROFILES.length;++p)
                result.add(new Voice(item.voice(p),item.locale,Voice.QUALITY_NORMAL,
                    Voice.LATENCY_VERY_LOW,false,Collections.emptySet()));
        return result;
    }
    @Override public int onIsValidVoiceName(String name) {
        return Catalog.forVoice(name)==null ? TextToSpeech.ERROR : TextToSpeech.SUCCESS;
    }
    @Override public int onLoadVoice(String name) {
        Catalog.Language item=Catalog.forVoice(name);
        if(item==null) return TextToSpeech.ERROR;
        selected=item;
        return TextToSpeech.SUCCESS;
    }
    @Override public String onGetDefaultVoiceNameFor(String lang,String country,String variant) {
        Catalog.Language item=Catalog.find(lang,country);
        int p=getSharedPreferences("settings",MODE_PRIVATE).getInt("profile",0);
        return item==null ? null : item.voice(Catalog.profile(null,p));
    }

    /** Disk I/O and core creation stay on the persistent engine worker. */
    private long ensureContext() throws Exception {
        synchronized(contextGuard) {
            if(destroyed) return 0;
            if(context!=0) return context;
        }
        File target=LexiconInstaller.path(this);
        long created=NativeEngine.create(target.getAbsolutePath());
        if(created==0) throw new IllegalStateException("Native engine initialization failed");
        synchronized(contextGuard) {
            if(!destroyed) { context=created; return created; }
        }
        NativeEngine.destroy(created);
        return 0;
    }
    @Override protected void onSynthesizeText(SynthesisRequest request,SynthesisCallback callback) {
        android.content.SharedPreferences preferences=getSharedPreferences("settings",MODE_PRIVATE);
        Catalog.Language item=Catalog.forVoice(request.getVoiceName());
        if(item==null) item=Catalog.find(request.getLanguage(),request.getCountry());
        boolean androidVoice=preferences.getBoolean("androidVoice",true);
        if(!androidVoice)
            item=Catalog.LANGUAGES[VoiceSettings.bounded(preferences.getInt("language",3),0,Catalog.LANGUAGES.length-1)];
        if(item==null) { callback.error(TextToSpeech.ERROR_INVALID_REQUEST); callback.done(); return; }
        selected=item;
        int profile=Catalog.profile(androidVoice?request.getVoiceName():null,
            preferences.getInt("profile",0));
        VoiceSettings settings=Preferences.read(preferences,profile);
        int rate=VoiceSettings.bounded(settings.rate,0,Catalog.RATES.length-1);
        int[] controls=settings.controls(request.getSpeechRate(),request.getPitch());
        byte[] utf8=request.getCharSequenceText().toString().getBytes(StandardCharsets.UTF_8);
        PcmQueue queue=new PcmQueue(16);
        active=queue;
        final int dialect=item.id;
        final DictionaryStore.Lease dictionaries=DictionaryStore.acquire(this);
        boolean submitted=false;
        try {
            if(callback.start(Catalog.RATES[rate],AudioFormat.ENCODING_PCM_16BIT,1)!=TextToSpeech.SUCCESS)
                { queue.cancel(); return; }
            worker.execute(() -> {
                boolean success=false;
                try {
                    if(!queue.isCancelled()) {
                        long handle=ensureContext();
                        if(dialect==0x40000)LexiconInstaller.ensure(this);
                        if(handle!=0 && !queue.isCancelled()) {
                            NativeEngine.dictionary(handle,dictionaries.path);
                            success=NativeEngine.synthesize(handle,dialect,controls,utf8,queue);
                        }
                    }
                } catch(Throwable error) { Log.e("ReLoq","Synthesis failed",error); }
                finally {dictionaries.close();queue.finish(success);}
            });
            submitted=true;
            byte[] block;
            while((block=queue.read())!=null) {
                for(int offset=0;offset<block.length;) {
                    int count=Math.min(callback.getMaxBufferSize(),block.length-offset);
                    if(count<=0 || queue.isCancelled() ||
                        callback.audioAvailable(block,offset,count)!=TextToSpeech.SUCCESS) {
                        queue.cancel(); break;
                    }
                    offset+=count;
                }
            }
            if(queue.hasFailed() && !queue.isCancelled()) callback.error(TextToSpeech.ERROR_SYNTHESIS);
        } catch(InterruptedException error) {
            Thread.currentThread().interrupt(); queue.cancel();
        } catch(RejectedExecutionException error) {
            queue.cancel(); callback.error(TextToSpeech.ERROR_SERVICE);
        } finally {
            if(!submitted)dictionaries.close();
            if(queue.isCancelled()) stopNative(queue);
            if(active==queue) active=null;
            callback.done();
        }
    }
    private void stopNative(PcmQueue queue) {
        synchronized(contextGuard) { if(context!=0) NativeEngine.stop(context,queue); }
    }
    @Override protected void onStop() {
        PcmQueue queue=active;
        if(queue!=null) queue.cancel();
        if(queue!=null) stopNative(queue);
    }
    @Override public void onDestroy() {
        onStop();
        DictionaryStore.stop();
        final long detached;
        synchronized(contextGuard) { destroyed=true; detached=context; context=0; }
        worker.execute(() -> { if(detached!=0) NativeEngine.destroy(detached); });
        worker.shutdown();
        super.onDestroy();
    }
}
