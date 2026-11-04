param([string]$ReportPath = "japanese-results.json")
$ErrorActionPreference = 'Stop'
$voice = New-Object -ComObject SAPI.SpVoice
$token = $null
foreach ($candidate in $voice.GetVoices()) {
    if ($candidate.GetDescription() -eq 'ReLoq C - Japanese') { $token = $candidate; break }
}
if (!$token) { throw 'Japanese ReLoq C voice not found.' }
$voice.Voice = $token
$voice.Rate = 0
$voice.Volume = 100
function Render([string]$value, [int]$flags) {
    $stream = New-Object -ComObject SAPI.SpMemoryStream
    $voice.AudioOutputStream = $stream
    [void]$voice.Speak($value, $flags)
    [byte[]]$data = $stream.GetData()
    if (!$data -or $data.Length -lt 100) { throw 'The voice returned no PCM.' }
    $hash = [System.BitConverter]::ToString([System.Security.Cryptography.SHA256]::Create().ComputeHash($data)).Replace('-', '')
    return @{ length = $data.Length; sha256 = $hash }
}
$texts = @('これは音声のテストです。Windowsとcomputer。',
           'こんにちは、今日は良い天気です。次の文章です。',
           'NVDAの音声テストです。私は日本語を話します。',
           'こんにちは 世界 こんにちは 世界。',
           'Windows 11 を使います。')
$results = @()
foreach ($value in $texts) {
    $segments = for ($i=0; $i -lt $value.Length; $i++) {
        '<Bookmark Mark="' + ($i + 1) + '"/>' + $value.Substring($i, 1)
    }
    $plain = Render $value 16
    $basic = Render ('<pitch absmiddle="0">' + $value + '</pitch>') 8
    $marked = Render ('<pitch absmiddle="0"><lang langid="411">' + ($segments -join '') + '</lang></pitch>') 8
    $results += [pscustomobject]@{text=$value; plain=$plain; pitch=$basic; bookmarks=$marked;
        pitchEqualsPlain=($plain.sha256 -eq $basic.sha256);
        bookmarksEqualPlain=($plain.sha256 -eq $marked.sha256)}
}
$results | ConvertTo-Json -Depth 5 | Set-Content -Encoding utf8 $ReportPath
$results | Format-List | Out-String | Write-Output
if (@($results | Where-Object { !$_.pitchEqualsPlain -or !$_.bookmarksEqualPlain }).Count -ne 0) {
    throw 'Japanese SAPI XML or bookmark handling changed the PCM.'
}
