#include <portaudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sndfile.h>
#include <pthread.h>
#include <rtaudio/rtaudio_c.h>
#include <Windows.h>

#define BUFFER_FRAMES 256

rtaudio_t audioInstance = 0;

int micInputCallback(void* out, void* in, unsigned int nFrames,
    double stream_time, rtaudio_stream_status_t status,
    void* userdata) {

    return 0;
}

int virtualMicCallback(void* out, void* in, unsigned int nFrames,
    double stream_time, rtaudio_stream_status_t status,
    void* userdata) {

    return 0;
}

int headphoneAudioCallback(void* out, void* in, unsigned int nFrames,
    double stream_time, rtaudio_stream_status_t status,
    void* userdata) {

}

void listDevices() {
    int devices = rtaudio_device_count(audioInstance);
    for (int i = 0; i < devices; i++) {
        rtaudio_device_info_t deviceInfo = rtaudio_get_device_info(audioInstance, i);
        
        printf("[%d]: %s, input channels: %d, output channels: %d\n", i, deviceInfo.name, deviceInfo.input_channels, deviceInfo.output_channels);
    }
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

    rtaudio_device_info_t realMicInfo = rtaudio_get_device_info(audioInstance, micId);
    rtaudio_stream_parameters_t realMicParams = { 0 };
    realMicParams.device_id = micId;
    realMicParams.num_channels = realMicInfo.input_channels;

    int prefFrames = BUFFER_FRAMES;
    int err = rtaudio_open_stream(audioInstance, NULL, &realMicParams, RTAUDIO_FORMAT_SINT16, realMicInfo.preferred_sample_rate, &prefFrames, &micInputCallback, NULL, NULL, NULL);
    if (err != RTAUDIO_NO_ERROR) {
        printf("Failed to open microphone device with id %d due to error: %s\n", micId, rtaudio_error(audioInstance));
        return TRUE;
    }

    rtaudio_device_info_t virtualMicInfo = rtaudio_get_device_info(audioInstance, virtualMicId);
    rtaudio_stream_parameters_t virtualMicParams = { 0 };
    virtualMicParams.device_id = virtualMicId;
    virtualMicParams.num_channels = virtualMicInfo.output_channels;

    err = rtaudio_open_stream(audioInstance, &virtualMicParams, NULL, RTAUDIO_FORMAT_SINT16, virtualMicInfo.preferred_sample_rate, &prefFrames, &virtualMicCallback, NULL, NULL, NULL);
    if (err != RTAUDIO_NO_ERROR) {
        printf("Failed to open virtual microphone device with id %d due to error: %s\n", virtualMicId, rtaudio_error(audioInstance));
        return TRUE;
    }

    rtaudio_device_info_t headphonesInfo = rtaudio_get_device_info(audioInstance, headphonesId);
    rtaudio_stream_parameters_t headphoneParams = { 0 };
    headphoneParams.device_id = headphonesId;
    headphoneParams.num_channels = headphonesInfo.output_channels;

    err = rtaudio_open_stream(audioInstance, &headphoneParams, NULL, RTAUDIO_FORMAT_SINT16, headphonesInfo.preferred_sample_rate, &prefFrames, &headphoneAudioCallback, NULL, NULL, NULL);
    if (err != RTAUDIO_NO_ERROR) {
        printf("Failed to open headphone device with id %d due to error: %s\n", virtualMicId, rtaudio_error(audioInstance));
        return TRUE;
    }
}

int main() {
    audioInstance = rtaudio_create(RTAUDIO_API_WINDOWS_WASAPI);
    if (!audioInstance) {
        const char* error = rtaudio_error(audioInstance);
        printf("Error creating RtAudio instance: %s\n", error);

        return 1;
    }

    if (!selectMicAndAudioDevices()) return 1;

    return 0;
}