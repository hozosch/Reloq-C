"""Exercise the real service across a locked/unlocked transition with host stubs.

The stubs refuse credential storage while locked and replace JNI with a PCM
producer. This checks service lifecycle and routing, not Android discovery,
native library loading or actual device playback.
"""
import argparse
from pathlib import Path
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET
from codegen import generate
from verify_apk import verify_direct_boot_manifest

ROOT=Path(__file__).resolve().parents[1]
STUBS={
"android/content/SharedPreferences.java": """package android.content;
import java.util.Map;
public interface SharedPreferences {
 Map<String,?> getAll(); int getInt(String k,int d); boolean getBoolean(String k,boolean d); String getString(String k,String d); Editor edit();
 interface Editor { Editor clear(); Editor remove(String k); Editor putInt(String k,int v); Editor putBoolean(String k,boolean v); Editor putString(String k,String v); void apply(); boolean commit(); }
}""",
"android/content/Context.java": """package android.content;
import java.io.File; import java.nio.file.Files; import java.util.*; import android.os.UserManager;
public class Context {
 public static final int MODE_PRIVATE=0; public static final String USER_SERVICE="user";
 public static boolean locked; private final boolean device;
 public static final Memory ce=new Memory(),de=new Memory(),deDictionaries=new Memory();
 private static File ceFiles,deFiles;
 static {try{File root=Files.createTempDirectory("reloq-boot-test").toFile();
  ceFiles=new File(root,"ce");deFiles=new File(root,"de");ceFiles.mkdirs();deFiles.mkdirs();}
  catch(Exception e){throw new RuntimeException(e);}}
 public Context(){this(false);} private Context(boolean d){device=d;}
 public Object getSystemService(String n){return new UserManager();}
 public Context createDeviceProtectedStorageContext(){return new Context(true);}
 public SharedPreferences getSharedPreferences(String n,int m){
  if(!device&&locked)throw new AssertionError("credential preferences accessed before unlock");
  return device?(n.equals("speech-dictionaries")?deDictionaries:de):ce;
 }
 public File getFilesDir(){if(!device&&locked)throw new AssertionError("credential files before unlock");return device?deFiles:ceFiles;}
 public Context getApplicationContext(){return this;}
 public static class Memory implements SharedPreferences {
  public final Map<String,Object> data=new HashMap<>(); public Map<String,?> getAll(){return new HashMap<>(data);}
  public int getInt(String k,int d){return (Integer)data.getOrDefault(k,d);}
  public boolean getBoolean(String k,boolean d){return (Boolean)data.getOrDefault(k,d);}
  public String getString(String k,String d){return (String)data.getOrDefault(k,d);}
  public Editor edit(){return new Editor(){Map<String,Object> v=new HashMap<>(); boolean clear;
   public Editor clear(){clear=true;return this;} public Editor putInt(String k,int x){v.put(k,x);return this;}
   public Editor putBoolean(String k,boolean x){v.put(k,x);return this;}
   public Editor putString(String k,String x){v.put(k,x);return this;} public Editor remove(String k){v.put(k,null);return this;}
   public void apply(){if(clear)data.clear();for(Map.Entry<String,Object> e:v.entrySet())if(e.getValue()==null)data.remove(e.getKey());else data.put(e.getKey(),e.getValue());}
   public boolean commit(){apply();return true;}};}
 }
}""",
"android/os/UserManager.java": """package android.os; public class UserManager {
 public boolean isUserUnlocked(){return !android.content.Context.locked;} }""",
"android/content/BroadcastReceiver.java": "package android.content; public abstract class BroadcastReceiver { public abstract void onReceive(Context c,Intent i); public PendingResult goAsync(){return new PendingResult();} public static class PendingResult { public void finish(){} } }",
"android/content/Intent.java": """package android.content; public class Intent {
 public static final String ACTION_MY_PACKAGE_REPLACED="android.intent.action.MY_PACKAGE_REPLACED";
 private String action; public Intent(String a){action=a;} public String getAction(){return action;} }""",
"android/media/AudioFormat.java": "package android.media; public class AudioFormat { public static final int ENCODING_PCM_16BIT=2; }",
"android/util/Log.java": "package android.util; public class Log { public static int e(String a,String b,Throwable c){return 0;} }",
"android/speech/tts/TextToSpeech.java": """package android.speech.tts; public class TextToSpeech {
 public static final int SUCCESS=0, ERROR=-1, ERROR_INVALID_REQUEST=-8, ERROR_SYNTHESIS=-3, ERROR_SERVICE=-4,
 LANG_NOT_SUPPORTED=-2, LANG_COUNTRY_AVAILABLE=1, LANG_AVAILABLE=0; }""",
"android/speech/tts/Voice.java": """package android.speech.tts; import java.util.*; public class Voice {
 public static final int QUALITY_NORMAL=300,LATENCY_VERY_LOW=100;
 public Voice(String n,Locale l,int q,int t,boolean net,Set<String> features){} }""",
"android/speech/tts/SynthesisRequest.java": """package android.speech.tts; public class SynthesisRequest {
 public String getVoiceName(){return null;} public String getLanguage(){return "de";} public String getCountry(){return "DE";}
 public int getSpeechRate(){return 100;} public int getPitch(){return 100;} public CharSequence getCharSequenceText(){return "Test";} }""",
"android/speech/tts/SynthesisCallback.java": """package android.speech.tts; public class SynthesisCallback {
 public int rate,bytes,errors,done; public int start(int r,int f,int c){rate=r;return 0;} public int getMaxBufferSize(){return 2;}
 public int audioAvailable(byte[] b,int o,int n){bytes+=n;return 0;} public void error(int e){++errors;} public int done(){++done;return 0;} }""",
"android/speech/tts/TextToSpeechService.java": """package android.speech.tts; import java.util.List;
public abstract class TextToSpeechService extends android.content.Context {
 public void onCreate(){} public void onDestroy(){} protected abstract String[] onGetLanguage();
 protected abstract int onIsLanguageAvailable(String a,String b,String c); protected abstract int onLoadLanguage(String a,String b,String c);
 public abstract List<Voice> onGetVoices(); public abstract int onIsValidVoiceName(String n); public abstract int onLoadVoice(String n);
 public abstract String onGetDefaultVoiceNameFor(String a,String b,String c); protected abstract void onSynthesizeText(SynthesisRequest r,SynthesisCallback c);
 protected abstract void onStop(); }""",
"org/reloq/tts/DictionaryStore.java": """package org.reloq.tts; import android.content.Context;
final class DictionaryStore { static int starts,stops,acquires,closes;static java.util.concurrent.Executor WORK=Runnable::run;
 static java.util.concurrent.CompletableFuture<Void> startup=java.util.concurrent.CompletableFuture.completedFuture(null);
 static void syncBoot(Context c) throws Exception {BootDictionaries.publish(c,BootDictionaries.prepare(c,Context.ce.getString("dictionarySnapshot",null)));}
 static java.util.concurrent.Future<?> start(Context c){if(Context.locked)throw new AssertionError();++starts;try{syncBoot(c);}catch(Exception e){throw new RuntimeException(e);}return startup;}
 static java.util.concurrent.Future<?> retryStartup(Context c){return startup;}
 static void stop(){++stops;} static Lease acquire(Context c){if(Context.locked)throw new AssertionError();++acquires;return new Lease(Context.ce.getString("dictionarySnapshot",null));}
 static class Lease { final String path;Lease(String p){path=p;} void close(){++closes;} } }""",
"org/reloq/tts/NativeEngine.java": """package org.reloq.tts;
final class NativeEngine { static int created,synthesized;static int[] controls;static String path;
 static long create(){++created;return 42;} static void dictionary(long c,String p){path=p;}
 static boolean synthesize(long c,int d,int[] v,byte[] t,PcmQueue q){++synthesized;controls=v;return q.write(new byte[]{1,2,3,4});}
 static void stop(long c,PcmQueue q){} static void destroy(long c){} }""",
"org/reloq/tts/DirectBootTest.java": """package org.reloq.tts;
import android.content.*;import android.speech.tts.*;import java.io.*;import java.nio.file.*;import java.util.concurrent.*;
public class DirectBootTest {
 static void check(boolean b){if(!b)throw new AssertionError();}
 static void speak(ReLoqService s){SynthesisCallback c=new SynthesisCallback();s.onSynthesizeText(new SynthesisRequest(),c);
  check(c.errors==0&&c.done==1&&c.bytes==4&&c.rate==16000);}
 public static void main(String[] x) throws Exception {
  Context.ce.data.put("profile",5);Context.ce.data.put("language",1);Context.ce.data.put("androidVoice",false);
  Context.ce.data.put("rate",2);Context.ce.data.put("androidPitch",false);Context.ce.data.put("voice.5.pitch",48);
  Context.ce.data.put("dictionarySources","private URI");
  File first=new File(new Context().getFilesDir(),"dictionaries/snapshots/"+"a".repeat(64));check(first.mkdirs());
  Files.writeString(new File(first,"DEUmain.dic").toPath(),"Apfel\\tApfelwort\\n");
  Context.ce.data.put("dictionarySnapshot",first.getAbsolutePath());
  new UpgradeReceiver().onReceive(new Context(),new Intent(Intent.ACTION_MY_PACKAGE_REPLACED));
  check(!Context.de.data.containsKey("dictionarySources"));
  Context.locked=true;BootPreferences.refresh(new Context());
  ReLoqService s=new ReLoqService();s.onCreate();
  check(s.onGetDefaultVoiceNameFor("de","DE","").endsWith("-Sandy"));speak(s);
  check(NativeEngine.created==1&&NativeEngine.synthesized==1&&NativeEngine.path!=null&&NativeEngine.controls[2]==48);
  check(Files.readString(new File(NativeEngine.path,"DEUmain.dic").toPath()).contains("Apfelwort"));
  check(DictionaryStore.starts==0&&DictionaryStore.acquires==0);
  Context.locked=false;Context.ce.data.put("voice.5.pitch",62);
  Context.ce.data.put("voice.5.pitchHertz",151);Context.ce.data.put("voice.5.pitchHertzMode",true);
  File second=new File(new Context().getFilesDir(),"dictionaries/snapshots/"+"b".repeat(64));check(second.mkdirs());
  Files.writeString(new File(second,"DEUmain.dic").toPath(),"Birne\\tBirnenwort\\nTokenX\\tbei\\n");
  BootDictionaries.Lease old=BootDictionaries.acquire(new Context());String oldPath=old.path;
  BootDictionaries.Lease another=BootDictionaries.acquire(new Context());
  String prepared=BootDictionaries.prepare(new Context(),second.getAbsolutePath());another.close();
  File preparedFile=new File(new Context().createDeviceProtectedStorageContext().getFilesDir(),"speech-dictionaries/"+prepared+"/ready");
  check(preparedFile.isFile());BootDictionaries.publish(new Context(),prepared);
  check(new File(oldPath).isDirectory());old.close();check(!new File(oldPath).exists());
  Context.ce.data.put("dictionarySnapshot",second.getAbsolutePath());
  DictionaryStore.startup=new CompletableFuture<>();
  CountDownLatch entered=new CountDownLatch(1);
  FutureTask<Void> firstSpeech=new FutureTask<>(() -> {entered.countDown();speak(s);return null;});
  new Thread(firstSpeech).start();check(entered.await(2,TimeUnit.SECONDS));
  Thread.sleep(100);check(NativeEngine.synthesized==1&&DictionaryStore.acquires==0);
  DictionaryStore.startup.complete(null);firstSpeech.get(3,TimeUnit.SECONDS);
  check(NativeEngine.created==1&&NativeEngine.synthesized==2&&NativeEngine.controls[2]==1151);
  check(second.getAbsolutePath().equals(NativeEngine.path));
  check(DictionaryStore.starts==1&&DictionaryStore.acquires==1&&DictionaryStore.closes==1);
  check(Context.de.data.get("voice.5.pitch").equals(62)&&Context.de.data.get("voice.5.pitchHertz").equals(151));s.onDestroy();check(DictionaryStore.stops==1);
  Context.locked=true;
  new UpgradeReceiver().onReceive(new Context(),new Intent(Intent.ACTION_MY_PACKAGE_REPLACED));
  ReLoqService next=new ReLoqService();next.onCreate();speak(next);
  check(NativeEngine.created==2&&NativeEngine.controls[2]==1151&&NativeEngine.path!=null);
  check(Files.readString(new File(NativeEngine.path,"DEUmain.dic").toPath()).contains("Birnenwort"));
  check(DictionaryStore.starts==1&&DictionaryStore.acquires==1);next.onDestroy();check(DictionaryStore.stops==1);
  Context.deDictionaries.data.put("active","f".repeat(64));
  ReLoqService missing=new ReLoqService();missing.onCreate();
  int beforeMissing=NativeEngine.synthesized;SynthesisCallback unavailable=new SynthesisCallback();
  missing.onSynthesizeText(new SynthesisRequest(),unavailable);
  check(unavailable.errors==1&&unavailable.done==1&&unavailable.bytes==0&&NativeEngine.synthesized==beforeMissing);
  missing.onDestroy();
  Context.locked=false;Context.ce.data.remove("dictionarySnapshot");Context.ce.data.remove("dictionarySources");
  BootPreferences.refresh(new Context());DictionaryStore.syncBoot(new Context());
  Context.locked=true;ReLoqService disabled=new ReLoqService();disabled.onCreate();speak(disabled);
  check(NativeEngine.path==null);disabled.onDestroy();
  Context.locked=false;Context.ce.data.put("dictionarySnapshot",second.getAbsolutePath());DictionaryStore.syncBoot(new Context());
  Context.locked=true;BootDictionaries.Lease nativeCopy=BootDictionaries.acquire(new Context());
  check(nativeCopy.path!=null);System.out.println("BOOT_SNAPSHOT\\t"+nativeCopy.path);
  System.out.println("Direct Boot service: locked PCM and active dictionaries, no credential access, unlock, migration, replacement and removal passed");
 } }"""
}

def main():
    p=argparse.ArgumentParser();p.add_argument("--java-home",type=Path,required=True)
    p.add_argument("--library",type=Path);args=p.parse_args()
    manifest=ET.parse(ROOT/"android/AndroidManifest.xml").getroot()
    a="{http://schemas.android.com/apk/res/android}"
    service=manifest.find("application/service")
    assert service is not None and service.get(a+"name")==".ReLoqService"
    assert service.get(a+"directBootAware")=="true" and service.get(a+"exported")=="true"
    assert [node.get(a+"name") for node in manifest.findall("uses-permission")]==["android.permission.INTERNET"]
    for namespace,value in (("android","(type 0x12)0xffffffff"),
                            ("http://schemas.android.com/apk/res/android","true")):
        tree=(f"    E: service (line=9)\n      A: {namespace}:name=\".ReLoqService\"\n"
              f"      A: {namespace}:directBootAware(0x01010505)={value}\n      E: intent-filter (line=10)\n")
        verify_direct_boot_manifest(tree)
    try:verify_direct_boot_manifest(tree.replace("=true","=false"))
    except ValueError:pass
    else:raise AssertionError("Disabled Direct Boot unexpectedly accepted")
    with tempfile.TemporaryDirectory() as temporary:
        build=Path(temporary);generate(build/"generated")
        for path,content in STUBS.items():
            target=build/path;target.parent.mkdir(parents=True,exist_ok=True);target.write_text(content)
        folder=ROOT/"android/java/org/reloq/tts"
        sources=[folder/(name+".java") for name in ("Catalog","VoiceSettings","Preferences","BootPreferences",
            "DictionaryData","BootDictionaries","UpgradeReceiver","PcmQueue","ReLoqService")]
        sources+=list(build.rglob("*.java"))
        subprocess.run([str(args.java_home/"bin/javac"),"-encoding","UTF-8","-d",str(build),*map(str,sources)],check=True)
        result=subprocess.run([str(args.java_home/"bin/java"),"-Djava.io.tmpdir="+str(build),"-cp",str(build),
            "org.reloq.tts.DirectBootTest"],check=True,timeout=30,capture_output=True,text=True)
        lines=result.stdout.splitlines();print(lines[-1])
        snapshot=Path(next(line.split("\t",1)[1] for line in lines if line.startswith("BOOT_SNAPSHOT\t")))
        assert build in snapshot.parents and "de" in snapshot.parts
        if args.library:
            sys.path.insert(0,str(ROOT/"nvda"))
            from _reloq_eci_direct import DirectEciEngine
            engine=DirectEciEngine(args.library.resolve(),dialect=0x40000,encoding="utf8",native_voice=True)
            try:
                engine.set_dictionary_path(snapshot)
                def capture(text):
                    chunks=[];assert engine.speak([text],chunks.append,lambda index:None)
                    return b"".join(chunks)
                assert capture("TokenX.")==capture("bei.")
            finally:engine.close()
            print("Copied Direct Boot dictionary: actual native pronunciation matches the enabled entry")

if __name__=="__main__":main()
