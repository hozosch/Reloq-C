# Persian Unicode character labels

`fa-annotations-single.json` is a pinned extraction of the Persian `tts`
annotations for individual Unicode scalars in Unicode CLDR release 48. It
combines these two upstream files, excluding multi-scalar emoji sequences:

- https://github.com/unicode-org/cldr/blob/release-48/common/annotations/fa.xml
  (Git blob `919e5b92e09d8a81b78955d42e4ea1c6cf2cd9c1`)
- https://github.com/unicode-org/cldr/blob/release-48/common/annotationsDerived/fa.xml
  (Git blob `e649f7af72482dfc804ddda657fea86dc9813811`)

The extraction contains 1,789 unique scalar labels. The generator applies
separately maintained overrides and validates the synthesized names before
creating the native character table. CLDR data is covered by the Unicode
License V3 in `UNICODE-LICENSE.txt`.
