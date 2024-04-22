#include <iostream>
#include <xaudio2.h>
#include <Windows.h>

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

	std::cout << "XAudio2 successfully initialized\n";

    // Cleanup
    master_voice->DestroyVoice();
    xaudio2->Release();
    CoUninitialize();

	return 0;
}