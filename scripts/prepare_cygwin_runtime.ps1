param(
    [Parameter(Mandatory = $true)]
    [string]$SourceRoot,

    [Parameter(Mandatory = $true)]
    [string]$DestinationRoot
)

$ErrorActionPreference = 'Stop'
$SourceRoot = [System.IO.Path]::GetFullPath($SourceRoot)
$DestinationRoot = [System.IO.Path]::GetFullPath($DestinationRoot)
$symlinkMagic = [System.Text.Encoding]::ASCII.GetBytes('!<symlink>')

function Restore-CygwinSymlinkAttributes([string]$Root) {
    foreach ($filePath in [System.IO.Directory]::EnumerateFiles(
        $Root,
        '*',
        [System.IO.SearchOption]::AllDirectories
    )) {
        $stream = $null
        try {
            $stream = [System.IO.File]::Open(
                $filePath,
                [System.IO.FileMode]::Open,
                [System.IO.FileAccess]::Read,
                [System.IO.FileShare]::ReadWrite
            )
            if ($stream.Length -lt $symlinkMagic.Length) {
                continue
            }

            $prefix = New-Object byte[] $symlinkMagic.Length
            if ($stream.Read($prefix, 0, $prefix.Length) -ne $prefix.Length) {
                continue
            }
        }
        catch {
            continue
        }
        finally {
            if ($null -ne $stream) {
                $stream.Dispose()
            }
        }

        $isCygwinSymlink = $true
        for ($index = 0; $index -lt $symlinkMagic.Length; $index++) {
            if ($prefix[$index] -ne $symlinkMagic[$index]) {
                $isCygwinSymlink = $false
                break
            }
        }

        if ($isCygwinSymlink) {
            $attributes = [System.IO.File]::GetAttributes($filePath)
            if (($attributes -band [System.IO.FileAttributes]::System) -eq 0) {
                [System.IO.File]::SetAttributes(
                    $filePath,
                    $attributes -bor [System.IO.FileAttributes]::System
                )
            }
        }
    }
}

if (-not (Test-Path -LiteralPath (Join-Path $SourceRoot 'bin\cygwin1.dll'))) {
    throw "Bundled Cygwin runtime is missing from $SourceRoot"
}

Restore-CygwinSymlinkAttributes $SourceRoot
New-Item -ItemType Directory -Force -Path $DestinationRoot | Out-Null

$robocopy = Join-Path $env:SystemRoot 'System32\robocopy.exe'
& $robocopy $SourceRoot $DestinationRoot /E /COPY:DAT /DCOPY:DAT /R:2 /W:1 /XJ /IT /NFL /NDL /NJH /NJS /NC /NS /NP
$copyResult = $LASTEXITCODE
if ($copyResult -ge 8) {
    throw "Robocopy failed with exit code $copyResult"
}

New-Item -ItemType Directory -Force -Path @(
    (Join-Path $DestinationRoot 'dev'),
    (Join-Path $DestinationRoot 'home'),
    (Join-Path $DestinationRoot 'tmp')
) | Out-Null

Restore-CygwinSymlinkAttributes $DestinationRoot

$bash = Join-Path $DestinationRoot 'bin\bash.exe'
$env:PATH = "$DestinationRoot\bin;$DestinationRoot\compat;$env:PATH"
& $bash --noprofile --norc /compat/generate_windows_aliases.sh
$aliasResult = $LASTEXITCODE
if ($aliasResult -ne 0) {
    throw "Generating Cygwin Windows aliases failed with exit code $aliasResult"
}
