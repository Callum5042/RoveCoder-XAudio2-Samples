#include <iostream>
#include <xaudio2.h>
#include <Windows.h>

#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

void DisplayAudioDevices()
{
    IMMDeviceEnumerator* enumerator = nullptr;
    if (CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&enumerator) != S_OK)
    {
        std::cerr << "CoCreateInstance failed\n";
        return;
    }

    IMMDeviceCollection* collection = nullptr;
    if (enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &collection) != S_OK)
    {
        std::cerr << "IMMDeviceEnumerator::EnumAudioEndpoints failed\n";
        return;
    }

    UINT count = 0;
    collection->GetCount(&count);
    for (UINT i = 0; i < count; i++)
    {
        IMMDevice* device = nullptr;
        collection->Item(i, &device);

        // Get device properties store
        IPropertyStore* property_store = nullptr;
        if (device->OpenPropertyStore(STGM_READ, &property_store) != S_OK)
        {
            device->Release();
            continue;
        }

        // Get device name
        PROPVARIANT property;
        PropVariantInit(&property);
        if (property_store->GetValue(PKEY_Device_FriendlyName, &property) == S_OK)
        {
            std::wcout << L"Device: " << property.pwszVal << '\n';
            PropVariantClear(&property);
        }

        // Release resources
        device->Release();
    }

    // Release resources
    collection->Release();
    enumerator->Release();
}

int main(int args, char** argv)
{
    // Initialize COM
    if (CoInitializeEx(nullptr, COINIT_MULTITHREADED) != S_OK)
    {
        std::cerr << "CoInitializeEx failed\n";
        return -1;
    }

    // Initialize XAudio2 - XAudio2 can also be wrapped in a ComPtr
    IXAudio2* xaudio2 = nullptr;
    if (XAudio2Create(&xaudio2) != S_OK)
    {
        std::cerr << "XAudio2Create failed\n";
        CoUninitialize();
        return -1;
    }

    // Create mastering voice
    IXAudio2MasteringVoice* master_voice = nullptr;
    if (xaudio2->CreateMasteringVoice(&master_voice) != S_OK)
    {
        std::cerr << "CreateMasteringVoice failed\n";
        CoUninitialize();
        return -1;
    }

	std::cout << "XAudio2 successfully initialized\n\n";
    DisplayAudioDevices();

    // Cleanup
    master_voice->DestroyVoice();
    xaudio2->Release();
    CoUninitialize();

	return 0;
}