param(
    [string]$DeviceName
)

Add-Type -TypeDefinition @"
using System;
using System.Runtime.InteropServices;

public class AudioDevice {
    [DllImport("winmm.dll", CharSet = CharSet.Auto)]
    public static extern int waveOutSetVolume(IntPtr hwo, uint dwVolume);
}
"@

#Change the default playback device
Get-WmiObject -Class Win32_SoundDevice | Where-Object { $_.Name -eq $DeviceName } | ForEach-Object { $_.SetDefault() }