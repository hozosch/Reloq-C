package org.reloq.tts.preview;

import java.io.*;
import java.net.URI;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;
import java.security.MessageDigest;
import java.util.regex.*;

/** Bounded package transfer and metadata validation; no speech-worker state. */
final class UpdateData {
    static final String RELEASES="https://api.github.com/repos/hozosch/Reloq-C/releases?per_page=30";
    static final long LIMIT=128L*1024*1024;
    private static final Pattern NAME=Pattern.compile("reloq-c-android-arm64-((\\d+)\\.(\\d+)\\.(\\d+)(?:-preview\\d+)?)\\.apk");
    static final class Package {
        final String name,version,url,sha256;
        final long code,size;
        Package(String n,String v,String u,String h,long c,long s) {name=n;version=v;url=u;sha256=h;code=c;size=s;}
    }
    static Package candidate(String name,String url,long size,String digest,long installed) {
        Matcher m=NAME.matcher(name);
        if(!m.matches())return null;
        long major=Long.parseLong(m.group(2)),minor=Long.parseLong(m.group(3)),patch=Long.parseLong(m.group(4));
        if(major>2000||minor>=100||patch>=10000)throw new IllegalArgumentException("Invalid package version");
        long code=major*1000000+minor*10000+patch;
        if(code<=installed)return null;
        URI uri=URI.create(url);
        if(!"https".equals(uri.getScheme())||!"github.com".equals(uri.getRawAuthority())||
            uri.getQuery()!=null||uri.getFragment()!=null||
            !uri.getPath().startsWith("/hozosch/Reloq-C/releases/download/")||
            !uri.getPath().endsWith("/"+name))throw new IllegalArgumentException("Invalid package URL");
        if(size<1||size>LIMIT||digest==null||!digest.matches("sha256:[0-9a-fA-F]{64}"))
            throw new IllegalArgumentException("Invalid package size or SHA-256 checksum");
        return new Package(name,m.group(1),url,digest.substring(7).toLowerCase(java.util.Locale.ROOT),code,size);
    }
    static void copyVerified(InputStream input,File target,Package data) throws Exception {
        File part=new File(target.getPath()+".part");
        MessageDigest hash=MessageDigest.getInstance("SHA-256");long size=0;
        try {
            try(OutputStream output=new FileOutputStream(part)) {
                byte[] buffer=new byte[65536];int count;
                while((count=input.read(buffer))!=-1) {
                    if(Thread.currentThread().isInterrupted())throw new InterruptedIOException();
                    size+=count;if(size>data.size)throw new IOException("Package exceeds published size");
                    hash.update(buffer,0,count);output.write(buffer,0,count);
                }
            }
            StringBuilder hex=new StringBuilder();
            for(byte value:hash.digest())hex.append(String.format(java.util.Locale.ROOT,"%02x",value&255));
            if(size!=data.size||!hex.toString().equals(data.sha256))throw new IOException("Package checksum or size mismatch");
            Files.move(part.toPath(),target.toPath(),StandardCopyOption.REPLACE_EXISTING);
        } finally {Files.deleteIfExists(part.toPath());}
    }
}
