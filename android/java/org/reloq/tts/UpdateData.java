package org.reloq.tts;

import java.net.URI;
import java.util.regex.*;

/** Public release metadata validation; no package installer or speech-worker state. */
final class UpdateData {
    static final String RELEASES="https://api.github.com/repos/hozosch/Reloq-C/releases?per_page=30";
    static final long LIMIT=128L*1024*1024;
    private static final Pattern NAME=Pattern.compile("reloq-c-android-(arm64|x86_64)-((\\d+)\\.(\\d+)\\.(\\d+))\\.apk");
    private static final Pattern BUILD=Pattern.compile("(?m)^<!-- ReLoq-C build:([1-9]\\d{0,11}) -->$");
    private static final long MAX_EPOCH=253402300799L;
    static final class Package {
        final String name,version,url,sha256;
        final long code,size,buildEpoch;
        Package(String n,String v,String u,String h,long c,long s) {this(n,v,u,h,c,s,0);}
        Package(String n,String v,String u,String h,long c,long s,long epoch) {name=n;version=v;url=u;sha256=h;code=c;size=s;buildEpoch=epoch;}
        String identity() {return buildEpoch>0?version+"@"+buildEpoch:version;}
    }
    static long buildEpoch(String body) {
        if(body==null)return 0;
        Matcher match=BUILD.matcher(body);if(!match.find())return 0;
        long value=Long.parseLong(match.group(1));
        return value>MAX_EPOCH||match.find()?0:value;
    }
    static Package candidate(String name,String url,long size,String digest,long installed) {
        return candidate(name,url,size,digest,installed,0,0);
    }
    static Package candidate(String name,String url,long size,String digest,long installed,long installedEpoch,long releaseEpoch) {
        return candidate(name,url,size,digest,installed,installedEpoch,releaseEpoch,"arm64");
    }
    static Package candidate(String name,String url,long size,String digest,long installed,long installedEpoch,long releaseEpoch,String packageArch) {
        if(!"arm64".equals(packageArch)&&!"x86_64".equals(packageArch))
            throw new IllegalArgumentException("Unsupported package architecture");
        Matcher m=NAME.matcher(name);
        if(!m.matches()||!packageArch.equals(m.group(1)))return null;
        long major=Long.parseLong(m.group(3)),minor=Long.parseLong(m.group(4)),patch=Long.parseLong(m.group(5));
        if(major>2000||minor>=100||patch>=10000)throw new IllegalArgumentException("Invalid package version");
        long code=major*1000000+minor*10000+patch;
        // Earlier 1.1 builds used codes up to 10900xx, higher than the original
        // 1010000 formula for 1.1.0. Start a new code epoch for 1.1+ so the
        // stable versions and future versions remain ordered.
        if(major>1 || (major==1 && minor>=1))code+=10000000;
        if(code<installed||(code==installed&&!(installedEpoch>0&&releaseEpoch>installedEpoch)))return null;
        URI uri=URI.create(url);
        if(!"https".equals(uri.getScheme())||!"github.com".equals(uri.getRawAuthority())||
            uri.getQuery()!=null||uri.getFragment()!=null||
            !uri.getRawPath().matches("/hozosch/Reloq-C/releases/download/[A-Za-z0-9][A-Za-z0-9._-]*/"+
                Pattern.quote(name)))throw new IllegalArgumentException("Invalid package URL");
        if(size<1||size>LIMIT||digest==null||!digest.matches("sha256:[0-9a-fA-F]{64}"))
            throw new IllegalArgumentException("Invalid package size or SHA-256 checksum");
        return new Package(name,m.group(2),url,digest.substring(7).toLowerCase(java.util.Locale.ROOT),code,size,releaseEpoch);
    }
}
