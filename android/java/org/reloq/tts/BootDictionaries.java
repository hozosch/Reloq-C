package org.reloq.tts;

import android.content.Context;
import android.content.SharedPreferences;
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.*;

/** Immutable copies of the activated dictionary snapshot in device storage. */
final class BootDictionaries {
    private static final Object GUARD=new Object();
    private static final HashMap<String,Integer> leases=new HashMap<>();
    private static final HashMap<String,Integer> prepared=new HashMap<>();
    private BootDictionaries() {}
    private static Context device(Context c){return c.createDeviceProtectedStorageContext();}
    private static File root(Context c){return new File(device(c).getFilesDir(),"speech-dictionaries");}
    private static SharedPreferences prefs(Context c){return device(c).getSharedPreferences("speech-dictionaries",Context.MODE_PRIVATE);}
    private static void erase(File file) {
        File[] children=file.listFiles();if(children!=null)for(File child:children)erase(child);
        file.delete();
    }
    private static void prune(Context c,String active) {
        File[] all=root(c).listFiles();if(all==null)return;
        for(File file:all)if(!file.getName().equals(active)&&!leases.containsKey(file.getName())&&
            !prepared.containsKey(file.getName()))erase(file);
    }
    /** Copy only known dictionary volumes from an already-published CE snapshot. */
    static String prepare(Context c,String path) throws Exception {
        if(path==null)return null;
        File source=new File(path).getCanonicalFile();
        File ce=new File(c.getFilesDir(),"dictionaries").getCanonicalFile();
        File parent=source.getParentFile();String id=source.getName();
        if(parent==null||!((parent.equals(new File(ce,"snapshots"))&&id.matches("[a-f0-9]{64}"))||
            (parent.equals(new File(ce,"community"))&&id.matches("[a-f0-9]{40}"))))
            throw new IOException("Invalid active dictionary snapshot");
        String name=DictionaryData.hash("SHA-256",source.getPath().getBytes(StandardCharsets.UTF_8));
        synchronized(GUARD) {
            File folder=root(c),target=new File(folder,name);
            if(new File(target,"ready").isFile()) {
                prepared.put(name,prepared.getOrDefault(name,0)+1);return name;
            }
            if(!folder.isDirectory()&&!folder.mkdirs())throw new IOException("Device dictionary directory unavailable");
            File stage=new File(folder,"stage");erase(stage);
            if(!stage.mkdir())throw new IOException("Device dictionary staging unavailable");
            try {
                int copied=0;Set<String> names=new TreeSet<>();
                for(Catalog.Language language:Catalog.LANGUAGES)
                    for(int volume=0;volume<3;++volume)names.add(DictionaryData.name(language.id,volume));
                File[] speech=source.listFiles((dir,item) -> SpeechDictionary.isSnapshotName(item));
                if(speech==null)throw new IOException("Dictionary snapshot unavailable");
                for(File file:speech)names.add(file.getName());
                for(String item:names) {
                    File input=new File(source,item);if(!input.isFile())continue;
                    if(!source.equals(input.getCanonicalFile().getParentFile()))throw new IOException("Invalid dictionary volume");
                    byte[] data;try(InputStream in=new FileInputStream(input)){data=DictionaryData.read(in,DictionaryData.LIMIT);}
                    try(FileOutputStream out=new FileOutputStream(new File(stage,item))){out.write(data);out.getFD().sync();}
                    ++copied;
                }
                // An explicitly selected empty source is still a complete
                // snapshot, marked by the atomic publisher's ready file.
                if(copied==0&&!new File(source,"ready").isFile())throw new IOException("Active dictionary snapshot is empty");
                try(FileOutputStream out=new FileOutputStream(new File(stage,"ready"))){out.getFD().sync();}
                if(target.exists())erase(target);
                if(!stage.renameTo(target))throw new IOException("Device dictionary publication failed");
                prepared.put(name,prepared.getOrDefault(name,0)+1);
                return name;
            } finally {erase(stage);}
        }
    }
    /** Called on the dictionary worker before reporting a successful change. */
    static void publish(Context c,String name) throws IOException {
        synchronized(GUARD) {
            SharedPreferences.Editor edit=prefs(c).edit();
            if(name==null)edit.remove("active");else edit.putString("active",name);
            if(!edit.commit())throw new IOException("Device dictionary selection could not be saved");
            if(name!=null)prepared.computeIfPresent(name,(k,n)->n==1?null:n-1);
            prune(c,name);
        }
    }
    static void discard(Context c,String name) {
        if(name==null)return;
        synchronized(GUARD) {
            prepared.computeIfPresent(name,(k,n)->n==1?null:n-1);
            prune(c,prefs(c).getString("active",null));
        }
    }
    static final class Lease implements AutoCloseable {
        private final Context context;private final String name;private boolean closed;
        final String path;
        Lease(Context c,String n,String p){context=c;name=n;path=p;}
        public void close() {
            synchronized(GUARD) {
                if(closed)return;closed=true;
                if(name!=null)leases.computeIfPresent(name,(k,n)->n==1?null:n-1);
                if(name!=null)prune(context,prefs(context).getString("active",null));
            }
        }
    }
    static Lease acquire(Context c) {
        synchronized(GUARD) {
            String name=prefs(c).getString("active",null);String path=null;
            if(name!=null&&name.matches("[a-f0-9]{64}")) {
                File folder=new File(root(c),name);
                if(new File(folder,"ready").isFile()) {
                    path=folder.getAbsolutePath();leases.put(name,leases.getOrDefault(name,0)+1);
                } else name=null;
            } else name=null;
            return new Lease(c.getApplicationContext(),name,path);
        }
    }
}
