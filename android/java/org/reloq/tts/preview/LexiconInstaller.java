package org.reloq.tts.preview;

import android.content.Context;
import java.io.*;
import java.security.MessageDigest;

final class LexiconInstaller {
    static File path(Context context) {return new File(context.getFilesDir(),BuildData.LEXICON_NAME);}
    static void ensure(Context context) throws Exception {
        File target=path(context);
        if(target.isFile()&&target.length()==BuildData.LEXICON_BYTES)return;
        File temporary=new File(context.getFilesDir(),"german.rlex.tmp");
        MessageDigest hash=MessageDigest.getInstance("SHA-256");
        try(InputStream input=context.getAssets().open("german.rlex");FileOutputStream output=new FileOutputStream(temporary)) {
            byte[] bytes=new byte[16384];int count;
            while((count=input.read(bytes))!=-1){output.write(bytes,0,count);hash.update(bytes,0,count);}
            output.getFD().sync();
        }
        if(temporary.length()!=BuildData.LEXICON_BYTES||!DictionaryData.hex(hash.digest()).equals(BuildData.LEXICON_SHA256))
            throw new IOException("Invalid bundled lexicon");
        if(!temporary.renameTo(target))throw new IOException("Lexicon installation failed");
        // These are generated data files from earlier previews, not user dictionaries.
        File[] files=context.getFilesDir().listFiles();
        if(files!=null)for(File old:files)
            if(old.getName().matches("german-0\\.2\\.(189|190)\\.rlex"))old.delete();
    }
}
