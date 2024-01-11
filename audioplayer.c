#include <portaudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sndfile.h>
#include <pthread.h>
#include <rtaudio/rtaudio_c.h>
#include <Windows.h>

#include "audioswitcher.h"
#include "consts.h"

rtaudio_t realDeviceAudio = 0;
rtaudio_t virtualDeviceAudio = 0;

void listDevices() {
    int devices = rtaudio_device_count(realDeviceAudio);
    for (int i = 0; i < devices; i++) {
        rtaudio_device_info_t deviceInfo = rtaudio_get_device_info(realDeviceAudio, i);
        
        printf("[%d]: %s, input channels: %d, output channels: %d\n", i, deviceInfo.name, deviceInfo.input_channels, deviceInfo.output_channels);
    }
}

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

_Bool selectMicAndAudioDevices() {
    char micSelectedId[8] = { 0 }, headphonesSelectedId[8] = { 0 }, virtualMicSelectedId[8] = { 0 };

    listDevices();

    printf("Choose your real microphone device's number from the list: ");
    fgets(micSelectedId, sizeof(micSelectedId), stdin);
    printf("Choose your virtual microphone device's number from the list: ");
    fgets(virtualMicSelectedId, sizeof(virtualMicSelectedId), stdin);
    printf("Choose your headphone device's number from the list: ");
    fgets(headphonesSelectedId, sizeof(headphonesSelectedId), stdin);
    int micId = atoi(micSelectedId);
    int virtualMicId = atoi(virtualMicSelectedId);
    int headphonesId = atoi(headphonesSelectedId);
    VIRTUAL_AUDIO_DEVICE_ID = queryDeviceIDbyIndex(virtualMicId);
    AUDIO_DEVICE_ID = queryDeviceIDbyIndex(micId);
    HEADPHONES_ID = queryDeviceIDbyIndex(headphonesId);
    rtaudio_device_info_t realMicInfo = rtaudio_get_device_info(realDeviceAudio, micId);
    rtaudio_stream_parameters_t realMicParams = { 0 };
    realMicParams.device_id = micId;
    realMicParams.num_channels = 1; // hardcoded to only one channel

    rtaudio_device_info_t virtualMicInfo = rtaudio_get_device_info(virtualDeviceAudio, virtualMicId);
    rtaudio_stream_parameters_t virtualMicParams = { 0 };
    virtualMicParams.device_id = virtualMicId;
    virtualMicParams.num_channels = 1;

    rtaudio_device_info_t headphonesInfo = rtaudio_get_device_info(realDeviceAudio, headphonesId);
    rtaudio_stream_parameters_t headphoneParams = { 0 };
    headphoneParams.device_id = headphonesId;
    headphoneParams.num_channels = headphonesInfo.output_channels;

    int prefFrames = BUFFER_FRAMES;
    int err = rtaudio_open_stream(realDeviceAudio, &headphoneParams, &realMicParams, RTAUDIO_FORMAT_SINT16, realMicInfo.preferred_sample_rate, &prefFrames, &realMicAndHeadphonesCallback, NULL, NULL, NULL);
    if (err > RTAUDIO_ERROR_DEBUG_WARNING) {
        printf("Failed to open microphone or headphone device due to error: %s\n", rtaudio_error(realDeviceAudio));
        return FALSE;
    }

    // use the real mic sample rate on virtual mic too
    err = rtaudio_open_stream(virtualDeviceAudio, &virtualMicParams, NULL, RTAUDIO_FORMAT_SINT16, realMicInfo.preferred_sample_rate, &prefFrames, &virtualMicCallback, NULL, NULL, NULL);
    if (err > RTAUDIO_ERROR_DEBUG_WARNING) {
        printf("Failed to open virtual microphone device with id %d due to error: %s\n", virtualMicId, rtaudio_error(virtualDeviceAudio));
        return TRUE;
    }

    return TRUE;
}

int main() {
    realDeviceAudio = rtaudio_create(RTAUDIO_API_WINDOWS_WASAPI);
    if (!realDeviceAudio) {
        printf("Error creating RtAudio instance for physical devices: %s\n", rtaudio_error(realDeviceAudio));

        return 1;
    }

    virtualDeviceAudio = rtaudio_create(RTAUDIO_API_WINDOWS_WASAPI);
    if (!virtualDeviceAudio) {
        printf("Error creating RtAudio instance for virtual devices: %s\n", rtaudio_error(virtualDeviceAudio));

        return 1;
    }

    if (!selectMicAndAudioDevices()) return 1;

    startSwitchingAudio(realDeviceAudio, virtualDeviceAudio);

    return 0;
}