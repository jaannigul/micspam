if (-not (Get-Module -ListAvailable -Name AudioDeviceCmdlets)) {
    Write-Output "AudioDeviceCmdlets module is not installed. Attempting to install..."

    try {
        Install-Module -Name AudioDeviceCmdlets -Force -Scope CurrentUser -Repository PSGallery -ErrorAction Stop
        Write-Output "AudioDeviceCmdlets module installed successfully."
    } catch {
        Write-Error "Failed to install AudioDeviceCmdlets module. Error: $_"
        exit
    }
} else {
    Write-Output "AudioDeviceCmdlets module is already installed."
}

Import-Module AudioDeviceCmdlets -ErrorAction Stop
Write-Output "AudioDeviceCmdlets module imported successfully."
