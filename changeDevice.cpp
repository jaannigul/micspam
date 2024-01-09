#include <iostream>
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <functiondiscoverykeys_devpkey.h>
#include <propvarutil.h>
#include <propidl.h>
#include <Objbase.h>
#include <Functiondiscoverykeys.h>
#include "audioDevice.h"

#pragma comment(lib, "Ole32.lib")

// GUID for the PolicyConfigClient interface (Windows 7 and later)
static const GUID CLSID_PolicyConfigClient = { 0x870AF99C, 0x171D, 0x4F9E, {0xAF, 0x0D, 0xE6, 0x3D, 0xF4, 0x0D, 0xD3, 0x4E} };
static const GUID IID_IPolicyConfig = { 0xF8679F50, 0x850A, 0x41CF, {0x9C, 0x72, 0x43, 0x0F, 0x29, 0x3D, 0x1C, 0x35} };

interface IPolicyConfig : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetMixFormat(PCWSTR, WAVEFORMATEX**) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetDeviceFormat(PCWSTR, INT, WAVEFORMATEX**) = 0;
    virtual HRESULT STDMETHODCALLTYPE ResetDeviceFormat(PCWSTR) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetDeviceFormat(PCWSTR, WAVEFORMATEX*, WAVEFORMATEX*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetProcessingPeriod(PCWSTR, INT, PINT64, PINT64) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetProcessingPeriod(PCWSTR, PINT64) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetShareMode(PCWSTR, struct DeviceShareMode*) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetShareMode(PCWSTR, struct DeviceShareMode*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetPropertyValue(PCWSTR, const PROPERTYKEY&, PROPVARIANT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetPropertyValue(PCWSTR, const PROPERTYKEY&, PROPVARIANT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetDefaultEndpoint(__in PCWSTR wszDeviceId, __in ERole eRole) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetEndpointVisibility(PCWSTR, INT) = 0;
};

void SetDefaultAudioPlaybackDevice(LPCWSTR devID)
{
    CoInitialize(NULL);
    try{
        IPolicyConfig* pPolicyConfig;
        HRESULT hr = CoCreateInstance(CLSID_PolicyConfigClient, NULL, CLSCTX_ALL, IID_IPolicyConfig, (LPVOID*)&pPolicyConfig);
        if (SUCCEEDED(hr))
        {
            hr = pPolicyConfig->SetDefaultEndpoint(devID, eConsole);
            pPolicyConfig->Release();
        }
    }
    catch(int e){
        printf("error");
    }
  
    CoUninitialize();
   
   

}
extern "C" {
    void SetDefaultAudioPlaybackDeviceCWrapper(LPCWSTR devID) {
        // Call the original C++ function here
        SetDefaultAudioPlaybackDevice(devID);
    }
}

