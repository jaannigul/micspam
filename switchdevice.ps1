param(
    [string]$targetDeviceID
)
Import-Module AudioDeviceCmdlets
Write-Output "Setting the default audio device to $targetDeviceID"
Set-AudioDevice -id $targetDeviceID

