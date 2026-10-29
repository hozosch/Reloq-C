#!/usr/bin/env python3
"""Build the offline ARM64 preview with official SDK/NDK tools and JDK 17.

No Gradle, network access, original modules or audio files are involved.
The checked-in signing identity is PUBLIC and for preview builds only.
"""
from pathlib import Path
import argparse
import os
import shutil
import subprocess
import sys
import zipfile

ROOT=Path(__file__).resolve().parents[1]
HERE=ROOT/"android"


def run(*command, **kwargs):
    subprocess.run([str(x) for x in command],check=True,**kwargs)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sdk",type=Path,default=os.environ.get("ANDROID_SDK_ROOT"))
    parser.add_argument("--ndk",type=Path,default=os.environ.get("ANDROID_NDK_HOME"))
    parser.add_argument("--java-home",type=Path,default=os.environ.get("JAVA_HOME"))
    parser.add_argument("--build-tools",type=Path)
    parser.add_argument("--android-jar",type=Path)
    parser.add_argument("--jobs",type=int,default=4)
    parser.add_argument("--output",type=Path,default=ROOT/"dist/reloq-c-android-arm64-0.2.189-preview1.apk")
    parser.add_argument("--keystore",type=Path,default=HERE/"preview-debug.p12")
    args=parser.parse_args()
    if not args.sdk or not args.ndk or not args.java_home:
        parser.error("set --sdk, --ndk and --java-home (or their environment variables)")
    sdk,ndk,java=(x.resolve() for x in (args.sdk,args.ndk,args.java_home))
    tools=(args.build_tools or sdk/"build-tools/35.0.1").resolve()
    android_jar=(args.android_jar or sdk/"platforms/android-35/android.jar").resolve()
    host=ndk/"toolchains/llvm/prebuilt/linux-x86_64/bin"
    compiler=host/"aarch64-linux-android26-clang"
    for path in (compiler,host/"llvm-ar",android_jar,tools/"aapt2",tools/"d8",tools/"apksigner",java/"bin/javac",args.keystore):
        if not path.is_file(): parser.error(f"missing build input: {path}")
    build=HERE/"build";build.mkdir(exist_ok=True)
    environment={**os.environ,"JAVA_HOME":str(java),"PATH":str(java/"bin")+os.pathsep+os.environ["PATH"]}
    run("make","-C",ROOT/"eci",f"-j{args.jobs}","libreloq_android.so",
        f"ANDROID_CC={compiler}",f"ANDROID_AR={host/'llvm-ar'}")
    generated=build/"generated";generated.mkdir(exist_ok=True)
    assets=build/"assets";assets.mkdir(exist_ok=True)
    sys.path.insert(0,str(ROOT/"addon"))
    from build_synth import merged_german_lexicon
    (assets/"german.rlex").write_bytes(merged_german_lexicon())
    licenses={"OpenEVV-LICENSE.txt":ROOT/"vendor/openevv61/LICENSE",
              "OpenEVV-NOTICE.txt":ROOT/"vendor/openevv61/NOTICE",
              "Unicode-LICENSE.txt":ROOT/"reference-apple/behavior-0.2.82/unicode-input/LICENSE.txt",
              "SCOWL-Copyright.txt":ROOT/"reference-apple/behavior-0.2.83/input/SCOWL-Copyright.txt"}
    for name,source in licenses.items(): shutil.copyfile(source,assets/name)
    run(tools/"aapt2","compile","--dir",HERE/"res","-o",build/"resources.zip")
    run(tools/"aapt2","link","-o",build/"resources.apk","-I",android_jar,
        "--manifest",HERE/"AndroidManifest.xml","--java",generated,"-A",assets,build/"resources.zip")
    classes=build/"classes";shutil.rmtree(classes,ignore_errors=True);classes.mkdir()
    sources=sorted((HERE/"java").rglob("*.java"))+sorted(generated.rglob("*.java"))
    run(java/"bin/javac","-encoding","UTF-8","-source","8","-target","8","-Xlint:-options",
        "-bootclasspath",str(android_jar)+os.pathsep+str(tools/"core-lambda-stubs.jar"),"-d",classes,*sources)
    run(java/"bin/jar","cf",build/"classes.jar","-C",classes,".")
    dex=build/"dex";dex.mkdir(exist_ok=True)
    run(tools/"d8","--release","--min-api","26","--lib",android_jar,
        "--output",dex,build/"classes.jar",env=environment)
    native=build/"libreloq_android.so";shutil.copyfile(ROOT/"eci/libreloq_android.so",native)
    run(host/"llvm-strip","--strip-unneeded",native)
    with zipfile.ZipFile(build/"resources.apk") as resources, zipfile.ZipFile(
            build/"unsigned.apk","w",compression=zipfile.ZIP_DEFLATED) as apk:
        for entry in resources.infolist(): apk.writestr(entry.filename,resources.read(entry))
        apk.write(dex/"classes.dex","classes.dex")
        apk.write(native,"lib/arm64-v8a/libreloq_android.so")
    run(tools/"zipalign","-P","16","-f","4",build/"unsigned.apk",build/"aligned.apk")
    output=args.output.resolve();output.parent.mkdir(parents=True,exist_ok=True)
    run(tools/"apksigner","sign","--ks",args.keystore.resolve(),"--ks-key-alias","androiddebugkey",
        "--ks-pass","pass:android","--key-pass","pass:android","--out",output,build/"aligned.apk",env=environment)
    run(tools/"apksigner","verify","--verbose",output,env=environment)
    run(tools/"zipalign","-c","-P","16","4",output)
    print(output)


if __name__=="__main__": main()
