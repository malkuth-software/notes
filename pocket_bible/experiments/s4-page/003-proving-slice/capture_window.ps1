# Starts the book, waits for its window, saves a picture of the window's
# client area as a PNG at device resolution, and closes it.
#
#   powershell -File capture_window.ps1 -Executable <pocket-bible.exe> -Output <file.png>
#       [-Form reading|study] [-Face garamond|portsmouth] [-Step small|medium|large] [-Page N]
#
# The picture is taken from the window itself (PrintWindow), not from the
# screen, so it holds the book and nothing else whatever is in front of it.
# Windows only; it is how the screenshots under planning/reports/ were made,
# not part of the product.
param(
    [Parameter(Mandatory = $true)][string]$Executable,
    [Parameter(Mandatory = $true)][string]$Output,
    [string]$Form = "reading",
    [string]$Face = "garamond",
    [string]$Step = "medium",
    [int]$Page = 0,
    [int]$SettleMilliseconds = 1500
)

Add-Type -AssemblyName System.Drawing
Add-Type @"
using System;
using System.Runtime.InteropServices;
public static class BookWindow {
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left, Top, Right, Bottom; }
    [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X, Y; }
    [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
    [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr window, out RECT rect);
    [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr window, ref POINT point);
    [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr window, IntPtr deviceContext, uint flags);
}
"@
[void][BookWindow]::SetProcessDPIAware()

$env:POCKET_BIBLE_FORM = $Form
$env:POCKET_BIBLE_FACE = $Face
$env:POCKET_BIBLE_STEP = $Step
$env:POCKET_BIBLE_PAGE = "$Page"
$process = Start-Process -FilePath $Executable -PassThru
try {
    $deadline = (Get-Date).AddSeconds(15)
    while ($process.MainWindowHandle -eq [IntPtr]::Zero -and (Get-Date) -lt $deadline) {
        Start-Sleep -Milliseconds 100
        $process.Refresh()
    }
    if ($process.MainWindowHandle -eq [IntPtr]::Zero) {
        throw "The book's window did not appear."
    }
    Start-Sleep -Milliseconds $SettleMilliseconds

    $client = New-Object BookWindow+RECT
    [void][BookWindow]::GetClientRect($process.MainWindowHandle, [ref]$client)
    $width = $client.Right - $client.Left
    $height = $client.Bottom - $client.Top
    $bitmap = New-Object System.Drawing.Bitmap $width, $height
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    $deviceContext = $graphics.GetHdc()
    # 1: the client area only. 2: render the full content even when covered.
    $printed = [BookWindow]::PrintWindow($process.MainWindowHandle, $deviceContext, 3)
    $graphics.ReleaseHdc($deviceContext)
    $graphics.Dispose()
    if (-not $printed) {
        throw "The window could not be captured."
    }
    $bitmap.Save($Output, [System.Drawing.Imaging.ImageFormat]::Png)
    $bitmap.Dispose()
    Write-Output "$Output $width x $height"
}
finally {
    if (-not $process.HasExited) {
        [void]$process.CloseMainWindow()
        if (-not $process.WaitForExit(5000)) {
            $process.Kill()
        }
    }
}
