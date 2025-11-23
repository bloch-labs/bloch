Param(
    [string]$Version = "latest",
    [string]$InstallDir = "$env:LOCALAPPDATA\Programs\Bloch"
)

$ErrorActionPreference = "Stop"
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12

function Step($msg)    { Write-Host "-> $msg" }
function Ok($msg)      { Write-Host "[OK] $msg" -ForegroundColor Green }
function Warn($msg)    { Write-Warning $msg }
function Fail($msg)    { Write-Error $msg; exit 1 }

$Repo = "bloch-labs/bloch"
$Tmp = New-Item -ItemType Directory -Path ([System.IO.Path]::Combine([System.IO.Path]::GetTempPath(), "bloch-install-" + [System.Guid]::NewGuid().ToString("N"))) -Force
try {
    function Resolve-Version {
        param($inputVersion)
        if ($inputVersion -eq "latest") {
            $resp = Invoke-WebRequest -UseBasicParsing -Uri "https://api.github.com/repos/$Repo/releases/latest"
            $json = $resp.Content | ConvertFrom-Json
            if (-not $json.tag_name) { Fail "Could not resolve latest release tag" }
            return $json.tag_name
        }
        elseif ($inputVersion -eq "latest-rc") {
            $resp = Invoke-WebRequest -UseBasicParsing -Uri "https://api.github.com/repos/$Repo/releases"
            $json = $resp.Content | ConvertFrom-Json
            $rc = $json | Where-Object { $_.prerelease -eq $true } | Select-Object -First 1
            if (-not $rc) { Fail "Could not resolve latest prerelease tag" }
            return $rc.tag_name
        }
        else {
            return $inputVersion
        }
    }

    $Tag = Resolve-Version $Version
    if (-not $Tag) { Fail "Empty tag resolved" }

    $Asset = "bloch-$Tag-Windows-X64.zip"
    $BaseUrl = "https://github.com/$Repo/releases/download/$Tag"
    $AssetUrl = "$BaseUrl/$Asset"
    $ChecksumsUrl = "$BaseUrl/checksums-Windows-X64.txt"

    Step "Installing Bloch $Tag for Windows/X64 into $InstallDir"
    Step "Downloading: $AssetUrl"
    $assetPath = Join-Path $Tmp $Asset
    Invoke-WebRequest -UseBasicParsing -Uri $AssetUrl -OutFile $assetPath

    $checksumPath = Join-Path $Tmp "checksums-Windows-X64.txt"
    $haveChecksum = $true
    try {
        Invoke-WebRequest -UseBasicParsing -Uri $ChecksumsUrl -OutFile $checksumPath
    } catch {
        Warn "checksums-Windows-X64.txt not found; skipping verification"
        $haveChecksum = $false
    }

    if ($haveChecksum) {
        Step "Verifying checksum (checksums-Windows-X64.txt)"
        $expected = (Select-String -Path $checksumPath -Pattern "  $Asset`$").Line -replace "\s+$Asset",""
        if (-not $expected) { Warn "Expected checksum not found; skipping verification" }
        else {
            $actual = (Get-FileHash -Algorithm SHA256 -Path $assetPath).Hash.ToLower()
            if ($expected.ToLower() -ne $actual) {
                Fail "Checksum mismatch for $Asset`nExpected: $expected`nActual:   $actual"
            } else {
                Ok "Checksum OK"
            }
        }
    }

    Step "Extracting"
    $extractDir = Join-Path $Tmp "extract"
    Expand-Archive -LiteralPath $assetPath -DestinationPath $extractDir -Force

    $blochExe = Get-ChildItem -LiteralPath $extractDir -Recurse -Filter "bloch.exe" | Select-Object -First 1
    if (-not $blochExe) { Fail "Extracted archive did not contain bloch.exe" }

    if (-not (Test-Path $InstallDir)) {
        New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
    }
    $targetExe = Join-Path $InstallDir "bloch.exe"
    Copy-Item -LiteralPath $blochExe.FullName -Destination $targetExe -Force
    Ok "Installed: $targetExe"

    Step "Probing binary"
    $probe = & $targetExe --version 2>$null
    if ($LASTEXITCODE -ne 0) { Fail "Downloaded binary failed to run. Output: $probe" }
    Ok $probe

    # Add to user PATH if needed
    $userPath = [Environment]::GetEnvironmentVariable("Path", "User")
    $pathEntries = $userPath -split ";" | Where-Object { $_ -ne "" }
    if ($pathEntries -notcontains $InstallDir) {
        Step "Adding $InstallDir to user PATH"
        $newPath = ($pathEntries + $InstallDir) -join ";"
        setx PATH $newPath | Out-Null
        Warn "PATH updated for future shells. Open a new PowerShell or CMD to use 'bloch' directly."
    } else {
        Ok "PATH already contains $InstallDir"
    }

} finally {
    if (Test-Path $Tmp) { Remove-Item -Recurse -Force $Tmp }
}
