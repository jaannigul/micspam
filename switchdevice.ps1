﻿Import-Module AudioDeviceCmdlets
params(
	[string]$targetDeviceID
)
Set-AudioDevice -id $targetDeviceID
Write-Output 'Setting to device id $targetDeviceID'
