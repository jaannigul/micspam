#include <portaudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sndfile.h>
#include <pthread.h>
#include <rtaudio/rtaudio_c.h>
#include <Windows.h>

#include "deviceIDs.h"
#include "audioswitcher.h"
#include "consts.h"

#include "audioplayer.h"
#include "kbdcommands.h"

#include "gui/uiaccess.h"
#include "gui/gui_main.h"

rtaudio_t realDeviceAudio = 0;
rtaudio_t virtualDeviceAudio = 0;
char** fileList;
int numFiles;
int headphoneChannelCount = 1;

void listDevices() {
    int devices = rtaudio_device_count(realDeviceAudio);
    for (int i = 0; i < devices; i++) {
        rtaudio_device_info_t deviceInfo = rtaudio_get_device_info(realDeviceAudio, i);
        
        printf("[%d]: %s, input channels: %d, output channels: %d\n", i, deviceInfo.name, deviceInfo.input_channels, deviceInfo.output_channels);
    }
}

_Bool selectMicAndAudioDevices() {
    char micSelectedId[8] = { 0 }, headphonesSelectedId[8] = { 0 }, virtualMicOutputSelectedId[8] = { 0 }, virtualMicInputSelectedId[8] = { 0 };
    char *micName = NULL, *vacOutputName = NULL, *headphonesName = NULL, *vacInputName = NULL;
    listDevices();

    printf("Choose your real microphone device's number from the list: ");
    fgets(micSelectedId, sizeof(micSelectedId), stdin);
    printf("Choose your headphone device's number from the list: ");
    fgets(headphonesSelectedId, sizeof(headphonesSelectedId), stdin);
    printf("Choose your virtual microphone device's number from the list (has output channels): ");
    fgets(virtualMicOutputSelectedId, sizeof(virtualMicOutputSelectedId), stdin);
    printf("Choose your virtual microphone device's number from the list (has input channels): ");
    fgets(virtualMicInputSelectedId, sizeof(virtualMicInputSelectedId), stdin);
    
    int micId = atoi(micSelectedId);
    int virtualMicOutputId = atoi(virtualMicOutputSelectedId);
    int virtualMicInputId = atoi(virtualMicInputSelectedId);
    int headphonesId = atoi(headphonesSelectedId);
    micName = rtaudio_get_device_info(realDeviceAudio, micId).name;
    vacOutputName = rtaudio_get_device_info(realDeviceAudio, virtualMicOutputId).name;
    vacInputName = rtaudio_get_device_info(realDeviceAudio, virtualMicInputId).name;
    headphonesName = rtaudio_get_device_info(realDeviceAudio, headphonesId).name;
    //rtaudio indices are not the same as powershell's output indices
    //queryDeviceIDbyName(vacOutputName, VIRTUAL_AUDIO_DEVICE_OUTPUT_ID, sizeof(VIRTUAL_AUDIO_DEVICE_OUTPUT_ID));
    queryDeviceIDbyName(vacInputName, VIRTUAL_AUDIO_DEVICE_INPUT_ID, sizeof(VIRTUAL_AUDIO_DEVICE_INPUT_ID));
    queryDeviceIDbyName(micName, AUDIO_DEVICE_ID, sizeof(AUDIO_DEVICE_ID));
    //queryDeviceIDbyName(headphonesName, HEADPHONES_ID, sizeof(HEADPHONES_ID));
    rtaudio_device_info_t realMicInfo = rtaudio_get_device_info(realDeviceAudio, micId);
    rtaudio_stream_parameters_t realMicParams = { 0 };
    realMicParams.device_id = micId;
    realMicParams.num_channels = 1; // hardcoded to only one channel

    realMicSampleRate = realMicInfo.preferred_sample_rate;

    rtaudio_device_info_t virtualMicInfo = rtaudio_get_device_info(virtualDeviceAudio, virtualMicOutputId);
    rtaudio_stream_parameters_t virtualMicParams = { 0 };
    virtualMicParams.device_id = virtualMicOutputId;
    virtualMicParams.num_channels = 1;

    rtaudio_device_info_t headphonesInfo = rtaudio_get_device_info(realDeviceAudio, headphonesId);
    rtaudio_stream_parameters_t headphoneParams = { 0 };
    headphoneParams.device_id = headphonesId;
    headphoneParams.num_channels = headphonesInfo.output_channels;

    headphoneChannelCount = headphonesInfo.output_channels;

    int prefFrames = BUFFER_FRAMES;
    int err = rtaudio_open_stream(realDeviceAudio, &headphoneParams, &realMicParams, RTAUDIO_FORMAT_FLOAT32, realMicInfo.preferred_sample_rate, &prefFrames, &realMicAndHeadphonesCallback, &headphoneChannelCount, NULL, NULL);
    if (err > RTAUDIO_ERROR_DEBUG_WARNING) {
        printf("Failed to open microphone or headphone device due to error: %s\n", rtaudio_error(realDeviceAudio));
        return FALSE;
    }

    // use the real mic sample rate on virtual mic too
    err = rtaudio_open_stream(virtualDeviceAudio, &virtualMicParams, NULL, RTAUDIO_FORMAT_FLOAT32, realMicInfo.preferred_sample_rate, &prefFrames, &virtualMicCallback, NULL, NULL, NULL);
    if (err > RTAUDIO_ERROR_DEBUG_WARNING) {
        printf("Failed to open virtual microphone device with id %d due to error: %s\n", virtualMicOutputId, rtaudio_error(virtualDeviceAudio));
        return TRUE;
    }

    return TRUE;
}

void printFileList() {
    printf("////////////////////////////////////////////////////\n");
    for (int index = 0; index < numFiles; index++) {
        printf("%d. %s\n", index, fileList[index]);
    }
    printf("////////////////////////////////////////////////////\n");
}

int main() {

    DWORD err = PrepareForUIAccess();
    if (err != ERROR_SUCCESS) {
        printf("Error setting up UI access. This is required for overlaying things over an exclusive fullscreen game. GUI popups for the micspammer are disabled.\n");
        printf("To fix this issue, run the application in administrator mode.\n");
    }
    else
        guiTestEntryPoint();

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

    if (!setupAudioPlayer()) {
        printf("Error setting up the audio player to asynchronously play custom audio.\n");

        return 1;
    }

    if (!selectMicAndAudioDevices()) return 1;

    //main input from user
    /*
    * list - list all audio files in audiosamples
    * 
    */

    numFiles = countFilesInDirectory(USER_AUDIO_FILES_PATH_WILDCARD);
    fileList = allocateFileList(numFiles);
    numFiles = getUserAudioFiles(USER_AUDIO_FILES_PATH_WILDCARD, fileList);
    printFileList();
    startSwitchingAudio(realDeviceAudio, virtualDeviceAudio);

    pthread_t keyboardThread;
    void* threadArgs[2] = {fileList, &numFiles};
    pthread_create(&keyboardThread, NULL, keyboardCommandListener, threadArgs);
    pthread_detach(keyboardThread);

    char command[16] = { 0 };
    while (1) {
        printf("Command: ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0;

        if (isdigit(command[0])) { // choose audio file via index
            int audioFileIndex = atoi(command);
            togglePlayingAudio(fileList[audioFileIndex]);
        }
        else if (strcmp(command, "exit") == 0) { // exit
            switchDefaultAudioInputDevice(AUDIO_DEVICE_ID);
            break;
        }
        else if (strcmp(command, "list") == 0) {
            printFileList();
            continue;
        }
        else if (strstr(command, "volume") != NULL) {
            printf("Setting current volume to\n"); //todo
            continue;
        }
    }

    pthread_cancel(keyboardThread);

    return 0;
}
