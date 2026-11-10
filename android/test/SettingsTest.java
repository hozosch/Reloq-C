package org.reloq.tts;

import android.content.SharedPreferences;
import java.io.*;
import java.lang.reflect.Proxy;
import java.nio.charset.StandardCharsets;
import java.util.*;

/** Exercise request controls, old preferences and exact native dictionary bytes. */
public final class SettingsTest {
    private static int checks;
    private static void require(boolean value) {++checks;if(!value)throw new AssertionError("check "+checks);}
    private static SharedPreferences memory(Map<String,Object> values) {
        return (SharedPreferences)Proxy.newProxyInstance(SettingsTest.class.getClassLoader(),
            new Class<?>[]{SharedPreferences.class},(proxy,method,args) -> {
                String name=method.getName();
                if(name.equals("edit")) {
                    Map<String,Object> pending=new HashMap<>();
                    return Proxy.newProxyInstance(SettingsTest.class.getClassLoader(),
                        new Class<?>[]{SharedPreferences.Editor.class},(editor,operation,items) -> {
                            String op=operation.getName();
                            if(op.equals("clear")){pending.clear();pending.put("__clear__",true);return editor;}
                            if(op.startsWith("put")){pending.put((String)items[0],items[1]);return editor;}
                            if(op.equals("apply")||op.equals("commit")){
                                if(pending.remove("__clear__")!=null)values.clear();
                                values.putAll(pending);return op.equals("commit")?true:null;}
                            throw new AssertionError(op);
                        });
                }
                if(name.equals("getAll"))return new HashMap<>(values);
                if(name.startsWith("get"))return values.getOrDefault((String)args[0],args[1]);
                throw new AssertionError(name);
            });
    }
    private static byte[] data(String value){return value.getBytes(StandardCharsets.ISO_8859_1);}
    private static void rejected(String value) throws Exception {
        try{new DictionaryData.Merger().add("ENUmain.dic",data(value));throw new AssertionError("accepted invalid file");}
        catch(IOException expected){++checks;}
    }
    public static void main(String[] arguments) throws Exception {
        int previous=40;
        for(int percent=1;percent<=800;++percent) {
            int nativeSpeed=VoiceSettings.androidSpeed(percent);
            require(nativeSpeed>=previous&&nativeSpeed>=40&&nativeSpeed<=250);previous=nativeSpeed;
        }
        require(VoiceSettings.androidSpeed(100)==55);
        require(VoiceSettings.androidSpeed(0)==40);
        double twice=VoiceSettings.timeScale(55)/VoiceSettings.timeScale(VoiceSettings.androidSpeed(200));
        require(Math.abs(twice-2)<.04);
        for(int profile=0;profile<Catalog.PROFILES.length;++profile) {
            VoiceSettings s=new VoiceSettings(profile);
            require(s.controls(100,100)[2]==s.pitch);
            s.androidRate=false;s.speedWpmMode=false;s.manualSpeed=87;s.manualWpm=280;s.androidPitch=false;s.manualPitch=43;
            s.gender=1;s.head=22;s.inflection=65;s.roughness=12;s.breathiness=60;
            s.volume=80;s.rate=6;s.pauses=2;s.tags=true;
            int[] expected={1,22,43,65,12,60,80,87,6,2,1,0,0,0,0,0,12,12,12,
                0,125,12,100,0,500,12,100,0,1200,12,100,0,3000,12,100,0,6000,12,100};
            require(Arrays.equals(s.controls(20,20),expected));
            require(Arrays.equals(s.controls(400,400),expected));
            Map<String,Object> values=new HashMap<>();SharedPreferences p=memory(values);
            Preferences.save(p,profile,s);
            require(Arrays.equals(Preferences.read(p,profile).controls(100,100),expected));
            s.speedWpmMode=true;require(s.controls(100,100)[15]==280&&s.controls(100,100)[7]==87);
            Preferences.save(p,profile,s);
            VoiceSettings wpm=Preferences.read(p,profile);
            require(wpm.speedWpmMode&&wpm.manualWpm==280&&wpm.manualSpeed==87&&wpm.controls(1,100)[15]==280);
            wpm.androidRate=true;require(wpm.controls(100,100)[15]==0);
            s.pitchHertz=151;s.pitchHertzMode=true;
            require(s.controls(100,100)[2]==1151);
            Preferences.save(p,profile,s);
            VoiceSettings exact=Preferences.read(p,profile);
            require(exact.pitchHertzMode&&exact.pitchHertz==151&&exact.controls(100,100)[2]==1151);
            exact.androidPitch=true;require(exact.controls(100,100)[2]==exact.pitch);
            VoiceSettings other=Preferences.read(p,(profile+1)%8);
            require(other.head==Catalog.PARAMETERS[(profile+1)%8][1]&&other.manualPitch==Catalog.PARAMETERS[(profile+1)%8][2]&&!other.pitchHertzMode);
            s.androidRate=true;s.androidPitch=true;s.rateBoost=true;
            require(s.controls(100,200)[7]>55);
        }
        VoiceSettings options=new VoiceSettings(0);options.abbreviations=false;options.prediction=true;options.originalQuestions=true;options.mellowMode=true;
        Map<String,Object> stored=new HashMap<>();Preferences.save(memory(stored),0,options);
        VoiceSettings restored=Preferences.read(memory(stored),0);
        require(!restored.abbreviations&&restored.prediction&&restored.originalQuestions&&restored.mellowMode&&restored.controls(100,100)[11]==1&&restored.controls(100,100)[12]==1&&restored.controls(100,100)[13]==1&&restored.controls(100,100)[14]==1);
        options.eqEnabled=true;options.eqPreset=1;options.eqLow=-3;options.eqMid=2;options.eqHigh=4;
        options.eqFilters[0]=new int[]{1,777,6,50};options.eqFilters[4]=new int[]{1,22000,-4,150};
        Preferences.save(memory(stored),0,options);restored=Preferences.read(memory(stored),3);
        int[] equalized=restored.controls(100,100);
        require(restored.eqEnabled&&restored.eqPreset==1&&restored.eqLow==-3&&restored.eqMid==2&&restored.eqHigh==4);
        require(equalized[16]==16&&equalized[17]==12&&equalized[18]==11);
        require(equalized[19]==1&&equalized[20]==777&&equalized[21]==18&&equalized[22]==50);
        require(equalized[35]==1&&equalized[36]==22000&&equalized[37]==8&&equalized[38]==150);
        restored.eqPreset=4;equalized=restored.controls(100,100);
        require(equalized[16]==9&&equalized[17]==14&&equalized[18]==16);
        restored.eqEnabled=false;equalized=restored.controls(100,100);
        require(equalized[16]==12&&equalized[17]==12&&equalized[18]==12&&equalized[19]==0&&equalized[35]==0);
        double f0=VoiceSettings.pitchHz(65),f2=VoiceSettings.pitchHz(VoiceSettings.androidPitch(65,200));
        require(Math.abs(f2/f0-2)<.04);
        require(VoiceSettings.androidPitch(65,1000)==100&&VoiceSettings.androidPitch(65,1)==0);
        Map<String,Object> legacy=new HashMap<>();legacy.put("rate",6);legacy.put("pauses",2);legacy.put("profile",2);
        VoiceSettings migrated=Preferences.read(memory(legacy),2);
        require(migrated.rate==6&&migrated.pauses==2&&migrated.androidRate&&migrated.androidPitch);
        require(migrated.controls(100,100)[2]==93&&migrated.manualWpm==190);
        legacy.put("androidRate",false);legacy.put("manualWpm",280);legacy.put("rateBoost",true);
        migrated=Preferences.read(memory(legacy),2);
        require(migrated.speedWpmMode&&migrated.manualWpm==280&&
            migrated.manualSpeed==VoiceSettings.targetSpeed(280.0/190*1.6)&&
            migrated.controls(100,100)[15]==280);

        // A direct-boot copy includes every voice control but never private
        // dictionary paths, imported text, or update credentials/metadata.
        Map<String,Object> privateData=new HashMap<>();
        privateData.put("profile",5);privateData.put("language",1);privateData.put("androidVoice",false);privateData.put("originalQuestions",true);privateData.put("mellowMode",true);
        privateData.put("voice.5.pitch",48);privateData.put("voice.5.volume",71);
        privateData.put("voice.5.pitchHertz",151);privateData.put("voice.5.pitchHertzMode",true);
        privateData.put("manualSpeed",87);privateData.put("manualWpm",210);privateData.put("speedWpmMode",true);
        privateData.put("eqEnabled",true);privateData.put("eqFilter.0.enabled",true);
        privateData.put("eqFilter.0.frequency",777);privateData.put("eqFilter.0.gain",6);privateData.put("eqFilter.0.width",50);
        privateData.put("dictionarySnapshot","/private/dictionary");
        privateData.put("dictionarySources","content://private/words");
        privateData.put("communityCommit","secret");privateData.put("voice.99.pitch",33);
        privateData.put("voice.5.private",55);privateData.put("rate","wrong type");
        Map<String,Object> bootData=new HashMap<>();bootData.put("stalePrivate", "remove");
        BootPreferences.mirror(memory(privateData),memory(bootData));
        require(bootData.get("manualSpeed").equals(87)&&bootData.get("manualWpm").equals(210)&&bootData.get("speedWpmMode").equals(true));
        require(bootData.size()==18&&bootData.get("profile").equals(5)&&bootData.get("language").equals(1)&&
            bootData.get("androidVoice").equals(false)&&bootData.get("originalQuestions").equals(true)&&bootData.get("mellowMode").equals(true)&&
            bootData.get("dictionaryRequired").equals(true)&&bootData.get("voice.5.pitch").equals(48)&&bootData.get("voice.5.volume").equals(71)&&
            bootData.get("voice.5.pitchHertz").equals(151)&&bootData.get("voice.5.pitchHertzMode").equals(true));
        VoiceSettings bootVoice=Preferences.read(memory(bootData),5);
        require(bootVoice.manualPitch==48&&bootVoice.pitchHertz==151&&bootVoice.pitchHertzMode&&bootVoice.volume==71&&bootVoice.rate==1&&bootVoice.originalQuestions&&bootVoice.mellowMode&&bootVoice.speedWpmMode&&bootVoice.manualWpm==210&&bootVoice.manualSpeed==87);
        require(bootVoice.eqEnabled&&Arrays.equals(bootVoice.eqFilters[0],new int[]{1,777,6,50}));
        privateData.put("voice.5.pitch",52);privateData.remove("voice.5.volume");
        BootPreferences.mirror(memory(privateData),memory(bootData));
        require(bootData.get("voice.5.pitch").equals(52)&&!bootData.containsKey("voice.5.volume"));
        require(privateData.containsKey("dictionarySources"));

        DictionaryData.Merger merge=new DictionaryData.Merger();
        merge.add("ENUmain.dic",data("  Word\tapple  \r\nword\tpear\nignored\naccent\t\u0080\u00fc\nphoneme\t`0h@l'oU\n"));
        merge.add("ENUmain.dic",data("Word\tbanana\n"));
        merge.add("DEURoot.dic",data("\u00c4pfel\talt\n"));
        merge.add("DEURoot.dic",data("\u00e4pfel\tneu\n"));
        merge.add("DEUmain.dic",data("A\tein\na\tzwei\n"));
        Map<String,byte[]> result=merge.finish();
        require(Arrays.equals(result.get("ENUmain.dic"),data("Word\tbanana\naccent\t\u0080\u00fc\nphoneme\t`0h@l'oU\nword\tpear\n")));
        require(Arrays.equals(result.get("DEURoot.dic"),data("\u00e4pfel\tneu\n")));
        require(Arrays.equals(result.get("DEUmain.dic"),data("A\tein\na\tzwei\n")));
        Map<String,byte[]> before=Map.of("DEUmain.dic",data("Keep\told\nDelete\told\nDuplicate\tfirst\nDuplicate\tsecond\n"),
            "DEURoot.dic",data("READ\told\n"));
        Map<String,byte[]> after=Map.of("DEUmain.dic",data("Keep\tchanged\nDuplicate\tsecond\nAdded\tnew\nAdded\tnewer\n"),
            "DEURoot.dic",data("read\tchanged\nNEW\troot\n"));
        DictionaryData.UpdateSummary delta=DictionaryData.compare(before,after,false);
        require(delta.known&&delta.compared&&delta.added==2&&delta.total==5&&delta.addedFiles==0&&delta.totalFiles==2);
        delta=DictionaryData.compare(null,after,false);
        require(delta.known&&!delta.compared&&delta.added==5&&delta.total==5&&delta.addedFiles==2&&delta.totalFiles==2);
        require(DictionaryData.compare(after,after,false).added==0&&DictionaryData.compare(after,after,false).addedFiles==0);
        Map<String,byte[]> movedBefore=Map.of("DEUmain.dic",data("keep\tvalue\n"),"ENUmain.dic",data("old\tvalue\n"));
        Map<String,byte[]> movedAfter=Map.of("DEUmain.dic",data("keep\tvalue\n"),"FRAmain.dic",data("new\tvalue\n"));
        delta=DictionaryData.compare(movedBefore,movedAfter,false);
        require(delta.added==1&&delta.addedFiles==1&&delta.totalFiles==2);
        Map<String,byte[]> utf8=Map.of("ENUmain.dic","café\tcoffee\nnew\tword\n".getBytes(StandardCharsets.UTF_8));
        Map<String,byte[]> ansi=Map.of("ENUmain.dic",data("café\told\n"));
        require(DictionaryData.compare(ansi,utf8,true).added==1);
        DictionaryData.Merger alternative=new DictionaryData.Merger();
        alternative.addAlternative("ENUmain.dic","café\tcafé\n".getBytes(java.nio.charset.StandardCharsets.UTF_8));
        alternative.add("ENUmain.dic",data("café\town\n"));
        require(new String(alternative.finish().get("ENUmain.dic"),java.nio.charset.StandardCharsets.ISO_8859_1).contains("café\town"));
        require(DictionaryData.ALTERNATIVE.length==3&&DictionaryData.ALTERNATIVE_REMOTE[0].equals("enumain.dic"));
        require(DictionaryData.name(0xa0000,1).equals("KORRoot.dic"));
        require(DictionaryData.blob(new byte[0]).equals("e69de29bb2d1d6434b8b29ae775ad8c2e48c5391"));
        require(DictionaryData.hash("SHA-256",data("abc")).equals("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"));
        rejected("a\tb\0\n");rejected(String.join("",Collections.nCopies(129,"a"))+"\tb\n");
        rejected("a\t"+String.join("",Collections.nCopies(513,"b"))+"\n");
        try{DictionaryData.read(new ByteArrayInputStream(new byte[9]),8);throw new AssertionError();}
        catch(IOException expected){++checks;}
        System.out.println("Settings/dictionary data: "+checks+" checks passed");
        System.out.println("native_speeds\t"+VoiceSettings.androidSpeed(100)+"\t"+VoiceSettings.androidSpeed(200));
    }
}
