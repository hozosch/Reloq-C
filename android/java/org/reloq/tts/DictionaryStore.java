package org.reloq.tts;

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
    /** Check the complete, immutable snapshot before the first utterance.
     * The merged directory name is a hash of every volume and its contents;
     * the bundled set has a manifest with Git blob hashes instead. */
    private static boolean completeSnapshot(Context c,String path) {
        if(path==null)return false;
        try {
            File folder=new File(path).getCanonicalFile();
            File root=root(c).getCanonicalFile(),parent=folder.getParentFile();
            String name=folder.getName();
            if(parent!=null&&parent.equals(new File(root,"community"))&&name.matches("[a-f0-9]{40}")) {
                JSONObject manifest=json(new File(folder,"manifest.json"));
                if(!name.equals(manifest.getString("commit")))return false;
                for(String item:DictionaryData.COMMUNITY)verify(manifest,item,bytes(new File(folder,item)));
                return true;
            }
            if(parent==null||!parent.equals(new File(root,"snapshots"))||
                    !name.matches("[a-f0-9]{64}")||!new File(folder,"ready").isFile())return false;
            File[] entries=folder.listFiles((dir,item) -> item.endsWith(".dic"));
            if(entries==null)return false;
            Arrays.sort(entries,Comparator.comparing(File::getName));
            MessageDigest hash=MessageDigest.getInstance("SHA-256");
            for(File entry:entries) {
                if(!folder.equals(entry.getCanonicalFile().getParentFile()))return false;
                byte[] data=bytes(entry);
                hash.update(entry.getName().getBytes(StandardCharsets.US_ASCII));hash.update((byte)0);
                hash.update(Integer.toString(data.length).getBytes(StandardCharsets.US_ASCII));hash.update((byte)0);hash.update(data);
            }
            return name.equals(DictionaryData.hex(hash.digest()));
        } catch(Exception error) {return false;}
    }
    private static void prepareStartup(Context c) throws Exception {
        SharedPreferences settings=prefs(c);
        // Invalid saved selections must not silently become an empty set.
        JSONArray own=new JSONArray(settings.getString("dictionarySources","[]"));
        boolean enabled=settings.getBoolean("communityDictionaries",false);
        String path=settings.getString("dictionarySnapshot",null);
        boolean expected=enabled||own.length()!=0;
        if(!expected&&path==null)return;
        boolean complete=expected?completeSnapshot(c,path):path==null;
        String content=sourceContent(c,own);
        if(!complete||!Objects.equals(state(c,own),settings.getString("dictionarySourceState",null))||
                !content.equals(settings.getString("dictionaryContentState",null)))
            rebuild(c,enabled,own);
    }
    private static void hashSource(MessageDigest hash,byte[] data) {
        hash.update(Integer.toString(data.length).getBytes(StandardCharsets.US_ASCII));
        hash.update((byte)0);hash.update(data);
    }
    private static String sourceContent(Context c,JSONArray own) throws Exception {
        MessageDigest hash=MessageDigest.getInstance("SHA-256");
        hash.update(own.toString().getBytes(StandardCharsets.UTF_8));hash.update((byte)0);
        for(int i=0;i<own.length();++i) {
            Uri uri=Uri.parse(own.getJSONObject(i).getString("uri"));
            try(InputStream in=c.getContentResolver().openInputStream(uri)) {
                if(in==null)throw new IOException("Dictionary unavailable");
                hashSource(hash,DictionaryData.read(in,DictionaryData.LIMIT));
            }
        }
        return DictionaryData.hex(hash.digest());
    }
    private static void prune(Context c) {
        ArrayList<String> remove=new ArrayList<>();
        synchronized(GUARD) {
            String active=prefs(c).getString("dictionarySnapshot",null);
            String commit=prefs(c).getString("communityCommit","");
            String community=new File(new File(root(c),"community"),commit).getAbsolutePath();
            String altCommit=prefs(c).getString("alternativeCommit","");
            String alternative=new File(new File(root(c),"alternative"),altCommit).getAbsolutePath();
            for(String path:published)if(!path.equals(active)&&!path.equals(community)&&!path.equals(alternative)&&!leases.containsKey(path))remove.add(path);
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
    static String repository(Context c) {
        return "alternative".equals(prefs(c).getString("dictionaryRepository","community")) ? "alternative" : "community";
    }
    private static String commitKey(String repository) {return repository.equals("alternative") ? "alternativeCommit" : "communityCommit";}
    private static File community(Context c) throws Exception {
        String repository=repository(c);
        File base=new File(root(c),repository);base.mkdirs();
        String active=prefs(c).getString(commitKey(repository),null);
        if(active!=null&&active.matches("[a-f0-9]{40}")) {
            File cached=new File(base,active);if(new File(cached,"manifest.json").isFile())return cached;
        }
        if(repository.equals("alternative")) {
            File downloaded=downloadCommunity(c,repository);
            prefs(c).edit().putString(commitKey(repository),downloaded.getName()).apply();
            return downloaded;
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
        MessageDigest sourceHash=MessageDigest.getInstance("SHA-256");
        sourceHash.update(own.toString().getBytes(StandardCharsets.UTF_8));sourceHash.update((byte)0);
        File original=null;
        if(enabled) {
            File base=community(c);JSONObject manifest=json(new File(base,"manifest.json"));
            boolean alternative=repository(c).equals("alternative");
            String[] names=alternative?DictionaryData.ALTERNATIVE:DictionaryData.COMMUNITY;
            for(String name:names)if(name.endsWith(".dic")) {
                byte[] data=bytes(new File(base,name));verify(manifest,name,data);
                if(alternative)merger.addAlternative(name,data);
                else if(own.length()!=0)merger.add(name,data);
            }
            if(!alternative&&own.length()==0)original=base;
        }
        for(int i=0;i<own.length();++i) {
            JSONObject source=own.getJSONObject(i);byte[] data;
            try(InputStream in=c.getContentResolver().openInputStream(Uri.parse(source.getString("uri")))) {
                if(in==null)throw new IOException("Dictionary unavailable");data=DictionaryData.read(in,DictionaryData.LIMIT);
            }
            hashSource(sourceHash,data);
            merger.add(DictionaryData.name(source.getInt("language"),source.getInt("volume")),data);
        }
        TreeMap<String,byte[]> volumes=merger.finish();String path=original==null?null:original.getAbsolutePath();
        if(!volumes.isEmpty()||((enabled||own.length()!=0)&&original==null)) {
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
        String bootSnapshot=BootDictionaries.prepare(c,path);
        boolean bootPublished=false;
        try {
            synchronized(GUARD) {
                BootDictionaries.publish(c,bootSnapshot);bootPublished=true;
                SharedPreferences.Editor editor=prefs(c).edit().putBoolean("communityDictionaries",enabled)
                    .putString("dictionarySources",own.toString()).putString("dictionarySourceState",after)
                    .putString("dictionaryContentState",DictionaryData.hex(sourceHash.digest()));
                if(path==null)editor.remove("dictionarySnapshot");else {editor.putString("dictionarySnapshot",path);published.add(path);}
                editor.apply();watched=after;candidate=null;
            }
        } finally {if(!bootPublished)BootDictionaries.discard(c,bootSnapshot);}
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
    private static File downloadCommunity(Context c,String repository) throws Exception {
        boolean alternative=repository.equals("alternative");
        String repo=alternative?"Mohamed00/AltIBMTTSDictionaries":"eigencrow/IBMTTSDictionaries";
        String[] names=alternative?DictionaryData.ALTERNATIVE:DictionaryData.COMMUNITY;
        String[] remote=alternative?DictionaryData.ALTERNATIVE_REMOTE:DictionaryData.COMMUNITY;
        String api="https://api.github.com/repos/"+repo;
        String commit=new JSONObject(new String(download(api+"/branches/master",1024*1024),StandardCharsets.UTF_8))
            .getJSONObject("commit").getString("sha");
        if(!commit.matches("[a-f0-9]{40}"))throw new IOException("Invalid master identity");
        File base=new File(root(c),repository);base.mkdirs();File target=new File(base,commit);
        if(!new File(target,"manifest.json").isFile()) {
            JSONObject response=new JSONObject(new String(download(api+"/git/trees/"+commit,2*1024*1024),StandardCharsets.UTF_8));
            if(response.optBoolean("truncated"))throw new IOException("Incomplete dictionary tree");
            JSONArray tree=response.getJSONArray("tree");JSONObject records=new JSONObject();
            for(int i=0;i<tree.length();++i) {
                JSONObject row=tree.getJSONObject(i);String path=row.getString("path");
                for(int n=0;n<remote.length;++n)if(remote[n].equals(path)&&row.optString("type").equals("blob"))
                    records.put(names[n],new JSONObject().put("sha",row.getString("sha")).put("size",row.getInt("size")));
            }
            if(records.length()!=names.length)throw new IOException("Missing dictionary volume");
            JSONObject manifest=new JSONObject().put("repository",repo).put("branch","master")
                .put("commit",commit).put("files",records);
            File stage=new File(base,"download");erase(stage);stage.mkdirs();
            try {
                for(int n=0;n<names.length;++n) {
                    byte[] data=download("https://raw.githubusercontent.com/"+repo+"/"+commit+"/"+remote[n],DictionaryData.LIMIT);
                    verify(manifest,names[n],data);write(new File(stage,names[n]),data);
                }
                write(new File(stage,"manifest.json"),manifest.toString().getBytes(StandardCharsets.UTF_8));
                if(!stage.renameTo(target))throw new IOException("Dictionary update failed");
            }finally{erase(stage);}
        }
        for(String name:names)verify(json(new File(target,"manifest.json")),name,bytes(new File(target,name)));
        return target;
    }
    private static Map<String,byte[]> sourceVolumes(File folder,String source) throws Exception {
        if(folder==null)throw new IOException("Invalid dictionary snapshot");
        JSONObject manifest=json(new File(folder,"manifest.json"));
        String expected=source.equals("alternative")?"Mohamed00/AltIBMTTSDictionaries":"eigencrow/IBMTTSDictionaries";
        if(!expected.equalsIgnoreCase(manifest.getString("repository"))||
           !"master".equals(manifest.getString("branch"))||!folder.getName().equals(manifest.getString("commit")))
            throw new IOException("Invalid dictionary snapshot identity");
        String[] names=source.equals("alternative")?DictionaryData.ALTERNATIVE:DictionaryData.COMMUNITY;
        Map<String,byte[]> volumes=new TreeMap<>();
        for(String name:names)if(name.endsWith(".dic")) {
            byte[] data=bytes(new File(folder,name));verify(manifest,name,data);volumes.put(name,data);
        }
        return volumes;
    }
    private static File sourceRevision(Context c,String source,String commit) {
        return commit!=null&&commit.matches("[a-f0-9]{40}")?
            new File(new File(root(c),source),commit):null;
    }
    private static DictionaryData.UpdateSummary summarize(Context c,String source,String previous,File target) {
        try {
            File old=sourceRevision(c,source,previous);
            if(previous!=null&&old==null)return DictionaryData.UpdateSummary.unknown();
            Map<String,byte[]> before=old==null?null:sourceVolumes(old,source);
            return DictionaryData.compare(before,sourceVolumes(target,source),source.equals("alternative"));
        } catch(Exception ignored) { return DictionaryData.UpdateSummary.unknown(); }
    }
    static DictionaryData.UpdateSummary updateCommunity(Context c) throws Exception {
        String source=repository(c),key=commitKey(source);
        String previous=prefs(c).getString(key,null);
        if(previous==null&&source.equals("community"))previous=community(c).getName();
        File target=downloadCommunity(c,source);
        DictionaryData.UpdateSummary summary=summarize(c,source,previous,target);
        prefs(c).edit().putString(key,target.getName()).apply();
        try{rebuild(c,prefs(c).getBoolean("communityDictionaries",false),sources(c));}
        catch(Exception error){prefs(c).edit().putString(key,previous).apply();throw error;}
        synchronized(GUARD){published.add(target.getAbsolutePath());}
        prune(c);
        return summary;
    }
    static DictionaryData.UpdateSummary selectRepository(Context c,String selected) throws Exception {
        if(!selected.equals("community")&&!selected.equals("alternative"))throw new IOException("Invalid dictionary source");
        String previous=repository(c);if(selected.equals(previous))return null;
        String oldCommit=prefs(c).getString(commitKey(selected),null);
        File old=sourceRevision(c,selected,oldCommit);
        boolean missing=old==null||!new File(old,"manifest.json").isFile();
        prefs(c).edit().putString("dictionaryRepository",selected).apply();
        try{rebuild(c,prefs(c).getBoolean("communityDictionaries",false),sources(c));}
        catch(Exception error){prefs(c).edit().putString("dictionaryRepository",previous).apply();throw error;}
        String current=prefs(c).getString(commitKey(selected),null);
        if(selected.equals("alternative")&&current!=null&&(missing||!current.equals(oldCommit)))
            return summarize(c,selected,oldCommit,sourceRevision(c,selected,current));
        return null;
    }
    static DictionaryData.UpdateSummary rebuildWithFeedback(Context c,boolean enabled,JSONArray own) throws Exception {
        String source=repository(c),old=prefs(c).getString(commitKey(source),null);
        File oldFolder=sourceRevision(c,source,old);
        boolean missing=oldFolder==null||!new File(oldFolder,"manifest.json").isFile();
        rebuild(c,enabled,own);
        String current=prefs(c).getString(commitKey(source),null);
        if(enabled&&source.equals("alternative")&&current!=null&&(missing||!current.equals(old)))
            return summarize(c,source,old,sourceRevision(c,source,current));
        return null;
    }
    private static Map<String,byte[]> activeVolumes(String path) throws IOException {
        if(path==null)return null;
        File[] files=new File(path).listFiles();if(files==null)throw new IOException("Dictionary snapshot unavailable");
        Map<String,byte[]> result=new TreeMap<>();
        for(File file:files)if(file.isFile()&&file.getName().endsWith(".dic"))result.put(file.getName(),bytes(file));
        return result;
    }
    static DictionaryData.UpdateSummary reloadWithFeedback(Context c,boolean enabled,JSONArray own) throws Exception {
        Map<String,byte[]> before=null;boolean known=true;
        try(Lease lease=acquire(c)) {
            try{before=activeVolumes(lease.path);}catch(IOException error){known=false;}
            rebuild(c,enabled,own);
            try(Lease current=acquire(c)) {
                if(!known)return DictionaryData.UpdateSummary.unknown();
                try {
                    Map<String,byte[]> after=activeVolumes(current.path);
                    return DictionaryData.compare(before,after==null?Collections.emptyMap():after,false);
                } catch(IOException error) {return DictionaryData.UpdateSummary.unknown();}
            }
        }
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
    static Future<?> start(Context context) {
        Context c=context.getApplicationContext();initialize(c);
        synchronized(GUARD){++users;}
        monitor(c);
        return retryStartup(c);
    }
    static Future<?> retryStartup(Context context) {
        Context c=context.getApplicationContext();
        return WORK.submit(() -> {
            try {prepareStartup(c);syncBoot(c);}
            catch(Exception error) {throw new CompletionException(error);}
        });
    }
    static void queueSyncBoot(Context context) {
        Context c=context.getApplicationContext();
        WORK.execute(() -> {try{syncBoot(c);}catch(Exception error){android.util.Log.e("ReLoq","Direct Boot dictionary copy failed",error);}});
    }
    /** Copy an existing installation's active snapshot without changing it. */
    static void syncBoot(Context c) throws Exception {
        if(!BootPreferences.unlocked(c))return;
        try(Lease lease=acquire(c)) {
            String prepared=BootDictionaries.prepare(c,lease.path);
            boolean bootPublished=false;
            try {
                synchronized(GUARD) {
                    if(Objects.equals(lease.path,prefs(c).getString("dictionarySnapshot",null))) {
                        BootDictionaries.publish(c,prepared);bootPublished=true;
                    }
                }
            } finally {if(!bootPublished)BootDictionaries.discard(c,prepared);}
        }
    }
    static void stop() {
        synchronized(GUARD){if(users>0&&--users==0&&watcher!=null){watcher.shutdownNow();watcher=null;}}
    }
}
