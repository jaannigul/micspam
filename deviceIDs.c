#include "deviceIDs.h"

#include <stdio.h>
#include <stdlib.h>

char VIRTUAL_AUDIO_DEVICE_ID[256] = {0};
char AUDIO_DEVICE_ID[256] = { 0 };
char HEADPHONES_ID[256] = { 0 };

double realMicSampleRate = 0.0;


void queryDeviceIDbyName(const char* deviceName, char* deviceID, size_t size) {
    char command[256];
    FILE* fp;
    errno_t err;

    sprintf_s(command, sizeof(command), "powershell -ExecutionPolicy Bypass -File GetDeviceIDByName.ps1 -DeviceName \"%s\" > deviceid.txt", deviceName);
    system(command);

    err = fopen_s(&fp, "deviceid.txt", "r");
    if (err != 0 || fp == NULL) {
        perror("Error opening file");
        return;
    }

    // Read the output from the file
    if (fgets(deviceID, size, fp) == NULL) {
        printf("No output captured\n");
    }
    else {
        printf("%s\n", deviceID);
    }

    // Close the file and remove it
    fclose(fp);
    remove("deviceid.txt");
}


void switchDefaultAudioInputDevice(const char* targetDeviceID) {
    char command[1024];
    int result = sprintf_s(command,sizeof(command), "powershell -ExecutionPolicy Bypass -File \"switchdevice.ps1\" -targetDeviceID \"%s\"", targetDeviceID);
    if (result < 0)
    {
        printf("Error with the command");
    }
    system(command);
}