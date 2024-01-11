#include "deviceIDs.h"

#include <stdio.h>
#include <stdlib.h>

char* VIRTUAL_AUDIO_DEVICE_ID = "";
char* AUDIO_DEVICE_ID = "";
char* HEADPHONES_ID = "";

char* queryDeviceIDbyIndex(int deviceIndex) {
    static char deviceID[256];
    char command[256];
    FILE* fp;
    errno_t err;

    // Construct the command to call the PowerShell script with the device index
    sprintf_s(command, sizeof(command), "powershell -ExecutionPolicy Bypass -File GetDeviceIDByIndex.ps1 -DeviceIndex %d > deviceid.txt", deviceIndex);

    // Execute the command
    system(command);

    // Open the file with fopen_s
    err = fopen_s(&fp, "deviceid.txt", "r");
    if (err != 0 || fp == NULL) {
        perror("Error opening file");
        return NULL;
    }

    // Read the output from the file
    if (fgets(deviceID, sizeof(deviceID), fp) == NULL) {
        printf("No output captured\n");
        fclose(fp);
        return NULL;
    }

    // Close the file and remove it
    fclose(fp);
    remove("deviceid.txt");

    // Return the device ID
    return deviceID;
}

void switchDefaultAudioInputDevice(const char* targetDeviceID) {
    char command[1024];
    sprintf_s(command, "powershell -ExecutionPolicy Bypass -File \"switchdevice.ps1\" -targetDeviceID \"%s\"", targetDeviceID);
    system(command);
}