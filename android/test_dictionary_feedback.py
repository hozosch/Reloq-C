"""Exercise the actual toast text with minimal Android host stubs."""
import argparse
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
STUBS = {
    'android/content/Context.java': '''package android.content;
public class Context {
  public String getString(int id,Object... args) {
    String[] texts={"", "Unknown entries", "First entries: %s", "New entries: %s",
                    "Unknown files", "Total files: %s", "New files: %s"};
    return args.length==0?texts[id]:String.format(texts[id],args);
  }
}''',
    'android/widget/Toast.java': '''package android.widget;
import android.content.Context;
public class Toast {
  public static final int LENGTH_LONG=1; public static int shown,duration; public static String text;
  public static Toast makeText(Context context,CharSequence message,int length) {
    if(context==null)throw new AssertionError();text=message.toString();duration=length;return new Toast();
  }
  public void show(){++shown;}
}''',
    'org/reloq/tts/R.java': '''package org.reloq.tts;
public final class R { public static final class string {
  public static final int dictionary_update_unknown=1,dictionary_update_first=2,dictionary_update_new=3,
    dictionary_update_files_unknown=4,dictionary_update_files_total=5,dictionary_update_files_new=6;
}}''',
}


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--java-home',type=Path,default=os.environ.get('JAVA_HOME'))
    args=parser.parse_args()
    if not args.java_home:parser.error('--java-home required')
    with tempfile.TemporaryDirectory() as temp:
        root=Path(temp);sources=[]
        for name,contents in STUBS.items():
            path=root/name;path.parent.mkdir(parents=True,exist_ok=True);path.write_text(contents,encoding='utf8');sources.append(path)
        sources.extend((ROOT/'android/java/org/reloq/tts/DictionaryData.java',
                        ROOT/'android/java/org/reloq/tts/DictionaryUpdateFeedback.java',
                        ROOT/'android/test/DictionaryFeedbackTest.java'))
        subprocess.run([str(args.java_home/'bin/javac'),'-encoding','UTF-8','-d',str(root),*map(str,sources)],check=True)
        subprocess.run([str(args.java_home/'bin/java'),'-cp',str(root),'org.reloq.tts.DictionaryFeedbackTest'],check=True)


if __name__=='__main__':main()
