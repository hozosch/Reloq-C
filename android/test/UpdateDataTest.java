package org.reloq.tts.preview;

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
        String name="reloq-c-android-arm64-0.2.197-preview9.apk";
        String url="https://github.com/hozosch/Reloq-C/releases/download/v0.2.197/"+name;
        UpdateData.Package data=UpdateData.candidate(name,url,18000000,digest,20196);
        require(data!=null&&data.code==20197&&data.version.equals("0.2.197-preview9"));
        require(data.name.equals(name)&&data.url.equals(url)&&data.size==18000000&&data.sha256.equals(hash));
        require(UpdateData.candidate(name,url,18000000,digest,20197)==null);
        require(UpdateData.candidate(name,url,18000000,digest,20198)==null);
        require(UpdateData.candidate("other.apk",url,18000000,digest,0)==null);
        require(UpdateData.candidate("reloq-c-android-x86-0.2.198.apk",url,18000000,digest,0)==null);
        String next="reloq-c-android-arm64-0.2.198-preview10.apk";
        require(UpdateData.candidate(next,"https://github.com/hozosch/Reloq-C/releases/download/v0.2.198/"+next,
            18000000,digest,20197).code==20198);
        String latest="reloq-c-android-arm64-0.2.199-preview11.apk";
        require(UpdateData.candidate(latest,"https://github.com/hozosch/Reloq-C/releases/download/v0.2.199/"+latest,
            18000000,digest,20198).code==20199);
        String compact="reloq-c-android-arm64-0.2.200-preview12.apk";
        require(UpdateData.candidate(compact,"https://github.com/hozosch/Reloq-C/releases/download/v0.2.200/"+compact,
            18000000,digest,20199).code==20200);
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
