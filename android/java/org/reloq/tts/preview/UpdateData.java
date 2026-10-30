package org.reloq.tts.preview;

import java.net.URI;
import java.util.regex.*;

/** Public release metadata validation; no package installer or speech-worker state. */
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
            !uri.getRawPath().matches("/hozosch/Reloq-C/releases/download/[A-Za-z0-9][A-Za-z0-9._-]*/"+
                Pattern.quote(name)))throw new IllegalArgumentException("Invalid package URL");
        if(size<1||size>LIMIT||digest==null||!digest.matches("sha256:[0-9a-fA-F]{64}"))
            throw new IllegalArgumentException("Invalid package size or SHA-256 checksum");
        return new Package(name,m.group(1),url,digest.substring(7).toLowerCase(java.util.Locale.ROOT),code,size);
    }
}
