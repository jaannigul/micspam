Import-Module AudioDeviceCmdlets

$microphoneid = '{0.0.1.00000000}.{5e90c3c0-6636-4788-8e35-b9acf52516c9}'
$micspamid = '{0.0.1.00000000}.{7db8508f-2487-4a9e-9cb6-6cac3c08f03f}'
#Set-AudioDevice -id $microphoneid
$microphone = get-audiodevice -id $microphoneid

if ($microphone.default) {
Set-AudioDevice -id $micspamid
Write-Output 'Setting to micspam'
} else {
Set-AudioDevice -id $microphoneid
Write-Output 'Setting to microphone'
}