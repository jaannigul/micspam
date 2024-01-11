param(
    [int]$DeviceIndex
)

Import-Module AudioDeviceCmdlets

# Get the list of all audio devices
$devices = Get-AudioDevice -List

# Check if the provided index is valid
if ($DeviceIndex -lt 0 -or $DeviceIndex -ge $devices.Count) {
    Write-Error "Invalid device index. Please provide an index between 0 and $($devices.Count - 1)."
    exit
}

# Retrieve the device at the specified index
$device = $devices[$DeviceIndex]

# Output the device ID as a string
$device.Id.ToString()
