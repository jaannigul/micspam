Import-Module AudioDeviceCmdlets
$devices = Get-AudioDevice -List

for ($i = 0; $i -lt $devices.Count; $i++) {
    $device = $devices[$i]
    Write-Output "[$($i+1)]: $($device.Name)"
}
