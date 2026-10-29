package org.reloq.tts.preview;

import android.content.Context;
import android.content.SharedPreferences;
import android.net.Uri;
import android.database.Cursor;
import android.provider.DocumentsContract;
import org.json.*;
import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicBoolean;

/** Immutable native-volume snapshots, independent of the speech worker. */
final class DictionaryStore {
    static final ExecutorService WORK=new ThreadPoolExecutor(0,1,15,TimeUnit.SECONDS,
        new LinkedBlockingQueue<Runnable>(),r -> new Thread(r,"ReLoq dictionaries"));
    private static final Object GUARD=new Object();
    private static final HashMap<String,Integer> leases=new HashMap<>();
    private static final HashSet<String> published=new HashSet<>();
    private static boolean initialized;
    private static ScheduledExecutorService watcher;
    private static String watched,candidate;
    private static int users;
    private static final AtomicBoolean pollQueued=new AtomicBoolean(),pruneQueued=new AtomicBoolean();
    static SharedPreferences prefs(Context c){return c.getSharedPreferences("settings",Context.MODE_PRIVATE);}
    static File root(Context c){return new File(c.getFilesDir(),"dictionaries");}
    static JSONArray sources(Context c) {
        try{return new JSONArray(prefs(c).getString("dictionarySources","[]"));}
        catch(JSONException e){return new JSONArray();}
    }
    static final class Lease implements AutoCloseable {
        final Context context;final String path;boolean closed;
        Lease(Context c,String p){context=c;path=p;}
        public void close() {
            boolean cleanup;
            synchronized(GUARD){
                if(closed)return;closed=true;if(path!=null)leases.computeIfPresent(path,(k,n) -> n==1?null:n-1);
                cleanup=path!=null&&!path.equals(prefs(context).getString("dictionarySnapshot",null))&&!leases.containsKey(path);
            }
            if(cleanup&&pruneQueued.compareAndSet(false,true))WORK.execute(() -> {
                try{prune(context);}finally{pruneQueued.set(false);}
            });
        }
    }
    private static void initialize(Context c) {
        synchronized(GUARD) {
            if(initialized)return;initialized=true;
            File[] files=new File(root(c),"snapshots").listFiles();
            if(files!=null)for(File f:files)if(new File(f,"ready").isFile())published.add(f.getAbsolutePath());
            files=new File(root(c),"community").listFiles();
            if(files!=null)for(File f:files)if(new File(f,"manifest.json").isFile())published.add(f.getAbsolutePath());
        }
    }
    static Lease acquire(Context c) {
        initialize(c);
        synchronized(GUARD) {
            String path=prefs(c).getString("dictionarySnapshot",null);
            if(path!=null)leases.put(path,leases.getOrDefault(path,0)+1);
            return new Lease(c.getApplicationContext(),path);
        }
    }
    private static void prune(Context c) {
        ArrayList<String> remove=new ArrayList<>();
        synchronized(GUARD) {
            String active=prefs(c).getString("dictionarySnapshot",null);
            String commit=prefs(c).getString("communityCommit","");
            String community=new File(new File(root(c),"community"),commit).getAbsolutePath();
            for(String path:published)if(!path.equals(active)&&!path.equals(community)&&!leases.containsKey(path))remove.add(path);
            published.removeAll(remove);
        }
        for(String path:remove)erase(new File(path));
    }
    private static void erase(File path) {
        File[] children=path.listFiles();if(children!=null)for(File child:children)erase(child);path.delete();
    }
    private static void write(File path,byte[] data) throws IOException {
        try(FileOutputStream out=new FileOutputStream(path)){out.write(data);out.getFD().sync();}
    }
    private static byte[] bytes(File path) throws IOException {
        try(InputStream in=new FileInputStream(path)){return DictionaryData.read(in,DictionaryData.LIMIT);}
    }
    private static JSONObject json(File path) throws Exception {return new JSONObject(new String(bytes(path),StandardCharsets.UTF_8));}
    private static File community(Context c) throws Exception {
        File base=new File(root(c),"community");base.mkdirs();
        String active=prefs(c).getString("communityCommit",null);
        if(active!=null&&active.matches("[a-f0-9]{40}")) {
            File cached=new File(base,active);if(new File(cached,"manifest.json").isFile())return cached;
        }
        byte[] metadata;
        try(InputStream in=c.getAssets().open("community/manifest.json")){metadata=DictionaryData.read(in,1024*1024);}
        JSONObject manifest=new JSONObject(new String(metadata,StandardCharsets.UTF_8));
        String commit=manifest.getString("commit");if(!commit.matches("[a-f0-9]{40}"))throw new IOException("Invalid bundled dictionary identity");
        File target=new File(base,commit);
        if(!new File(target,"manifest.json").isFile()) {
            File stage=new File(base,"stage");erase(stage);stage.mkdirs();
            try {
                for(String name:DictionaryData.COMMUNITY) {
                    byte[] data;try(InputStream in=c.getAssets().open("community/"+name)){data=DictionaryData.read(in,DictionaryData.LIMIT);}
                    verify(manifest,name,data);write(new File(stage,name),data);
                }
                write(new File(stage,"manifest.json"),metadata);
                if(!stage.renameTo(target))throw new IOException("Dictionary installation failed");
            }finally{erase(stage);}
        }
        prefs(c).edit().putString("communityCommit",commit).apply();return target;
    }
    private static void verify(JSONObject manifest,String name,byte[] data) throws Exception {
        JSONObject row=manifest.getJSONObject("files").getJSONObject(name);
        if(data.length!=row.getInt("size")||!DictionaryData.blob(data).equals(row.getString("sha")))
            throw new IOException("Dictionary verification failed: "+name);
    }
    static String rebuild(Context c,boolean enabled,JSONArray own) throws Exception {
        initialize(c);String before=state(c,own);DictionaryData.Merger merger=new DictionaryData.Merger();
        File original=null;
        if(enabled) {
            File base=community(c);JSONObject manifest=json(new File(base,"manifest.json"));
            for(String name:DictionaryData.COMMUNITY)if(name.endsWith(".dic")) {
                byte[] data=bytes(new File(base,name));verify(manifest,name,data);
                if(own.length()!=0)merger.add(name,data);
            }
            if(own.length()==0)original=base;
        }
        for(int i=0;i<own.length();++i) {
            JSONObject source=own.getJSONObject(i);byte[] data;
            try(InputStream in=c.getContentResolver().openInputStream(Uri.parse(source.getString("uri")))) {
                if(in==null)throw new IOException("Dictionary unavailable");data=DictionaryData.read(in,DictionaryData.LIMIT);
            }
            merger.add(DictionaryData.name(source.getInt("language"),source.getInt("volume")),data);
        }
        TreeMap<String,byte[]> volumes=merger.finish();String path=original==null?null:original.getAbsolutePath();
        if(!volumes.isEmpty()) {
            MessageDigest hash=MessageDigest.getInstance("SHA-256");
            for(Map.Entry<String,byte[]> row:volumes.entrySet()) {
                hash.update(row.getKey().getBytes(StandardCharsets.US_ASCII));hash.update((byte)0);
                hash.update(Integer.toString(row.getValue().length).getBytes(StandardCharsets.US_ASCII));hash.update((byte)0);hash.update(row.getValue());
            }
            File folder=new File(root(c),"snapshots");folder.mkdirs();
            File target=new File(folder,DictionaryData.hex(hash.digest()));
            if(!new File(target,"ready").isFile()) {
                File stage=new File(folder,"stage");erase(stage);stage.mkdirs();
                try {
                    for(Map.Entry<String,byte[]> row:volumes.entrySet())write(new File(stage,row.getKey()),row.getValue());
                    write(new File(stage,"ready"),new byte[0]);
                    if(!stage.renameTo(target))throw new IOException("Dictionary snapshot failed");
                }finally{erase(stage);}
            }
            path=target.getAbsolutePath();
        }
        String after=state(c,own);
        if(!before.equals(after))throw new IOException("Dictionary source changed during reload");
        synchronized(GUARD) {
            SharedPreferences.Editor editor=prefs(c).edit().putBoolean("communityDictionaries",enabled)
                .putString("dictionarySources",own.toString()).putString("dictionarySourceState",after);
            if(path==null)editor.remove("dictionarySnapshot");else {editor.putString("dictionarySnapshot",path);published.add(path);}
            editor.apply();watched=after;candidate=null;
        }
        prune(c);monitor(c);return path;
    }
    private static byte[] download(String url,int limit) throws Exception {
        HttpURLConnection connection=(HttpURLConnection)new URL(url).openConnection();
        connection.setConnectTimeout(8000);connection.setReadTimeout(12000);
        connection.setRequestProperty("User-Agent","ReLoq-C-Android-dictionaries");
        try {
            if(connection.getResponseCode()!=200)throw new IOException("Dictionary server: HTTP "+connection.getResponseCode());
            try(InputStream in=connection.getInputStream()){return DictionaryData.read(in,limit);}
        }finally{connection.disconnect();}
    }
    static void updateCommunity(Context c) throws Exception {
        String api="https://api.github.com/repos/eigencrow/IBMTTSDictionaries";
        String commit=new JSONObject(new String(download(api+"/branches/master",1024*1024),StandardCharsets.UTF_8))
            .getJSONObject("commit").getString("sha");
        if(!commit.matches("[a-f0-9]{40}"))throw new IOException("Invalid master identity");
        File base=new File(root(c),"community");base.mkdirs();File target=new File(base,commit);
        if(!new File(target,"manifest.json").isFile()) {
            JSONArray tree=new JSONObject(new String(download(api+"/git/trees/"+commit,2*1024*1024),StandardCharsets.UTF_8)).getJSONArray("tree");
            JSONObject records=new JSONObject();
            for(int i=0;i<tree.length();++i) {
                JSONObject row=tree.getJSONObject(i);String name=row.getString("path");
                if(Arrays.asList(DictionaryData.COMMUNITY).contains(name)&&row.optString("type").equals("blob"))
                    records.put(name,new JSONObject().put("sha",row.getString("sha")).put("size",row.getInt("size")));
            }
            JSONObject manifest=new JSONObject().put("repository","eigencrow/IBMTTSDictionaries").put("branch","master")
                .put("commit",commit).put("files",records);
            File stage=new File(base,"download");erase(stage);stage.mkdirs();
            try {
                for(String name:DictionaryData.COMMUNITY) {
                    byte[] data=download("https://raw.githubusercontent.com/eigencrow/IBMTTSDictionaries/"+commit+"/"+name,DictionaryData.LIMIT);
                    verify(manifest,name,data);write(new File(stage,name),data);
                }
                write(new File(stage,"manifest.json"),manifest.toString().getBytes(StandardCharsets.UTF_8));
                if(!stage.renameTo(target))throw new IOException("Dictionary update failed");
            }finally{erase(stage);}
        }
        String previous=prefs(c).getString("communityCommit",null);
        prefs(c).edit().putString("communityCommit",commit).apply();
        try{rebuild(c,prefs(c).getBoolean("communityDictionaries",false),sources(c));}
        catch(Exception error){prefs(c).edit().putString("communityCommit",previous).apply();throw error;}
        File[] old=base.listFiles();synchronized(GUARD){if(old!=null)for(File folder:old)
            if(folder.getName().matches("[a-f0-9]{40}"))published.add(folder.getAbsolutePath());}
        prune(c);
    }
    private static String state(Context c,JSONArray sources) {
        StringBuilder result=new StringBuilder(sources.toString());
        for(int i=0;i<sources.length();++i)try {
            Uri uri=Uri.parse(sources.getJSONObject(i).getString("uri"));
            try(Cursor cursor=c.getContentResolver().query(uri,new String[]{DocumentsContract.Document.COLUMN_LAST_MODIFIED,
                    DocumentsContract.Document.COLUMN_SIZE},null,null,null)) {
                if(cursor!=null&&cursor.moveToFirst())result.append(':').append(cursor.getString(0)).append(':').append(cursor.getString(1));
            }
        }catch(Exception e){result.append(":unavailable");}
        return result.toString();
    }
    private static void monitor(Context c) {
        synchronized(GUARD) {
            if(users==0||sources(c).length()==0) {
                if(watcher!=null){watcher.shutdownNow();watcher=null;}
                return;
            }
            if(watcher!=null)return;
            watched=prefs(c).getString("dictionarySourceState",null);candidate=null;
            watcher=Executors.newSingleThreadScheduledExecutor(r -> new Thread(r,"ReLoq dictionary monitor"));
            watcher.scheduleWithFixedDelay(() -> {if(pollQueued.compareAndSet(false,true))WORK.execute(() -> {
                try {
                JSONArray own=sources(c);if(own.length()==0)return;
                String current=state(c,own);boolean changed;
                synchronized(GUARD) {
                    if(watched==null){watched=current;return;}
                    changed=!current.equals(watched)&&current.equals(candidate);
                    candidate=current.equals(watched)?null:current;
                }
                if(changed)try{rebuild(c,prefs(c).getBoolean("communityDictionaries",false),own);}
                catch(Exception error){synchronized(GUARD){watched=current;candidate=null;}android.util.Log.w("ReLoq","Dictionary refresh failed",error);}
                }finally{pollQueued.set(false);}
            });},2,2,TimeUnit.SECONDS);
        }
    }
    static void start(Context context) {
        Context c=context.getApplicationContext();initialize(c);
        synchronized(GUARD){++users;}
        monitor(c);
    }
    static void stop() {
        synchronized(GUARD){if(users>0&&--users==0&&watcher!=null){watcher.shutdownNow();watcher=null;}}
    }
}
