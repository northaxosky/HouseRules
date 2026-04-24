# House Rules — run a FO4Edit script headlessly.
#
# Based on the pattern from PJM's GeneratePrevisibines.bat:
#   FO4Edit.exe -fo4 -autoexit -P:<plugins.txt> -Script:<pas> -Mod:<out.esp> -log:<log>
#
# The -P: flag supplies our own minimal plugins.txt, which bypasses the
# module-selection dialog. Each active plugin is listed on its own line,
# prefixed with `*` to mark it enabled.
#
# Usage:
#   .\run-fo4edit.ps1 -Script HR_HeadlessSmokeTest.pas
#   .\run-fo4edit.ps1 -Script BuildHouseRulesESP.pas -OutMod HouseRules.esp

[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$Script,
    [string]$OutMod = "",
    [string[]]$Plugins = @("Fallout4.esm"),
    [string]$FO4EditDir = "C:\Games\Modding\FO4Tools\FO4Edit 4.1.5f",
    [int]$TimeoutSec = 300
)

$ErrorActionPreference = "Stop"

$exe = Join-Path $FO4EditDir "FO4Edit.exe"
if (-not (Test-Path $exe)) { throw "FO4Edit.exe not found at $exe" }

$scriptSrc = Join-Path $PSScriptRoot $Script
if (-not (Test-Path $scriptSrc)) { throw "Script not found: $scriptSrc" }

# Deploy script into FO4Edit's Edit Scripts folder
$scriptDst = Join-Path $FO4EditDir "Edit Scripts\$Script"
Copy-Item $scriptSrc $scriptDst -Force
Write-Host "[deploy] $Script -> $scriptDst"

# Write a minimal plugins.txt. With -autoload, xEdit loads everything marked
# active in plugins.txt and skips the Module Selection dialog entirely.
# (See whatsnew.md §1590: "-autoload ... will not show the Module Selection
# dialog and just load all modules that are active according to plugins.txt.")
$pluginsFile = Join-Path $env:TEMP "HR_plugins.txt"
$lines = $Plugins | ForEach-Object { "*$_" }
[System.IO.File]::WriteAllLines($pluginsFile, $lines)
Write-Host "[plugins] wrote $pluginsFile with $($Plugins.Count) entries"

# Log path
$logFile = Join-Path $env:TEMP "HR_fo4edit.log"
Remove-Item $logFile -ErrorAction SilentlyContinue

$args = @(
    "-fo4",
    "-autoexit",
    "-IKnowWhatImDoing",
    "-P:$pluginsFile",
    "-Script:$Script",
    "-log:$logFile"
)
if ($OutMod) { $args += "-Mod:$OutMod" }

Write-Host "[run] $exe $($args -join ' ')"
$proc = Start-Process -FilePath $exe -ArgumentList $args -PassThru -WorkingDirectory $FO4EditDir
Write-Host "[run] PID=$($proc.Id); polling for Module Selection dialog..."

# Focus-independent dialog dismissal via Win32 EnumWindows + PostMessage.
# Poll every second, enumerate top-level windows, find "Module Selection",
# then find its OK button and post a BM_CLICK. Doesn't require foreground focus.
Add-Type -Namespace HR -Name Win32 -MemberDefinition @"
    public delegate bool EnumProc(System.IntPtr h, System.IntPtr lp);
    [System.Runtime.InteropServices.DllImport("user32.dll")]
    public static extern bool EnumWindows(EnumProc cb, System.IntPtr lp);
    [System.Runtime.InteropServices.DllImport("user32.dll")]
    public static extern bool EnumChildWindows(System.IntPtr parent, EnumProc cb, System.IntPtr lp);
    [System.Runtime.InteropServices.DllImport("user32.dll", CharSet=System.Runtime.InteropServices.CharSet.Unicode)]
    public static extern int GetWindowText(System.IntPtr h, System.Text.StringBuilder s, int m);
    [System.Runtime.InteropServices.DllImport("user32.dll", CharSet=System.Runtime.InteropServices.CharSet.Unicode)]
    public static extern int GetClassName(System.IntPtr h, System.Text.StringBuilder s, int m);
    [System.Runtime.InteropServices.DllImport("user32.dll")]
    public static extern uint GetWindowThreadProcessId(System.IntPtr h, out uint pid);
    [System.Runtime.InteropServices.DllImport("user32.dll")]
    public static extern bool PostMessage(System.IntPtr h, uint msg, System.IntPtr w, System.IntPtr l);
"@ -ErrorAction SilentlyContinue

function Find-WindowByTitle {
    param([int]$TargetPid, [string]$Title)
    $script:__match = [IntPtr]::Zero
    $cb = [HR.Win32+EnumProc]{
        param($h, $lp)
        $opid = 0
        [HR.Win32]::GetWindowThreadProcessId($h, [ref]$opid) | Out-Null
        if ($opid -ne $script:__targetPid) { return $true }
        $sb = New-Object System.Text.StringBuilder 256
        [HR.Win32]::GetWindowText($h, $sb, 256) | Out-Null
        if ($sb.ToString() -eq $script:__title) {
            $script:__match = $h
            return $false
        }
        return $true
    }
    $script:__targetPid = $TargetPid
    $script:__title = $Title
    [HR.Win32]::EnumWindows($cb, [IntPtr]::Zero) | Out-Null
    return $script:__match
}

function Find-OKButton {
    param([IntPtr]$Dialog)
    $script:__ok = [IntPtr]::Zero
    $cb = [HR.Win32+EnumProc]{
        param($h, $lp)
        $sb = New-Object System.Text.StringBuilder 64
        [HR.Win32]::GetClassName($h, $sb, 64) | Out-Null
        $cls = $sb.ToString()
        $sb2 = New-Object System.Text.StringBuilder 64
        [HR.Win32]::GetWindowText($h, $sb2, 64) | Out-Null
        $txt = $sb2.ToString()
        if ($cls -match "[Bb]utton" -and ($txt -eq "OK" -or $txt -eq "&OK")) {
            $script:__ok = $h
            return $false
        }
        return $true
    }
    [HR.Win32]::EnumChildWindows($Dialog, $cb, [IntPtr]::Zero) | Out-Null
    return $script:__ok
}

$dismissed = $false
for ($i = 0; $i -lt 120 -and -not $proc.HasExited -and -not $dismissed; $i++) {
    Start-Sleep -Seconds 1
    $hwnd = Find-WindowByTitle -TargetPid $proc.Id -Title "Module Selection"
    if ($hwnd -ne [IntPtr]::Zero) {
        Write-Host "[run] Module Selection hwnd=$hwnd"
        Start-Sleep -Milliseconds 500
        $ok = Find-OKButton -Dialog $hwnd
        if ($ok -ne [IntPtr]::Zero) {
            # BM_CLICK = 0x00F5
            [HR.Win32]::PostMessage($ok, 0x00F5, [IntPtr]::Zero, [IntPtr]::Zero) | Out-Null
            Write-Host "[run] posted BM_CLICK to OK button (hwnd=$ok)"
        } else {
            # Fallback: WM_COMMAND with IDOK=1
            [HR.Win32]::PostMessage($hwnd, 0x0111, [IntPtr]::new(1), [IntPtr]::Zero) | Out-Null
            Write-Host "[run] posted WM_COMMAND IDOK to dialog"
        }
        $dismissed = $true
    }
}
if (-not $dismissed -and -not $proc.HasExited) {
    Write-Warning "Module Selection dialog never appeared within 120s"
}

Write-Host "[run] waiting for script completion (up to ${TimeoutSec}s)..."
# -autoexit only auto-closes FO4Edit for -quickautoclean/-quickclean flows, NOT for
# -script: flows. So we detect completion via a sentinel file that our Pascal
# script writes last (HR_LastScriptDone.txt), then PostMessage WM_CLOSE.
$scriptLogSentinel = Join-Path $FO4EditDir "HR_LastScriptDone.txt"
Remove-Item $scriptLogSentinel -ErrorAction SilentlyContinue

$deadline = (Get-Date).AddSeconds($TimeoutSec)
$done = $false
while ((Get-Date) -lt $deadline -and -not $proc.HasExited) {
    Start-Sleep -Seconds 2
    if (Test-Path $scriptLogSentinel) { $done = $true; break }
}

if ($done -and -not $proc.HasExited) {
    Write-Host "[run] sentinel detected — script completed. Terminating FO4Edit."
    # FO4Edit shows a blocking "You can close this application now" dialog after
    # -script: runs, which blocks WM_CLOSE. All script work (including SaveFile)
    # has already been committed by the time the sentinel appears, so a clean
    # Kill() is safe. Exit code -1 from the kill is expected; we derive success
    # from the sentinel file existing, not from the process exit code.
    Start-Sleep -Milliseconds 500
    $proc.Kill()
}

$exited = $proc.WaitForExit(15000)
if (-not $exited -and -not $proc.HasExited) { $proc.Kill() }
Write-Host "[run] exit code: $($proc.ExitCode) (sentinel=$done)"
if (-not $done) { throw "Script did not complete within ${TimeoutSec}s (sentinel not written)" }

if (Test-Path $logFile) {
    Write-Host "--- FO4Edit script log ---"
    Get-Content $logFile
}
# Note: FO4Edit's -log: only writes on normal exit paths. Since we kill after the
# sentinel fires, -log: often doesn't flush. Rely on the sentinel + your script's
# own output for success signaling.
