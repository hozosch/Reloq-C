package org.reloq.tts.preview;

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
                            if(op.startsWith("put")){pending.put((String)items[0],items[1]);return editor;}
                            if(op.equals("apply")||op.equals("commit")){values.putAll(pending);return op.equals("commit")?true:null;}
                            throw new AssertionError(op);
                        });
                }
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
            s.androidRate=false;s.manualWpm=280;s.androidPitch=false;s.manualPitch=43;
            s.gender=1;s.head=22;s.inflection=65;s.roughness=12;s.breathiness=60;
            s.volume=80;s.rate=6;s.pauses=2;s.tags=true;
            int[] expected={1,22,43,65,12,60,80,VoiceSettings.targetSpeed(280.0/190),6,2,1};
            require(Arrays.equals(s.controls(20,20),expected));
            require(Arrays.equals(s.controls(400,400),expected));
            Map<String,Object> values=new HashMap<>();SharedPreferences p=memory(values);
            Preferences.save(p,profile,s);
            require(Arrays.equals(Preferences.read(p,profile).controls(100,100),expected));
            VoiceSettings other=Preferences.read(p,(profile+1)%8);
            require(other.head==Catalog.PARAMETERS[(profile+1)%8][1]&&other.manualPitch==Catalog.PARAMETERS[(profile+1)%8][2]);
            s.androidRate=true;s.androidPitch=true;s.rateBoost=true;
            require(s.controls(100,200)[7]>55);
        }
        double f0=VoiceSettings.pitchHz(65),f2=VoiceSettings.pitchHz(VoiceSettings.androidPitch(65,200));
        require(Math.abs(f2/f0-2)<.04);
        require(VoiceSettings.androidPitch(65,1000)==100&&VoiceSettings.androidPitch(65,1)==0);
        Map<String,Object> legacy=new HashMap<>();legacy.put("rate",6);legacy.put("pauses",2);legacy.put("profile",2);
        VoiceSettings migrated=Preferences.read(memory(legacy),2);
        require(migrated.rate==6&&migrated.pauses==2&&migrated.androidRate&&migrated.androidPitch);
        require(migrated.controls(100,100)[2]==93&&migrated.manualWpm==190);

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
