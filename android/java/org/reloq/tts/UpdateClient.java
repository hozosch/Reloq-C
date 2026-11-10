package org.reloq.tts;

import java.io.*;
import java.net.*;
import org.json.*;

/** Shared bounded network check for the foreground and background updater. */
final class UpdateClient {
    static UpdateData.Package check(long installed) throws Exception {
        URLConnection connection=new URL(UpdateData.RELEASES).openConnection();
        connection.setConnectTimeout(15000);connection.setReadTimeout(15000);
        connection.setRequestProperty("User-Agent","ReLoq-C-updater");
        ByteArrayOutputStream bytes=new ByteArrayOutputStream();
        try(InputStream input=connection.getInputStream()) {
            byte[] buffer=new byte[8192];int n;
            while((n=input.read(buffer))!=-1) {
                if(bytes.size()+n>2*1024*1024)throw new IOException("Release list too large");
                bytes.write(buffer,0,n);
            }
        }
        JSONArray releases=new JSONArray(bytes.toString("UTF-8"));UpdateData.Package best=null;
        for(int i=0;i<releases.length();++i) {
            JSONObject release=releases.getJSONObject(i);
            if(release.optBoolean("draft")||release.optBoolean("prerelease"))continue;
            JSONArray assets=release.optJSONArray("assets");if(assets==null)continue;
            for(int a=0;a<assets.length();++a) {
                JSONObject asset=assets.getJSONObject(a);
                UpdateData.Package next=UpdateData.candidate(asset.optString("name"),
                    asset.optString("browser_download_url"),asset.optLong("size"),
                    asset.optString("digest"),installed);
                if(next!=null&&(best==null||next.code>best.code))best=next;
            }
        }
        return best;
    }
}
