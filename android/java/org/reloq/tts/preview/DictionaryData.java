package org.reloq.tts.preview;

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.util.*;

/** File data only. Text and pronunciation interpretation stay in the C core. */
final class DictionaryData {
    static final int LIMIT=8*1024*1024;
    static final String[] COMMUNITY={"DEUmain.dic","DEURoot.dic","DEUabbr.dic",
        "ENUmain.dic","ENURoot.dic","ENUabbr.dic","LICENSE.md"};
    static final String[] SUFFIXES={"main","Root","abbr"};
    static String prefix(int language) {
        switch(language) {
        case 0x10000:return "ENU";case 0x10001:return "ENG";case 0x20000:return "ESP";
        case 0x20001:return "ESM";case 0x30000:return "FRA";case 0x30001:return "FRC";
        case 0x40000:return "DEU";case 0x50000:return "ITA";case 0x60000:return "CHS";
        case 0x70000:return "PTB";case 0x80000:return "JPN";case 0x90000:return "FIN";
        case 0xa0000:return "KOR";case 0x120000:return "ELL";default:throw new IllegalArgumentException("language");
        }
    }
    static String name(int language,int volume) {
        if(volume<0||volume>2)throw new IllegalArgumentException("volume");
        return prefix(language)+SUFFIXES[volume]+".dic";
    }
    static byte[] read(InputStream input,int limit) throws IOException {
        ByteArrayOutputStream out=new ByteArrayOutputStream();byte[] data=new byte[16384];int count;
        while((count=input.read(data))!=-1) {
            if(out.size()>limit-count)throw new IOException("Dictionary exceeds limit");out.write(data,0,count);
        }
        return out.toByteArray();
    }
    static String hex(byte[] data) {
        StringBuilder out=new StringBuilder();for(byte b:data)out.append(String.format(java.util.Locale.ROOT,"%02x",b&255));return out.toString();
    }
    static String hash(String algorithm,byte[] data) throws Exception {return hex(MessageDigest.getInstance(algorithm).digest(data));}
    static String blob(byte[] data) throws Exception {
        MessageDigest hash=MessageDigest.getInstance("SHA-1");
        hash.update(("blob "+data.length+"\0").getBytes(StandardCharsets.US_ASCII));hash.update(data);return hex(hash.digest());
    }
    static final class Merger {
        final TreeMap<String,TreeMap<String,String[]>> volumes=new TreeMap<>();
        void add(String name,byte[] data) throws IOException {
            if(data.length>LIMIT)throw new IOException("Dictionary exceeds 8 MiB");
            for(byte b:data)if(b==0)throw new IOException("NUL in dictionary");
            TreeMap<String,String[]> table=volumes.computeIfAbsent(name,k -> new TreeMap<>());
            String text=new String(data,StandardCharsets.ISO_8859_1);
            try(BufferedReader lines=new BufferedReader(new StringReader(text))) {
                String line;
                while((line=lines.readLine())!=null) {
                    int tab=line.indexOf('\t');if(tab<0)continue;
                    String key=trim(line.substring(0,tab)),value=trim(line.substring(tab+1));
                    if(key.isEmpty()||value.isEmpty())continue;
                    if(key.length()>128||value.length()>512)throw new IOException("Dictionary entry too long");
                    String identity=key;
                    if(name.equals("DEURoot.dic")) {
                        StringBuilder lower=new StringBuilder();
                        for(char c:key.toCharArray())lower.append((char)((c>='A'&&c<='Z'||c>=0xc0&&c<=0xde&&c!=0xd7)?c+32:c));
                        identity=lower.toString();
                    }
                    table.put(identity,new String[]{key,value});
                }
            }
        }
        private static String trim(String text) {
            int first=0,last=text.length();while(first<last&&space(text.charAt(first)))++first;
            while(last>first&&space(text.charAt(last-1)))--last;return text.substring(first,last);
        }
        private static boolean space(char c){return c==' '||c>='\t'&&c<='\r';}
        TreeMap<String,byte[]> finish() throws IOException {
            TreeMap<String,byte[]> result=new TreeMap<>();
            for(Map.Entry<String,TreeMap<String,String[]>> volume:volumes.entrySet()) {
                ArrayList<String[]> rows=new ArrayList<>(volume.getValue().values());
                rows.sort(Comparator.comparing(row -> row[0]));
                ByteArrayOutputStream out=new ByteArrayOutputStream();
                for(String[] row:rows) {
                    byte[] bytes=(row[0]+"\t"+row[1]+"\n").getBytes(StandardCharsets.ISO_8859_1);
                    if(out.size()>LIMIT-bytes.length)throw new IOException("Merged dictionary exceeds 8 MiB");
                    out.write(bytes,0,bytes.length);
                }
                result.put(volume.getKey(),out.toByteArray());
            }
            return result;
        }
    }
}
