package org.reloq.tts;

public final class UpdateDataTest {
    private static int checks;
    private static void require(boolean value) {checks++;if(!value)throw new AssertionError(checks);}
    private static void reject(Runnable action) {
        try {action.run();throw new AssertionError("Invalid metadata accepted");}
        catch(IllegalArgumentException expected) {checks++;}
    }
    public static void main(String[] args) {
        String hash="0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
        String digest="sha256:"+hash.toUpperCase(java.util.Locale.ROOT);
        String name="reloq-c-android-arm64-0.2.197.apk";
        String url="https://github.com/hozosch/Reloq-C/releases/download/v0.2.197/"+name;
        UpdateData.Package data=UpdateData.candidate(name,url,18000000,digest,20196);
        require(data!=null&&data.code==20197&&data.version.equals("0.2.197"));
        require(data.name.equals(name)&&data.url.equals(url)&&data.size==18000000&&data.sha256.equals(hash));
        require(UpdateData.candidate(name,url,18000000,digest,20197)==null);
        require(UpdateData.candidate(name,url,18000000,digest,20198)==null);
        require(UpdateData.candidate("other.apk",url,18000000,digest,0)==null);
        require(UpdateData.candidate("reloq-c-android-x86-0.2.198.apk",url,18000000,digest,0)==null);
        String next="reloq-c-android-arm64-0.2.198.apk";
        require(UpdateData.candidate(next,"https://github.com/hozosch/Reloq-C/releases/download/v0.2.198/"+next,
            18000000,digest,20197).code==20198);
        String latest="reloq-c-android-arm64-0.2.199.apk";
        require(UpdateData.candidate(latest,"https://github.com/hozosch/Reloq-C/releases/download/v0.2.199/"+latest,
            18000000,digest,20198).code==20199);
        String compact="reloq-c-android-arm64-0.2.200.apk";
        require(UpdateData.candidate(compact,"https://github.com/hozosch/Reloq-C/releases/download/v0.2.200/"+compact,
            18000000,digest,20199).code==20200);
        String sapi="reloq-c-android-arm64-0.2.201.apk";
        require(UpdateData.candidate(sapi,"https://github.com/hozosch/Reloq-C/releases/download/v0.2.201/"+sapi,
            18000000,digest,20200).code==20201);
        String official="reloq-c-android-arm64-1.0.0.apk";
        String officialUrl="https://github.com/hozosch/Reloq-C/releases/download/v1.0/"+official;
        UpdateData.Package release=UpdateData.candidate(official,officialUrl,18000000,digest,20205);
        require(release!=null&&release.version.equals("1.0.0")&&release.code==1000000);
        require(UpdateData.candidate(official,officialUrl,18000000,digest,1000000)==null);
        String patch="reloq-c-android-arm64-1.0.1.apk";
        String patchUrl="https://github.com/hozosch/Reloq-C/releases/download/v1.0.1/"+patch;
        UpdateData.Package update=UpdateData.candidate(patch,patchUrl,18000000,digest,1000000);
        require(update!=null&&update.version.equals("1.0.1")&&update.code==1000001);
        require(UpdateData.candidate(patch,patchUrl,18000000,digest,1000001)==null);
        String stable="reloq-c-android-arm64-1.1.0.apk";
        String stableUrl="https://github.com/hozosch/Reloq-C/releases/download/v1.1/"+stable;
        UpdateData.Package upgrade=UpdateData.candidate(stable,stableUrl,18000000,digest,1090027);
        require(upgrade!=null&&upgrade.code==11010000&&upgrade.version.equals("1.1.0"));
        require(UpdateData.candidate(stable,stableUrl,18000000,digest,11010000)==null);
        String following="reloq-c-android-arm64-1.1.1.apk";
        String followingUrl="https://github.com/hozosch/Reloq-C/releases/download/v1.1.1/"+following;
        require(UpdateData.candidate(following,followingUrl,18000000,digest,11010000).code==11010001);
        String nextMajor="reloq-c-android-arm64-2.0.0.apk";
        String nextMajorUrl="https://github.com/hozosch/Reloq-C/releases/download/v2.0/"+nextMajor;
        require(UpdateData.candidate(nextMajor,nextMajorUrl,18000000,digest,11999999).code==12000000);
        for(String bad:new String[]{url.replace("github.com","example.net"),
            url.replace("/hozosch/","/other/"),url+"?x=1",url+"#file",
            url.replace("https:","http:"),url.replace("github.com","github.com:443"),
            url.replace("github.com","user@github.com"),
            url.replace("v0.2.197/","../../other/"),url.replace("v0.2.197/","%2e%2e/"),
            url.replace("v0.2.197/","v0.2.197/extra/"),url.replace(".apk",".zip")})
            reject(() -> UpdateData.candidate(name,bad,18000000,digest,0));
        for(long size:new long[]{-1,0,UpdateData.LIMIT+1,Long.MAX_VALUE})
            reject(() -> UpdateData.candidate(name,url,size,digest,0));
        for(String invalid:new String[]{null,"",hash,"sha256:"+hash.substring(1),"sha256:"+hash+"0","sha256:"+hash.replace('a','z')})
            reject(() -> UpdateData.candidate(name,url,18000000,invalid,0));
        require(UpdateData.candidate(name,url,1,digest,0)!=null);
        require(UpdateData.candidate(name,url,UpdateData.LIMIT,digest,0)!=null);
        reject(() -> UpdateData.candidate("reloq-c-android-arm64-2001.0.0.apk",url,1,digest,0));
        reject(() -> UpdateData.candidate("reloq-c-android-arm64-0.100.0.apk",url,1,digest,0));
        reject(() -> UpdateData.candidate("reloq-c-android-arm64-0.2.10000.apk",url,1,digest,0));
        System.out.println("Android release links: "+checks+" version/source/metadata checks passed");
    }
}
