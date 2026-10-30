package org.reloq.tts.preview;
import java.io.*;
import java.nio.file.*;
import java.security.*;
import java.util.*;

public final class UpdateDataTest {
    private static int checks;
    private static void require(boolean value) {checks++;if(!value)throw new AssertionError(checks);}
    public static void main(String[] args) throws Exception {
        byte[] bytes=new byte[90000];new Random(12).nextBytes(bytes);
        StringBuilder hex=new StringBuilder();
        for(byte b:MessageDigest.getInstance("SHA-256").digest(bytes))hex.append(String.format("%02x",b&255));
        String name="reloq-c-android-arm64-0.2.197-preview9.apk";
        String url="https://github.com/hozosch/Reloq-C/releases/download/v0.2.197/"+name;
        UpdateData.Package data=UpdateData.candidate(name,url,bytes.length,"sha256:"+hex,20196);
        require(data!=null&&data.code==20197&&data.version.equals("0.2.197-preview9"));
        require(UpdateData.candidate(name,url,bytes.length,"sha256:"+hex,20197)==null);
        require(UpdateData.candidate("other.apk",url,bytes.length,"sha256:"+hex,0)==null);
        for(String bad:new String[]{url.replace("github.com","example.net"),url.replace("/hozosch/","/other/"),url+"?x=1"}) {
            try {UpdateData.candidate(name,bad,bytes.length,"sha256:"+hex,0);throw new AssertionError();}
            catch(IllegalArgumentException expected) {checks++;}
        }
        Path dir=Files.createTempDirectory("reloq-update-test-");File target=dir.resolve("update.apk").toFile();
        try {
            UpdateData.copyVerified(new ByteArrayInputStream(bytes),target,data);
            require(Arrays.equals(bytes,Files.readAllBytes(target.toPath())));
            for(int size:new int[]{bytes.length-1,bytes.length+1,bytes.length}) {
                byte[] bad=Arrays.copyOf(bytes,size);if(size==bytes.length)bad[0]^=1;
                try {UpdateData.copyVerified(new ByteArrayInputStream(bad),target,data);throw new AssertionError();}
                catch(IOException expected) {checks++;}
                require(Arrays.equals(bytes,Files.readAllBytes(target.toPath())));
                require(!Files.exists(dir.resolve("update.apk.part")));
            }
        } finally {Files.deleteIfExists(target.toPath());Files.deleteIfExists(dir);}
        System.out.println("Android package updates: "+checks+" version/source/integrity/recovery checks passed");
    }
}
