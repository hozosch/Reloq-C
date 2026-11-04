# Create a separate disposable test folder with realistic xplorer2 entries.
# Run in PowerShell; nothing is removed or changed outside that new folder.
$ErrorActionPreference = 'Stop'
$folder = Join-Path $env:TEMP ('Reloq-issue-10-' + [guid]::NewGuid().ToString('N').Substring(0, 8))
New-Item -ItemType Directory -Path $folder | Out-Null
$names = @(
    '20171111 - Can_A_Real_Meal_Easy_Bake_Oven_Cook_Raw_Meat_Macaroni_Cheese_-_Sloppy_Joe_Dinner',
    '20261102 - Ein_langer_Dateiname_mit_mehreren_Woertern_und_Zahlen_1234567890_und_Bindestrich',
    '20261031 - Different_long_video_title_part_one_part_two_and_part_three_with_more_words'
)
$contents = New-Object byte[] 22528
for ($i = 0; $i -lt 60; $i++) {
    $base = $names[$i % $names.Count]
    $name = '{0}_{1:D3}.vtt' -f $base, $i
    $path = Join-Path $folder $name
    [IO.File]::WriteAllBytes($path, $contents)
    (Get-Item -LiteralPath $path).LastWriteTime = [datetime]'2026-11-02T18:20:58'
}
Write-Output "Test folder with 60 files: $folder"
