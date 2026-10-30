package org.reloq.tts.preview;

import android.content.*;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.provider.OpenableColumns;
import java.io.*;

/** Grants the system installer read access to exactly one private APK. */
public final class UpdateProvider extends ContentProvider {
    static File packageFile(Context context) {return new File(context.getCacheDir(),"updates/update.apk");}
    static Uri uri(Context context) {return Uri.parse("content://"+context.getPackageName()+".updates/update.apk");}
    private File file(Uri uri) throws FileNotFoundException {
        if(getContext()==null||!uri.equals(uri(getContext())))throw new FileNotFoundException();
        return packageFile(getContext());
    }
    @Override public boolean onCreate() {return true;}
    @Override public String getType(Uri uri) {return "application/vnd.android.package-archive";}
    @Override public ParcelFileDescriptor openFile(Uri uri,String mode) throws FileNotFoundException {
        if(!"r".equals(mode))throw new FileNotFoundException("Read-only package");
        return ParcelFileDescriptor.open(file(uri),ParcelFileDescriptor.MODE_READ_ONLY);
    }
    @Override public Cursor query(Uri uri,String[] projection,String selection,String[] args,String order) {
        try {
            File f=file(uri);
            String[] columns=projection==null?new String[]{OpenableColumns.DISPLAY_NAME,OpenableColumns.SIZE}:projection;
            MatrixCursor cursor=new MatrixCursor(columns);Object[] row=new Object[columns.length];
            for(int i=0;i<columns.length;++i) {
                if(OpenableColumns.DISPLAY_NAME.equals(columns[i]))row[i]="ReLoq-update.apk";
                if(OpenableColumns.SIZE.equals(columns[i]))row[i]=f.length();
            }
            cursor.addRow(row);return cursor;
        } catch(FileNotFoundException e) {return null;}
    }
    @Override public Uri insert(Uri uri,ContentValues values) {throw new UnsupportedOperationException();}
    @Override public int delete(Uri uri,String where,String[] args) {throw new UnsupportedOperationException();}
    @Override public int update(Uri uri,ContentValues values,String where,String[] args) {throw new UnsupportedOperationException();}
}
