param(
    [string]$DeviceName
)

Import-Module AudioDeviceCmdlets

$devices = Get-AudioDevice -List
$selectedDevice = $devices | Where-Object { $_.Name -eq $DeviceName }

if ($selectedDevice) {
    Write-Output $selectedDevice.Id
} else {
    Write-Error "Device not found"
}

