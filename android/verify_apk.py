"""Check the install-critical resource table and ARM64 ELF/ZIP alignment.

Signature checks are separate (apksigner). This catches the compressed ARSC
failure that a valid signature and an ordinary zipalign check do not reject.
"""
from pathlib import Path
import argparse
import re
import struct
import subprocess
import zipfile


def data_offset(stream, entry):
    stream.seek(entry.header_offset)
    header=stream.read(30)
    if len(header)!=30 or header[:4]!=b"PK\x03\x04":
        raise ValueError("invalid ZIP local header")
    name,extra=struct.unpack_from("<HH",header,26)
    return entry.header_offset+30+name+extra


def verify_apk(path):
    path=Path(path)
    with path.open("rb") as stream,zipfile.ZipFile(path) as apk:
        entry=apk.getinfo("resources.arsc")
        if entry.compress_type!=zipfile.ZIP_STORED:
            raise ValueError("resources.arsc is compressed: Android target 30+ rejects installation")
        offset=data_offset(stream,entry)
        if offset%4:raise ValueError("resources.arsc is not 4-byte aligned")
        if not apk.read("classes.dex").startswith(b"dex\n"):
            raise ValueError("missing DEX code")
        native=apk.getinfo("lib/arm64-v8a/libreloq_android.so")
        if native.compress_type==zipfile.ZIP_STORED and data_offset(stream,native)%16384:
            raise ValueError("uncompressed ARM64 library is not 16-KiB ZIP aligned")
        data=apk.read(native)
        if data[:6]!=b"\x7fELF\x02\x01" or struct.unpack_from("<H",data,18)[0]!=183:
            raise ValueError("native library is not little-endian ELF64 AArch64")
        at=struct.unpack_from("<Q",data,32)[0]
        size,count=struct.unpack_from("<HH",data,54)
        loads=0
        for i in range(count):
            kind,flags,location,address,physical,files,memory,alignment=struct.unpack_from("<IIQQQQQQ",data,at+i*size)
            if kind==1:
                loads+=1
                if alignment<16384 or location%alignment!=address%alignment:
                    raise ValueError("ARM64 LOAD segment is not 16-KiB aligned")
        if not loads:raise ValueError("no native LOAD segments")
    print(f"APK install rules: stored resources.arsc at {offset}, 4-byte alignment and ARM64 16-KiB segments passed")


def verify_upgrade(new,old,aapt2,apksigner,environment=None):
    """Compare actual signed artifacts, not just the source manifest/key path."""
    def inspect(path):
        badging=subprocess.run([str(aapt2),"dump","badging",str(path)],check=True,capture_output=True,text=True).stdout
        package=re.search(r"package: name='([^']+)' versionCode='(\d+)'",badging)
        certificates=subprocess.run([str(apksigner),"verify","--print-certs",str(path)],
            check=True,capture_output=True,text=True,env=environment).stdout
        digests=re.findall(r"Signer #\d+ certificate SHA-256 digest: ([a-f0-9]+)",certificates)
        if not package or not digests:raise ValueError("APK identity or signing certificate missing")
        return dict(package=package[1],version_code=int(package[2]),certificates=sorted(digests))
    before,after=inspect(old),inspect(new)
    if before["package"]!=after["package"]:raise ValueError("Package name changed: installs a second app")
    if before["certificates"]!=after["certificates"]:raise ValueError("Signing certificate changed: existing app cannot update")
    if after["version_code"]<=before["version_code"]:raise ValueError("Update versionCode must increase")
    print(f"APK upgrade: {after['package']}, same certificate, versionCode {before['version_code']} -> {after['version_code']} passed")
    return dict(before=before,after=after)


def verify_permissions(path,aapt2):
    """Inspect the compiled manifest, including permissions merged at build time."""
    listing=subprocess.run([str(aapt2),"dump","permissions",str(path)],
        check=True,capture_output=True,text=True).stdout
    permissions=re.findall(r"uses-permission: name='([^']+)'",listing)
    if permissions!=["android.permission.INTERNET"]:
        raise ValueError("Unexpected APK permissions: "+repr(permissions))
    with zipfile.ZipFile(path) as apk:
        dex=apk.read("classes.dex")
        for removed in (b"Lorg/reloq/tts/preview/UpdateProvider;",
                        b"android.settings.MANAGE_UNKNOWN_APP_SOURCES",
                        b"android.intent.action.INSTALL_PACKAGE"):
            if removed in dex:raise ValueError("Removed APK installer code remains in DEX")
    print("APK permissions: INTERNET only; removed APK installer code absent from DEX")
    return permissions


if __name__=="__main__":
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("apk",type=Path)
    p.add_argument("--reject",type=Path,help="Regression: an older broken APK that must fail validation")
    a=p.parse_args()
    if a.reject:
        try:verify_apk(a.reject)
        except ValueError as error:print("Previous APK rejected as expected:",error)
        else:raise AssertionError("known-broken APK unexpectedly passed")
    verify_apk(a.apk)
