#!/usr/bin/env python3
"""Reject a release whose three platform versions or Android code disagree."""
from __future__ import annotations

import ast
from pathlib import Path
import re
import sys
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
sys.path[:0] = [str(ROOT / 'addon'), str(ROOT / 'android')]
from build_synth import addon_version
from build import android_version

ANDROID = '{http://schemas.android.com/apk/res/android}'
VERSION = re.compile(r'(\d+)\.(\d+)\.(\d+)(?:-(?:dev|rc)\d+)?\Z')


def check(nvda_manifest, android_manifest, sapi_version):
    nvda = addon_version(nvda_manifest)
    android = android_version(android_manifest)
    tree = ast.parse(sapi_version.read_text(encoding='utf-8'), str(sapi_version))
    values = [node.value.value for node in tree.body
              if isinstance(node, ast.Assign) and len(node.targets) == 1
              and isinstance(node.targets[0], ast.Name) and node.targets[0].id == 'VERSION'
              and isinstance(node.value, ast.Constant) and isinstance(node.value.value, str)]
    if len(values) != 1 or nvda != android or values[0] != nvda:
        raise ValueError(f'platform versions differ: NVDA={nvda}, Android={android}, SAPI={values}')
    match = VERSION.fullmatch(nvda)
    if not match:
        raise ValueError(f'invalid package version: {nvda}')
    major, minor, patch = map(int, match.groups())
    expected = major * 1_000_000 + minor * 10_000 + patch
    if (major, minor) >= (1, 1):
        expected += 10_000_000
    if '-' in nvda:
        expected -= 1
    code = ET.parse(android_manifest).getroot().get(ANDROID + 'versionCode')
    if code != str(expected):
        raise ValueError(f'Android versionCode {code!r} does not match {nvda} ({expected})')
    return nvda


if __name__ == '__main__':
    print('Platform package versions agree:', check(
        ROOT / 'addon/synth/manifest.ini', ROOT / 'android/AndroidManifest.xml', ROOT / 'sapi/version.py'))
