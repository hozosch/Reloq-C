"""Host controls/persistence/merge tests and native duration-based speed check.

PCM stays in RAM. Android framework interaction remains a device check.
"""
from pathlib import Path
import argparse
import json
import os
import subprocess
import sys
import tempfile
from codegen import generate

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/"nvda"))
from _reloq_eci_direct import DirectEciEngine,NATIVE_SPEED


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--java-home",type=Path,default=os.environ.get("JAVA_HOME"))
    p.add_argument("--android-jar",type=Path,required=True)
    p.add_argument("--library",type=Path,default=ROOT/"eci/libreloq_eci.so")
    p.add_argument("--report",type=Path)
    a=p.parse_args()
    if not a.java_home:p.error("--java-home required")
    with tempfile.TemporaryDirectory() as temporary:
        build=Path(temporary);generate(build/"generated")
        folder=ROOT/"android/java/org/reloq/tts/preview"
        sources=[folder/(n+".java") for n in ("PcmQueue","Catalog","VoiceSettings","Preferences","DictionaryData")]
        sources.extend((build/"generated").rglob("RateCurve.java"))
        sources.extend(ROOT/"android/test"/(n+".java") for n in ("PcmQueueTest","SettingsTest"))
        subprocess.run([str(a.java_home/"bin/javac"),"-encoding","UTF-8","-cp",str(a.android_jar),"-d",str(build),*map(str,sources)],check=True)
        classpath=str(build)+os.pathsep+str(a.android_jar)
        base=[str(a.java_home/"bin/java"),"-cp",classpath]
        subprocess.run([*base,"org.reloq.tts.preview.PcmQueueTest"],check=True,timeout=30)
        result=subprocess.run([*base,"org.reloq.tts.preview.SettingsTest"],check=True,capture_output=True,text=True,timeout=30)
        print(result.stdout,end="")
        controls=list(map(int,result.stdout.splitlines()[-1].split("\t")[1:]))
    texts=[("The clear morning light was coming through the window while we talked about "
          "the work that needed to be done and the places we hoped to visit during "
          "the coming summer when the weather would be warm and the days would be long."),
          ("The application provides several accessibility features. You can change the pronunciation, "
           "select a different language, or adjust the volume and speed. Additional settings control "
           "the individual characteristics of the selected voice. Dictionary updates are optional and "
           "can be downloaded whenever an internet connection is available. Your personal settings "
           "are preserved when you install a newer version of the application."),
          ("Please open the document and check the information on the following page. The meeting "
           "will begin tomorrow afternoon at three thirty. Remember to save your changes before "
           "closing the window. You can use the keyboard to move between the available options "
           "and select the setting you prefer.")]
    rows=[]
    engine=DirectEciEngine(a.library.resolve(),dialect=0x10000,encoding="utf8",experimental_generic=True)
    try:
        for percent,speed in zip((100,200),controls):
            engine.set_parameter(NATIVE_SPEED,speed);samples=[]
            for text in texts:
                size=[0]
                def take(data):size[0]+=len(data)
                assert engine.speak([text],take,lambda i:None)
                duration=size[0]/2/11025
                samples.append(dict(words=len(text.split()),seconds=duration,wpm=len(text.split())*60/duration))
            duration=sum(s["seconds"] for s in samples)
            wpm=sum(s["words"] for s in samples)*60/duration
            rows.append(dict(android_percent=percent,native_speed=speed,seconds=duration,wpm=wpm,passages=samples))
    finally:engine.close()
    ratio=rows[0]["seconds"]/rows[1]["seconds"]
    assert 165<rows[0]["wpm"]<215,rows
    assert 1.85<ratio<2.15,(rows,ratio)
    report=dict(normal_target_wpm=190,measurements=rows,duration_ratio=ratio,no_audio_files=True)
    if a.report:a.report.write_text(json.dumps(report,indent=2)+"\n")
    print(json.dumps(report))


if __name__=="__main__":main()
