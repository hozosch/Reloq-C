package org.reloq.tts;

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
import java.util.concurrent.Future;

public final class ReLoqService extends TextToSpeechService {
    private final Object contextGuard = new Object();
    private final Object dictionaryGuard = new Object();
    private final ExecutorService worker = Executors.newSingleThreadExecutor(r ->
        new Thread(r,"ReLoq C engine"));
    private long context;
    private volatile boolean destroyed;
    private boolean dictionariesStarted,legacyFilesCleaned;
    private Future<?> dictionaryStartup;
    private volatile PcmQueue active;
    private volatile Catalog.Language selected = Catalog.LANGUAGES[3];

    @Override public void onCreate(){super.onCreate();startDictionariesIfUnlocked();}

    /** Never touch credential-protected files before the first unlock. */
    private boolean startDictionariesIfUnlocked() {
        if(!BootPreferences.unlocked(this))return false;
        synchronized(dictionaryGuard) {
            if(destroyed)return false;
            if(!dictionariesStarted) {
                BootPreferences.refresh(this);
                dictionaryStartup=DictionaryStore.start(this);
                dictionariesStarted=true;
            } else if(dictionaryStartup!=null&&dictionaryStartup.isDone()) {
                try {dictionaryStartup.get();}
                catch(java.util.concurrent.ExecutionException error) {dictionaryStartup=DictionaryStore.retryStartup(this);}
                catch(InterruptedException error) {Thread.currentThread().interrupt();}
            }
            return true;
        }
    }
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
        int p=BootPreferences.forService(this,BootPreferences.unlocked(this)).getInt("profile",0);
        return item==null ? null : item.voice(Catalog.profile(null,p));
    }

    /** Disk I/O and core creation stay on the persistent engine worker. */
    private long ensureContext() throws Exception {
        // Generated lexicons live in credential storage; defer cleanup until
        // an unlocked request even if the native context was created earlier.
        if(!legacyFilesCleaned&&BootPreferences.unlocked(this)) {
            File[] oldData=getFilesDir().listFiles();
            if(oldData!=null)for(File old:oldData)
                if(old.getName().matches("german-(?:[0-9a-f]{64}|0\\.2\\.(?:189|190))\\.rlex") ||
                   old.getName().equals("german.rlex.tmp"))old.delete();
            legacyFilesCleaned=true;
        }
        synchronized(contextGuard) {
            if(destroyed) return 0;
            if(context!=0) return context;
        }
        long created=NativeEngine.create();
        if(created==0) throw new IllegalStateException("Native engine initialization failed");
        synchronized(contextGuard) {
            if(!destroyed) { context=created; return created; }
        }
        NativeEngine.destroy(created);
        return 0;
    }
    @Override protected void onSynthesizeText(SynthesisRequest request,SynthesisCallback callback) {
        android.content.SharedPreferences preferences=BootPreferences.forService(this,BootPreferences.unlocked(this));
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
        final boolean unlocked=startDictionariesIfUnlocked();
        final Future<?> startup;
        synchronized(dictionaryGuard) { startup=unlocked?dictionaryStartup:null; }
        // If unlock races this request after the credential check, the
        // device copy remains usable for this utterance.
        final BootDictionaries.Lease bootDictionaries=!unlocked?BootDictionaries.acquire(this):null;
        boolean submitted=false;
        try {
            if(callback.start(Catalog.RATES[rate],AudioFormat.ENCODING_PCM_16BIT,1)!=TextToSpeech.SUCCESS)
                { queue.cancel(); return; }
            worker.execute(() -> {
                boolean success=false;
                DictionaryStore.Lease dictionaries=null;
                try {
                    if(!queue.isCancelled()) {
                        if(startup!=null)startup.get();
                        if(queue.isCancelled())return;
                        if(unlocked)dictionaries=DictionaryStore.acquire(this);
                        else if(preferences.getBoolean("dictionaryRequired",false)&&
                                (bootDictionaries==null||bootDictionaries.path==null))
                            throw new IllegalStateException("Direct Boot dictionary snapshot unavailable");
                        long handle=ensureContext();
                        if(handle!=0 && !queue.isCancelled()) {
                            NativeEngine.dictionary(handle,dictionaries!=null?dictionaries.path:
                                bootDictionaries!=null?bootDictionaries.path:null);
                            success=NativeEngine.synthesize(handle,dialect,controls,utf8,queue);
                        }
                    }
                } catch(Throwable error) { Log.e("ReLoq","Synthesis failed",error); }
                finally {if(dictionaries!=null)dictionaries.close();if(bootDictionaries!=null)bootDictionaries.close();queue.finish(success);}
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
            if(!submitted&&bootDictionaries!=null)bootDictionaries.close();
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
        final long detached;
        synchronized(contextGuard) { destroyed=true; detached=context; context=0; }
        synchronized(dictionaryGuard) {if(dictionariesStarted){DictionaryStore.stop();dictionariesStarted=false;}}
        worker.execute(() -> { if(detached!=0) NativeEngine.destroy(detached); });
        worker.shutdown();
        super.onDestroy();
    }
}
