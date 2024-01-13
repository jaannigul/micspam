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

rtaudio_t realDeviceAudio = 0;
rtaudio_t virtualDeviceAudio = 0;

void listDevices() {
    int devices = rtaudio_device_count(realDeviceAudio);
    for (int i = 0; i < devices; i++) {
        rtaudio_device_info_t deviceInfo = rtaudio_get_device_info(realDeviceAudio, i);
        
        printf("[%d]: %s, input channels: %d, output channels: %d\n", i, deviceInfo.name, deviceInfo.input_channels, deviceInfo.output_channels);
    }
}

_Bool selectMicAndAudioDevices() {
    char micSelectedId[8] = { 0 }, headphonesSelectedId[8] = { 0 }, virtualMicSelectedId[8] = { 0 };
    char *micName = NULL, *vacName = NULL, *headphonesName = NULL;
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
    micName = rtaudio_get_device_info(realDeviceAudio, micId).name;
    vacName = rtaudio_get_device_info(realDeviceAudio, virtualMicId).name;
    headphonesName = rtaudio_get_device_info(realDeviceAudio, headphonesId).name;
    //rtaudio indices are not the same as powershell's output indices
    queryDeviceIDbyName(vacName, VIRTUAL_AUDIO_DEVICE_ID, sizeof(VIRTUAL_AUDIO_DEVICE_ID));
    queryDeviceIDbyName(micName, AUDIO_DEVICE_ID, sizeof(AUDIO_DEVICE_ID));
    queryDeviceIDbyName(headphonesName, HEADPHONES_ID, sizeof(HEADPHONES_ID));
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
    int err = rtaudio_open_stream(realDeviceAudio, &headphoneParams, &realMicParams, RTAUDIO_FORMAT_FLOAT32, realMicInfo.preferred_sample_rate, &prefFrames, &realMicAndHeadphonesCallback, NULL, NULL, NULL);
    if (err > RTAUDIO_ERROR_DEBUG_WARNING) {
        printf("Failed to open microphone or headphone device due to error: %s\n", rtaudio_error(realDeviceAudio));
        return FALSE;
    }

    // use the real mic sample rate on virtual mic too
    err = rtaudio_open_stream(virtualDeviceAudio, &virtualMicParams, NULL, RTAUDIO_FORMAT_FLOAT32, realMicInfo.preferred_sample_rate, &prefFrames, &virtualMicCallback, NULL, NULL, NULL);
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

    if (!setupAudioPlayer()) {
        printf("Error setting up the audio player to asyncronously play custom audio.\n");

        return 1;
    }

    if (!selectMicAndAudioDevices()) return 1;

    startSwitchingAudio(realDeviceAudio, virtualDeviceAudio);

    return 0;
}

/*int main(int argc, char** argv)
{
    // read whole file
    SNDFILE* sndfile;
    SF_INFO sfinfo;
    sndfile = sf_open("./audiosamples/skrillex.wav", SFM_READ, &sfinfo);
    if (sndfile == 0)
    {
        exit(1);
    }

    float* audioIn = calloc(sfinfo.channels * sfinfo.frames, sizeof(float));
    sf_read_float(sndfile, audioIn, sfinfo.channels * sfinfo.frames);
    // mixdown
    float* audioOut = calloc(sfinfo.frames, sizeof(float));
    for (int i = 0; i < sfinfo.frames; i++)
    {
        audioOut[i] = 0;
        for (int j = 0; j < sfinfo.channels; j++)
            audioOut[i] += audioIn[i * sfinfo.channels + j];
        audioOut[i] /= sfinfo.channels;
    }
    sf_close(sndfile);
    // write output
    int frames = sfinfo.frames;
    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    sfinfo.channels = 1;
    sndfile = sf_open("./audiosamples/skrillex2.wav", SFM_WRITE, &sfinfo);
    sf_write_float(sndfile, audioOut, frames);
    sf_close(sndfile);
    // free memory
    return 0;
}*/
